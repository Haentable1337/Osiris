#include "../fnv.h"
#include "../Helpers.h"
#include "Visuals.h"

#include "../SDK/ConVar.h"
#include "../SDK/Entity.h"
#include "../SDK/FrameStage.h"
#include "../SDK/GameEvent.h"
#include "../SDK/GlobalVars.h"
#include "../SDK/Input.h"
#include "../SDK/Material.h"
#include "../SDK/MaterialSystem.h"
#include "../SDK/NetworkStringTable.h"
#include "../SDK/RenderContext.h"
#include "../SDK/Surface.h"
#include "../SDK/ModelInfo.h"

#include <array>

void Visuals::playerModel(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static int originalIdx = 0;

    if (!localPlayer) {
        originalIdx = 0;
        return;
    }

    constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
        constexpr std::array models{
        "models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
        "models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
        "models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
        "models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
        "models/player/custom_player/legacy/ctm_sas_variantf.mdl",
        "models/player/custom_player/legacy/ctm_st6_variante.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantg.mdl",
        "models/player/custom_player/legacy/ctm_st6_varianti.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantk.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantm.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantf.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantg.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianth.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianti.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantj.mdl",
        "models/player/custom_player/legacy/tm_leet_variantf.mdl",
        "models/player/custom_player/legacy/tm_leet_variantg.mdl",
        "models/player/custom_player/legacy/tm_leet_varianth.mdl",
        "models/player/custom_player/legacy/tm_leet_varianti.mdl",
        "models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
        "models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
        "models/player/custom_player/legacy/tm_phoenix_varianth.mdl",

        "models/player/custom_player/legacy/tm_pirate.mdl",
        "models/player/custom_player/legacy/tm_pirate_varianta.mdl",
        "models/player/custom_player/legacy/tm_pirate_variantb.mdl",
        "models/player/custom_player/legacy/tm_pirate_variantc.mdl",
        "models/player/custom_player/legacy/tm_pirate_variantd.mdl",
        "models/player/custom_player/legacy/tm_anarchist.mdl",
        "models/player/custom_player/legacy/tm_anarchist_varianta.mdl",
        "models/player/custom_player/legacy/tm_anarchist_variantb.mdl",
        "models/player/custom_player/legacy/tm_anarchist_variantc.mdl",
        "models/player/custom_player/legacy/tm_anarchist_variantd.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianta.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantb.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantc.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantd.mdl",
        "models/player/custom_player/legacy/tm_balkan_variante.mdl",
        "models/player/custom_player/legacy/tm_jumpsuit_varianta.mdl",
        "models/player/custom_player/legacy/tm_jumpsuit_variantb.mdl",
        "models/player/custom_player/legacy/tm_jumpsuit_variantc.mdl"
        };

        switch (team) {
        case 2: return static_cast<std::size_t>(config->visuals.playerModelT - 1) < models.size() ? models[config->visuals.playerModelT - 1] : nullptr;
        case 3: return static_cast<std::size_t>(config->visuals.playerModelCT - 1) < models.size() ? models[config->visuals.playerModelCT - 1] : nullptr;
        default: return nullptr;
        }
    };

    if (const auto model = getModel(localPlayer->team())) {
        if (stage == FrameStage::RENDER_START) {
            originalIdx = localPlayer->modelIndex();
            if (const auto modelprecache = interfaces->networkStringTableContainer->findTable("modelprecache")) {
                modelprecache->addString(false, model);
                const auto viewmodelArmConfig = memory->getPlayerViewmodelArmConfigForPlayerModel(model);
                modelprecache->addString(false, viewmodelArmConfig[2]);
                modelprecache->addString(false, viewmodelArmConfig[3]);
            }
        }

        const auto idx = stage == FrameStage::RENDER_END && originalIdx ? originalIdx : interfaces->modelInfo->getModelIndex(model);

        localPlayer->setModelIndex(idx);

        if (const auto ragdoll = interfaces->entityList->getEntityFromHandle(localPlayer->ragdoll()))
            ragdoll->setModelIndex(idx);
    }
}

