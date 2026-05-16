#pragma once
#include "BattleUI.hpp"
#include "AnimationPlayer.hpp"
#include "BattleAnimation.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include <string>
#include <functional>
#include <memory>

// Identifies which side is being targeted by an animation
enum class BattleSide {
    PLAYER,
    ENEMY
};

// Tracks the exact visual offsets and states for a single sprite
struct BattlerVisualState {
    float offsetX = 0.0f;       // For shaking / lunging
    float offsetY = 0.0f;       // For fainting (sinking into the ground)
    float opacity = 1.0f;       // For fading out
    bool isVisible = true;
};

// Tracks an active spritesheet animation (like a scratch or fireball)
struct ActiveEffect {
    bool isPlaying = false;
    std::string texturePath;
    int currentFrame = 0;
    int totalFrames = 0;
    float frameTimer = 0.0f;
    float timePerFrame = 0.05f; // E.g., 20 FPS
    BattleSide targetSide;
};

class BattleAnimator {
public:
    BattleAnimator(std::shared_ptr<Util::GameObject> playerSprite,
                   std::shared_ptr<Util::GameObject> enemySprite,
                   std::shared_ptr<Util::Renderer> renderer);

    ~BattleAnimator() = default;

    // --- MAIN LOOP ---
    // Call this every frame inside your BattleUI::Update()
    void Update(float deltaTime);

    // --- COMMANDS ---
    // 1. Battler Sprite Effects
    void ShakeSprite(BattleSide side, float duration = 30.0f, float intensity = 8.0f);
    void LungeSprite(BattleSide side, float duration = 15.0f);
    void PlayFaint(BattleSide side);

    // 2. UI Effects
    void AnimateHPDrain(BattleSide side, float startPercent, float targetPercent, float speed = 0.01f);

    // 3. Attack Effects
    void PlayAttackEffect(const BattleAnimDef& def, 
                      BattleSide target, 
                      glm::vec2 playerPos, 
                      glm::vec2 enemyPos, 
                      std::function<void()> onFinished);

    // --- GATEKEEPERS & GETTERS ---
    // Returns true if ANY animation is currently happening (UI should pause text)
    bool IsBusy() const;

    // Getters for the UI to read the current smoothed HP values
    float GetPlayerHPPercent() const { return m_PlayerHPPercent; }
    float GetEnemyHPPercent() const { return m_EnemyHPPercent; }
    // Resets a specific side back to idle 
    void ResetState(BattleSide side);
    bool IsPlayingEffect() const { return m_IsPlayingEffect; }

private:
    // --- ENGINE COMPONENTS ---
    std::shared_ptr<Util::GameObject> m_PlayerSprite;
    std::shared_ptr<Util::GameObject> m_EnemySprite;
    std::shared_ptr<Util::GameObject> m_EffectSprite; 
    std::shared_ptr<AnimationPlayer> m_AnimPlayer;
    std::function<void()> m_OnMoveComplete = nullptr;
    bool m_IsPlayingEffect = false;

    // --- HARDCODED ANCHORS (From your BattleUI) ---
    const float PLAYER_BASE_X = -270.0f;
    const float PLAYER_BASE_Y = -50.0f;
    const float ENEMY_BASE_X = 400.0f;
    const float ENEMY_BASE_Y = 150.0f;

    // --- STATE DATA ---
    BattlerVisualState m_PlayerState;
    BattlerVisualState m_EnemyState;
    ActiveEffect m_ActiveEffect;

    float m_AnimTime = 0.0f; // Used for idle bobbing

    // HP Bar tracking
    float m_PlayerHPPercent = 1.0f;
    float m_EnemyHPPercent = 1.0f;
    
    bool m_DrainingPlayerHP = false;
    float m_PlayerHPTarget = 1.0f;
    float m_PlayerHPDrainSpeed = 0.01f;

    bool m_DrainingEnemyHP = false;
    float m_EnemyHPTarget = 1.0f;
    float m_EnemyHPDrainSpeed = 0.01f;

    // Internal Timers (Using frames/deltaTime interchangeably based on your setup)
    float m_PlayerShakeTimer = 0.0f;
    float m_PlayerShakeIntensity = 0.0f;
    float m_EnemyShakeTimer = 0.0f;
    float m_EnemyShakeIntensity = 0.0f;

    float m_PlayerLungeTimer = 0.0f;
    float m_EnemyLungeTimer = 0.0f;

    bool m_PlayerFainting = false;
    bool m_EnemyFainting = false;

    // Helper to fetch the right state struct
    BattlerVisualState& GetState(BattleSide side);
    Util::AssetStore<std::shared_ptr<Util::Image>> m_SheetCache{
        [](const std::string& path) {
            return std::make_shared<Util::Image>(path);
        }
    };
};