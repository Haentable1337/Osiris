#pragma once

enum class FrameStage;
class GameEvent;

namespace Visuals
{
    void playerModel(FrameStage stage) noexcept;
    void colorWorld() noexcept;
    void modifySmoke(FrameStage stage) noexcept;
    void thirdperson() noexcept;
    void removeVisualRecoil(FrameStage stage) noexcept;
    void removeBlur(FrameStage stage) noexcept;
    void updateBrightness() noexcept;
    void removeGrass(FrameStage stage) noexcept;
    void remove3dSky() noexcept;
    void removeShadows() noexcept;
    void applyZoom(FrameStage stage) noexcept;
    void applyScreenEffects() noexcept;
    void hitEffect(GameEvent* event = nullptr) noexcept;
    void hitMarker(GameEvent* event = nullptr) noexcept;
    void disablePostProcessing(FrameStage stage) noexcept;
    void reduceFlashEffect() noexcept;
    bool removeHands(const char* modelName) noexcept;
    bool removeSleeves(const char* modelName) noexcept;
    bool removeWeapons(const char* modelName) noexcept;
    void skybox(FrameStage stage) noexcept;

    void physicsTimescale() noexcept;
    void fullBright() noexcept;
    void hitMarkerSetDamageIndicator(GameEvent* = nullptr) noexcept;
    void hitMarkerDamageIndicator() noexcept;
    void noZoom() noexcept;
    void noBloom() noexcept;
    void NightMode() noexcept;
    void rainbowBar(ImDrawList* drawList) noexcept;
    void bulletBeams(GameEvent* event = nullptr) noexcept;
}