void Visuals::colorWorld() noexcept
{
    if (!config->visuals.world.enabled && !config->visuals.sky.enabled)
        return;

    for (short h = interfaces->materialSystem->firstMaterial(); h != interfaces->materialSystem->invalidMaterial(); h = interfaces->materialSystem->nextMaterial(h)) {
        const auto mat = interfaces->materialSystem->getMaterial(h);

        if (!mat || !mat->isPrecached())
            continue;

        const std::string_view textureGroup = mat->getTextureGroupName();

        if (config->visuals.world.enabled && (textureGroup.starts_with("World") || textureGroup.starts_with("StaticProp"))) {
            if (config->visuals.world.rainbow)
                mat->colorModulate(rainbowColor(config->visuals.world.rainbowSpeed));
            else
                mat->colorModulate(config->visuals.world.color);
        } else if (config->visuals.sky.enabled && textureGroup.starts_with("SkyBox")) {
            if (config->visuals.sky.rainbow)
                mat->colorModulate(rainbowColor(config->visuals.sky.rainbowSpeed));
            else
                mat->colorModulate(config->visuals.sky.color);
        }
    }
}

void Visuals::modifySmoke(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr std::array smokeMaterials{
        "particle/vistasmokev1/vistasmokev1_emods",
        "particle/vistasmokev1/vistasmokev1_emods_impactdust",
        "particle/vistasmokev1/vistasmokev1_fire",
        "particle/vistasmokev1/vistasmokev1_smokegrenade"
    };

    for (const auto mat : smokeMaterials) {
        const auto material = interfaces->materialSystem->findMaterial(mat);
        material->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && config->visuals.noSmoke);
        material->setMaterialVarFlag(MaterialVarFlag::WIREFRAME, stage == FrameStage::RENDER_START && config->visuals.wireframeSmoke);
    }
}

void Visuals::thirdperson() noexcept
{
    static bool isInThirdperson{ true };
    static float lastTime{ 0.0f };

    if (GetAsyncKeyState(config->visuals.thirdpersonKey) && memory->globalVars->realtime - lastTime > 0.5f) {
        isInThirdperson = !isInThirdperson;
        lastTime = memory->globalVars->realtime;
    }

    if (config->visuals.thirdperson)
        if (memory->input->isCameraInThirdPerson = (!config->visuals.thirdpersonKey || isInThirdperson)
            && localPlayer && localPlayer->isAlive())
            memory->input->cameraOffset.z = static_cast<float>(config->visuals.thirdpersonDistance);
        else if (!localPlayer->isAlive() && config->visuals.deadThirdperson)
            localPlayer->setObserverMode() = (!config->visuals.thirdpersonKey || isInThirdperson) ? setObsMode::Chase : setObsMode::InEye;
}

void Visuals::removeVisualRecoil(FrameStage stage) noexcept
{
    if (!localPlayer || !localPlayer->isAlive())
        return;

    static Vector aimPunch;
    static Vector viewPunch;

    if (stage == FrameStage::RENDER_START) {
        aimPunch = localPlayer->aimPunchAngle();
        viewPunch = localPlayer->viewPunchAngle();

        if (config->visuals.noAimPunch)
            localPlayer->aimPunchAngle() = Vector{ };

        if (config->visuals.noViewPunch)
            localPlayer->viewPunchAngle() = Vector{ };

    } else if (stage == FrameStage::RENDER_END) {
        localPlayer->aimPunchAngle() = aimPunch;
        localPlayer->viewPunchAngle() = viewPunch;
    }
}

void Visuals::removeBlur(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    static auto blur = interfaces->materialSystem->findMaterial("dev/scope_bluroverlay");
    blur->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && config->visuals.noBlur);
}

void Visuals::updateBrightness() noexcept
{
    static auto brightness = interfaces->cvar->findVar("mat_force_tonemap_scale");
    brightness->setValue(config->visuals.brightness);
}

