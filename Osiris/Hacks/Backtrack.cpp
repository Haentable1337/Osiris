#include "Backtrack.h"
#include "Aimbot.h"
#include "../Config.h"
#include "../SDK/ConVar.h"
#include "../SDK/Entity.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/LocalPlayer.h"
#include "../SDK/NetworkChannel.h"
#include "../SDK/UserCmd.h"

std::deque<Backtrack::IncomingSequence>Backtrack::sequences;
std::array<std::deque<Backtrack::Record>, 65> Backtrack::records;

struct Cvars {
    ConVar* updateRate;
    ConVar* maxUpdateRate;
    ConVar* interp;
    ConVar* interpRatio;
    ConVar* minInterpRatio;
    ConVar* maxInterpRatio;
    ConVar* maxUnlag;
};

static Cvars cvars;

void Backtrack::update(FrameStage stage) noexcept
{
    if (stage == FrameStage::RENDER_START) {
        if (!config->backtrack.enabled || !localPlayer || !localPlayer->isAlive()) {
            for (auto& record : records)
                record.clear();
            return;
        }

        for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
            auto entity = interfaces->entityList->getEntity(i);
            if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive() || !entity->isOtherEnemy(localPlayer.get())) {
                records[i].clear();
                continue;
            }

            if (!records[i].empty() && (records[i].front().simulationTime == entity->simulationTime()))
                continue;

            Record record{ };
            record.head = entity->getBonePosition(8);
            record.origin = entity->getAbsOrigin();
            record.simulationTime = entity->simulationTime();

            entity->setupBones(record.matrix, 256, 0x7FF00, memory->globalVars->currenttime);

            records[i].push_front(record);

            config->backtrack.pingBasedVal = config->backtrack.timeLimit;
            if (config->backtrack.pingBased)
                if (auto networkChannel = interfaces->engine->getNetworkChannel(); networkChannel && networkChannel->getLatency(0) > 0.0f)
                    config->backtrack.pingBasedVal = static_cast<int>(networkChannel->getLatency(0) * 1000);

            while (records[i].size() > 3 && records[i].size() > static_cast<size_t>(timeToTicks(static_cast<float>(config->backtrack.pingBasedVal) / 1000.f + getExtraTicks())))
                records[i].pop_back();

            if (auto invalid = std::find_if(std::cbegin(records[i]), std::cend(records[i]), [](const Record & rec) { return !valid(rec.simulationTime); }); invalid != std::cend(records[i]))
                records[i].erase(invalid, std::cend(records[i]));
        }
    }
}

void Backtrack::run(UserCmd* cmd) noexcept
{
    if (!config->backtrack.enabled)
        return;

    if (!(cmd->buttons & UserCmd::IN_ATTACK))
        return;

    if (!localPlayer)
        return;

    auto localPlayerEyePosition = localPlayer->getEyePosition();

    auto bestFov{ 255.f };
    Entity * bestTarget{ };
    int bestTargetIndex{ };
    Vector bestTargetHead{ };
    int bestRecord{ };

    const auto aimPunch = localPlayer->getAimPunch();

    for (int i = 1; i <= interfaces->engine->getMaxClients(); i++) {
        auto entity = interfaces->entityList->getEntity(i);
        if (!entity || entity == localPlayer.get() || entity->isDormant() || !entity->isAlive()
            || !entity->isOtherEnemy(localPlayer.get()))
            continue;

        auto head = entity->getBonePosition(8);
        auto origin = entity->getAbsOrigin();

        auto angle = Aimbot::calculateRelativeAngle(localPlayerEyePosition, head, cmd->viewangles + (config->backtrack.recoilBasedFov ? aimPunch : Vector{ }));
        auto fov = std::hypotf(angle.x, angle.y);
        if (fov < bestFov) {
            bestFov = fov;
            bestTarget = entity;
            bestTargetIndex = i;
            bestTargetHead = head;
        }
    }

    if (bestTarget) {
        if (records[bestTargetIndex].size() <= 3 || (!config->backtrack.ignoreSmoke && memory->lineGoesThroughSmoke(localPlayer->getEyePosition(), bestTargetHead, 1)))
            return;

        bestFov = 255.f;

        for (size_t i = 0; i < records[bestTargetIndex].size(); i++) {
            const auto& record = records[bestTargetIndex][i];
            if (!valid(record.simulationTime))
                continue;

            auto angle = Aimbot::calculateRelativeAngle(localPlayerEyePosition, record.head, cmd->viewangles + (config->backtrack.recoilBasedFov ? aimPunch : Vector{ }));
            auto fov = std::hypotf(angle.x, angle.y);
            if (fov < bestFov) {
                bestFov = fov;
                bestRecord = i;
            }
        }
    }

    if (bestRecord) {
        const auto& record = records[bestTargetIndex][bestRecord];
        memory->setAbsOrigin(bestTarget, record.origin);
        cmd->tickCount = timeToTicks(record.simulationTime + getLerp());
    }
}

