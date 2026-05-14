#include "BattleUI.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Logger.hpp"
#include "string"
#include <cmath>   // For std::sin
#include <cstdlib> // For rand()
#include <algorithm> // For std::clamp

const std::string POKEMON_RES = std::string(RESOURCE_DIR) + "/Pokemon/";
const std::string BATTLE_RES = std::string(RESOURCE_DIR) + "/battle/";
const std::string FONT_PATH = std::string(RESOURCE_DIR) + "/Fonts/PokemonClassic.ttf"; 

BattleUI::BattleUI(std::shared_ptr<Util::Renderer> renderer) 
    : m_Renderer(renderer) 
{
    
    // Inside BattleUI::BattleUI(std::shared_ptr<Util::Renderer> renderer)
    m_AnimPlayer = std::make_shared<AnimationPlayer>(m_Renderer, m_SheetCache, 100);
    if (m_AnimPlayer) {
        LOG_INFO("AnimationPlayer successfully initialized in BattleUI.");
    }
    else{
        LOG_INFO("AnimationPlayer unsuccessfully initialized in BattleUI.");

    }
    // ==========================================
    // MASTER SCALE
    // ==========================================
    glm::vec2 battleScale = {3.5f, 3.5f}; 
    m_PokemonMenu = std::make_shared<PokemonMenu>(renderer);
    // ==========================================
    // 1. BACKGROUND & BASES
    // ==========================================
    m_Background = std::make_shared<Util::GameObject>();
    m_Background->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "battle_bg_grass.png"));
    m_Background->m_Transform.scale = {5.0f, 5.0f};
    m_Background->m_Transform.translation = {0.0f, 0.0f}; 
    m_Background->SetZIndex(10); 

    m_PlayerBase = std::make_shared<Util::GameObject>();
    m_PlayerBase->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "player_base_grass.png"));
    m_PlayerBase->m_Transform.scale = battleScale;
    m_PlayerBase->m_Transform.translation = {-250.0f, -120.0f}; 
    m_PlayerBase->SetZIndex(11); 

    m_EnemyBase = std::make_shared<Util::GameObject>();
    m_EnemyBase->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "enemy_base_grass.png"));
    m_EnemyBase->m_Transform.scale = battleScale;
    m_EnemyBase->m_Transform.translation = {400.0f, 80.0f}; 
    m_EnemyBase->SetZIndex(11);

    // ==========================================
    // 2. SPRITES
    // ==========================================
    m_EnemySprite = std::make_shared<Util::GameObject>();
    m_EnemySprite->m_Transform.scale = battleScale;
    m_EnemySprite->m_Transform.translation = {400.0f, 150.0f}; 
    m_EnemySprite->SetZIndex(12);

    m_PlayerSprite = std::make_shared<Util::GameObject>();
    m_PlayerSprite->m_Transform.scale = battleScale;
    m_PlayerSprite->m_Transform.translation = {-270.0f, -50.0f}; 
    m_PlayerSprite->SetZIndex(12);

    // ==========================================
    // 3. PANELS (HP Bars)
    // ==========================================
    m_EnemyPanel = std::make_shared<Util::GameObject>();
    m_EnemyPanel->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "enemy_panel.png"));
    m_EnemyPanel->m_Transform.scale = {5.0f, 4.0f};
    m_EnemyPanel->m_Transform.translation = {-340.0f, 280.0f}; 
    m_EnemyPanel->SetZIndex(13);

    m_PlayerPanel = std::make_shared<Util::GameObject>();
    m_PlayerPanel->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "player_panel.png"));
    m_PlayerPanel->m_Transform.scale = {4.8f, 4.0f};
    m_PlayerPanel->m_Transform.translation = {340.0f, -50.0f}; 
    m_PlayerPanel->SetZIndex(13);

    // --- NEW: DYNAMIC BARS ---
    m_EnemyHPBar = std::make_shared<Util::GameObject>();
    m_EnemyHPBar->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "bar_green.png"));
    m_EnemyHPBar->m_Transform.scale = {6.0f, 4.0f}; // Fixed scale Y
    m_EnemyHPBar->SetZIndex(14); 

    m_PlayerHPBar = std::make_shared<Util::GameObject>();
    m_PlayerHPBar->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "bar_green.png"));
    m_PlayerHPBar->m_Transform.scale = {4.8f, 4.0f}; // Fixed scale Y
    m_PlayerHPBar->SetZIndex(14); 

    m_PlayerEXPBar = std::make_shared<Util::GameObject>();
    m_PlayerEXPBar->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "bar_blue.png"));
    m_PlayerEXPBar->m_Transform.scale = {4.8f, 4.0f}; // Fixed scale Y
    m_PlayerEXPBar->SetZIndex(14); 

    // --- NEW: PLAYER HP NUMBERS ---
    m_PlayerHPText = std::make_shared<Util::Text>(FONT_PATH, 25, "HP: 50/50", Util::Color{0, 0, 0, 255});
    m_PlayerHPTextObj = std::make_shared<Util::GameObject>();
    m_PlayerHPTextObj->SetDrawable(m_PlayerHPText);
    m_PlayerHPTextObj->SetZIndex(14); 

    // ==========================================
    // 4. DIALOGUE & COMMAND MENU
    // ==========================================
    m_DialogueBox = std::make_shared<Util::GameObject>();
    m_DialogueBox->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "dialogue_box.png"));
    m_DialogueBox->m_Transform.scale = {5.33f, 4.0f}; 
    m_DialogueBox->m_Transform.translation = {0.20f, -265.0f}; 
    m_DialogueBox->SetZIndex(14); 

    m_CommandBox = std::make_shared<Util::GameObject>();
    m_CommandBox->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "command_box.png"));
    m_CommandBox->m_Transform.scale = {4.0f, 4.0f}; 
    m_CommandBox->m_Transform.translation = {400.0f, -260.0f}; 
    m_CommandBox->SetZIndex(15); 

    m_MoveBox = std::make_shared<Util::GameObject>();
    m_MoveBox->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "move_box.png")); 
    m_MoveBox->m_Transform.scale = {4.0f, 4.0f}; 
    m_MoveBox->m_Transform.translation = {0.0f, -250.0f}; 
    m_MoveBox->SetZIndex(14); 

    m_MenuCursor = std::make_shared<Util::GameObject>();
    m_MenuCursor->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "cursor.png"));
    m_MenuCursor->m_Transform.scale = {3.5f, 3.5f}; ;
    m_MenuCursor->m_Transform.translation = {170.0f, -200.0f}; 
    m_MenuCursor->SetZIndex(16); 

    m_DialogueText = std::make_shared<Util::Text>(FONT_PATH, 20, "What will you do?", Util::Color{255, 255, 255, 255});
    
    m_DialogueTextObj = std::make_shared<Util::GameObject>();
    m_DialogueTextObj->SetDrawable(m_DialogueText);
    m_DialogueTextObj->SetZIndex(100); 
    m_DialogueTextObj->m_Transform.translation = {-460.0f, -120.0f}; 
        // levels

    // --- ENEMY LEVEL TEXT ---
    //m_EnemyLevelText = std::make_shared<Util::GameObject>();
    m_EnemyLevelTextDrawable = std::make_shared<Util::Text>(
    FONT_PATH, 26, "Lv99", Util::Color::FromName(Util::Colors::BLACK));
    
    // 2. Assign it to the GameObject
    m_EnemyLevelText = std::make_shared<Util::GameObject>();
    m_EnemyLevelText->SetDrawable(m_EnemyLevelTextDrawable);
    
    m_EnemyLevelText->SetZIndex(16);
    m_EnemyLevelText->m_Transform.translation = {-195.0f, 295.0f}; 
    


    // --- PLAYER LEVEL TEXT ---
    //m_PlayerLevelText = std::make_shared<Util::GameObject>();    
    m_PlayerLevelTextDrawable = std::make_shared<Util::Text>(
    FONT_PATH, 26, "Lv99", Util::Color::FromName(Util::Colors::BLACK));
    
    // 2. Assign it to the GameObject
    m_PlayerLevelText = std::make_shared<Util::GameObject>();
    m_PlayerLevelText->SetDrawable(m_PlayerLevelTextDrawable);
    
    m_PlayerLevelText->SetZIndex(16);
    m_PlayerLevelText->m_Transform.translation = {582.0f, -13.0f}; 
    m_PokeballAnimator = std::make_shared<PokeballAnimator>(m_Renderer);
    
    m_Renderer->AddChild(m_EnemyLevelText);
    m_Renderer->AddChild(m_PlayerLevelText);
    // ADD MAIN GAMEOBJECTS TO RENDERER
    m_Renderer->AddChild(m_MoveBox);
    m_Renderer->AddChild(m_DialogueTextObj);
    m_Renderer->AddChild(m_Background);
    m_Renderer->AddChild(m_PlayerBase);
    m_Renderer->AddChild(m_EnemyBase);
    m_Renderer->AddChild(m_EnemySprite);
    m_Renderer->AddChild(m_PlayerSprite);
    m_Renderer->AddChild(m_EnemyPanel);
    m_Renderer->AddChild(m_PlayerPanel);
    
    // ADD THE NEW BARS AND HP TEXT
    m_Renderer->AddChild(m_EnemyHPBar);
    m_Renderer->AddChild(m_PlayerHPBar);
    m_Renderer->AddChild(m_PlayerEXPBar);
    m_Renderer->AddChild(m_PlayerHPTextObj);

    m_Renderer->AddChild(m_DialogueBox);
    m_Renderer->AddChild(m_CommandBox);
    m_Renderer->AddChild(m_MenuCursor);

    m_Animator = std::make_unique<BattleAnimator>(m_PlayerSprite, m_EnemySprite, m_Renderer);