void Visuals::removeGrass(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    constexpr auto getGrassMaterialName = []() noexcept -> const char* {
        switch (fnv::hashRuntime(interfaces->engine->getLevelName())) {
        case fnv::hash("dz_blacksite"): return "detail/detailsprites_survival";
        case fnv::hash("dz_sirocco"): return "detail/dust_massive_detail_sprites";
        case fnv::hash("dz_junglety"): return "detail/tropical_grass";
        default: return nullptr;
        }
    };

    if (const auto grassMaterialName = getGrassMaterialName())
        interfaces->materialSystem->findMaterial(grassMaterialName)->setMaterialVarFlag(MaterialVarFlag::NO_DRAW, stage == FrameStage::RENDER_START && config->visuals.noGrass);
}

void Visuals::remove3dSky() noexcept
{
    static auto sky = interfaces->cvar->findVar("r_3dsky");
    sky->setValue(!config->visuals.no3dSky);
}

void Visuals::removeShadows() noexcept
{
    static auto shadows = interfaces->cvar->findVar("cl_csm_enabled");
    shadows->setValue(!config->visuals.noShadows);
}

void Visuals::applyZoom(FrameStage stage) noexcept
{
    if (config->visuals.zoom && localPlayer) {
        if (stage == FrameStage::RENDER_START && (localPlayer->fov() == 90 || localPlayer->fovStart() == 90)) {
            static bool scoped{ false };

            if (GetAsyncKeyState(config->visuals.zoomKey) & 1)
                scoped = !scoped;

            if (scoped) {
                localPlayer->fov() = 40;
                localPlayer->fovStart() = 40;
            }
        }
    }
}

#define DRAW_SCREEN_EFFECT(material) \
{ \
    const auto drawFunction = memory->drawScreenEffectMaterial; \
    int w, h; \
    interfaces->surface->getScreenSize(w, h); \
    __asm { \
        __asm push h \
        __asm push w \
        __asm push 0 \
        __asm xor edx, edx \
        __asm mov ecx, material \
        __asm call drawFunction \
        __asm add esp, 12 \
    } \
}

void Visuals::applyScreenEffects() noexcept
{
    if (!config->visuals.screenEffect)
        return;

    const auto material = interfaces->materialSystem->findMaterial([] {
        constexpr std::array effects{
            "effects/dronecam",
            "effects/underwater_overlay",
            "effects/healthboost",
            "effects/dangerzone_screen"
        };

        if (config->visuals.screenEffect <= 2 || static_cast<std::size_t>(config->visuals.screenEffect - 2) >= effects.size())
            return effects[0];
        return effects[config->visuals.screenEffect - 2];
    }());

    if (config->visuals.screenEffect == 1)
        material->findVar("$c0_x")->setValue(0.0f);
    else if (config->visuals.screenEffect == 2)
        material->findVar("$c0_x")->setValue(0.1f);
    else if (config->visuals.screenEffect >= 4)
        material->findVar("$c0_x")->setValue(1.0f);

    DRAW_SCREEN_EFFECT(material)
}

void Visuals::hitEffect(GameEvent* event) noexcept
{
    if (config->visuals.hitEffect && localPlayer) {
        static float lastHitTime = 0.0f;

        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
            lastHitTime = memory->globalVars->realtime;
            return;
        }

        if (lastHitTime + config->visuals.hitEffectTime >= memory->globalVars->realtime) {
            constexpr auto getEffectMaterial = [] {
                static constexpr const char* effects[]{
                "effects/dronecam",
                "effects/underwater_overlay",
                "effects/healthboost",
                "effects/dangerzone_screen"
                };

                if (config->visuals.hitEffect <= 2)
                    return effects[0];
                return effects[config->visuals.hitEffect - 2];
            };


            auto material = interfaces->materialSystem->findMaterial(getEffectMaterial());
            if (config->visuals.hitEffect == 1)
                material->findVar("$c0_x")->setValue(0.0f);
            else if (config->visuals.hitEffect == 2)
                material->findVar("$c0_x")->setValue(0.1f);
            else if (config->visuals.hitEffect >= 4)
                material->findVar("$c0_x")->setValue(1.0f);

            DRAW_SCREEN_EFFECT(material)
        }
    }
}

