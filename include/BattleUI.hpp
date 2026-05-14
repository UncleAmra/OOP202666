#pragma once
#include "pch.hpp"
#include "Pokemon.hpp"
#include "Player.hpp"
#include "BattleAnimator.hpp"
#include "BattleManager.hpp"
#include "InventoryMenu.hpp"
#include "PokemonMenu.hpp"
#include "Util/GameObject.hpp"
#include "PokeballAnimator.hpp"
#include "Util/Renderer.hpp"
#include "ResourceManager.hpp"
#include "Util/Text.hpp"
#include "Util/AssetStore.hpp" // Make sure this is included at the top of your header!

// --- NEW INCLUDES ---
#include "AnimationPlayer.hpp"
#include "BattleAnimation.hpp"
#include "MoveDatabase.hpp"

#include <memory>
#include <cmath>   // For std::sin
#include <cstdlib> // For rand()
#include <vector>
#include <queue>      
#include <sstream>


class BattleAnimator;

class BattleUI {
public:
    BattleUI(std::shared_ptr<Util::Renderer> renderer);
    void SetInventoryMenu(std::shared_ptr<InventoryMenu> invMenu) { 
        m_InventoryMenu = invMenu; 
    }
    void SetPlayer(std::shared_ptr<Player> player) { 
        m_Player = player; 
    }
    
    // --- NEW SETTER ---
    void SetAnimationLibrary(std::shared_ptr<AnimationLibrary> animLib) {
        m_AnimLibrary = animLib;
    }
   
    void Show(std::vector<std::shared_ptr<Pokemon>> playerParty, std::shared_ptr<Pokemon> wildPokemon);
    void Hide();
    bool Update();
    bool IsBattleOver() const { return m_BattleOver; }
    void StartTrainerBattle(std::vector<std::shared_ptr<Pokemon>> playerParty, std::vector<std::shared_ptr<Pokemon>> enemyParty);

private:
    enum class UIState {
        ANIMATING,
        MAIN_MENU,
        MOVE_MENU,
        WAITING_TEXT,
        POKEMON_MENU,
        BAG_MENU,
        CATCH_ANIMATION
    };

    

    std::shared_ptr<Pokemon> m_PlayerPokemon;
    std::shared_ptr<Pokemon> m_EnemyPokemon;
    bool m_EscapeSuccessful = false;

    UIState m_UIState = UIState::ANIMATING;
    int m_CursorIndex = 0;
    void UpdateCursorPosition();
    void UpdateMenuVisibility();    
    void SetDialogue(const std::string& text);  
    void HandleEnemyFaint();  
    void AttemptRun();
    std::shared_ptr<Util::Renderer> m_Renderer;
    std::unique_ptr<BattleManager> m_BattleLogic;
    std::shared_ptr<Util::GameObject> m_PlayerBase;
    std::shared_ptr<Util::GameObject> m_EnemyBase;
    bool m_IsVisible = false;

    //ENEMY NPCs
    std::vector<std::shared_ptr<Pokemon>> m_EnemyTeam;
    int m_CurrentEnemyIndex = 0;
    
    // A flag to change battle rules (block Pokéballs, block running)
    bool m_IsTrainerBattle = false;

    // ==========================================
    // 1. BACKGROUND & SPRITES
    // ==========================================
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_PlayerSprite;
    std::shared_ptr<Util::GameObject> m_EnemySprite;

    // ==========================================
    // 2. HP BARS & INFO PANELS
    // ==========================================
    std::shared_ptr<Util::GameObject> m_PlayerPanel;
    std::shared_ptr<Util::Text> m_PlayerNameText;
    std::shared_ptr<Util::Text> m_PlayerHPText;

    std::shared_ptr<Util::GameObject> m_EnemyPanel;
    std::shared_ptr<Util::Text> m_EnemyNameText;

    std::shared_ptr<Util::GameObject> m_PlayerLevelText;
    std::shared_ptr<Util::GameObject> m_EnemyLevelText;

    std::shared_ptr<Util::Text> m_PlayerLevelTextDrawable;
    std::shared_ptr<Util::Text> m_EnemyLevelTextDrawable;

