#include "AnimationPlayer.hpp"
#include "Util/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <filesystem>

// ============================================================================
// AnimationPlayer.cpp – Full diagnostic version
// ============================================================================

AnimationPlayer::AnimationPlayer(std::shared_ptr<Util::Renderer> renderer,
                                 Util::AssetStore<std::shared_ptr<Util::Image>>& sheetCache,
                                 int fps)
    : m_FrameMs(1000.f / static_cast<float>(fps))
    , m_Renderer(renderer)
    , m_SheetCache(sheetCache)
{
    for (int i = 0; i < MAX_CELS; ++i) {
        m_CelObjects[i] = std::make_shared<Util::GameObject>();
        m_CelImages[i] = nullptr;
        m_CelObjects[i]->SetVisible(false);
        m_CelObjects[i]->SetZIndex(50 + i);
        if (m_Renderer) {
            m_Renderer->AddChild(m_CelObjects[i]);
            LOG_INFO("[Anim] Cel {} added to renderer (z={})", i, 50 + i);
        }
    }
}

AnimationPlayer::~AnimationPlayer() {
    Stop();
    for (int i = 0; i < MAX_CELS; ++i) {
        if (m_Renderer && m_CelObjects[i])
            m_Renderer->RemoveChild(m_CelObjects[i]);
    }
}

void AnimationPlayer::Play(const BattleAnimDef& def,
                           glm::vec2 userPos,
                           glm::vec2 targetPos,
                           std::function<void()> onFinished) {
    LOG_INFO("[AnimPlayer] Play called for '{}'", def.name);

    if (def.graphic.empty()) {
        LOG_ERROR("[AnimPlayer] No graphic for '{}'", def.name);
        if (onFinished) onFinished();   // ← CRITICAL
        return;
    }
    std::string sheetPath = std::string(RESOURCE_DIR) + "/Battle_anim/" + def.graphic;
    if (!std::ifstream(sheetPath).good()) {
        LOG_ERROR("[AnimPlayer] Missing spritesheet: '{}'", sheetPath);
        if (onFinished) onFinished();   // ← CRITICAL
        return;
    }
    if (def.frames.empty()) {
        LOG_ERROR("[AnimPlayer] No frames for '{}'", def.name);
        if (onFinished) onFinished();   // ← CRITICAL
        return;
    }

    // Log animation metadata
    LOG_INFO("[AnimPlayer] Animation '{}' frames: {}, position: {}, userPos=({:.1f},{:.1f}), targetPos=({:.1f},{:.1f})",
             def.name, def.frames.size(), static_cast<int>(def.position),
             userPos.x, userPos.y, targetPos.x, targetPos.y);

    // Remove the test Pikachu object – it only clutters the screen
    // (The test object block has been removed – it was not part of the animation system)

    m_Def        = &def;
    m_UserPos    = userPos;
    m_TargetPos  = targetPos;
    m_OnFinished = onFinished;
    m_Frame      = 0;
    m_Timer      = 0.f;
    m_Playing    = true;

    ApplyFrame(0);
    FireTimings(0);
}

void AnimationPlayer::Stop() {
    m_Playing = false;
    HideAllCels();
}

void AnimationPlayer::Update(float deltaMs) {
    if (!m_Playing || !m_Def) return;

    m_Timer += deltaMs;
    if (m_Timer < m_FrameMs) return;

    m_Timer -= m_FrameMs;
    m_Frame++;

    if (m_Frame >= static_cast<int>(m_Def->frames.size())) {
        LOG_DEBUG("[AnimPlayer] Animation '{}' finished (frame {} of {})",
                  m_Def->name, m_Frame, m_Def->frames.size());
        HideAllCels();
        m_Playing = false;
        if (m_OnFinished) m_OnFinished();
        return;
    }

    ApplyFrame(m_Frame);
    FireTimings(m_Frame);
}

glm::vec2 AnimationPlayer::ResolveAnchor() const {
    glm::vec2 anchor;
    switch (m_Def->position) {
        case AnimPosition::SCREEN: anchor = {0.f, 0.f}; break;
        case AnimPosition::USER:   anchor = m_UserPos; break;
        case AnimPosition::BOTH:   anchor = {0.f, 0.f}; break;
        //case AnimPosition::BOTH:   anchor = (m_UserPos + m_TargetPos) * 0.5f; break;
        case AnimPosition::TARGET: anchor = m_TargetPos; break;
        default:                   anchor = m_TargetPos;
    }
    LOG_DEBUG("[Anim] Anchor resolved to ({:.1f},{:.1f}) for position mode {}",
              anchor.x, anchor.y, static_cast<int>(m_Def->position));
    return anchor;
}