void Visuals::hitMarker(GameEvent* event) noexcept
{
    if (config->visuals.hitMarker == 0 || !localPlayer)
        return;

    static float lastHitTime = 0.0f;

    if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index()) {
        lastHitTime = memory->globalVars->realtime;
        return;
    }

    if (lastHitTime + config->visuals.hitMarkerTime < memory->globalVars->realtime)
        return;

    switch (config->visuals.hitMarker) {
    case 1:
        const auto [width, height] = interfaces->surface->getScreenSize();

        const auto width_mid = width / 2;
        const auto height_mid = height / 2;

        interfaces->surface->setDrawColor(255, 255, 255, 255);
        interfaces->surface->drawLine(width_mid + 10, height_mid + 10, width_mid + 4, height_mid + 4);
        interfaces->surface->drawLine(width_mid - 10, height_mid + 10, width_mid - 4, height_mid + 4);
        interfaces->surface->drawLine(width_mid + 10, height_mid - 10, width_mid + 4, height_mid - 4);
        interfaces->surface->drawLine(width_mid - 10, height_mid - 10, width_mid - 4, height_mid - 4);
        break;
    }
}

void Visuals::disablePostProcessing(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    *memory->disablePostProcessing = stage == FrameStage::RENDER_START && config->visuals.disablePostProcessing;
}

void Visuals::reduceFlashEffect() noexcept
{
    if (localPlayer)
        localPlayer->flashMaxAlpha() = 255.0f - config->visuals.flashReduction * 2.55f;
}

bool Visuals::removeHands(const char* modelName) noexcept
{
    return config->visuals.noHands && std::strstr(modelName, "arms") && !std::strstr(modelName, "sleeve");
}

bool Visuals::removeSleeves(const char* modelName) noexcept
{
    return config->visuals.noSleeves && std::strstr(modelName, "sleeve");
}

bool Visuals::removeWeapons(const char* modelName) noexcept
{
    return config->visuals.noWeapons && std::strstr(modelName, "models/weapons/v_")
        && !std::strstr(modelName, "arms") && !std::strstr(modelName, "tablet")
        && !std::strstr(modelName, "parachute") && !std::strstr(modelName, "fists");
}

void Visuals::skybox(FrameStage stage) noexcept
{
    if (stage != FrameStage::RENDER_START && stage != FrameStage::RENDER_END)
        return;

    if (const auto& skyboxes = Helpers::getSkyboxes(); stage == FrameStage::RENDER_START && config->visuals.skybox > 0 && static_cast<std::size_t>(config->visuals.skybox) < skyboxes.size() && config->visuals.skybox != 1) {
        memory->loadSky(skyboxes[config->visuals.skybox]);
    }
    else if (config->visuals.skybox == 1)
        memory->loadSky(config->visuals.customSkybox.c_str());
    else {
        static const auto sv_skyname = interfaces->cvar->findVar("sv_skyname");
        memory->loadSky(sv_skyname->string);
    }
}


void Visuals::physicsTimescale() noexcept {

    static ConVar* cl_phys_timescale = interfaces->cvar->findVar("cl_phys_timescale");

    cl_phys_timescale->setValue(config->visuals.ragdollTimescale ? config->visuals.ragdollTimescaleValue : 1);
}

void Visuals::fullBright() noexcept {

    static ConVar* full_bright = interfaces->cvar->findVar("mat_fullbright");

    full_bright->setValue(config->visuals.fullBright ? 1 : 0);
}

struct HitMarkerInfo {
    float hitMarkerExpTime;
    int hitMarkerDmg;
};

std::vector<HitMarkerInfo> hitMarkerInfo;

void Visuals::hitMarkerSetDamageIndicator(GameEvent* event) noexcept {
    if (!localPlayer)
        return;

    if (config->visuals.hitMarkerDamageIndicator.enabled)
        if (event && interfaces->engine->getPlayerForUserID(event->getInt("attacker")) == localPlayer->index())
            hitMarkerInfo.push_back({ memory->globalVars->realtime + config->visuals.hitMarkerTime, event->getInt("dmg_health") });
}