// Pass the renderer, our new asset store, and a Z-index of 100
    //m_AnimPlayer = std::make_shared<AnimationPlayer>(renderer, m_AnimAssets, 100);

    // ==========================================
    // 5. MOVE TEXTS
    // ==========================================
    for (int i = 0; i < 4; i++) {
        m_MoveTexts[i] = std::make_shared<Util::Text>(FONT_PATH, 20, "-", Util::Color{0, 0, 0, 255});
        
        m_MoveTextObjs[i] = std::make_shared<Util::GameObject>();
        m_MoveTextObjs[i]->SetDrawable(m_MoveTexts[i]);
        m_MoveTextObjs[i]->SetZIndex(100); 
        
        float startX = -310.0f; 
        float startY = -220.0f; 
        float xOffset = (i % 2 == 1) ? 300.0f : 0.0f; 
        float yOffset = (i >= 2) ? -65.0f : 0.0f; 
        
        m_MoveTextObjs[i]->m_Transform.translation = {startX + xOffset, startY + yOffset};
        m_Renderer->AddChild(m_MoveTextObjs[i]);
    }

    
    
    Hide();
}

void BattleUI::Show(std::vector<std::shared_ptr<Pokemon>> playerParty, std::shared_ptr<Pokemon> wildPokemon) {
    
    m_Animator->ResetState(BattleSide::PLAYER);
    m_Animator->ResetState(BattleSide::ENEMY);
    m_Animator->AnimateHPDrain(BattleSide::PLAYER, m_DisplayPlayerHPPercent, m_DisplayPlayerHPPercent);

    // --- CRASH PREVENTION ---
    if (playerParty.empty() || !playerParty[0]) {
        LOG_ERROR("CRASH PREVENTED: Player party is empty or null!");
        return;
    }
    if (!wildPokemon) {
        LOG_ERROR("CRASH PREVENTED: Enemy Pokemon is null!");
        return;
    }

    //m_TargetPlayerHP = m_PlayerPokemon->GetCurrentHP();
    //m_TargetEnemyHP = m_EnemyPokemon->GetCurrentHP();

    m_BattleOver = false;
    m_EnemySprite->m_Transform.translation.y = 150.0f;  // Snap enemy to base height
    m_PlayerSprite->m_Transform.translation.y = -30.0f;
    m_UIState = UIState::ANIMATING;
    m_IsVisible = true;
    
    // --- FIND FIRST ALIVE POKEMON ---
    std::shared_ptr<Pokemon> startingPokemon = playerParty[0]; 
    for (auto p : playerParty) {
        if (p && p->GetCurrentHP() > 0) {
            startingPokemon = p;
            break;
        }
    }
    
    m_BattleLogic = std::make_unique<BattleManager>(startingPokemon, wildPokemon, true);
    m_MoveBox->SetVisible(false);
    
    // --- CACHE POKEMON FOR UI UPDATES ---
    m_PlayerPokemon = startingPokemon;
    m_EnemyPokemon = wildPokemon;
    
    // ADD THESE LINES HERE to fix the HP bars draining to 0
    m_TargetPlayerHP = m_PlayerPokemon->GetCurrentHP();
    m_TargetEnemyHP = m_EnemyPokemon->GetCurrentHP();
    m_EscapeSuccessful = false;

    SetDialogue("What will you do?"); 
    
    std::string enemyName = wildPokemon->GetName();
    std::transform(enemyName.begin(), enemyName.end(), enemyName.begin(), ::tolower);
    
    std::string playerName = startingPokemon->GetName();
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), ::tolower);

    m_EnemyFrame1 = ResourceManager::GetImageStore().Get(POKEMON_RES + enemyName + "_front_1.png");
    m_EnemyFrame2 = ResourceManager::GetImageStore().Get(POKEMON_RES + enemyName + "_front_2.png");

    m_PlayerFrame1 = ResourceManager::GetImageStore().Get(POKEMON_RES + playerName + "_back_1.png");
    m_PlayerFrame2 = ResourceManager::GetImageStore().Get(POKEMON_RES + playerName + "_back_2.png");


   


    m_EnemySprite->SetDrawable(m_EnemyFrame1);
    m_PlayerSprite->SetDrawable(m_PlayerFrame1);

    m_PlayerSprite->m_Transform.translation.x = -870.0f;
    m_EnemyBase->m_Transform.translation.x = 1000.0f;
    m_EnemySprite->m_Transform.translation.x = 1000.0f;

    m_SlideTimer = 0;
    m_IsSlidingIn = true;
    m_IsIntroAnimating = false;
    m_IntroAnimTimer = 0;

    m_Background->SetVisible(true);
    m_PlayerBase->SetVisible(true);
    m_EnemyBase->SetVisible(true);
    m_EnemySprite->SetVisible(true);
    m_PlayerSprite->SetVisible(true);

    m_EnemyPanel->SetVisible(false);
    m_PlayerPanel->SetVisible(false);
    
    // Hide the dynamic UI elements during the intro sliding
    m_EnemyHPBar->SetVisible(false);
    m_PlayerHPBar->SetVisible(false);
    m_PlayerEXPBar->SetVisible(false);
    m_PlayerHPTextObj->SetVisible(false);

    m_DialogueBox->SetVisible(true);
    m_CommandBox->SetVisible(false);
    m_MenuCursor->SetVisible(false);
    m_DialogueTextObj->SetVisible(true); 

    LOG_TRACE("Battle started against {}!", wildPokemon->GetName());

    // Get the levels from the Pokemon objects
    int playerLevel = m_PlayerPokemon->GetLevel();
    int enemyLevel = m_EnemyPokemon->GetLevel();

    // Update the Util::Text components with the new strings
    m_PlayerLevelTextDrawable->SetText(std::to_string(playerLevel));
    m_EnemyLevelTextDrawable->SetText(std::to_string(enemyLevel));
    m_PlayerLevelText->SetVisible(false);
    m_EnemyLevelText->SetVisible(false);
    
    auto moves = startingPokemon->GetMoves(); 
    for (size_t i = 0; i < 4; i++) {
        if (i < moves.size()) {
            m_MoveTexts[i]->SetText(moves[i]); 
        } else {
            m_MoveTexts[i]->SetText("-"); 
        }
    }
    m_DisplayPlayerHPPercent = (float)m_PlayerPokemon->GetCurrentHP() / m_PlayerPokemon->GetMaxHP();
    m_DisplayEnemyHPPercent = (float)m_EnemyPokemon->GetCurrentHP() / m_EnemyPokemon->GetMaxHP();
    
    // --- NEW: SNAP THE ANIMATOR TO THE TRUE STARTING HP ---
    m_Animator->AnimateHPDrain(BattleSide::PLAYER, m_DisplayPlayerHPPercent, m_DisplayPlayerHPPercent);
    m_Animator->AnimateHPDrain(BattleSide::ENEMY, m_DisplayEnemyHPPercent, m_DisplayEnemyHPPercent);
    
    if (m_PlayerPokemon->GetExpToNextLevel() > 0) {
        m_DisplayPlayerEXPPercent = (float)m_PlayerPokemon->GetCurrentExp() / m_PlayerPokemon->GetExpToNextLevel();
    } else {
        m_DisplayPlayerEXPPercent = 0.0f;
    }
}

