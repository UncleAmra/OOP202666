#include "BattleAnimator.hpp"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "string"
#include <iostream>


BattleAnimator::BattleAnimator(std::shared_ptr<Util::GameObject> playerSprite,
                               std::shared_ptr<Util::GameObject> enemySprite,
                               std::shared_ptr<Util::Renderer> renderer)
    : m_PlayerSprite(playerSprite)
    , m_EnemySprite(enemySprite)
    , m_SheetCache([](const std::string& path) { return std::make_shared<Util::Image>(path); }) // <-- ADD THIS
{
    // OLD effect sprite
    m_EffectSprite = std::make_shared<Util::GameObject>();
    m_EffectSprite->SetZIndex(100);
    m_EffectSprite->SetVisible(false);
    if (renderer) renderer->AddChild(m_EffectSprite);

    // NEW — the actual animation player
    m_AnimPlayer = std::make_shared<AnimationPlayer>(renderer, m_SheetCache, 20);
}
BattlerVisualState& BattleAnimator::GetState(BattleSide side) {
    return (side == BattleSide::PLAYER) ? m_PlayerState : m_EnemyState;
}

bool BattleAnimator::IsBusy() const {
    return m_DrainingPlayerHP || 
           m_DrainingEnemyHP || 
           m_PlayerShakeTimer > 0.0f || 
           m_EnemyShakeTimer > 0.0f ||
           m_PlayerLungeTimer > 0.0f ||
           m_EnemyLungeTimer > 0.0f ||
           (m_AnimPlayer && m_AnimPlayer->IsPlaying()) ||
           m_PlayerFainting ||
           m_EnemyFainting;
}

// ==========================================
// COMMAND TRIGGERS
// ==========================================
void BattleAnimator::ShakeSprite(BattleSide side, float duration, float intensity) {
    if (side == BattleSide::PLAYER) {
        m_PlayerShakeTimer = duration;
        m_PlayerShakeIntensity = intensity;
    } else {
        m_EnemyShakeTimer = duration;
        m_EnemyShakeIntensity = intensity;
    }
}

void BattleAnimator::LungeSprite(BattleSide side, float duration) {
    if (side == BattleSide::PLAYER) m_PlayerLungeTimer = duration;
    else m_EnemyLungeTimer = duration;
}

void BattleAnimator::AnimateHPDrain(BattleSide side, float startPercent, float targetPercent, float speed) {
    if (side == BattleSide::PLAYER) {
        m_PlayerHPPercent = startPercent;
        m_PlayerHPTarget = targetPercent;
        m_PlayerHPDrainSpeed = speed;
        m_DrainingPlayerHP = true;
    } else {
        m_EnemyHPPercent = startPercent;
        m_EnemyHPTarget = targetPercent;
        m_EnemyHPDrainSpeed = speed;
        m_DrainingEnemyHP = true;
    }
}

void BattleAnimator::PlayFaint(BattleSide side) {
    if (side == BattleSide::PLAYER) m_PlayerFainting = true;
    else m_EnemyFainting = true;
}

void BattleAnimator::PlayAttackEffect(const BattleAnimDef& def, 
                                      BattleSide target, 
                                      std::function<void()> onFinished) 
{
    if (!m_AnimPlayer) return;

    glm::vec2 playerPos = { PLAYER_BASE_X, PLAYER_BASE_Y };
    glm::vec2 enemyPos  = { ENEMY_BASE_X,  ENEMY_BASE_Y  };

    glm::vec2 userPos   = (target == BattleSide::ENEMY) ? playerPos : enemyPos;
    glm::vec2 targetPos = (target == BattleSide::ENEMY) ? enemyPos  : playerPos;

    m_IsPlayingEffect = true;

    // We pass a lambda that cleans up local state AND notifies the UI.
    // The AnimPlayer will store and call this once its frames run out.
    m_AnimPlayer->Play(def, userPos, targetPos, [this, onFinished]() {
        LOG_INFO("[BattleAnimator] AnimPlayer signal received: Cleaning up.");
        this->m_IsPlayingEffect = false; 
        if (onFinished) {
            onFinished(); // This sets m_IsMoveAnimating = false in BattleUI
        }
    });
}

void BattleAnimator::Update(float dt) {
    // 1. Tick the Move Animation Player (Cels/Sprites)
    if (m_AnimPlayer) {
        m_AnimPlayer->Update(dt);
    }

    // 2. Tick Shake/Lunge timers
    if (m_PlayerShakeTimer > 0) m_PlayerShakeTimer -= dt;
    if (m_EnemyShakeTimer > 0)  m_EnemyShakeTimer -= dt;
    if (m_PlayerLungeTimer > 0) m_PlayerLungeTimer -= dt;
    if (m_EnemyLungeTimer > 0)  m_EnemyLungeTimer -= dt;

    // 3. Handle HP Bar Draining
    auto updateHP = [&](bool& draining, float& current, float target, float speed) {
        if (!draining) return;
        float step = speed * (dt / 10.0f); // Adjust speed to ms
        if (std::abs(current - target) <= step) {
            current = target;
            draining = false;
        } else {
            current += (current < target) ? step : -step;
        }
    };
    updateHP(m_DrainingPlayerHP, m_PlayerHPPercent, m_PlayerHPTarget, m_PlayerHPDrainSpeed);
    updateHP(m_DrainingEnemyHP, m_EnemyHPPercent, m_EnemyHPTarget, m_EnemyHPDrainSpeed);

    // 4. Handle Fainting Visual (Sliding down)
    if (m_PlayerFainting) {
        m_PlayerState.offsetY -= 600.0f * (dt / 1000.0f); 
        if (m_PlayerState.offsetY < -300.0f) {
            m_PlayerFainting = false;
            m_PlayerSprite->SetVisible(false);
        }
    }
    if (m_EnemyFainting) {
        m_EnemyState.offsetY -= 600.0f * (dt / 1000.0f);
        if (m_EnemyState.offsetY < -300.0f) {
            m_EnemyFainting = false;
            m_EnemySprite->SetVisible(false);
        }
    }

    // 5. APPLY ALL OFFSETS TO THE ACTUAL SPRITES
    // This is the part that actually moves the images on screen!
    if (m_PlayerSprite) {
        m_PlayerSprite->m_Transform.translation = {
            PLAYER_BASE_X + m_PlayerState.offsetX,
            PLAYER_BASE_Y + m_PlayerState.offsetY
        };
    }
    if (m_EnemySprite) {
        m_EnemySprite->m_Transform.translation = {
            ENEMY_BASE_X + m_EnemyState.offsetX,
            ENEMY_BASE_Y + m_EnemyState.offsetY
        };
    }
}

void BattleAnimator::ResetState(BattleSide side) {
    BattlerVisualState& state = GetState(side);
    
    // 1. Reset all visual offsets back to 0
    state.offsetX = 0.0f;
    state.offsetY = 0.0f;
    state.opacity = 1.0f;
    state.isVisible = true;

    // 2. Kill any lingering animations for this side
    if (side == BattleSide::PLAYER) {
        m_PlayerFainting = false;
        m_PlayerLungeTimer = 0.0f;
        m_PlayerShakeTimer = 0.0f;
        if (m_PlayerSprite) m_PlayerSprite->SetVisible(true);
    } else {
        m_EnemyFainting = false;
        m_EnemyLungeTimer = 0.0f;
        m_EnemyShakeTimer = 0.0f;
        if (m_EnemySprite) m_EnemySprite->SetVisible(true);
    }
}