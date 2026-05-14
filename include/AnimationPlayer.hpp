#pragma once
#include "BattleAnimation.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include "Util/AssetStore.hpp"
// ============================================================================
// AnimationPlayer.hpp
//
// Plays one BattleAnimDef at runtime.
// Each move use creates a NEW AnimationPlayer instance so multiple animations
// can run simultaneously without sharing state.
//
// Usage:
//   auto player = std::make_shared<AnimationPlayer>(renderer, sheetCache);
//   player->Play(def, userPos, targetPos, [](){ /* onFinished */ });
//   // Each frame: player->Update(deltaMs);
//   // In draw:    player->Draw();         ← if your engine needs explicit draw
// ============================================================================

// Cache so we don't reload the same PNG twice
using SheetCache = std::unordered_map<std::string, std::shared_ptr<Util::Image>>;

class AnimationPlayer {
public:
    // fps: playback speed. Pokemon Essentials default = 20 fps (50ms/frame)
    explicit AnimationPlayer(std::shared_ptr<Util::Renderer> renderer,
                             Util::AssetStore<std::shared_ptr<Util::Image>>& sheetCache,
                             int targetFps = 60);
    ~AnimationPlayer();

    // Start playing an animation.
    //   def        — the animation definition (from AnimationLibrary)
    //   userPos    — world position of the attacking Pokemon sprite
    //   targetPos  — world position of the defending Pokemon sprite
    //   onFinished — called once when the last frame completes
    void Play(const BattleAnimDef& def,
              glm::vec2 userPos,
              glm::vec2 targetPos,
              std::function<void()> onFinished = nullptr);

    void Update(float deltaMs);
    bool IsPlaying() const { return m_Playing; }
    void Stop();

private:
    // ── Playback state ────────────────────────────────────────────────────
    const BattleAnimDef*      m_Def        = nullptr;
    int                       m_Frame      = 0;     // current frame index
    float                     m_Timer      = 0.f;   // ms accumulated this frame
    float                     m_FrameMs    = 50.f;  // ms per frame (1000/fps)
    bool                      m_Playing    = false;
    std::function<void()>     m_OnFinished;

    // ── Anchor positions ──────────────────────────────────────────────────
    glm::vec2                 m_UserPos    = {0.f, 0.f};
    glm::vec2                 m_TargetPos  = {0.f, 0.f};

    // ── Rendering ─────────────────────────────────────────────────────────
    // We keep a pool of GameObjects — one per concurrent cel.
    // Pokemon Essentials animations have at most ~8 cels per frame.
    static constexpr int MAX_CELS = 16;
    std::shared_ptr<Util::GameObject> m_CelObjects[MAX_CELS];
    std::shared_ptr<Util::Image>      m_CelImages[MAX_CELS];
    int m_ActiveCels = 0;

    std::shared_ptr<Util::Renderer> m_Renderer;
    Util::AssetStore<std::shared_ptr<Util::Image>>& m_SheetCache;


    // ── Helpers ───────────────────────────────────────────────────────────
    glm::vec2 ResolveAnchor() const;
    void      ApplyFrame(int frameIndex);
    void      HideAllCels();
    void      FireTimings(int frameIndex);

    std::shared_ptr<Util::Image> GetOrLoadSheet(const std::string& sheetName);

    // RPG Maker → screen coordinate conversion.
    // RPG Maker uses +Y = down, centered on the anchor.
    // PTSD uses +Y = up (standard OpenGL). We flip Y here.
    // AnimationPlayer.hpp — RpgToScreen must account for RPG Maker origin:
    // In AnimationPlayer.hpp or .cpp
    glm::vec2 RpgToScreen(float rpgX, float rpgY, glm::vec2 anchor) const {
    // This multiplier maps the 640x480 coordinate space to your 1280x720 space
    const float RES_SCALE = 2.0f; 

    // 1. Find the offset from the RPG Maker center (320, 240)
    float deltaX = rpgX - 320.0f;
    float deltaY = rpgY - 240.0f;

    // 2. Scale that offset so the "movement" covers the correct distance
    float scaledX = deltaX * RES_SCALE;
    float scaledY = deltaY * RES_SCALE;

    // 3. Apply the flipped Y for OpenGL and add to your world anchor
    // We use '-' for scaledY because in RPG Maker, +Y is DOWN.
    return { anchor.x + scaledX, anchor.y - scaledY };
}
    // Given a pattern index and sheet, compute the source rect (top-left corner
    // in the PNG) as a normalised UV or just the pixel offset.
    // Pattern layout: row-major, SHEET_COLS=5 columns, each cell 192x192.
    struct CelRect { int x, y, w, h; };
    static CelRect PatternRect(int pattern) {
        int col = pattern % BattleAnimDef::SHEET_COLS;
        int row = pattern / BattleAnimDef::SHEET_COLS;
        return { col * BattleAnimDef::CEL_W,
                 row * BattleAnimDef::CEL_H,
                 BattleAnimDef::CEL_W,
                 BattleAnimDef::CEL_H };
    }
};