// ============================================================================
// BattleUI::Update() — Patched
//
// Changes from original:
//   [Bug 1 FIX] Removed the direct `m_AnimPlayer->Update(16.6f)` block.
//               BattleAnimator now owns the only AnimationPlayer and its clock.
//
//   [Bug 2 FIX] PlayAttackEffect() now receives a lambda that sets
//               m_IsMoveAnimating = false when the last animation frame fires.
//               This was the freeze/deadlock: the flag was set to true but
//               never reset, permanently blocking WAITING_TEXT progression.
//
//   [No other logic changed] All state machine branches, queue handling,
//               HP/EXP bar updates, and input handling are identical to the
//               original. Only the two targeted lines are touched.
// ============================================================================

bool BattleUI::Update() {
    if (!m_IsVisible) return false;

    // ==========================================
    // 1. UPDATE ANIMATOR
    // ==========================================
    if (!m_IsSlidingIn && !m_IsIntroAnimating) {
        m_Animator->Update(16.6f); // [Bug 2 FIX] Pass ms so AnimationPlayer's
                                   // m_FrameMs threshold (~50ms at 20fps) works
                                   // correctly. BattleAnimator forwards this to
                                   // m_AnimPlayer->Update() internally — the
                                   // separate m_AnimPlayer->Update(16.6f) block
                                   // that was here has been REMOVED.
    }

    // ==========================================
    // 2. DYNAMIC UI UPDATES (SMOOTH BARS)
    // ==========================================
    float targetEXPPercent = m_DisplayPlayerEXPPercent;

    if (m_PlayerPokemon && m_EnemyPokemon) {
        if (m_AllowEXPAnimation && m_PlayerPokemon->GetExpToNextLevel() > 0) {
            targetEXPPercent = (float)m_PlayerPokemon->GetCurrentExp() / m_PlayerPokemon->GetExpToNextLevel();
            targetEXPPercent = std::clamp(targetEXPPercent, 0.0f, 1.0f);
        }
    }

    if (m_PlayerPokemon && m_EnemyPokemon && !m_IsSlidingIn) {
        float animSpeed = 0.01f;

        if (targetEXPPercent < m_DisplayPlayerEXPPercent && targetEXPPercent < 0.5f && m_DisplayPlayerEXPPercent > 0.5f) {
            m_DisplayPlayerEXPPercent = 0.0f;
        }
        if (m_DisplayPlayerEXPPercent < targetEXPPercent) m_DisplayPlayerEXPPercent += animSpeed;
        if (std::abs(m_DisplayPlayerEXPPercent - targetEXPPercent) < animSpeed) m_DisplayPlayerEXPPercent = targetEXPPercent;

        m_DisplayPlayerEXPPercent = std::clamp(m_DisplayPlayerEXPPercent, 0.0f, 1.0f);

        m_DisplayPlayerHPPercent = m_Animator->GetPlayerHPPercent();
        m_DisplayEnemyHPPercent  = m_Animator->GetEnemyHPPercent();

        int displayHP = std::round(m_DisplayPlayerHPPercent * m_PlayerPokemon->GetMaxHP());
        std::string hpStr = std::to_string(displayHP) + " / " + std::to_string(m_PlayerPokemon->GetMaxHP());
        m_PlayerHPText->SetText(hpStr);

        float hpTextLeftX = 350.0f;
        float hpTextTopY  = -70.0f;
        glm::vec2 hpTextSize = m_PlayerHPText->GetSize();
        m_PlayerHPTextObj->m_Transform.translation.x = hpTextLeftX + (hpTextSize.x / 2.0f);
        m_PlayerHPTextObj->m_Transform.translation.y = hpTextTopY  - (hpTextSize.y / 2.0f);

        UpdateBar(m_EnemyHPBar,   m_DisplayEnemyHPPercent,   -405.0f, 248.0f,  5.1f, true);
        UpdateBar(m_PlayerHPBar,  m_DisplayPlayerHPPercent,   375.0f, -60.0f,  5.0f, true);
        UpdateBar(m_PlayerEXPBar, m_DisplayPlayerEXPPercent,  145.0f, -128.0f, 2.0f, false);
    }

    // ==========================================
    // STATE 1: ANIMATIONS PLAYING
    // ==========================================
    if (m_UIState == UIState::ANIMATING) {
        if (m_IsSlidingIn) {
            m_SlideTimer++;
            m_PlayerSprite->m_Transform.translation.x += 15.0f;
            m_EnemyBase->m_Transform.translation.x   -= 15.0f;
            m_EnemySprite->m_Transform.translation.x -= 15.0f;

            if (m_SlideTimer >= 40) {
                m_IsSlidingIn = false;
                m_EnemyPanel->SetVisible(true);
                m_PlayerPanel->SetVisible(true);
                m_EnemyHPBar->SetVisible(true);
                m_PlayerHPBar->SetVisible(true);
                m_PlayerEXPBar->SetVisible(true);
                m_PlayerHPTextObj->SetVisible(true);
                m_DialogueTextObj->SetVisible(true);
                m_PlayerLevelText->SetVisible(true);
                m_EnemyLevelText->SetVisible(true);
                m_IsIntroAnimating = true;
            }
        }
        else if (m_IsIntroAnimating) {
            m_IntroAnimTimer++;
            if (m_IntroAnimTimer == 30) {
                m_EnemySprite->SetDrawable(m_EnemyFrame2);
                m_PlayerSprite->SetDrawable(m_PlayerFrame2);
            }
            else if (m_IntroAnimTimer == 60) {
                m_EnemySprite->SetDrawable(m_EnemyFrame1);
                m_PlayerSprite->SetDrawable(m_PlayerFrame1);
                m_IsIntroAnimating = false;

                m_UIState    = UIState::MAIN_MENU;
                m_CursorIndex = 0;
                UpdateCursorPosition();
            }
        }
    }
    // ==========================================
    // STATE 2: MAIN COMMAND MENU
    // ==========================================
    else if (m_UIState == UIState::MAIN_MENU) {
        if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
            m_CursorIndex = (m_CursorIndex + 2) % 4;
            UpdateCursorPosition();
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::LEFT) || Util::Input::IsKeyDown(Util::Keycode::RIGHT)) {
            if (m_CursorIndex % 2 == 0) m_CursorIndex++; else m_CursorIndex--;
            UpdateCursorPosition();
        }

        if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
            if (m_CursorIndex == 0) {
                m_UIState     = UIState::MOVE_MENU;
                m_CursorIndex = 0;
                UpdateCursorPosition();
            }
            else if (m_CursorIndex == 1) {
                if (!m_Player)         { return false; }
                if (!m_InventoryMenu)  { return false; }

                m_UIState = UIState::BAG_MENU;
                auto rawInv = m_Player->GetInventory();

                std::map<ItemCategory, std::vector<std::pair<std::string, int>>> m_CategorizedItems;
                for (const auto& [name, data] : rawInv) {
                    if (data.quantity > 0) {
                        m_CategorizedItems[data.category].push_back({name, data.quantity});
                    }
                }
                m_InventoryMenu->Show(m_CategorizedItems);
                m_CursorIndex = 0;
                UpdateCursorPosition();
            }
            else if (m_CursorIndex == 2) {
                m_UIState = UIState::POKEMON_MENU;
                m_PokemonMenu->Show(m_Player->GetParty());
            }
            else if (m_CursorIndex == 3) {
                bool isWildBattle = !m_IsTrainerBattle;
                while (!m_DialogueQueue.empty()) m_DialogueQueue.pop();

                if (!isWildBattle) {
                    m_DialogueQueue.push("You can't run from a trainer battle!");
                } else {
                    int playerLevel  = m_PlayerPokemon->GetLevel();
                    int enemyLevel   = m_EnemyPokemon->GetLevel();
                    int escapeChance = std::clamp(50 + ((playerLevel - enemyLevel) * 5), 10, 95);

                    if (rand() % 100 < escapeChance) {
                        m_DialogueQueue.push("Got away safely!");
                        m_EscapeSuccessful = true;
                    } else {
                        m_DialogueQueue.push("Failed to escape!");
                        m_EscapeSuccessful = false;
                    }
                }
                m_UIState      = UIState::WAITING_TEXT;
                m_TextWaitTimer = 15;
                ProcessNextMessage();
            }
        }
    }
    // ==========================================
    // STATE 3: MOVE SELECTION MENU
    // ==========================================
    else if (m_UIState == UIState::MOVE_MENU) {
        if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
            m_CursorIndex = (m_CursorIndex + 2) % 4;
            UpdateCursorPosition();
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::LEFT) || Util::Input::IsKeyDown(Util::Keycode::RIGHT)) {
            if (m_CursorIndex % 2 == 0) m_CursorIndex++; else m_CursorIndex--;
            UpdateCursorPosition();
        }

        if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
            BattleManager::TurnResult result = m_BattleLogic->SelectMove(m_CursorIndex);
            m_AllowEXPAnimation = false;

            while (!m_DialogueQueue.empty()) m_DialogueQueue.pop();

            // ── Inject player animation tag FIRST ─────────────────────────────
            std::string moveName = m_PlayerPokemon->GetMoves()[m_CursorIndex];
            auto& moveData = MoveDatabase::GetMove(moveName);

            if (!moveData.animation_key.empty()) {
                // Format: [ANIM:Move:THUNDERSHOCK:TARGET_ENEMY]
                m_DialogueQueue.push("[ANIM:" + moveData.animation_key + ":TARGET_ENEMY]");
            }
            // ──────────────────────────────────────────────────────────────────

            // Then push the battle result text lines
            std::stringstream ss(result.message);
            std::string line;
            while (std::getline(ss, line, '\n')) {
                if (!line.empty()) {
                    m_DialogueQueue.push(line);
                    LOG_INFO("[BattleUI] Pushing to queue: '{}'", line);
                }
            }

            m_UIState       = UIState::WAITING_TEXT;
            m_TextWaitTimer = 15;
            ProcessNextMessage();
        }

        if (Util::Input::IsKeyDown(Util::Keycode::X)) {
            m_UIState     = UIState::MAIN_MENU;
            m_CursorIndex = 0;
            UpdateCursorPosition();
        }
    }
    else if (m_UIState == UIState::WAITING_TEXT) {
    // 1. ANIMATION LOCK
    if (m_IsMoveAnimating) {
        m_MoveAnimatingTimeout++;
        if (m_MoveAnimatingTimeout > 300) { 
            LOG_WARN("[BattleUI] Animation timeout — forcing unlock.");
            m_IsMoveAnimating = false;
        }
        return true; 
    }
    m_MoveAnimatingTimeout = 0;

  // 2. EMPTY QUEUE CHECK
if (m_DialogueQueue.empty()) {
    // ═══════════════════════════════════════════════
    // FIX 1 – Wait for all visuals to finish
    // (Faint animation, HP bar still draining, etc.)
    // ═══════════════════════════════════════════════
    if (m_Animator->IsBusy()) {
        // Let the current animation play out before
        // switching to the next pokémon or ending the battle.
        return true;
    }

    auto state = m_BattleLogic->GetState();

    // --- BULLETPROOF WIN/LOSS CHECK ---
    if (m_EnemyPokemon->GetCurrentHP() <= 0) {
        state = BattleManager::BattleState::BATTLE_WON;
    }
    else if (m_PlayerPokemon->GetCurrentHP() <= 0) {
        state = BattleManager::BattleState::BATTLE_LOST;
    }

    if (state == BattleManager::BattleState::BATTLE_WON) {
        if (m_IsTrainerBattle) {
            m_CurrentEnemyIndex++;
            if (m_CurrentEnemyIndex < static_cast<int>(m_EnemyTeam.size())) {

                m_EnemyPokemon = m_EnemyTeam[m_CurrentEnemyIndex]; // duplicate removed

                // Reset enemy visual state (position, visibility)
                m_Animator->ResetState(BattleSide::ENEMY);

                // ═══════════════════════════════════════════════
                // FIX 2 – AnimateHPDrain now requires a speed.
                // Since the new pokémon is at full HP, we can
                // snap the bar by calling with a very high speed.
                // Alternatively, just set the display percent directly.
                // I'll use a snap: speed = 1.0f makes it instant.
                // ═══════════════════════════════════════════════
                m_Animator->AnimateHPDrain(BattleSide::ENEMY,
                                           1.0f,   // start (full)
                                           1.0f,   // target (full)
                                           1.0f);  // speed (instant)

                // Load new enemy sprites
                std::string enemyName = m_EnemyPokemon->GetName();
                std::transform(enemyName.begin(), enemyName.end(),
                               enemyName.begin(), ::tolower);
                m_EnemyFrame1 = ResourceManager::GetImageStore().Get(
                    POKEMON_RES + enemyName + "_front_1.png");
                m_EnemyFrame2 = ResourceManager::GetImageStore().Get(
                    POKEMON_RES + enemyName + "_front_2.png");
                m_EnemySprite->SetDrawable(m_EnemyFrame1);
                m_EnemySprite->SetVisible(true);

                // ═══════════════════════════════════════════════
                // FIX 3 – Adjust Y position to your layout.
                // Version 1 uses ENEMY_BASE_Y (likely 150), but
                // if your intro slide already set it, you may
                // not need this line. I'll keep it but commented
                // if it's not needed.
                // ═══════════════════════════════════════════════
                // m_EnemySprite->m_Transform.translation.y = ENEMY_BASE_Y;

                // Create a new BattleManager for the fresh enemy
                m_BattleLogic = std::make_unique<BattleManager>(
                    m_PlayerPokemon, m_EnemyPokemon, true);
                m_EnemyLevelTextDrawable->SetText(
                    std::to_string(m_EnemyPokemon->GetLevel()));

                m_DialogueQueue.push("Trainer sent out " +
                                     m_EnemyPokemon->GetName() + "!");
                m_UIState = UIState::WAITING_TEXT;
                m_TextWaitTimer = 15;
                ProcessNextMessage();
                return true;
            }
        }

        // Wild battle or last trainer pokémon defeated
        m_BattleOver = true;
        m_IsTrainerBattle = false;
        Hide();
        return true;

    }
    else if (state == BattleManager::BattleState::BATTLE_LOST) {
        bool hasSurvivingPokemon = false;
        for (auto p : m_Player->GetParty()) {
            if (p->GetCurrentHP() > 0) {
                hasSurvivingPokemon = true;
                break;
            }
        }

        if (hasSurvivingPokemon) {
            m_UIState = UIState::POKEMON_MENU;
            m_PokemonMenu->Show(m_Player->GetParty());
            return true;
        } else {
            m_BattleOver = true;
            m_IsTrainerBattle = false;
            Hide();
            return true;
        }
    }
    // BATTLE_ESCAPED removed – not present in your current BattleState enum.
    // If you add it later, you can re‑enable this block.
    else {
        // Normal turn end – return to main menu with fresh cursor
        SetDialogue("What will you do?");
        m_UIState = UIState::MAIN_MENU;
        m_CursorIndex = 0;
        UpdateCursorPosition();
        return true;
    }
}
    

    std::string nextLine = m_DialogueQueue.front();

    // 3. TAG INTERCEPTORS (Automatic progression)
    
    // --- ANIMATION TAGS ---
    if (nextLine.find("[ANIM:") != std::string::npos) {
        m_DialogueQueue.pop();
        size_t first = nextLine.find(':'), last = nextLine.rfind(':'), end = nextLine.find(']');
        if (first != std::string::npos && last != first) {
            std::string animName = nextLine.substr(first + 1, last - first - 1);
            std::string targetStr = nextLine.substr(last + 1, end - last - 1);
            const BattleAnimDef* animDef = AnimationLibrary::Get().Find(animName);
            if (animDef) {
                m_IsMoveAnimating = true;
                BattleSide side = (targetStr == "TARGET_ENEMY") ? BattleSide::ENEMY : BattleSide::PLAYER;
                m_Animator->PlayAttackEffect(*animDef, side, [this](){ 
                    m_IsMoveAnimating = false;
                    m_TextWaitTimer = 0;          // skip any remaining delay
                    ProcessNextMessage();  
                });
            }
        }
        return true;
    }

    // --- HP SYNC TAGS ---
    else if (nextLine.find("[SYNC_") != std::string::npos) {
        m_DialogueQueue.pop();
        bool isEnemy = (nextLine.find("ENEMY") != std::string::npos);
        size_t bracketEnd = nextLine.find(']');
        float targetHP = std::stof(nextLine.substr(bracketEnd + 1));
        
        if (isEnemy) {
            float targetPercent = targetHP / m_EnemyPokemon->GetMaxHP();
            m_Animator->AnimateHPDrain(BattleSide::ENEMY, m_DisplayEnemyHPPercent, targetPercent);
            if (targetHP <= 0) { 
                m_Animator->PlayFaint(BattleSide::ENEMY); 
                //m_DialogueQueue.push(m_EnemyPokemon->GetName() + " fainted!");
            }
        } else {
            float targetPercent = targetHP / m_PlayerPokemon->GetMaxHP();
            m_Animator->AnimateHPDrain(BattleSide::PLAYER, m_DisplayPlayerHPPercent, targetPercent);
            if (targetHP <= 0) { 
                m_Animator->PlayFaint(BattleSide::PLAYER); 
                //m_DialogueQueue.push(m_PlayerPokemon->GetName() + " fainted!");
            }
        }
        return true; 
    }

    // 4. TEXT PROGRESSION (Wait for Player Input)
    if (m_Animator->IsBusy() || m_IsMoveAnimating || m_TextWaitTimer > 0) {
        if (m_TextWaitTimer > 0) m_TextWaitTimer--;
            return true; 
    }

    if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
        m_TextWaitTimer = 10;
        ProcessNextMessage();
    }
}
    // ==========================================
    // STATE 5: BAG MENU (IN-BATTLE)
    // ==========================================
    else if (m_UIState == UIState::BAG_MENU) {
        if (m_InventoryMenu->Update()) {
            m_InventoryMenu->Hide();
            m_UIState     = UIState::MAIN_MENU;
            m_CursorIndex = 1;
            UpdateCursorPosition();
            return true;
        }

        if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
            std::string  selectedItem = m_InventoryMenu->GetSelectedItem();
            ItemCategory currentTab   = m_InventoryMenu->GetCurrentTab();

            if (selectedItem.empty())                        return true;
            if (currentTab != ItemCategory::POKEBALLS)       return true;

            m_InventoryMenu->Hide();
            while (!m_DialogueQueue.empty()) m_DialogueQueue.pop();

            m_BattleLogic->UseItem(m_Player, selectedItem);
            bool willSucceed = (m_BattleLogic->GetState() == BattleManager::BattleState::BATTLE_WON);

            glm::vec2 throwStart = { -270.0f, -50.0f };
            glm::vec2 throwEnd   = {  400.0f, 150.0f };

            m_PokeballAnimator->StartCatch(throwStart, throwEnd, willSucceed, m_EnemySprite);

            m_DialogueQueue.push("You threw a " + selectedItem + "!");
            ProcessNextMessage();

            m_UIState = UIState::CATCH_ANIMATION;
        }
    }
    // ==========================================
    // STATE: POKEMON MENU (PARTY SWITCHING)
    // ==========================================
    else if (m_UIState == UIState::POKEMON_MENU) {
        if (m_PokemonMenu->Update()) {
            if (m_PlayerPokemon->GetCurrentHP() <= 0) return true;

            m_PokemonMenu->Hide();
            m_UIState     = UIState::MAIN_MENU;
            m_CursorIndex = 2;
            UpdateCursorPosition();
            SetDialogue("What will you do?");
            return true;
        }

        if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
            int  selectedIdx = m_PokemonMenu->GetSelectedIndex();
            auto party       = m_Player->GetParty();

            if (selectedIdx >= 0 && selectedIdx < static_cast<int>(party.size())) {
                auto selectedPokemon = party[selectedIdx];

                if (selectedPokemon->GetCurrentHP() <= 0)    return true;
                if (selectedPokemon == m_PlayerPokemon)       return true;

                m_PokemonMenu->Hide();
                while (!m_DialogueQueue.empty()) m_DialogueQueue.pop();

                bool wasFainted = (m_PlayerPokemon->GetCurrentHP() <= 0);

                if (wasFainted) {
                    m_DialogueQueue.push("Go! " + selectedPokemon->GetName() + "!");
                } else {
                    m_DialogueQueue.push("Come back, " + m_PlayerPokemon->GetName() + "!");
                    m_DialogueQueue.push("Go! " + selectedPokemon->GetName() + "!");
                }

                m_PlayerPokemon = selectedPokemon;
                m_Animator->ResetState(BattleSide::PLAYER);

                float startPercent = (float)m_PlayerPokemon->GetCurrentHP() / m_PlayerPokemon->GetMaxHP();
                m_Animator->AnimateHPDrain(BattleSide::PLAYER, startPercent, startPercent);

                std::string pName = m_PlayerPokemon->GetName();
                std::transform(pName.begin(), pName.end(), pName.begin(), ::tolower);

                m_PlayerFrame1 = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/Pokemon/" + pName + "_back_1.png");
                m_PlayerFrame2 = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/Pokemon/" + pName + "_back_2.png");
                m_PlayerSprite->SetDrawable(m_PlayerFrame1);
                m_PlayerSprite->SetVisible(true);
                m_PlayerSprite->m_Transform.translation.y = -30.0f;
                m_PlayerLevelTextDrawable->SetText(std::to_string(m_PlayerPokemon->GetLevel()));

                if (wasFainted) {
                    m_BattleLogic = std::make_unique<BattleManager>(m_PlayerPokemon, m_EnemyPokemon, true);
                } else {
                    m_BattleLogic->SetPlayerPokemon(m_PlayerPokemon);

                    BattleManager::TurnResult enemyResult = m_BattleLogic->ExecuteEnemyMove();

                    // ── Inject enemy animation tag FIRST ──────────────────────────
                    std::string oppKey = MoveDatabase::GetMove(m_BattleLogic->GetLastEnemyMove()).animation_key;
                    if (!oppKey.empty()) {
                        if (oppKey.rfind("Move:", 0) == 0)
                            oppKey = "OppMove:" + oppKey.substr(5);
                        m_DialogueQueue.push("[ANIM:" + oppKey + ":TARGET_PLAYER]");
                    }
                    // ──────────────────────────────────────────────────────────────

                    std::stringstream ss(enemyResult.message);
                    std::string line;
                    while (std::getline(ss, line, '\n')) {
                        if (!line.empty()) m_DialogueQueue.push(line);
                    }
                }

                m_UIState       = UIState::WAITING_TEXT;
                m_TextWaitTimer = 15;
                ProcessNextMessage();
            }
        }
    }
    // ==========================================
    // STATE: CATCH ANIMATION
    // ==========================================
    else if (m_UIState == UIState::CATCH_ANIMATION) {
        if (m_PokeballAnimator->Update()) {
            if (m_PokeballAnimator->CatchSucceeded()) {
                m_DialogueQueue.push("Gotcha! " + m_EnemyPokemon->GetName() + " was caught!");
            } else {
                m_EnemySprite->SetVisible(true);
                m_DialogueQueue.push("Oh no! The wild Pokémon broke free!");

                BattleManager::TurnResult enemyResult = m_BattleLogic->ExecuteEnemyMove();

                // ── Inject enemy animation tag FIRST ──────────────────────────
                std::string oppKey = MoveDatabase::GetMove(m_BattleLogic->GetLastEnemyMove()).animation_key;
                if (!oppKey.empty()) {
                    if (oppKey.rfind("Move:", 0) == 0)
                        oppKey = "OppMove:" + oppKey.substr(5);
                    m_DialogueQueue.push("[ANIM:" + oppKey + ":TARGET_PLAYER]");
                }
                // ──────────────────────────────────────────────────────────────

                std::stringstream ss(enemyResult.message);
                std::string line;
                while (std::getline(ss, line, '\n')) {
                    if (!line.empty()) m_DialogueQueue.push(line);
                }
            }

            m_UIState       = UIState::WAITING_TEXT;
            m_TextWaitTimer = 15;
            ProcessNextMessage();
        }
    }

    UpdateMenuVisibility();

    return true;
}

