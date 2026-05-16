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
    void SetAnimScale(float s) { m_AnimScale = s; }
    float GetAnimScale() const { return m_AnimScale; }

private:
    // ── Playback state ────────────────────────────────────────────────────
    const BattleAnimDef*      m_Def        = nullptr;
    int                       m_Frame      = 0;     // current frame index
    float                     m_Timer      = 0.f;   // ms accumulated this frame
    float                     m_FrameMs    = 50.f;  // ms per frame (1000/fps)
    bool                      m_Playing    = false;
    std::function<void()>     m_OnFinished;
    float m_AnimScale = 0.35f; 
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
        // Base aspect ratio scales to map 640x480 space to 1280x720 space
        const float scaleX = (1280.f / 640.f) * m_AnimScale;
        const float scaleY = ( 720.f / 480.f) * m_AnimScale;

        // ====================================================================
        // 1. POSITION "BOTH" (e.g., Water Gun traveling between sprites)
        // ====================================================================
        if (m_Def->position == AnimPosition::BOTH) {
            // RPG Maker traditional layouts place the Player at X=160, Y=220 
            // and the Enemy at X=384, Y=100 on a 640x480 canvas.
            // We find out who is on the left and right in your engine to map them correctly.
            glm::vec2 screenLeft  = (m_UserPos.x < m_TargetPos.x) ? m_UserPos : m_TargetPos;
            glm::vec2 screenRight = (m_UserPos.x < m_TargetPos.x) ? m_TargetPos : m_UserPos;

            // Map X coordinate perfectly between your two sprites
            float ratioX = (rpgX - 160.f) / (384.f - 160.f); 
            float finalX = screenLeft.x + ratioX * (screenRight.x - screenLeft.x);

            // Map Y coordinate perfectly between your two sprites 
            // (RPG Maker Y=220 is lower screen, Y=100 is upper screen)
            float ratioY = (rpgY - 220.f) / (100.f - 220.f);
            float finalY = screenLeft.y + ratioY * (screenRight.y - screenLeft.y);

            return { finalX, finalY - 100 };
        }

        // ====================================================================
        // 2. STANDARD POSITIONING (Target / User / Screen, e.g., Thundershock)
        // ====================================================================
        float deltaX = (rpgX - 320.f) * scaleX;
        
        // Since your anchor is already lifted by +112px to find the body center,
        // we multiply the RPG offset calculation by 0.4f to damp the extra climb 
        // and keep the lightning bolts centered on the target.
        float deltaY = -(rpgY - 240.f) * scaleY * 0.4f; 

        return { anchor.x + deltaX -50, anchor.y + deltaY -50 };
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