    std::shared_ptr<Util::GameObject> m_PlayerHPBar;
    std::shared_ptr<Util::GameObject> m_EnemyHPBar;
    std::shared_ptr<Util::GameObject> m_PlayerEXPBar;
    std::shared_ptr<Util::GameObject> m_PlayerHPTextObj;

    // ==========================================
    // 3. DIALOGUE & COMMAND MENU
    // ==========================================
    std::shared_ptr<Util::GameObject> m_DialogueBox;
    std::shared_ptr<Util::Text> m_DialogueText;
    std::shared_ptr<Util::GameObject> m_DialogueTextObj;
    std::shared_ptr<Util::Text> m_MoveTexts[4];
    std::shared_ptr<Util::GameObject> m_MoveTextObjs[4];

    std::shared_ptr<Util::GameObject> m_CommandBox; 
    std::shared_ptr<Util::GameObject> m_MenuCursor;
    std::shared_ptr<Util::GameObject> m_MoveBox;

    int m_IntroAnimTimer = 0;
    bool m_IsIntroAnimating = false;

    std::shared_ptr<Util::Image> m_EnemyFrame1;
    std::shared_ptr<Util::Image> m_EnemyFrame2;
    std::shared_ptr<Util::Image> m_PlayerFrame1;
    std::shared_ptr<Util::Image> m_PlayerFrame2;

    int m_CommandCursorIndex = 0;
    bool m_IsSlidingIn = false;
    int m_SlideTimer = 0;
    int m_TextWaitTimer = 0;

    int m_AnimTime = 0;         
    int m_PlayerLungeTimer = 0; 
    int m_EnemyShakeTimer = 0;

    std::queue<std::string> m_DialogueQueue;
    int m_EnemyLungeTimer = 0;
    int m_PlayerShakeTimer = 0;

    float m_DisplayPlayerHPPercent = 1.0f;
    float m_DisplayEnemyHPPercent = 1.0f;
    float m_DisplayPlayerEXPPercent = 0.0f;
    bool m_AllowEXPAnimation = false;
    int m_TargetPlayerHP = 0;
    int m_TargetEnemyHP = 0;
    float m_VisualPlayerHP = 0.0f; 
    float m_VisualEnemyHP = 0.0f;
    
    void ProcessNextMessage();

    bool m_BattleOver = false;
    BattleManager::TurnResult m_LastResult;

    std::shared_ptr<InventoryMenu> m_InventoryMenu;
    std::shared_ptr<Player> m_Player;
    std::shared_ptr<PokemonMenu> m_PokemonMenu;
    std::unique_ptr<BattleAnimator> m_Animator;


    // BattleUI.hpp — in the private section
// Replace whatever you currently have for m_SheetCache with:
    Util::AssetStore<std::shared_ptr<Util::Image>> m_SheetCache{
        [](const std::string& path) {
            return std::make_shared<Util::Image>(path);
        }
    };
    
    // --- NEW ANIMATION SYSTEM VARIABLES ---
    bool m_IsMoveAnimating = false;
    std::shared_ptr<AnimationPlayer> m_AnimPlayer;
    //Util::AssetStore<std::shared_ptr<Util::Image>> m_AnimAssets; 
    std::shared_ptr<AnimationLibrary> m_AnimLibrary;
    // --------------------------------------

    std::shared_ptr<Util::GameObject> m_PokeballSprite;
    int m_CatchPhaseTimer = 0;
    int m_CatchShakes = 0;        
    int m_TargetShakes = 3;        
    bool m_CatchWillSucceed = false;
    std::shared_ptr<PokeballAnimator> m_PokeballAnimator;

    glm::vec2 m_ThrowStart = {-270.0f, -50.0f}; 
    glm::vec2 m_ThrowEnd = {400.0f, 150.0f};    

    void UpdateBar(std::shared_ptr<Util::GameObject> bar, float percent, float leftEdgeX, float fixedY, float maxScale, bool isHPBar);
    int m_MoveAnimatingTimeout = 0;

};