void BattleUI::Hide() {
    
    m_IsVisible = false;
    m_MoveBox->SetVisible(false);
    m_DialogueTextObj->SetVisible(false);
    m_Background->SetVisible(false);
    m_PlayerBase->SetVisible(false);
    m_EnemyBase->SetVisible(false);

    m_EnemySprite->SetVisible(false);
    m_PlayerSprite->SetVisible(false);
    m_EnemyPanel->SetVisible(false);
    m_PlayerPanel->SetVisible(false);
    m_PlayerLevelText->SetVisible(false);
    m_EnemyLevelText->SetVisible(false);
    if (m_EnemyHPBar) m_EnemyHPBar->SetVisible(false);
    if (m_PlayerHPBar) m_PlayerHPBar->SetVisible(false);
    if (m_PlayerEXPBar) m_PlayerEXPBar->SetVisible(false);
    if (m_PlayerHPTextObj) m_PlayerHPTextObj->SetVisible(false);
    
    // ADD THESE TWO LINES:
  
    
    // --- HIDE THE NEW UI ---
    if (m_EnemyHPBar) m_EnemyHPBar->SetVisible(false);
    if (m_PlayerHPBar) m_PlayerHPBar->SetVisible(false);
    if (m_PlayerEXPBar) m_PlayerEXPBar->SetVisible(false);
    if (m_PlayerHPTextObj) m_PlayerHPTextObj->SetVisible(false);
    if (m_PlayerLevelText) m_PlayerLevelText->SetVisible(false);
    if (m_EnemyLevelText) m_EnemyLevelText->SetVisible(false);
    m_DialogueBox->SetVisible(false); 
    m_CommandBox->SetVisible(false);
    m_MenuCursor->SetVisible(false);
    
    for (int i = 0; i < 4; i++) {
        if (m_MoveTextObjs[i]) {
            m_MoveTextObjs[i]->SetVisible(false);
        }
    }
}