void AnimationPlayer::ApplyFrame(int frameIndex) {
    if (!m_Def || frameIndex >= (int)m_Def->frames.size()) {
        HideAllCels();
        return;
    }

    const AnimFrame& frame = m_Def->frames[frameIndex];
    glm::vec2 anchor = ResolveAnchor();

    for (int i = (int)frame.size(); i < MAX_CELS; ++i)
        m_CelObjects[i]->SetVisible(false);

    std::string sheetPath = std::string(RESOURCE_DIR) + "/Battle_anim/" + m_Def->graphic;

    for (int ci = 0; ci < (int)frame.size() && ci < MAX_CELS; ++ci) {
        const AnimCel& cel = frame[ci];
        auto& obj = m_CelObjects[ci];
        auto& img = m_CelImages[ci];

        // ── Image ─────────────────────────────────────────────────────
        if (!img) {
            img = std::make_shared<Util::Image>(sheetPath);
            obj->SetDrawable(img);
        } else {
            img->SetImage(sheetPath);
        }

        // ── Position ──────────────────────────────────────────────────
        obj->m_Transform.translation = RpgToScreen(cel.x, cel.y, anchor);

        // ── Scale ─────────────────────────────────────────────────────
        // zoom_y=0 in RPG Maker means "same as zoom_x", not actually zero
        float zx = (cel.zoom_x > 0 ? cel.zoom_x : 100.f) / 100.f;
        float zy = (cel.zoom_y > 0 ? cel.zoom_y : zx * 100.f) / 100.f;

        // m_AnimScale shrinks from raw 192px to a reasonable battle size
        // Tune this value — start at 0.5f, adjust until it looks right
        float sx = zx * m_AnimScale * (cel.mirror ? -1.f : 1.f);
        float sy = zy * m_AnimScale;
        obj->m_Transform.scale    = {sx, sy};
        obj->m_Transform.rotation =  cel.rotation;

        // ── Spritesheet slice ─────────────────────────────────────────
        int col = cel.pattern % BattleAnimDef::SHEET_COLS;
        int row = cel.pattern / BattleAnimDef::SHEET_COLS;
        img->SetSrcRect(col * BattleAnimDef::CEL_W, row * BattleAnimDef::CEL_H,
                        BattleAnimDef::CEL_W,        BattleAnimDef::CEL_H);

        // ── Visibility ────────────────────────────────────────────────
        obj->SetVisible(cel.opacity > 0);
    }

    m_ActiveCels = (int)frame.size();
}

void AnimationPlayer::HideAllCels() {
    for (int i = 0; i < MAX_CELS; ++i) {
        if (m_CelObjects[i]) m_CelObjects[i]->SetVisible(false);
    }
    m_ActiveCels = 0;
    LOG_DEBUG("[Anim] All cels hidden");
}

void AnimationPlayer::FireTimings(int frameIndex) {
    if (!m_Def) return;

    for (const auto& timing : m_Def->timings) {
        if (timing.frame != frameIndex) continue;

        if (timing.type == TimingType::SE) {
            if (!timing.sound.empty()) {
                LOG_INFO("[Anim] FRAME {}: Play SE '{}' (vol={}, pitch={})",
                         frameIndex, timing.sound, timing.volume, timing.pitch);
            }
        }
        else if (timing.type == TimingType::SCREEN_FLASH) {
            LOG_INFO("[Anim] FRAME {}: Screen flash dur={} rgba=({},{},{},{})",
                     frameIndex, timing.flash_duration,
                     timing.flash_r, timing.flash_g, timing.flash_b, timing.flash_a);
        }
    }
}

std::shared_ptr<Util::Image> AnimationPlayer::GetOrLoadSheet(const std::string& path) {
    try {
        return m_SheetCache.Get(path);
    } catch (const std::exception& e) {
        LOG_WARN("[Anim] Could not load '{}': {}", path, e.what());
        return nullptr;
    }
}