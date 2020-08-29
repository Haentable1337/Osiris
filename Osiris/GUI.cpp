#include <fstream>
#include <functional>
#include <string>
#include <ShlObj.h>
#include <Windows.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_stdlib.h"

#include "imguiCustom.h"

#include "GUI.h"
#include "Config.h"
#include "Hacks/Misc.h"
#include "Hacks/SkinChanger.h"
#include "Helpers.h"
#include "Hooks.h"
#include "Interfaces.h"
#include "SDK/InputSystem.h"
#include "imgui/imgui_internal.h"

constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

GUI::GUI() noexcept
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 9.0f;
    style.GrabRounding = 12.0f;
    style.GrabMinSize = 17.0f;
    style.FrameRounding = 12.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);

        ImFontConfig cfg;
        cfg.OversampleV = 3;

        fonts.tahoma = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        fonts.segoeui = io.Fonts->AddFontFromFileTTF((path / "segoeui.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
    }
}

void GUI::render() noexcept
{
    if (!config->style.menuStyle) {
        renderMenuBar();
        renderAimbotWindow();
        renderAntiAimWindow();
        renderTriggerbotWindow();
        renderBacktrackWindow();
        renderGlowWindow();
        renderChamsWindow();
        renderStreamProofESPWindow();
        renderVisualsWindow();
        renderSkinChangerWindow();
        renderSoundWindow();
        renderStyleWindow();
        renderMiscWindow();
        renderConfigWindow();
        renderBETAWindow();
    } else {
        renderGuiStyle2();
    }
}

void GUI::updateColors() const noexcept
{
    switch (config->style.menuColors) {
    case 0: ImGui::StyleColorsDark(); break;
    case 1: ImGui::StyleColorsLight(); break;
    case 2: ImGui::StyleColorsClassic(); break;
    }
}

void GUI::hotkey(int& key) noexcept
{
    key ? ImGui::Text("[ %s ]", interfaces->inputSystem->virtualKeyToString(key)) : ImGui::TextUnformatted("[ key ]");

    if (!ImGui::IsItemHovered())
        return;

    ImGui::SetTooltip("Press any key to change keybind");
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
        if (ImGui::IsKeyPressed(i) && i != config->misc.menuKey)
            key = i != VK_ESCAPE ? i : 0;

    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
        if (ImGui::IsMouseDown(i) && i + (i > 1 ? 2 : 1) != config->misc.menuKey)
            key = i + (i > 1 ? 2 : 1);
}

static void menuBarItem(const char* name, bool& enabled) noexcept
{
    if (ImGui::MenuItem(name)) {
        enabled = true;
        ImGui::SetWindowFocus(name);
        ImGui::SetWindowPos(name, { 100.0f, 100.0f });
    }
}

void GUI::renderMenuBar() noexcept
{
    if (ImGui::BeginMainMenuBar()) {
        menuBarItem("Aimbot", window.aimbot);
        menuBarItem("Anti aim", window.antiAim);
        menuBarItem("Triggerbot", window.triggerbot);
        menuBarItem("Backtrack", window.backtrack);
        menuBarItem("Glow", window.glow);
        menuBarItem("Chams", window.chams);
        menuBarItem("ESP", window.streamProofESP);
        menuBarItem("Visuals", window.visuals);
        menuBarItem("Skin changer", window.skinChanger);
        menuBarItem("Sound", window.sound);
        menuBarItem("Style", window.style);
        menuBarItem("Misc", window.misc);
        menuBarItem("Config", window.config);
        menuBarItem("OsirisBETA ", window.BETA);
        ImGui::EndMainMenuBar();
    }
}

void GUI::renderAimbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.aimbot)
            return;
        ImGui::SetNextWindowSize({ 600.0f, 0.0f });
        ImGui::Begin("Aimbot (OsirisBETA by PlayDay)", &window.aimbot, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);

    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void* data, int idx, const char** out_text) {
            if (config->aimbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->aimbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 220.0f);
    ImGui::Checkbox("On key", &config->aimbot[currentWeapon].onKey);
    ImGui::SameLine();
    hotkey(config->aimbot[currentWeapon].key);
    ImGui::SameLine();
    ImGui::PushID(2);
    ImGui::PushItemWidth(70.0f);
    ImGui::Combo("", &config->aimbot[currentWeapon].keyMode, "Hold\0Toggle\0");
    ImGui::PopItemWidth();
    ImGui::PopID();
    ImGui::Checkbox("Aimlock", &config->aimbot[currentWeapon].aimlock);
    if (config->aimbot[currentWeapon].standaloneRCS) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }
    ImGui::Checkbox("Silent", &config->aimbot[currentWeapon].silent);
    if (config->aimbot[currentWeapon].standaloneRCS) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    }
    ImGui::Checkbox("Friendly fire", &config->aimbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Visible only", &config->aimbot[currentWeapon].visibleOnly);
    ImGui::Checkbox("Scoped only", &config->aimbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->aimbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->aimbot[currentWeapon].ignoreSmoke);
    ImGui::Checkbox("Auto shot", &config->aimbot[currentWeapon].autoShot);
    ImGui::Checkbox("Auto scope", &config->aimbot[currentWeapon].autoScope);
    ImGui::Combo("Bone", &config->aimbot[currentWeapon].bone, "Nearest\0Best damage\0Head\0Neck\0Sternum\0Chest\0Stomach\0Pelvis\0");
    ImGui::NextColumn();
    ImGui::PushItemWidth(240.0f);
    ImGui::SliderFloat("Fov", &config->aimbot[currentWeapon].fov, 0.0f, 255.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Smooth", &config->aimbot[currentWeapon].smooth, 1.0f, 100.0f, "%.2f");
    ImGui::SliderFloat("Max aim inaccuracy", &config->aimbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max shot inaccuracy", &config->aimbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::InputInt("Min damage", &config->aimbot[currentWeapon].minDamage);
    config->aimbot[currentWeapon].minDamage = std::clamp(config->aimbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->aimbot[currentWeapon].killshot);
    ImGui::Checkbox("Jump Check", &config->aimbot[currentWeapon].jumpCheck);
    ImGui::Checkbox("Between shots", &config->aimbot[currentWeapon].betweenShots);
    ImGui::Checkbox("Standalone RCS", &config->aimbot[currentWeapon].standaloneRCS);
    if (config->aimbot[currentWeapon].standaloneRCS) {
        ImGui::SameLine();
        ImGui::Checkbox("Random RCS factor", &config->aimbot[currentWeapon].randomRCS);
        config->aimbot[currentWeapon].silent = false;
        ImGui::InputInt("Ignore Shots", &config->aimbot[currentWeapon].shotsFired);
        config->aimbot[currentWeapon].shotsFired = std::clamp(config->aimbot[currentWeapon].shotsFired, 0, 150);
        ImGui::SliderFloat(config->aimbot[currentWeapon].randomRCS ? "Recoil control X odds" : "Recoil control X", &config->aimbot[currentWeapon].recoilControlX, 0.0f, 1.0f, "%.5f");
        ImGui::SliderFloat(config->aimbot[currentWeapon].randomRCS ? "Recoil control Y odds" : "Recoil control Y", &config->aimbot[currentWeapon].recoilControlY, 0.0f, 1.0f, "%.5f");
    }
    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderAntiAimWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.antiAim)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Anti aim (OsirisBETA by PlayDay)", &window.antiAim, windowFlags);
    }
    ImGui::Checkbox("Enabled", &config->antiAim.enabled);
    ImGui::Checkbox("##pitch", &config->antiAim.pitch);
    ImGui::SameLine();
    ImGui::SliderFloat("Pitch", &config->antiAim.pitchAngle, -89.0f, 89.0f, "%.2f");
    ImGui::Checkbox("Yaw", &config->antiAim.yaw);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderTriggerbotWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.triggerbot)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Triggerbot (OsirisBETA by PlayDay)", &window.triggerbot, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "All\0Pistols\0Heavy\0SMG\0Rifles\0Zeus x27\0");
    ImGui::PopID();
    ImGui::SameLine();
    static int currentWeapon{ 0 };
    ImGui::PushID(1);
    switch (currentCategory) {
    case 0:
        currentWeapon = 0;
        ImGui::NewLine();
        break;
    case 5:
        currentWeapon = 39;
        ImGui::NewLine();
        break;

    case 1: {
        static int currentPistol{ 0 };
        static constexpr const char* pistols[]{ "All", "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-Seven", "CZ-75", "Desert Eagle", "Revolver" };

        ImGui::Combo("", &currentPistol, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx : 35].enabled) {
                static std::string name;
                name = pistols[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = pistols[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(pistols));

        currentWeapon = currentPistol ? currentPistol : 35;
        break;
    }
    case 2: {
        static int currentHeavy{ 0 };
        static constexpr const char* heavies[]{ "All", "Nova", "XM1014", "Sawed-off", "MAG-7", "M249", "Negev" };

        ImGui::Combo("", &currentHeavy, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 10 : 36].enabled) {
                static std::string name;
                name = heavies[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = heavies[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(heavies));

        currentWeapon = currentHeavy ? currentHeavy + 10 : 36;
        break;
    }
    case 3: {
        static int currentSmg{ 0 };
        static constexpr const char* smgs[]{ "All", "Mac-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };

        ImGui::Combo("", &currentSmg, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 16 : 37].enabled) {
                static std::string name;
                name = smgs[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = smgs[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(smgs));

        currentWeapon = currentSmg ? currentSmg + 16 : 37;
        break;
    }
    case 4: {
        static int currentRifle{ 0 };
        static constexpr const char* rifles[]{ "All", "Galil AR", "Famas", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20" };

        ImGui::Combo("", &currentRifle, [](void* data, int idx, const char** out_text) {
            if (config->triggerbot[idx ? idx + 23 : 38].enabled) {
                static std::string name;
                name = rifles[idx];
                *out_text = name.append(" *").c_str();
            } else {
                *out_text = rifles[idx];
            }
            return true;
            }, nullptr, IM_ARRAYSIZE(rifles));

        currentWeapon = currentRifle ? currentRifle + 23 : 38;
        break;
    }
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->triggerbot[currentWeapon].enabled);
    ImGui::Separator();
    ImGui::Checkbox("On key", &config->triggerbot[currentWeapon].onKey);
    ImGui::SameLine();
    hotkey(config->triggerbot[currentWeapon].key);
    ImGui::Checkbox("Friendly fire", &config->triggerbot[currentWeapon].friendlyFire);
    ImGui::Checkbox("Scoped only", &config->triggerbot[currentWeapon].scopedOnly);
    ImGui::Checkbox("Ignore flash", &config->triggerbot[currentWeapon].ignoreFlash);
    ImGui::Checkbox("Ignore smoke", &config->triggerbot[currentWeapon].ignoreSmoke);
    ImGui::SetNextItemWidth(85.0f);
    ImGui::Combo("Hitgroup", &config->triggerbot[currentWeapon].hitgroup, "All\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Shot delay", &config->triggerbot[currentWeapon].shotDelay, 0, 250, "%d ms");
    ImGui::InputInt("Min damage", &config->triggerbot[currentWeapon].minDamage);
    config->triggerbot[currentWeapon].minDamage = std::clamp(config->triggerbot[currentWeapon].minDamage, 0, 250);
    ImGui::Checkbox("Killshot", &config->triggerbot[currentWeapon].killshot);
    ImGui::SliderFloat("Burst Time", &config->triggerbot[currentWeapon].burstTime, 0.0f, 0.5f, "%.3f s");
    ImGui::SliderFloat("Max aim inaccuracy", &config->triggerbot[currentWeapon].maxAimInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("Max shot inaccuracy", &config->triggerbot[currentWeapon].maxShotInaccuracy, 0.0f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderBacktrackWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.backtrack)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Backtrack (OsirisBETA by PlayDay)", &window.backtrack, windowFlags);
    }
    ImGui::Checkbox("Enabled", &config->backtrack.enabled);
    ImGui::Checkbox("Ignore smoke", &config->backtrack.ignoreSmoke);
    ImGui::Checkbox("Recoil based fov", &config->backtrack.recoilBasedFov);
    ImGui::Checkbox("Draw all ticks", &config->backtrack.drawAllTicks);
    ImGui::Checkbox("Ping based", &config->backtrack.pingBased);
    if (config->backtrack.pingBased) {
        ImGui::SameLine();
        ImGui::Text("(%d ms)", config->backtrack.pingBasedVal);
    }
    else {
        ImGui::PushItemWidth(220.0f);
        ImGui::SliderInt("Time limit", &config->backtrack.timeLimit, 1, 200, "%d ms");
        ImGui::PopItemWidth();
    };
    ImGui::Checkbox("Enabled Fake Latency", &config->backtrack.fakeLatency);
    ImGui::PushItemWidth(220.0f);
    ImGui::SliderInt("Latency Ammount", &config->backtrack.fakeLatencyAmmount, 1, 200, "%d ms");
    ImGui::PopItemWidth();
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderGlowWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.glow)
            return;
        ImGui::SetNextWindowSize({ 450.0f, 0.0f });
        ImGui::Begin("Glow (OsirisBETA by PlayDay)", &window.glow, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);
    ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Weapons\0C4\0Planted C4\0Chickens\0Defuse kits\0Projectiles\0Hostages\0Ragdolls\0");
    ImGui::PopID();
    static int currentItem{ 0 };
    if (currentCategory <= 3) {
        ImGui::SameLine();
        static int currentType{ 0 };
        ImGui::PushID(1);
        ImGui::Combo("", &currentType, "All\0Visible\0Occluded\0");
        ImGui::PopID();
        currentItem = currentCategory * 3 + currentType;
    } else {
        currentItem = currentCategory + 8;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &config->glow[currentItem].enabled);
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 150.0f);
    ImGui::Checkbox("Health based", &config->glow[currentItem].healthBased);

    ImGuiCustom::colorPopup("Color", config->glow[currentItem].color, &config->glow[currentItem].rainbow, &config->glow[currentItem].rainbowSpeed);

    ImGui::NextColumn();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("Style", &config->glow[currentItem].style, "Default\0Rim3d\0Edge\0Edge Pulse\0");

    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderChamsWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.chams)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Chams (OsirisBETA by PlayDay)", &window.chams, windowFlags);
    }
    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::PushID(0);

    static int material = 1;

    if (ImGui::Combo("", &currentCategory, "Allies\0Enemies\0Planting\0Defusing\0Local player\0Weapons\0Hands\0Backtrack\0Sleeves\0"))
        material = 1;

    ImGui::PopID();

    ImGui::SameLine();

    if (material <= 1)
        ImGuiCustom::arrowButtonDisabled("##left", ImGuiDir_Left);
    else if (ImGui::ArrowButton("##left", ImGuiDir_Left))
        --material;

    ImGui::SameLine();
    ImGui::Text("%d", material);

    constexpr std::array categories{ "Allies", "Enemies", "Planting", "Defusing", "Local player", "Weapons", "Hands", "Backtrack", "Sleeves" };

    ImGui::SameLine();

    if (material >= int(config->chams[categories[currentCategory]].materials.size()))
        ImGuiCustom::arrowButtonDisabled("##right", ImGuiDir_Right);
    else if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        ++material;

    ImGui::SameLine();

    auto& chams{ config->chams[categories[currentCategory]].materials[material - 1] };

    ImGui::Checkbox("Enabled", &chams.enabled);
    ImGui::Separator();
    ImGui::Checkbox("Health based", &chams.healthBased);
    ImGui::Checkbox("Blinking", &chams.blinking);
    ImGui::Combo("Material", &chams.material, "Normal\0Flat\0Animated\0Platinum\0Glass\0Chrome\0Crystal\0Silver\0Gold\0Plastic\0Glow\0Pearlescent\0Metallic\0");
    ImGui::Checkbox("Wireframe", &chams.wireframe);
    ImGui::Checkbox("Cover", &chams.cover);
    ImGui::Checkbox("Ignore-Z", &chams.ignorez);
    ImGuiCustom::colorPopup("Color", chams.color, &chams.rainbow, &chams.rainbowSpeed);

    if (!contentOnly) {
        ImGui::End();
    }
}