void BattleUI::UpdateCursorPosition() {
    float startX = 0.0f; 
    float startY = 0.0f; 
    float xOffset = 0.0f; 
    float yOffset = 0.0f; 

    if (m_UIState == UIState::MAIN_MENU) {
        startX = 200.0f; 
        startY = -230.0f; 
        xOffset = (m_CursorIndex % 2 == 1) ? 220.0f : 0.0f; 
        yOffset = (m_CursorIndex >= 2) ? -65.0f : 0.0f; 
    }
    else if (m_UIState == UIState::MOVE_MENU) {
        startX = -420.0f; 
        startY = -220.0f; 
        xOffset = (m_CursorIndex % 2 == 1) ? 300.0f : 0.0f; 
        yOffset = (m_CursorIndex >= 2) ? -65.0f : 0.0f; 
    }
    
    m_MenuCursor->m_Transform.translation = {startX + xOffset, startY + yOffset};
}

void BattleUI::UpdateMenuVisibility() {
    bool isMainMenu = (m_UIState == UIState::MAIN_MENU);
    bool isMoveMenu = (m_UIState == UIState::MOVE_MENU);
    bool isWaitingText = (m_UIState == UIState::WAITING_TEXT);
    
    m_CommandBox->SetVisible(isMainMenu);
    m_MoveBox->SetVisible(isMoveMenu);

    for (int i = 0; i < 4; i++) {
        if (m_MoveTextObjs[i]) {
            m_MoveTextObjs[i]->SetVisible(isMoveMenu);
        }
    }
    
    m_DialogueBox->SetVisible(isMainMenu || isWaitingText);
    m_DialogueTextObj->SetVisible(isMainMenu || isWaitingText);
    m_MenuCursor->SetVisible(isMainMenu || isMoveMenu);
}