void Visuals::hitMarkerDamageIndicator() noexcept
{
    if (config->visuals.hitMarkerDamageIndicator.enabled) {
        if (hitMarkerInfo.empty()) return;

        const auto [width, height] = interfaces->surface->getScreenSize();

        for (size_t i = 0; i < hitMarkerInfo.size(); i++) {
            const auto diff = hitMarkerInfo.at(i).hitMarkerExpTime - memory->globalVars->realtime;

            if (diff < 0.f) {
                hitMarkerInfo.erase(hitMarkerInfo.begin() + i);
                continue;
            }

            const auto dist = config->visuals.hitMarkerDamageIndicatorCustomize ? config->visuals.hitMarkerDamageIndicatorDist : 150;
            const auto ratio = (config->visuals.hitMarkerDamageIndicatorCustomize ? config->visuals.hitMarkerDamageIndicatorRatio : 0.0f) - diff;
            const auto alpha = diff * (config->visuals.hitMarkerDamageIndicatorCustomize ? config->visuals.hitMarkerDamageIndicatorAlpha : 800);
            const auto font_id = config->visuals.hitMarkerDamageIndicatorCustomize ? config->visuals.hitMarkerDamageIndicatorFont : 31;

            interfaces->surface->setTextFont(font_id);
            interfaces->surface->setTextPosition(width / 2 + config->visuals.hitMarkerDamageIndicatorTextX + ratio * dist / 2, height / 2 + config->visuals.hitMarkerDamageIndicatorTextY + ratio * dist);
            if (config->visuals.hitMarkerDamageIndicator.rainbow)
                interfaces->surface->setTextColor(rainbowColor(config->visuals.hitMarkerDamageIndicator.rainbowSpeed));
            else
                interfaces->surface->setTextColor(config->visuals.hitMarkerDamageIndicator.color);
            interfaces->surface->printText(std::to_wstring(hitMarkerInfo.at(i).hitMarkerDmg));
        }
    }
}

void Visuals::noZoom() noexcept
{
    if (config->visuals.noZoom) {
        if (localPlayer && localPlayer->isScoped()) {
            localPlayer->fov() = 90 + config->visuals.fov;
            localPlayer->fovStart() = 90 + config->visuals.fov;
        }
    }
}

void Visuals::noBloom() noexcept
{
    static ConVar* bloomCvar = interfaces->cvar->findVar("mat_disable_bloom");
    bloomCvar->setValue(config->visuals.noBloom ? 1 : 0);
}

void Visuals::NightMode()noexcept
{
    static std::string old_Skyname = "";
    static bool OldNightmode;
    static int OldSky;
    if (!interfaces->engine->isInGame() || !interfaces->engine->isConnected() || !localPlayer || !localPlayer->isAlive()) {
        old_Skyname = "";
        OldNightmode = false;
        OldSky = 0;
        return;
    }

    static ConVar* r_DrawSpecificStaticProp;

    if (OldNightmode != config->visuals.nightMode)
    {
        r_DrawSpecificStaticProp = interfaces->cvar->findVar("r_DrawSpecificStaticProp");
        r_DrawSpecificStaticProp->setValue(0);

        for (auto i = interfaces->materialSystem->firstMaterial(); i != interfaces->materialSystem->invalidMaterial(); i = interfaces->materialSystem->nextMaterial(i))
        {
            Material* pMaterial = interfaces->materialSystem->getMaterial(i);
            if (!pMaterial)
                continue;
            if (strstr(pMaterial->getTextureGroupName(), "World") || strstr(pMaterial->getTextureGroupName(), "StaticProp")) {
                if (config->visuals.nightMode) {
                    memory->loadSky("sky_csgo_night02");

                    if (strstr(pMaterial->getTextureGroupName(), "StaticProp"))
                        pMaterial->colorModulate(0.11f, 0.11f, 0.11f);
                    else
                        pMaterial->colorModulate(0.05f, 0.05f, 0.05f);
                }
                else {
                    memory->loadSky("sky_cs15_daylight04_hdr");
                    pMaterial->colorModulate(1.0f, 1.0f, 1.0f);
                }
            }
        }
        OldNightmode = config->visuals.nightMode;
    }

}