void GUI::renderStreamProofESPWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.streamProofESP)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("ESP (OsirisBETA by PlayDay)", &window.streamProofESP, windowFlags);
    }

    static std::size_t currentCategory;
    static auto currentItem = "All";

    constexpr auto getConfigShared = [](std::size_t category, const char* item) noexcept -> Shared& {
        switch (category) {
        case 0: default: return config->streamProofESP.enemies[item];
        case 1: return config->streamProofESP.allies[item];
        case 2: return config->streamProofESP.weapons[item];
        case 3: return config->streamProofESP.projectiles[item];
        case 4: return config->streamProofESP.lootCrates[item];
        case 5: return config->streamProofESP.otherEntities[item];
        }
    };

    constexpr auto getConfigPlayer = [](std::size_t category, const char* item) noexcept -> Player& {
        switch (category) {
        case 0: default: return config->streamProofESP.enemies[item];
        case 1: return config->streamProofESP.allies[item];
        }
    };

    if (ImGui::ListBoxHeader("##list", { 170.0f, 300.0f })) {
        constexpr std::array categories{ "Enemies", "Allies", "Weapons", "Projectiles", "Loot Crates", "Other Entities" };

        for (std::size_t i = 0; i < categories.size(); ++i) {
            if (ImGui::Selectable(categories[i], currentCategory == i && std::string_view{ currentItem } == "All")) {
                currentCategory = i;
                currentItem = "All";
            }

            if (ImGui::BeginDragDropSource()) {
                switch (i) {
                case 0: case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, "All"), sizeof(Player), ImGuiCond_Once); break;
                case 2: ImGui::SetDragDropPayload("Weapon", &config->streamProofESP.weapons["All"], sizeof(Weapon), ImGuiCond_Once); break;
                case 3: ImGui::SetDragDropPayload("Projectile", &config->streamProofESP.projectiles["All"], sizeof(Projectile), ImGuiCond_Once); break;
                default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, "All"), sizeof(Shared), ImGuiCond_Once); break;
                }
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                    const auto& data = *(Player*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->streamProofESP.weapons["All"] = data; break;
                    case 3: config->streamProofESP.projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                    const auto& data = *(Weapon*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->streamProofESP.weapons["All"] = data; break;
                    case 3: config->streamProofESP.projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                    const auto& data = *(Projectile*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->streamProofESP.weapons["All"] = data; break;
                    case 3: config->streamProofESP.projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                    const auto& data = *(Shared*)payload->Data;

                    switch (i) {
                    case 0: case 1: getConfigPlayer(i, "All") = data; break;
                    case 2: config->streamProofESP.weapons["All"] = data; break;
                    case 3: config->streamProofESP.projectiles["All"] = data; break;
                    default: getConfigShared(i, "All") = data; break;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PushID(i);
            ImGui::Indent();

            const auto items = [](std::size_t category) noexcept -> std::vector<const char*> {
                switch (category) {
                case 0:
                case 1: return { "Visible", "Occluded" };
                case 2: return { "Pistols", "SMGs", "Rifles", "Sniper Rifles", "Shotguns", "Machineguns", "Grenades", "Melee", "Other" };
                case 3: return { "Flashbang", "HE Grenade", "Breach Charge", "Bump Mine", "Decoy Grenade", "Molotov", "TA Grenade", "Smoke Grenade", "Snowball" };
                case 4: return { "Pistol Case", "Light Case", "Heavy Case", "Explosive Case", "Tools Case", "Cash Dufflebag" };
                case 5: return { "Defuse Kit", "Chicken", "Planted C4", "Hostage", "Sentry", "Cash", "Ammo Box", "Radar Jammer", "Snowball Pile" };
                default: return { };
                }
            }(i);

            const auto categoryEnabled = getConfigShared(i, "All").enabled;

            for (std::size_t j = 0; j < items.size(); ++j) {
                static bool selectedSubItem;
                if (!categoryEnabled || getConfigShared(i, items[j]).enabled) {
                    if (ImGui::Selectable(items[j], currentCategory == i && !selectedSubItem && std::string_view{ currentItem } == items[j])) {
                        currentCategory = i;
                        currentItem = items[j];
                        selectedSubItem = false;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        switch (i) {
                        case 0: case 1: ImGui::SetDragDropPayload("Player", &getConfigPlayer(i, items[j]), sizeof(Player), ImGuiCond_Once); break;
                        case 2: ImGui::SetDragDropPayload("Weapon", &config->streamProofESP.weapons[items[j]], sizeof(Weapon), ImGuiCond_Once); break;
                        case 3: ImGui::SetDragDropPayload("Projectile", &config->streamProofESP.projectiles[items[j]], sizeof(Projectile), ImGuiCond_Once); break;
                        default: ImGui::SetDragDropPayload("Entity", &getConfigShared(i, items[j]), sizeof(Shared), ImGuiCond_Once); break;
                        }
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->streamProofESP.weapons[items[j]] = data; break;
                            case 3: config->streamProofESP.projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->streamProofESP.weapons[items[j]] = data; break;
                            case 3: config->streamProofESP.projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->streamProofESP.weapons[items[j]] = data; break;
                            case 3: config->streamProofESP.projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;

                            switch (i) {
                            case 0: case 1: getConfigPlayer(i, items[j]) = data; break;
                            case 2: config->streamProofESP.weapons[items[j]] = data; break;
                            case 3: config->streamProofESP.projectiles[items[j]] = data; break;
                            default: getConfigShared(i, items[j]) = data; break;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                if (i != 2)
                    continue;

                ImGui::Indent();

                const auto subItems = [](std::size_t item) noexcept -> std::vector<const char*> {
                    switch (item) {
                    case 0: return { "Glock-18", "P2000", "USP-S", "Dual Berettas", "P250", "Tec-9", "Five-SeveN", "CZ75-Auto", "Desert Eagle", "R8 Revolver" };
                    case 1: return { "MAC-10", "MP9", "MP7", "MP5-SD", "UMP-45", "P90", "PP-Bizon" };
                    case 2: return { "Galil AR", "FAMAS", "AK-47", "M4A4", "M4A1-S", "SG 553", "AUG" };
                    case 3: return { "SSG 08", "AWP", "G3SG1", "SCAR-20" };
                    case 4: return { "Nova", "XM1014", "Sawed-Off", "MAG-7" };
                    case 5: return { "M249", "Negev" };
                    case 6: return { "Flashbang", "HE Grenade", "Smoke Grenade", "Molotov", "Decoy Grenade", "Incendiary", "TA Grenade", "Fire Bomb", "Diversion", "Frag Grenade", "Snowball" };
                    case 7: return { "Axe", "Hammer", "Wrench" };
                    case 8: return { "C4", "Healthshot", "Bump Mine", "Zone Repulsor", "Shield" };
                    default: return { };
                    }
                }(j);

                const auto itemEnabled = getConfigShared(i, items[j]).enabled;

                for (const auto subItem : subItems) {
                    auto& subItemConfig = config->streamProofESP.weapons[subItem];
                    if ((categoryEnabled || itemEnabled) && !subItemConfig.enabled)
                        continue;

                    if (ImGui::Selectable(subItem, currentCategory == i && selectedSubItem && std::string_view{ currentItem } == subItem)) {
                        currentCategory = i;
                        currentItem = subItem;
                        selectedSubItem = true;
                    }

                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("Weapon", &subItemConfig, sizeof(Weapon), ImGuiCond_Once);
                        ImGui::EndDragDropSource();
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Player")) {
                            const auto& data = *(Player*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Weapon")) {
                            const auto& data = *(Weapon*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Projectile")) {
                            const auto& data = *(Projectile*)payload->Data;
                            subItemConfig = data;
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity")) {
                            const auto& data = *(Shared*)payload->Data;
                            subItemConfig = data;
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::Unindent();
            }
            ImGui::Unindent();
            ImGui::PopID();
        }
        ImGui::ListBoxFooter();
    }

    ImGui::SameLine();

    if (ImGui::BeginChild("##child", { 400.0f, 0.0f })) {
        auto& sharedConfig = getConfigShared(currentCategory, currentItem);

        ImGui::Checkbox("Enabled", &sharedConfig.enabled);
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 260.0f);
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::BeginCombo("Font", config->systemFonts[sharedConfig.font.index].c_str())) {
            for (size_t i = 0; i < config->systemFonts.size(); i++) {
                bool isSelected = config->systemFonts[i] == sharedConfig.font.name;
                if (ImGui::Selectable(config->systemFonts[i].c_str(), isSelected, 0, { 250.0f, 0.0f })) {
                    sharedConfig.font.index = i;
                    sharedConfig.font.name = config->systemFonts[i];
                    config->scheduleFontLoad(sharedConfig.font.name);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        constexpr auto spacing = 250.0f;
        ImGuiCustom::colorPicker("Snapline", sharedConfig.snapline);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        ImGui::Combo("##1", &sharedConfig.snapline.type, "Bottom\0Top\0Crosshair\0");
        ImGui::SameLine(spacing);
        ImGuiCustom::colorPicker("Box", sharedConfig.box);
        ImGui::SameLine();

        ImGui::PushID("Box");

        if (ImGui::Button("..."))
            ImGui::OpenPopup("");

        if (ImGui::BeginPopup("")) {
            ImGui::SetNextItemWidth(95.0f);
            ImGui::Combo("Type", &sharedConfig.box.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
            ImGui::SetNextItemWidth(275.0f);
            ImGui::SliderFloat3("Scale", sharedConfig.box.scale.data(), 0.0f, 0.50f, "%.2f");
            ImGuiCustom::colorPicker("Fill", sharedConfig.box.fill);
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGuiCustom::colorPicker("Name", sharedConfig.name);
        ImGui::SameLine(spacing);

        if (currentCategory < 2) {
            auto& playerConfig = getConfigPlayer(currentCategory, currentItem);

            ImGuiCustom::colorPicker("Weapon", playerConfig.weapon);
            ImGuiCustom::colorPicker("Flash Duration", playerConfig.flashDuration);
            ImGui::SameLine(spacing);
            ImGuiCustom::colorPicker("Skeleton", playerConfig.skeleton);
            ImGui::Checkbox("Audible Only", &playerConfig.audibleOnly);
            ImGui::SameLine(spacing);
            ImGui::Checkbox("Spotted Only", &playerConfig.spottedOnly);

            ImGuiCustom::colorPicker("Head Box", playerConfig.headBox);
            ImGui::SameLine();

            ImGui::PushID("Head Box");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                ImGui::SetNextItemWidth(95.0f);
                ImGui::Combo("Type", &playerConfig.headBox.type, "2D\0" "2D corners\0" "3D\0" "3D corners\0");
                ImGui::SetNextItemWidth(275.0f);
                ImGui::SliderFloat3("Scale", playerConfig.headBox.scale.data(), 0.0f, 0.50f, "%.2f");
                ImGuiCustom::colorPicker("Fill", playerConfig.headBox.fill);
                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::SameLine(spacing);
            ImGui::Checkbox("Health Bar", &playerConfig.healthBar);
        } else if (currentCategory == 2) {
            auto& weaponConfig = config->streamProofESP.weapons[currentItem];
            ImGuiCustom::colorPicker("Ammo", weaponConfig.ammo);
        } else if (currentCategory == 3) {
            auto& trails = config->streamProofESP.projectiles[currentItem].trails;

            ImGui::Checkbox("Trails", &trails.enabled);
            ImGui::SameLine(spacing + 77.0f);
            ImGui::PushID("Trails");

            if (ImGui::Button("..."))
                ImGui::OpenPopup("");

            if (ImGui::BeginPopup("")) {
                constexpr auto trailPicker = [](const char* name, Trail& trail) noexcept {
                    ImGui::PushID(name);
                    ImGuiCustom::colorPicker(name, trail);
                    ImGui::SameLine(150.0f);
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::Combo("", &trail.type, "Line\0Circles\0Filled Circles\0");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(95.0f);
                    ImGui::InputFloat("Time", &trail.time, 0.1f, 0.5f, "%.1fs");
                    trail.time = std::clamp(trail.time, 1.0f, 60.0f);
                    ImGui::PopID();
                };

                trailPicker("Local Player", trails.localPlayer);
                trailPicker("Allies", trails.allies);
                trailPicker("Enemies", trails.enemies);
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::SetNextItemWidth(95.0f);
        ImGui::InputFloat("Text Cull Distance", &sharedConfig.textCullDistance, 0.4f, 0.8f, "%.1fm");
        sharedConfig.textCullDistance = std::clamp(sharedConfig.textCullDistance, 0.0f, 999.9f);
    }

    ImGui::EndChild();

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderVisualsWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.visuals)
            return;
        ImGui::SetNextWindowSize({ 680.0f, 0.0f });
        ImGui::Begin("Visuals (OsirisBETA by PlayDay)", &window.visuals, windowFlags);
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 280.0f);
    constexpr auto playerModels = "Default\0Special Agent Ava | FBI\0Operator | FBI SWAT\0Markus Delrow | FBI HRT\0Michael Syfers | FBI Sniper\0B Squadron Officer | SAS\0Seal Team 6 Soldier | NSWC SEAL\0Buckshot | NSWC SEAL\0Lt. Commander Ricksaw | NSWC SEAL\0Third Commando Company | KSK\0'Two Times' McCoy | USAF TACP\0Dragomir | Sabre\0Rezan The Ready | Sabre\0'The Doctor' Romanov | Sabre\0Maximus | Sabre\0Blackwolf | Sabre\0The Elite Mr. Muhlik | Elite Crew\0Ground Rebel | Elite Crew\0Osiris | Elite Crew\0Prof. Shahmat | Elite Crew\0Enforcer | Phoenix\0Slingshot | Phoenix\0Soldier | Phoenix\0Pirate\0Pirate Variant A\0Pirate Variant B\0Pirate Variant C\0Pirate Variant D\0Anarchist\0Anarchist Variant A\0Anarchist Variant B\0Anarchist Variant C\0Anarchist Variant D\0Balkan Variant A\0Balkan Variant B\0Balkan Variant C\0Balkan Variant D\0Balkan Variant E\0Jumpsuit Variant A\0Jumpsuit Variant B\0Jumpsuit Variant C\0";
    ImGui::Combo("T Player Model", &config->visuals.playerModelT, playerModels);
    ImGui::Combo("CT Player Model", &config->visuals.playerModelCT, playerModels);
    ImGui::Checkbox("Disable post-processing", &config->visuals.disablePostProcessing);
    ImGui::Checkbox("Inverse ragdoll gravity", &config->visuals.inverseRagdollGravity);
    if (config->visuals.inverseRagdollGravity) {
        ImGui::SameLine();
        ImGui::Checkbox("Custom Value", &config->visuals.inverseRagdollGravityCustomize);
        if (config->visuals.inverseRagdollGravityCustomize) {
            ImGui::PushItemWidth(280.0f);
            ImGui::PushID(0);
            ImGui::SliderInt("", &config->visuals.inverseRagdollGravityValue, -2400, 2400, "Ragdoll gravity value: %d");
            ImGui::PopID();
        };
    };
    ImGui::Checkbox("Custom Physics Timescale", &config->visuals.ragdollTimescale);
    if (config->visuals.ragdollTimescale) {
        ImGui::PushItemWidth(280.0f);
        ImGui::PushID(1);
        ImGui::SliderFloat("", &config->visuals.ragdollTimescaleValue, 0, 10, "Physics timescale: %.2f");
        ImGui::PopID();
        ImGui::PopItemWidth();
    };
    ImGui::Checkbox("Night Mode", &config->visuals.nightMode);
    ImGui::Checkbox("No fog", &config->visuals.noFog);
    ImGui::Checkbox("No 3d sky", &config->visuals.no3dSky);
    ImGui::Checkbox("No aim punch", &config->visuals.noAimPunch);
    ImGui::Checkbox("No view punch", &config->visuals.noViewPunch);
    ImGui::Checkbox("No hands", &config->visuals.noHands);
    ImGui::Checkbox("No sleeves", &config->visuals.noSleeves);
    ImGui::Checkbox("No weapons", &config->visuals.noWeapons);
    ImGui::Checkbox("No smoke", &config->visuals.noSmoke);
    ImGui::Checkbox("No blur", &config->visuals.noBlur);
    ImGui::Checkbox("No bloom", &config->visuals.noBloom);
    ImGui::Checkbox("No scope overlay", &config->visuals.noScopeOverlay);
    ImGui::Checkbox("No grass", &config->visuals.noGrass);
    ImGui::Checkbox("No shadows", &config->visuals.noShadows);
    ImGui::Checkbox("Wireframe smoke", &config->visuals.wireframeSmoke);
    ImGuiCustom::colorPicker("Show velocity", config->visuals.showvelocity);
    if (config->visuals.showvelocity.enabled) {
        ImGui::SameLine();
        ImGui::Checkbox("Custom pos", &config->visuals.showvelocityM);
        if (config->visuals.showvelocityM) {
            ImGui::PushID(2);
            ImGui::SliderInt("", &config->visuals.showvelocityPosX, 0, config->visuals.showvelocityResX, "X pos: %d");
            ImGui::PopID();
            ImGui::PushID(3);
            ImGui::SliderInt("", &config->visuals.showvelocityPosY, 0, config->visuals.showvelocityResY, "Y pos: %d");
            ImGui::PopID();
        }
    }
    ImGui::NextColumn();
    ImGui::Checkbox("Zoom", &config->visuals.zoom);
    ImGui::SameLine();
    hotkey(config->visuals.zoomKey);
    ImGui::Checkbox("No zoom", &config->visuals.noZoom);
    ImGui::Checkbox("Thirdperson", &config->visuals.thirdperson);
    ImGui::SameLine();
    hotkey(config->visuals.thirdpersonKey);
    if (config->visuals.thirdperson) {
        ImGui::SameLine();
        ImGui::Checkbox("Dead thirdperson", &config->visuals.deadThirdperson);
    }
    else
        config->visuals.deadThirdperson = false;
    ImGui::PushItemWidth(290.0f);
    ImGui::PushID(4);
    ImGui::SliderInt("", &config->visuals.thirdpersonDistance, 0, 1000, "Thirdperson distance: %d");
    ImGui::PopID();
    ImGui::PushID(5);
    ImGui::SliderInt("", &config->visuals.viewmodelFov, -60, 60, "Viewmodel FOV: %d");
    ImGui::PopID();
    ImGui::PushID(6);
    ImGui::SliderInt("", &config->visuals.fov, -60, 60, "FOV: %d");
    ImGui::PopID();
    ImGui::PushID(7);
    ImGui::SliderInt("", &config->visuals.farZ, 0, 2000, "Far Z: %d");
    ImGui::PopID();
    ImGui::PushID(8);
    ImGui::SliderInt("", &config->visuals.flashReduction, 0, 100, "Flash reduction: %d%%");
    ImGui::PopID();
    if (!config->visuals.fullBright) {
        ImGui::PushID(9);
        ImGui::SliderFloat("", &config->visuals.brightness, 0.0f, 1.0f, "Brightness: %.2f");
        ImGui::PopID();
    }
    else {
        ImGui::PushID(10);
        ImGui::SliderFloat("", &config->visuals.brightness, 0.0f, 0.0f, "Disabled for Full Bright");
        ImGui::PopID();
    }
    ImGui::PopItemWidth();
    ImGui::Checkbox("Full Bright", &config->visuals.fullBright);
    ImGui::Combo("Skybox", &config->visuals.skybox, Helpers::getSkyboxes().data(), Helpers::getSkyboxes().size());
    if (config->visuals.skybox == 1) {
        ImGui::InputText("Skybox filename", &config->visuals.customSkybox);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("skybox file must be put in csgo/materials/skybox directory");
    }
    ImGuiCustom::colorPicker("Rainbow Bar", config->visuals.rainbowBar);
    if (config->visuals.rainbowBar.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Rainbow Bar");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("RB");

        if (ImGui::BeginPopup("RB")) {
            ImGui::Text("Position:");
            ImGui::Checkbox("Upper", &config->visuals.rainbowUp);
            ImGui::Checkbox("Bottom", &config->visuals.rainbowBottom);
            ImGui::Checkbox("Left", &config->visuals.rainbowLeft);
            ImGui::Checkbox("Right", &config->visuals.rainbowRight);
            ImGui::Text("Scale:");
            ImGui::SliderFloat("Scale", &config->visuals.rainbowScale, 0.03125f, 1.0f, "%.5f", ImGuiSliderFlags_Logarithmic);
            ImGui::Text("Scale presets:");
            if (ImGui::Button("0.25x"))
                config->visuals.rainbowScale = 0.03125f;
            ImGui::SameLine();
            if (ImGui::Button("0.5x"))
                config->visuals.rainbowScale = 0.0625f;
            ImGui::SameLine();
            if (ImGui::Button("1x"))
                config->visuals.rainbowScale = 0.125f;
            ImGui::SameLine();
            if (ImGui::Button("2x"))
                config->visuals.rainbowScale = 0.25f;
            ImGui::SameLine();
            if (ImGui::Button("4x"))
                config->visuals.rainbowScale = 0.5f;
            ImGui::SameLine();
            if (ImGui::Button("8x"))
                config->visuals.rainbowScale = 1.0f;
            ImGui::Text("Pulse:");
            ImGui::Checkbox("Enable", &config->visuals.rainbowPulse);
            ImGui::SliderFloat("Speed", &config->visuals.rainbowPulseSpeed, 0.1f, 25.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGuiCustom::colorPicker("World color", config->visuals.world);
    ImGuiCustom::colorPicker("Sky color", config->visuals.sky);
    ImGui::Checkbox("Deagle spinner", &config->visuals.deagleSpinner);
    ImGui::Combo("Screen effect", &config->visuals.screenEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::Combo("Hit effect", &config->visuals.hitEffect, "None\0Drone cam\0Drone cam with noise\0Underwater\0Healthboost\0Dangerzone\0");
    ImGui::SliderFloat("Hit effect time", &config->visuals.hitEffectTime, 0.1f, 1.5f, "%.2fs");
    ImGui::Combo("Hit marker", &config->visuals.hitMarker, "None\0Default (Cross)\0");
    ImGui::SliderFloat("Hit marker time", &config->visuals.hitMarkerTime, 0.1f, 1.5f, "%.2fs");
    ImGuiCustom::colorPicker("Hit marker damage indicator", config->visuals.hitMarkerDamageIndicator);
    if (config->visuals.hitMarkerDamageIndicator.enabled) {
        ImGui::SameLine();
        ImGui::PushID("Hit marker damage indicator");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("HMDI");

        if (ImGui::BeginPopup("HMDI")) {
            ImGui::Checkbox("Customize Hitmarker", &config->visuals.hitMarkerDamageIndicatorCustomize);
            if (config->visuals.hitMarkerDamageIndicatorCustomize) {
                ImGui::InputInt("Font", &config->visuals.hitMarkerDamageIndicatorFont, 1, 294);
                ImGui::InputInt("Alpha", &config->visuals.hitMarkerDamageIndicatorAlpha, 1, 1000);
                ImGui::InputInt("Dist", &config->visuals.hitMarkerDamageIndicatorDist, -100, 100);
                ImGui::InputInt("Text X", &config->visuals.hitMarkerDamageIndicatorTextX, -100, 100);
                ImGui::InputInt("Text Y", &config->visuals.hitMarkerDamageIndicatorTextY, -100, 100);
                ImGui::PushID(11);
                ImGui::SliderFloat(" ", &config->visuals.hitMarkerDamageIndicatorRatio, -1.0f, 1.0f, "Ratio: %.2f");
                ImGui::PopID();
            };
            ImGui::EndPopup();
        }
        ImGui::PopID();
    };
    ImGuiCustom::colorPicker("Bullet Tracers", config->visuals.bulletTracers);
    ImGui::Checkbox("Color correction", &config->visuals.colorCorrection.enabled);
    ImGui::SameLine();
    bool ccPopup = ImGui::Button("Edit");

    if (ccPopup)
        ImGui::OpenPopup("##popup");

    if (ImGui::BeginPopup("##popup")) {
        ImGui::VSliderFloat("##1", { 40.0f, 160.0f }, &config->visuals.colorCorrection.blue, 0.0f, 1.0f, "Blue\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##2", { 40.0f, 160.0f }, &config->visuals.colorCorrection.red, 0.0f, 1.0f, "Red\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##3", { 40.0f, 160.0f }, &config->visuals.colorCorrection.mono, 0.0f, 1.0f, "Mono\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##4", { 40.0f, 160.0f }, &config->visuals.colorCorrection.saturation, 0.0f, 1.0f, "Sat\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##5", { 40.0f, 160.0f }, &config->visuals.colorCorrection.ghost, 0.0f, 1.0f, "Ghost\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##6", { 40.0f, 160.0f }, &config->visuals.colorCorrection.green, 0.0f, 1.0f, "Green\n%.3f"); ImGui::SameLine();
        ImGui::VSliderFloat("##7", { 40.0f, 160.0f }, &config->visuals.colorCorrection.yellow, 0.0f, 1.0f, "Yellow\n%.3f"); ImGui::SameLine();
        ImGui::EndPopup();
    }
    ImGui::Columns(1);

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderSkinChangerWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.skinChanger)
            return;
        ImGui::SetNextWindowSize({ 700.0f, 0.0f });
        ImGui::Begin("Skin changer (OsirisBETA by PlayDay)", &window.skinChanger, windowFlags);
    }

    static auto itemIndex = 0;

    ImGui::PushItemWidth(110.0f);
    ImGui::Combo("##1", &itemIndex, [](void* data, int idx, const char** out_text) {
        *out_text = game_data::weapon_names[idx].name;
        return true;
        }, nullptr, IM_ARRAYSIZE(game_data::weapon_names), 5);
    ImGui::PopItemWidth();

    auto& selected_entry = config->skinChanger[itemIndex];
    selected_entry.itemIdIndex = itemIndex;

    {
        ImGui::SameLine();
        ImGui::Checkbox("Enabled", &selected_entry.enabled);
        ImGui::Separator();
        ImGui::Columns(2, nullptr, false);
        ImGui::InputInt("Seed", &selected_entry.seed);
        ImGui::InputInt("StatTrak\u2122", &selected_entry.stat_trak);
        selected_entry.stat_trak = (std::max)(selected_entry.stat_trak, -1);
        ImGui::SliderFloat("Wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", ImGuiSliderFlags_Logarithmic);

        static std::string filter;
        ImGui::PushID("Search");
        ImGui::InputTextWithHint("", "Search", &filter);
        ImGui::PopID();

        if (ImGui::ListBoxHeader("Paint Kit")) {
            const auto& kits = itemIndex == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;

            const std::locale original;
            std::locale::global(std::locale{ "en_US.utf8" });

            const auto& facet = std::use_facet<std::ctype<wchar_t>>(std::locale{});
            std::wstring filterWide(filter.length(), L'\0');
            const auto newLen = mbstowcs(filterWide.data(), filter.c_str(), filter.length());
            if (newLen != static_cast<std::size_t>(-1))
                filterWide.resize(newLen);
            std::transform(filterWide.begin(), filterWide.end(), filterWide.begin(), [&facet](wchar_t w) { return facet.toupper(w); });

            std::locale::global(original);

            for (std::size_t i = 0; i < kits.size(); ++i) {
                if (filter.empty() || wcsstr(kits[i].nameUpperCase.c_str(), filterWide.c_str())) {
                    ImGui::PushID(i);
                    if (ImGui::Selectable(kits[i].name.c_str(), i == selected_entry.paint_kit_vector_index))
                        selected_entry.paint_kit_vector_index = i;
                    ImGui::PopID();
                }
            }
            ImGui::ListBoxFooter();
        }

        ImGui::Combo("Quality", &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text) {
            *out_text = game_data::quality_names[idx].name;
            return true;
            }, nullptr, IM_ARRAYSIZE(game_data::quality_names), 5);

        if (itemIndex == 0) {
            ImGui::Combo("Knife", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
                *out_text = game_data::knife_names[idx].name;
                return true;
                }, nullptr, IM_ARRAYSIZE(game_data::knife_names), 5);
        } else if (itemIndex == 1) {
            ImGui::Combo("Glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
                *out_text = game_data::glove_names[idx].name;
                return true;
                }, nullptr, IM_ARRAYSIZE(game_data::glove_names), 5);
        } else {
            static auto unused_value = 0;
            selected_entry.definition_override_vector_index = 0;
            ImGui::Combo("Unavailable", &unused_value, "For knives or gloves\0");
        }

        ImGui::InputText("Name Tag", selected_entry.custom_name, 32);
    }

    ImGui::NextColumn();

    {
        ImGui::PushID("sticker");

        static auto selectedStickerSlot = 0;

        ImGui::PushItemWidth(-1);

        if (ImGui::ListBoxHeader("", 5)) {
            for (int i = 0; i < 5; ++i) {
                ImGui::PushID(i);

                const auto kit_vector_index = config->skinChanger[itemIndex].stickers[i].kit_vector_index;
                const std::string text = '#' + std::to_string(i + 1) + "  " + SkinChanger::stickerKits[kit_vector_index].name;

                if (ImGui::Selectable(text.c_str(), i == selectedStickerSlot))
                    selectedStickerSlot = i;

                ImGui::PopID();
            }
            ImGui::ListBoxFooter();
        }

        ImGui::PopItemWidth();

        auto& selected_sticker = selected_entry.stickers[selectedStickerSlot];

        static std::string filter;
        ImGui::PushID("Search");
        ImGui::InputTextWithHint("", "Search", &filter);
        ImGui::PopID();

        if (ImGui::ListBoxHeader("Sticker")) {
            const auto& kits = SkinChanger::stickerKits;

            const std::locale original;
            std::locale::global(std::locale{ "en_US.utf8" });

            const auto& facet = std::use_facet<std::ctype<wchar_t>>(std::locale{});
            std::wstring filterWide(filter.length(), L'\0');
            const auto newLen = mbstowcs(filterWide.data(), filter.c_str(), filter.length());
            if (newLen != static_cast<std::size_t>(-1))
                filterWide.resize(newLen);
            std::transform(filterWide.begin(), filterWide.end(), filterWide.begin(), [&facet](wchar_t w) { return facet.toupper(w); });

            std::locale::global(original);

            for (std::size_t i = 0; i < kits.size(); ++i) {
                if (filter.empty() || wcsstr(kits[i].nameUpperCase.c_str(), filterWide.c_str())) {
                    ImGui::PushID(i);
                    if (ImGui::Selectable(kits[i].name.c_str(), i == selected_sticker.kit_vector_index))
                        selected_sticker.kit_vector_index = i;
                    ImGui::PopID();
                }
            }
            ImGui::ListBoxFooter();
        }

        ImGui::SliderFloat("Wear", &selected_sticker.wear, FLT_MIN, 1.0f, "%.10f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Scale", &selected_sticker.scale, 0.1f, 5.0f);
        ImGui::SliderFloat("Rotation", &selected_sticker.rotation, 0.0f, 360.0f);

        ImGui::PopID();
    }
    selected_entry.update();

    ImGui::Columns(1);

    ImGui::Separator();

    if (ImGui::Button("Update", { 130.0f, 30.0f }))
        SkinChanger::scheduleHudUpdate();

    ImGui::TextUnformatted("nSkinz by namazso");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderSoundWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.sound)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Sound (OsirisBETA by PlayDay)", &window.sound, windowFlags);
    }
    ImGui::SliderInt("Chicken volume", &config->sound.chickenVolume, 0, 200, "%d%%");

    static int currentCategory{ 0 };
    ImGui::PushItemWidth(110.0f);
    ImGui::Combo("", &currentCategory, "Local player\0Allies\0Enemies\0");
    ImGui::PopItemWidth();
    ImGui::SliderInt("Master volume", &config->sound.players[currentCategory].masterVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Headshot volume", &config->sound.players[currentCategory].headshotVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Weapon volume", &config->sound.players[currentCategory].weaponVolume, 0, 200, "%d%%");
    ImGui::SliderInt("Footstep volume", &config->sound.players[currentCategory].footstepVolume, 0, 200, "%d%%");

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderStyleWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.style)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Style (OsirisBETA by PlayDay)", &window.style, windowFlags);
    }

    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu style", &config->style.menuStyle, "Classic\0One window\0"))
        window = { };
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark\0Light\0Classic\0Custom\0"))
        updateColors();
    ImGui::PopItemWidth();

    if (config->style.menuColors == 3) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPopup(ImGui::GetStyleColorName(i), (std::array<float, 4>&)style.Colors[i]);
        }
    }

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderMiscWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.misc)
            return;
        ImGui::SetNextWindowSize({ 580.0f, 0.0f });
        ImGui::Begin("Misc (OsirisBETA by PlayDay)", &window.misc, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 230.0f);
    ImGui::TextUnformatted("Menu key");
    ImGui::SameLine();
    hotkey(config->misc.menuKey);

    ImGui::Checkbox("Anti AFK kick", &config->misc.antiAfkKick);
    ImGui::Checkbox("Auto strafe", &config->misc.autoStrafe);
    ImGui::SameLine();
    hotkey(config->misc.autoStrafeKey);
    ImGui::Combo("Auto strafe style", &config->misc.autoStrafeStyle, "Legit\0Normal\0");
    ImGui::Checkbox("Bunny hop", &config->misc.bunnyHop);
    if (config->misc.bunnyHop) {
        ImGui::SameLine();
        ImGui::PushID("Bunny hop");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("BH");

        if (ImGui::BeginPopup("BH")) {
            ImGui::Checkbox("Hitchance", &config->misc.bhopHitchanceEnable);
            if (config->misc.bhopHitchanceEnable) {
                ImGui::SliderInt("Bhop hitchance", &config->misc.bhopHitchance, 0, 100, "%d%");
                ImGui::SliderInt("Min hits", &config->misc.bhopMinHits, 0, 20, "%d%");
                ImGui::SliderInt("Max hits", &config->misc.bhopMaxHits, 0, 20, "%d%");
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::Checkbox("Fast duck", &config->misc.fastDuck);
    ImGui::Checkbox("Moonwalk", &config->misc.moonwalk);
    ImGui::Checkbox("Edge Jump", &config->misc.edgejump);
    ImGui::SameLine();
    hotkey(config->misc.edgejumpkey);
    ImGui::Checkbox("Jump Bug", &config->misc.jumpbug);
    ImGui::SameLine();
    hotkey(config->misc.jumpbugkey);
    if (config->misc.jumpbug) {
        ImGui::SameLine();
        ImGui::Checkbox("Hold", &config->misc.jumpbughold);
    }
    ImGui::Checkbox("Slowwalk", &config->misc.slowwalk);
    ImGui::SameLine();
    hotkey(config->misc.slowwalkKey);
    ImGui::Checkbox("Door spam", &config->misc.doorSpam);
    ImGui::Checkbox("BuyBot", &config->misc.buyBot);
    ImGui::SameLine();
    ImGui::PushID("BuyBot");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("BB");
    if (ImGui::BeginPopup("BB")) {
        ImGui::Combo("Primary Weapon", &config->misc.buyBotPrimary, "None\0Famas/Galil\0M4A1-S/M4A4/AK-47\0SSG 08\0AUG/SG 553\0AWP\0SCAR-20/G3GS1\0MP9/MAC-10\0MP7/MP5-SD\0UMP-45\0P90\0PP-Bizon\0Nova\0XM1014\0MAG-7/Sawed-Off\0M249\0Negev\0");
        ImGui::Combo("Secnodary Weapon", &config->misc.buyBotSecondary, "None\0P2000/USP-S/Glock-18\0P250\0Dual Berettas\0Five-Seven/Tec-9/CZ75-Auto\0Desert Eagle/R8 Revolver");
        ImGui::Text("Gear:");
        ImGui::Checkbox("Kevlar Vest", &config->misc.buyBotVest);
        if (config->misc.buyBotVest) {
            ImGui::SameLine();
            ImGui::Checkbox("+ Helmet", &config->misc.buyBotVestHelm);
        }
        ImGui::Checkbox("Zeus x27", &config->misc.buyBotTaser);
        ImGui::Checkbox("Defuser", &config->misc.buyBotDefuser);
        ImGui::Text("Grenade:");
        ImGui::Checkbox("Incendiary/Molotov", &config->misc.buyBotMolotov);
        ImGui::Checkbox("Decoy", &config->misc.buyBotDecoy);
        ImGui::Checkbox("Flashbang", &config->misc.buyBotFlashbang);
        if (config->misc.buyBotFlashbang) {
            ImGui::SameLine();
            ImGui::Checkbox("x2", &config->misc.buyBotFlashbangX2);
        }
        ImGui::Checkbox("High Explosive", &config->misc.buyBotHE);
        ImGui::Checkbox("Smoke", &config->misc.buyBotSmoke);

        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGuiCustom::colorPicker("Noscope crosshair", config->misc.noscopeCrosshair);
    ImGuiCustom::colorPicker("Recoil crosshair", config->misc.recoilCrosshair);
    ImGui::Checkbox("Auto pistol", &config->misc.autoPistol);
    ImGui::Checkbox("Auto reload", &config->misc.autoReload);
    ImGui::Checkbox("Auto accept", &config->misc.autoAccept);
    ImGui::Checkbox("Radar hack", &config->misc.radarHack);
    ImGui::Checkbox("Reveal ranks", &config->misc.revealRanks);
    ImGui::Checkbox("Reveal money", &config->misc.revealMoney);
    ImGui::Checkbox("Reveal suspect", &config->misc.revealSuspect);
    ImGuiCustom::colorPicker("Spectator list", config->misc.spectatorList);
    ImGuiCustom::colorPicker("Watermark", config->misc.watermark);
    ImGui::Checkbox("Fix animation LOD", &config->misc.fixAnimationLOD);
    ImGui::Checkbox("Fix bone matrix", &config->misc.fixBoneMatrix);
    ImGui::Checkbox("Fix movement", &config->misc.fixMovement);
    ImGui::Checkbox("Disable model occlusion", &config->misc.disableModelOcclusion);
    ImGui::SliderFloat("Aspect Ratio", &config->misc.aspectratio, 0.0f, 5.0f, "%.2f");
    ImGui::Checkbox("Bypass sv_pure", &config->misc.svpurebypass);
    ImGui::Checkbox("Bypass sv_pure (OLD)", &config->misc.svpurebypassOLD);
    ImGui::Checkbox("Draw aimbot FOV", &config->misc.drawAimbotFov);
    ImGuiCustom::colorPicker("Draw Inaccuracy", config->misc.drawInaccuracy);
    ImGui::Checkbox("Team Damage Counter", &config->misc.teamDamageCounter);
    if (config->misc.teamDamageCounter)
        if (ImGui::Button("Reset Counter")) {
            Misc::teamKills = 0;
            Misc::teamDamage = 0;
        }
    ImGui::NextColumn();
    ImGui::Checkbox("Disable HUD blur", &config->misc.disablePanoramablur);
    ImGui::Combo("Clantag", &config->misc.clanTagStyle, "None\0Clock tag\0Custom clantag\0OsirisBETA\0Custom multiline clantag\0");
    if (config->misc.clanTagStyle == 2) {
        ImGui::Checkbox("Animated clan tag", &config->misc.animatedClanTag);
        ImGui::SliderFloat("Clantag Speed", &config->misc.customClanTagSpeed, 10.0f, 0.1f, "%.1f", ImGuiSliderFlags_ClampOnInput);
        ImGui::PushItemWidth(120.0f);
        ImGui::PushID(0);

        if (ImGui::InputText("Clantag Text", config->misc.clanTag, sizeof(config->misc.clanTag)))
            Misc::updateClanTag(true);
    }
    else if (config->misc.clanTagStyle == 3)
        ImGui::SliderFloat("Clantag Speed", &config->misc.OsirisBETAClanTagSpeed, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_ClampOnInput);
    else if (config->misc.clanTagStyle == 4) {
        ImGui::SameLine();
        ImGui::PushID("Custom multiline clantag");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("CMC");

        if (ImGui::BeginPopup("CMC")) {
            ImGui::SliderFloat("Clantag Speed", &config->misc.customMultiClanTagSpeed, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_ClampOnInput);
            ImGui::Text("Custom multiline clantag:");
            ImGui::InputTextMultiline("", &config->misc.customMultiClanTag);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::Checkbox("Chat spam", &config->misc.chatSpam);
    ImGui::SameLine();
    ImGui::PushID("Chat Spam");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("CS");

    if (ImGui::BeginPopup("CS")) {
        ImGui::Checkbox("Random", &config->misc.chatSpamRandom);
        ImGui::SliderInt("Delay", &config->misc.chatSpamDelay, 1, 60, "%d s");
        ImGui::TextUnformatted("Phrases");
        ImGui::PushID(1);
        ImGui::InputTextMultiline("", &config->misc.chatSpamPhrases, { 280.0f, 120.0f });
        ImGui::PopID();
        ImGui::EndPopup();
    }
    ImGui::PopID();
    ImGui::PopID();
    ImGui::Checkbox("Kill message", &config->misc.killMessage);
    ImGui::SameLine();
    ImGui::PushItemWidth(120.0f);
    ImGui::PushID(2);
    ImGui::InputText("", &config->misc.killMessageString);
    ImGui::PopID();
    ImGui::Checkbox("Name stealer", &config->misc.nameStealer);
    ImGui::PushID(3);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::Combo("", &config->misc.banColor, "White\0Red\0Purple\0Green\0Light green\0Turquoise\0Light red\0Gray\0Yellow\0Gray 2\0Light blue\0Gray/Purple\0Blue\0Pink\0Dark orange\0Orange\0");
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::PushID(4);
    ImGui::InputText("", &config->misc.banText);
    ImGui::PopID();
    ImGui::SameLine();
    if (ImGui::Button("Setup fake ban"))
        Misc::fakeBan(true);
    ImGui::InputText("Custom Name", &config->misc.customName);
    if (ImGui::Button("Change Name"))
        Misc::setName(true);
    ImGui::Checkbox("Fake Item", &config->misc.fakeItem);
    if (config->misc.fakeItem) {
        ImGui::SameLine();
        ImGui::PushID("Fake Item");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("FI");

        if (ImGui::BeginPopup("FI")) {
            ImGui::SetNextItemWidth(200.0f);
            ImGui::Text("Item flags:");
            ImGui::Checkbox("StatTrak", &config->misc.fakeItemFlagsST);
            ImGui::SameLine();
            ImGui::Checkbox("Star", &config->misc.fakeItemFlagsStar);
            ImGui::SetNextItemWidth(200.0f);
            ImGui::Combo("Team", &config->misc.fakeItemTeam, "Counter-Terrorist\0Terrorist\0");
            ImGui::SetNextItemWidth(200.0f);
            ImGui::Combo("Message Type", &config->misc.fakeItemMessageType, "Unbox\0Trade\0");
            ImGui::SetNextItemWidth(200.0f);
            ImGui::Combo("Item Type", &config->misc.fakeItemType, "AK-47\0AUG\0AWP\0Bayonet\0Bowie Knife\0Butterfly Knife\0CZ75-Auto\0Classic Knife\0Desert Eagle\0Dual Berettas\0FAMAS\0Falchion Knife\0Five-SeveN\0Flip Knife\0G3SG1\0Galil AR\0Glock-18\0Gut Knife\0Huntsman Knife\0Karambit\0M249\0M4A1-S\0M4A4\0M9 Bayonet\0MAC-10\0MAG-7\0MP5-SD\0MP7\0MP9\0Navaja Knife\0Negev\0Nomad Knife\0Nova\0P2000\0P250\0P90\0PP-Bizon\0Paracord Knife\0R8 Revolver\0SCAR-20\0SG 553\0SSG 08\0Sawed-Off\0Shadow Daggers\0Skeleton Knife\0Spectral Shiv\0Stiletto Knife\0Survival Knife\0Talon Knife\0Tec-9\0UMP-45\0USP-S\0Ursus Knife\0XM1014\0Hand Wraps\0Moto Gloves\0Specialist Gloves\0Sport Gloves\0Bloodhound Gloves\0Hydra Gloves\0Driver Gloves\0");
            ImGui::SetNextItemWidth(200.0f);
            ImGui::Combo("Item Color/Rarity", &config->misc.fakeItemRarity, "Consumer Grade (White)\0Industrial Grade (Light blue)\0Mil-Spec (Blue)\0Restricted (Purple)\0Classified (Pink)\0Covert (Red)\0Contrabanned(Orange/Gold)\0");
            ImGui::Combo("Player Color", &config->misc.fakeItemPlayerColor, "Yellow\0Green\0Blue\0Purple\0Orange\0");
            ImGui::InputText("Player Name", &config->misc.fakeItemPlayerName);
            ImGui::InputText("Skin Name", &config->misc.fakeItemName);
            if (ImGui::Button("Change Name"))
                Misc::fakeItem(true);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    ImGui::Checkbox("Fast plant", &config->misc.fastPlant);
    ImGui::Checkbox("Fast Stop", &config->misc.fastStop);
    ImGuiCustom::colorPicker("Bomb timer", config->misc.bombTimer);
    ImGui::Checkbox("Bomb Damage Indicator", &config->misc.bombDamage);
    ImGui::Checkbox("Quick reload", &config->misc.quickReload);
    ImGui::Checkbox("Prepare revolver", &config->misc.prepareRevolver);
    ImGui::SameLine();
    hotkey(config->misc.prepareRevolverKey);
    ImGui::Combo("Hit Sound", &config->misc.hitSound, "None\0Metal\0Gamesense\0Bell\0Glass\0Custom\0");
    if (config->misc.hitSound == 5) {
        ImGui::InputText("Hit Sound filename", &config->misc.customHitSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PushID(5);
    ImGui::Combo("Kill Sound", &config->misc.killSound, "None\0Metal\0Gamesense\0Bell\0Glass\0Custom\0");
    if (config->misc.killSound == 5) {
        ImGui::InputText("Kill Sound filename", &config->misc.customKillSound);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("audio file must be put in csgo/sound/ directory");
    }
    ImGui::PopID();
    ImGui::SetNextItemWidth(90.0f);
    ImGui::Combo("Choked packets", &config->misc.chokedPackets, "Off\0Normal\0Adaptive\0Random\0Switch");
    if (config->misc.chokedPackets != 0) {
        ImGui::SameLine();
        ImGui::PushID("Choked packets");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("CP");

        if (ImGui::BeginPopup("CP")) {
            ImGui::SetNextItemWidth(200.0f);
            ImGui::Checkbox("Ping Based", &config->misc.pingBasedChoked);
            ImGui::Text("Choked packets Flags:");
            ImGui::Checkbox("While Shooting", &config->misc.chokedPacketsShooting);
            ImGui::SameLine();
            ImGui::Checkbox("While Standing", &config->misc.chokedPacketsStanding);
            ImGui::Checkbox("While Moving", &config->misc.chokedPacketsMoving);
            ImGui::SameLine();
            ImGui::Checkbox("In Air", &config->misc.chokedPacketsAir);
            if (config->misc.chokedPackets != 4)
                if (config->misc.pingBasedChoked)
                    ImGui::Text("Choked packets Amount: %d", config->misc.pingBasedChokedVal);
                else
                    ImGui::SliderInt(config->misc.chokedPackets == 3 ? "Min Choked packets Amount" : "Choked packets Amount", &config->misc.chokedPacketsTicks, 1, 16, "%d", ImGuiSliderFlags_ClampOnInput);
            ImGui::EndPopup();
        }
        ImGui::PopID();
        ImGui::SameLine();
        hotkey(config->misc.chokedPacketsKey);
    }
    ImGui::Text("Quick healthshot");
    ImGui::SameLine();
    hotkey(config->misc.quickHealthshotKey);
    ImGui::Checkbox("Grenade Prediction", &config->misc.nadePredict);
    ImGui::Checkbox("Fix tablet signal", &config->misc.fixTabletSignal);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("Max angle delta", &config->misc.maxAngleDelta, 0.0f, 255.0f, "%.2f");
    ImGui::Checkbox("Fake prime", &config->misc.fakePrime);
    ImGui::Checkbox("Zeusbot", &config->misc.autoZeus);
    if (config->misc.autoZeus)
    {
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderInt("Zeus Max Wall Penetration Distance", &config->misc.autoZeusMaxPenDist, 0, 50, "%d", ImGuiSliderFlags_ClampOnInput);
    }
    ImGui::Checkbox("Player Blocker", &config->misc.playerBlocker);
    ImGui::SameLine();
    hotkey(config->misc.playerBlockerKey);
    ImGui::Checkbox("Viewmodel Position XYZ", &config->misc.customViewmodelToggle);
    if (config->misc.customViewmodelToggle) {
        ImGui::SameLine();
        ImGui::PushID("Viewmodel Position");
        if (ImGui::Button("..."))
            ImGui::OpenPopup("VP");

        if (ImGui::BeginPopup("VP")) {
            ImGui::Checkbox("Knife Position", &config->misc.customViewmodelKnifeToggle);
            if (!config->misc.customViewmodelKnifeToggle) {
                ImGui::PushItemWidth(280.0f);
                ImGui::PushID(6);
                ImGui::SliderFloat("", &config->misc.viewmodel_x, -20, 20, "Left/Right: %.2f");
                ImGui::PopID();
                ImGui::PushID(7);
                ImGui::SliderFloat("", &config->misc.viewmodel_y, -20, 20, "Close/Far: %.2f");
                ImGui::PopID();
                ImGui::PushID(8);
                ImGui::SliderFloat("", &config->misc.viewmodel_z, -20, 20, "Down/Up: %.2f");
                ImGui::PopID();
                ImGui::Checkbox("Right/Left hand Weapon", &config->misc.customViewmodelSwitchHand);
                ImGui::SameLine();
                hotkey(config->misc.customViewmodelSwitchHandBind);
            }
            else {
                ImGui::PushItemWidth(280.0f);
                ImGui::PushID(9);
                ImGui::SliderFloat("", &config->misc.viewmodel_x_knife, -20, 20, "Left/Right: %.2f");
                ImGui::PopID();
                ImGui::PushID(10);
                ImGui::SliderFloat("", &config->misc.viewmodel_y_knife, -20, 20, "Close/Far: %.2f");
                ImGui::PopID();
                ImGui::PushID(11);
                ImGui::SliderFloat("", &config->misc.viewmodel_z_knife, -20, 20, "Down/Up: %.2f");
                ImGui::PopID();
            }
            ImGui::Checkbox("HeadBob", &config->misc.view_bob);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    };
    ImGui::Checkbox("Opposite Hand Knife", &config->misc.oppositeHandKnife);
    ImGui::SameLine();
    hotkey(config->misc.oppositeHandKnifeBind);
    ImGui::Checkbox("Fakeduck", &config->misc.fakeDuck);
    ImGui::SameLine();
    hotkey(config->misc.fakeDuckKey);
    ImGui::Checkbox("Purchase List", &config->misc.purchaseList.enabled);
    ImGui::SameLine();

    ImGui::PushID("Purchase List");
    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::SetNextItemWidth(75.0f);
        ImGui::Combo("Mode", &config->misc.purchaseList.mode, "Details\0Summary\0");
        ImGui::Checkbox("Only During Freeze Time", &config->misc.purchaseList.onlyDuringFreezeTime);
        ImGui::Checkbox("Show Prices", &config->misc.purchaseList.showPrices);
        ImGui::Checkbox("No Title Bar", &config->misc.purchaseList.noTitleBar);
        ImGui::EndPopup();
    }
    ImGui::PopID();

    ImGui::Checkbox("Reportbot", &config->misc.reportbot.enabled);
    ImGui::SameLine();
    ImGui::PushID("Reportbot");

    if (ImGui::Button("..."))
        ImGui::OpenPopup("");

    if (ImGui::BeginPopup("")) {
        ImGui::PushItemWidth(80.0f);
        ImGui::Combo("Target", &config->misc.reportbot.target, "Enemies\0Allies\0All\0");
        ImGui::InputInt("Delay (s)", &config->misc.reportbot.delay);
        config->misc.reportbot.delay = (std::max)(config->misc.reportbot.delay, 1);
        ImGui::InputInt("Rounds", &config->misc.reportbot.rounds);
        config->misc.reportbot.rounds = (std::max)(config->misc.reportbot.rounds, 1);
        ImGui::PopItemWidth();
        ImGui::Checkbox("Abusive Communications", &config->misc.reportbot.textAbuse);
        ImGui::Checkbox("Griefing", &config->misc.reportbot.griefing);
        ImGui::Checkbox("Wall Hacking", &config->misc.reportbot.wallhack);
        ImGui::Checkbox("Aim Hacking", &config->misc.reportbot.aimbot);
        ImGui::Checkbox("Other Hacking", &config->misc.reportbot.other);
        if (ImGui::Button("Reset"))
            Misc::resetReportbot();
        ImGui::EndPopup();
    }
    ImGui::PopID();

    if (ImGui::Button("Unhook"))
        hooks->uninstall();

    ImGui::Columns(1);
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderConfigWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.config)
            return;
        ImGui::SetNextWindowSize({ 290.0f, 0.0f });
        ImGui::Begin("Config (OsirisBETA by PlayDay)", &window.config, windowFlags);
    }

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 170.0f);

    static bool incrementalLoad = false;
    ImGui::Checkbox("Incremental Load", &incrementalLoad);

    ImGui::PushItemWidth(160.0f);

    if (ImGui::Button("Reload configs", { 160.0f, 25.0f }))
        config->listConfigs();

    auto& configItems = config->getConfigs();
    static int currentConfig = -1;

    if (static_cast<std::size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;

    static std::string buffer;

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::string>*>(data);
        *out_text = vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 5) && currentConfig != -1)
            buffer = configItems[currentConfig];

        ImGui::PushID(0);
        if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer.c_str());
        }
        ImGui::PopID();
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        if (ImGui::Button("Create config", { 100.0f, 25.0f }))
            config->add(buffer.c_str());

        if (ImGui::Button("Reset config", { 100.0f, 25.0f }))
            ImGui::OpenPopup("Config to reset");

        if (ImGui::BeginPopup("Config to reset")) {
            static constexpr const char* names[]{ "Whole", "Aimbot", "Triggerbot", "Backtrack", "Anti aim", "Glow", "Chams", "ESP", "Visuals", "Skin changer", "Sound", "Style", "Misc" };
            for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                if (i == 1) ImGui::Separator();

                if (ImGui::Selectable(names[i])) {
                    switch (i) {
                    case 0: config->reset(); updateColors(); Misc::updateClanTag(true); SkinChanger::scheduleHudUpdate(); break;
                    case 1: config->aimbot = { }; break;
                    case 2: config->triggerbot = { }; break;
                    case 3: config->backtrack = { }; break;
                    case 4: config->antiAim = { }; break;
                    case 5: config->glow = { }; break;
                    case 6: config->chams = { }; break;
                    case 7: config->streamProofESP = { }; break;
                    case 8: config->visuals = { }; break;
                    case 9: config->skinChanger = { }; SkinChanger::scheduleHudUpdate(); break;
                    case 10: config->sound = { }; break;
                    case 11: config->style = { }; updateColors(); break;
                    case 12: config->misc = { };  Misc::updateClanTag(true); break;
                    }
                }
            }
            ImGui::EndPopup();
        }
        if (currentConfig != -1) {
            if (ImGui::Button("Load selected", { 100.0f, 25.0f }))
                ImGui::OpenPopup("Config to load");
            if (ImGui::BeginPopup("Config to load")) {
                if (ImGui::Selectable("Confirm")) {
                    config->load(currentConfig, incrementalLoad);
                    updateColors();
                    SkinChanger::scheduleHudUpdate();
                    Misc::updateClanTag(true);
                }
                if (ImGui::Selectable("Cancel")) {/*nothing to do*/ }
                ImGui::EndPopup();
            }
            if (ImGui::Button("Save selected", { 100.0f, 25.0f }))
                ImGui::OpenPopup("Config to save");
            if (ImGui::BeginPopup("Config to save")) {
                if (ImGui::Selectable("Confirm"))
                    config->save(currentConfig);
                if (ImGui::Selectable("Cancel")) {/*nothing to do*/ }
                ImGui::EndPopup();
            }
            if (ImGui::Button("Delete selected", { 100.0f, 25.0f }))
                ImGui::OpenPopup("Config to delete");
            if (ImGui::BeginPopup("Config to delete")) {
                if (ImGui::Selectable("Confirm")) {
                    config->remove(currentConfig);
                    currentConfig = -1;
                    buffer.clear();
                }
                if (ImGui::Selectable("Cancel")) {/*nothing to do*/ }
                ImGui::EndPopup();
            }
        }
        ImGui::Columns(1);
        if (!contentOnly)
            ImGui::End();
}

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void GUI::renderBETAWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.BETA)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Info (OsirisBETA by PlayDay)", &window.BETA, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    }
    ImGui::Text("Osiris by danielkrupinski;");
    ImGui::Text("OsirisBETA by PlayDay (playday3008(GitHub)), (FlutterShy);");
    ImGui::Text("Discord by w1ldac3 (https://discord.gg/xWEtQAn);");
    ImGui::Text(" ");
    ImGui::Text("Functions by:");
    ImGui::Text("Jump Bug by twangsriprasoet and ME;");
    ImGui::Text("Draw all backtrack Chams by JDeu;");
    ImGui::Text("Save/Load/Delete Config Confirmation by ME;");
    ImGui::Text("Friendly fire/team damage counter by xAkiraMiura;");
    ImGui::Text("Triggerbot max inaccuracy, Human Bhop, and Aimbot FOV Circles by Vonr;");
    ImGui::Text("Jump Check by zajkos;");
    ImGui::Text("Show Velocity by NekoRem, movable function by me;");
    ImGui::Text("Custom Skybox by cailloubr;");
    ImGui::Text("Bomb Damage Indicator by ZerGo0;");
    ImGui::Text("Player Blocker (have BUG (maybe)) by NekoRem;");
    ImGui::Text("Dead thirdperson by JDeu;");
    ImGui::Text("PingBased Backtrack by RyDeem;");
    ImGui::Text("Changable gravity to invert gravity function by RyDeem;");
    ImGui::Text("Custom Physics Timescale by RyDeem;");
    ImGui::Text("mat_fullbright setting in visuals window by RyDeem;");
    ImGui::Text("Hitmarker Damage Indicator by ZerGo0, improved by RyDeem;");
    ImGui::Text("Custom View model Offset by ME;");
    ImGui::Text("AntiDetection by 0xE232FE;");
    ImGui::Text("No zoom on scope by MinecraftGoodGame;");
    ImGui::Text("Provide hotkey for Auto Strafe by simonsmh;");
    ImGui::Text("AutoStrafe new style by notgoodusename;");
    ImGui::Text("mat_disable_bloom by RyDeem;");
    ImGui::Text("PingBased Choked packets by ME;");
    ImGui::Text("Bypass sv_pure and fake latency and better backtrack by notgoodusename;");
    ImGui::Text("Fast Stop by SunzBody;");
    ImGui::Text("Door Spam by ME");
    ImGui::Text("Chat Spam by ClaudiuHKS;");
    ImGui::Text("sv_pure Bypass (OLD) by Cyk-Fad;");
    ImGui::Text("BuyBot by ME;");
    ImGui::Text("Nightmode by Cyk-Fad;");
    //ImGui::Text("Reworked anti-aim and fakelag, made thirdperson show cmd->viewangles by DoomFishWasTaken;");
    ImGui::Text("Fake items (fake unbox and trade messages) by DoomFishWasTaken;");
    ImGui::Text("Standalone RCS by steel4me and r3muxd;");
    ImGui::Text("Zeusbot by DoomFishWasTaken and ME;");
    //ImGui::Text("Multipoints by ClaudiuHKS;");
    ImGui::Text("Rainbow Bar by ME");
    ImGui::Text("Bullet Tracer by DoomFishWasTaken");
    ImGui::Text("Fake Duck by DoomFishWasTaken");
    ImGui::Text("Osiris-Injector by danielkrupinski and ME;");
    ImGui::Text(" ");

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::CollapsingHeader("GUI Configuration by ME"))
    {
        const float MIN_SCALE = 0.3f;
        const float MAX_SCALE = 2.0f;
        ImGui::DragFloat("global scale", &io.FontGlobalScale, 0.005f, MIN_SCALE, MAX_SCALE, "%.2f", ImGuiSliderFlags_ClampOnInput); // Scale everything
        if (ImGui::TreeNode("Sizes##2"))
        {
            ImGui::Text("Main");
            ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
            ImGui::Text("Borders");
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::Text("Rounding");
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("LogSliderDeadzone", &style.LogSliderDeadzone, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("TabRounding", &style.TabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::Text("Alignment");
            ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            int window_menu_button_position = style.WindowMenuButtonPosition + 1;
            if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
                style.WindowMenuButtonPosition = window_menu_button_position - 1;
            ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
            ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
            ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
            ImGui::Text("Safe Area Padding");
            ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
            ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
        }
        if (ImGui::TreeNode("Rendering##2"))
        {
            ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
            ImGui::SameLine(); HelpMarker("When disabling anti-aliasing lines, you'll probably want to disable borders in your style as well.");
            ImGui::Checkbox("Anti-aliased lines use texture", &style.AntiAliasedLinesUseTex);
            ImGui::SameLine(); HelpMarker("Faster lines using texture data. Require back-end to render with bilinear filtering (not point/nearest filtering).");
            ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, 10.0f, "%.2f");
            if (style.CurveTessellationTol < 0.10f) style.CurveTessellationTol = 0.10f;
        }
    }

    ImGui::Text(" ");
    ImGui::Text("Build: " __DATE__ ", " __TIME__ "");
    if (ImGui::Button("MLP"))
        Misc::MLP();
    if (!contentOnly)
        ImGui::End();
}

void GUI::renderGuiStyle2() noexcept
{
    ImGui::Begin("Osiris by danielkrupinski, Beta by PlayDay", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
        if (ImGui::BeginTabItem("Aimbot")) {
            renderAimbotWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Anti aim")) {
            renderAntiAimWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Triggerbot")) {
            renderTriggerbotWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Backtrack")) {
            renderBacktrackWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Glow")) {
            renderGlowWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Chams")) {
            renderChamsWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ESP")) {
            renderStreamProofESPWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Visuals")) {
            renderVisualsWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Skin changer")) {
            renderSkinChangerWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Sound")) {
            renderSoundWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Style")) {
            renderStyleWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Misc")) {
            renderMiscWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Config")) {
            renderConfigWindow(true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Info")) {
            renderBETAWindow(true);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