void BattleUI::SetDialogue(const std::string& text) {
    m_DialogueText->SetText(text);

    glm::vec2 textSize = m_DialogueText->GetSize(); 
    
    float fixedLeftX = -580.0f; 
    float fixedTopY = -200.0f;  

    m_DialogueTextObj->m_Transform.translation.x = fixedLeftX + (textSize.x / 2.0f);
    m_DialogueTextObj->m_Transform.translation.y = fixedTopY - (textSize.y / 2.0f);
}

// =======================================================
// NEW HELPER: Keeps the left edge fixed while scaling!
// =======================================================
void BattleUI::UpdateBar(std::shared_ptr<Util::GameObject> bar, float percent, float leftEdgeX, float fixedY, float maxScale, bool isHPBar) {
    percent = std::clamp(percent, 0.0f, 1.0f);
    
    if (percent <= 0.0f) {
        bar->SetVisible(false);
        return; 
    } else {
        bar->SetVisible(true);
    }

    // --- COLOR LOGIC ---
    if (isHPBar) {
        if (percent > 0.50f) bar->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "bar_green.png"));
        else if (percent > 0.20f) bar->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "bar_yellow.png"));
        else bar->SetDrawable(ResourceManager::GetImageStore().Get(BATTLE_RES + "bar_red.png"));
    }

    // --- SCALING ---
    // Scale X proportionally
    bar->m_Transform.scale.x = maxScale * percent;
    
    // COMPLETELY REMOVED the hardcoded scale.y = 4.0f here so it keeps your constructor values!
    
    // --- POSITIONING ---
    // GetScaledSize already accounts for the new scale.x, so it is the true visual width.
    float exactVisualWidth = bar->GetScaledSize().x;
    
    // Shift the center right by EXACTLY half its new visual width (NO * percent here!)
    bar->m_Transform.translation.x = leftEdgeX + (exactVisualWidth / 2.0f);

    bar->m_Transform.translation.y = fixedY;
}