#include "../GUI.h"

auto ConvertRGB(float mult, float R, float G, float B, float A, float scale)
{
    float H, S, V;
    ImGui::ColorConvertRGBtoHSV(R, G, B, H, S, V);
    if ((H + (mult * scale)) > 1.0f)
        H = (mult * scale) - (1.0f - H);
    else
        H += mult * scale;
    ImGui::ColorConvertHSVtoRGB(H, S, V, R, G, B);
    return ImGui::ColorConvertFloat4ToU32({ R, G, B, A });
}

void Visuals::rainbowBar(ImDrawList* drawList)noexcept
{
    if (!config->visuals.rainbowBar.enabled)
        return;

    float colorR = 0;
    float colorG = 0;
    float colorB = 0;
    if (config->visuals.rainbowBar.rainbow) {
        auto rainbow = rainbowColor(config->visuals.rainbowBar.rainbowSpeed);
        colorR = std::get<0>(rainbow);
        colorG = std::get<1>(rainbow);
        colorB = std::get<2>(rainbow);
    }
    else {
        colorR = config->visuals.rainbowBar.color[0];
        colorG = config->visuals.rainbowBar.color[1];
        colorB = config->visuals.rainbowBar.color[2];
    }
    float colorA = config->visuals.rainbowBar.color[3];
    float tickness = config->visuals.rainbowBar.thickness;
    float scale = config->visuals.rainbowScale;
    float pulse, pulseAlpha;
    if (config->visuals.rainbowPulse) {
        pulse = std::sin(config->visuals.rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f;
        pulseAlpha = (std::sin(config->visuals.rainbowPulseSpeed * memory->globalVars->realtime) * 0.5f + 0.5f) * colorA;
    }
    else {
        pulse = 1.0f;
        pulseAlpha = colorA;
    }

    ImVec2 zero = { 0,0 };
    ImVec2 ds = ImGui::GetIO().DisplaySize;

    ImU32 red = ConvertRGB(0, colorR, colorG, colorB, pulse, scale);
    ImU32 amber = ConvertRGB(1, colorR, colorG, colorB, pulse, scale);
    ImU32 chartreuse = ConvertRGB(2, colorR, colorG, colorB, pulse, scale);
    ImU32 malachite = ConvertRGB(3, colorR, colorG, colorB, pulse, scale);
    ImU32 cyan = ConvertRGB(4, colorR, colorG, colorB, pulse, scale);
    ImU32 blue = ConvertRGB(5, colorR, colorG, colorB, pulse, scale);
    ImU32 indigo = ConvertRGB(6, colorR, colorG, colorB, pulse, scale);
    ImU32 magenta = ConvertRGB(7, colorR, colorG, colorB, pulse, scale);
    ImU32 red0 = ConvertRGB(0, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 amber0 = ConvertRGB(1, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 chartreuse0 = ConvertRGB(2, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 malachite0 = ConvertRGB(3, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 cyan0 = ConvertRGB(4, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 blue0 = ConvertRGB(5, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 indigo0 = ConvertRGB(6, colorR, colorG, colorB, pulseAlpha, scale);
    ImU32 magenta0 = ConvertRGB(7, colorR, colorG, colorB, pulseAlpha, scale);

    bool guiOpened = gui->open && config->style.menuStyle == 0;

    if (tickness > ds.y) {
        config->visuals.rainbowBar.thickness = ds.y;
        tickness = ds.y - guiOpened ? 21.0f : 0.0f;
    }

    //drawList->AddRectFilledMultiColor(upper - left, lower - right, Color Upper Left, Color Upper Right, Color Bottom Right, Color Bottom Left);

    if (config->visuals.rainbowBottom) {
        // Bottom
        drawList->AddRectFilledMultiColor({ zero.x, ds.y - tickness }, { ds.x / 2, ds.y }, indigo0, blue0, blue, indigo);
        drawList->AddRectFilledMultiColor({ ds.x / 2, ds.y - tickness }, { ds.x, ds.y }, blue0, cyan0, cyan, blue);
    }
    if (guiOpened) {
        zero.y += 21;
        ds.y += 21;
    }
    if (config->visuals.rainbowLeft) {
        // Left
        drawList->AddRectFilledMultiColor(zero, { tickness, ds.y / 2 }, red, red0, magenta0, magenta);
        drawList->AddRectFilledMultiColor({ zero.x, ds.y / 2 }, { tickness, ds.y }, magenta, magenta0, indigo0, indigo);
    }
    if (config->visuals.rainbowRight) {
        // Right
        drawList->AddRectFilledMultiColor({ ds.x - tickness, zero.y }, { ds.x, ds.y / 2 }, chartreuse0, chartreuse, malachite, malachite0);
        drawList->AddRectFilledMultiColor({ ds.x - tickness, ds.y / 2 }, ds, malachite0, malachite, cyan, cyan0);
    }
    if (config->visuals.rainbowUp) {
        // Upper
        drawList->AddRectFilledMultiColor(zero, { ds.x / 2, tickness + (guiOpened ? 21.0f : 0.0f) }, red, amber, amber0, red0);
        drawList->AddRectFilledMultiColor({ ds.x / 2, zero.y }, { ds.x, tickness + (guiOpened ? 21.0f : 0.0f) }, amber, chartreuse, chartreuse0, amber0);
    }
}

#include "../SDK/Beams.h"

void Visuals::bulletBeams(GameEvent* event) noexcept
{
    if (!config->visuals.bulletTracers.enabled || !interfaces->engine->isInGame() || !interfaces->engine->isConnected() || memory->renderBeams == nullptr)
        return;

    const auto player = interfaces->entityList->getEntity(interfaces->engine->getPlayerForUserID(event->getInt("userid")));

    if (!player || !localPlayer)
        return;

    Vector position;
    position.x = event->getFloat("x");
    position.y = event->getFloat("y");
    position.z = event->getFloat("z");

    BeamInfo_t beam_info;
    beam_info.m_nType = TE_BEAMPOINTS;
    beam_info.m_pszModelName = "sprites/physbeam.vmt";
    beam_info.m_nModelIndex = -1;
    beam_info.m_flHaloScale = 0.f;
    beam_info.m_flLife = 4.f;
    beam_info.m_flWidth = 1.f;
    beam_info.m_flEndWidth = 1.f;
    beam_info.m_flFadeLength = 0.1f;
    beam_info.m_flAmplitude = 2.f;
    beam_info.m_flBrightness = 255.f;
    beam_info.m_flSpeed = 0.2f;
    beam_info.m_nStartFrame = 0;
    beam_info.m_flFrameRate = 0.f;
    if (config->visuals.bulletTracers.rainbow) {
        auto rainbow = rainbowColor(config->visuals.bulletTracers.rainbowSpeed);
        beam_info.m_flRed = std::get<0>(rainbow) * 255;
        beam_info.m_flGreen = std::get<1>(rainbow) * 255;
        beam_info.m_flBlue = std::get<2>(rainbow) * 255;
    }
    else {
        beam_info.m_flRed = config->visuals.bulletTracers.color[0] * 255;
        beam_info.m_flGreen = config->visuals.bulletTracers.color[1] * 255;
        beam_info.m_flBlue = config->visuals.bulletTracers.color[2] * 255;
    }
    beam_info.m_nSegments = 2;
    beam_info.m_bRenderable = true;
    beam_info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

    // create beam backwards because it looks nicer.
    beam_info.m_vecStart = position;
    beam_info.m_vecEnd = player->getEyePosition();

    auto beam = memory->renderBeams->CreateBeamPoints(beam_info);
    if (beam)
        memory->renderBeams->DrawBeam(beam);
}