void Backtrack::AddLatencyToNetwork(NetworkChannel* network, float latency) noexcept
{
    for (auto& sequence : sequences)
        if (memory->globalVars->serverTime() - sequence.servertime >= latency)
        {
            network->InReliableState = sequence.inreliablestate;
            network->InSequenceNr = sequence.sequencenr;
            break;
        }
}

void Backtrack::UpdateIncomingSequences(bool reset) noexcept
{
    static int lastIncomingSequenceNumber = 0;

    if (!config->backtrack.fakeLatency)
        return;

    if (!localPlayer)
        return;

    auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return;

    if (network->InSequenceNr != lastIncomingSequenceNumber)
    {
        lastIncomingSequenceNumber = network->InSequenceNr;

        IncomingSequence sequence{ };
        sequence.inreliablestate = network->InReliableState;
        sequence.sequencenr = network->InSequenceNr;
        sequence.servertime = memory->globalVars->serverTime();
        sequences.push_front(sequence);
    }

    while (sequences.size() > 2048)
        sequences.pop_back();
}

float Backtrack::getLerp() noexcept
{
    auto ratio = std::clamp(cvars.interpRatio->getFloat(), cvars.minInterpRatio->getFloat(), cvars.maxInterpRatio->getFloat());
    return max(cvars.interp->getFloat(), (ratio / ((cvars.maxUpdateRate) ? cvars.maxUpdateRate->getFloat() : cvars.updateRate->getFloat())));
}

float getExtraTicks() noexcept
{
    auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return 0.f;

    return std::clamp(network->getLatency(1) - network->getLatency(0), 0.f, cvars.maxUnlag->getFloat());
}

bool Backtrack::valid(float simtime) noexcept
{
    const auto network = interfaces->engine->getNetworkChannel();
    if (!network)
        return false;

    auto delta = std::clamp(network->getLatency(0) + network->getLatency(1) + getLerp(), 0.f, cvars.maxUnlag->getFloat()) - (memory->globalVars->serverTime() - simtime);
    return std::abs(delta) <= 0.2f;
}

int Backtrack::timeToTicks(float time) noexcept
{
    return static_cast<int>(0.5f + time / memory->globalVars->intervalPerTick);
}

void Backtrack::init() noexcept
{
    cvars.updateRate = interfaces->cvar->findVar("cl_updaterate");
    cvars.maxUpdateRate = interfaces->cvar->findVar("sv_maxupdaterate");
    cvars.interp = interfaces->cvar->findVar("cl_interp");
    cvars.interpRatio = interfaces->cvar->findVar("cl_interp_ratio");
    cvars.minInterpRatio = interfaces->cvar->findVar("sv_client_min_interp_ratio");
    cvars.maxInterpRatio = interfaces->cvar->findVar("sv_client_max_interp_ratio");
    cvars.maxUnlag = interfaces->cvar->findVar("sv_maxunlag");
}