void BattleUI::ProcessNextMessage() {
    if (m_DialogueQueue.empty()) {
        m_UIState = UIState::MAIN_MENU;
        m_CursorIndex = 0;
        UpdateCursorPosition();
        SetDialogue("What will you do?");
        return;
    }

    // Peek at the front. If it's any kind of tag, ABORT.
    // Let State 4 handle the logic for tags.
    if (m_DialogueQueue.front().find("[") == 0) {
        m_UIState = UIState::WAITING_TEXT;
        return; 
    }

    // Since it's not a tag, it's safe to pop and display
    std::string text = m_DialogueQueue.front();
    m_DialogueQueue.pop();

    // --- Aesthetic Spritework (Lunge/Shake) ---
    if (text.find("used") != std::string::npos) {
        if (text.find(m_EnemyPokemon->GetName()) != std::string::npos) {
            m_Animator->LungeSprite(BattleSide::ENEMY);
            m_Animator->ShakeSprite(BattleSide::PLAYER);
        } else {
            m_Animator->LungeSprite(BattleSide::PLAYER);
            m_Animator->ShakeSprite(BattleSide::ENEMY);
        }
    }

    // --- EXP Visual Flag ---
    if (text.find("EXP") != std::string::npos || text.find("gained") != std::string::npos) {
        m_AllowEXPAnimation = true;
    }

    SetDialogue(text);
    m_UIState = UIState::WAITING_TEXT;
}
void BattleUI::StartTrainerBattle(std::vector<std::shared_ptr<Pokemon>> playerParty, 
                                  std::vector<std::shared_ptr<Pokemon>> enemyParty) {
    // 1. Reset battle state
    m_IsTrainerBattle = true; // Flips the rules for this fight!
    
    // 2. Assign the dynamically loaded enemy team
    m_EnemyTeam = enemyParty;
    m_CurrentEnemyIndex = 0;

    // --- CRASH PREVENTION ---
    if (m_EnemyTeam.empty()) {
        LOG_ERROR("CRASH PREVENTED: Enemy team is empty! Check your NPC data.");
        return;
    }
    if (!m_EnemyTeam[m_CurrentEnemyIndex]) {
        LOG_ERROR("CRASH PREVENTED: Enemy Pokemon is null!");
        return;
    }

    // Initialize the logic/UI with the first Pokemon
    // (Assuming Show() handles caching the active Pokemon pointers)
    Show(playerParty, m_EnemyTeam[m_CurrentEnemyIndex]);
}