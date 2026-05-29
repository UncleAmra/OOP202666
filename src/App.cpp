#include "App.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "TrainerDatabase.hpp"
#include "MoveDatabase.hpp"
#include "Item.hpp"
#include "NPC.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "GameConfig.hpp"
#include "Prop.hpp"
#include "SaveSystem.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Text.hpp"
#include <iostream>
#include "StartMenu.hpp"
#include "InventoryMenu.hpp"
#include "PokemonMenu.hpp"
#include "RandomEncounters.hpp"
#include "ResourceManager.hpp"
#include "PokemonDatabase.hpp"
#include "MapGenerator.hpp"
#include "BattleAnimation.hpp"

const std::string RES      = std::string(RESOURCE_DIR);
const std::string MAP_DIR  = RES + "/maps/";

// ==========================================
// CORE LIFECYCLE
// ==========================================

void App::Start() {
    LOG_TRACE("Start");
    
    InitSystems();
    InitGameLoad();
    InitUI();

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    if (Util::Input::IfExit()) {
        m_CurrentState = State::END;
        return;
    }

    HandleGlobalShortcuts();

    // Handle QuickSave trigger
    if (Util::Input::IsKeyDown(Util::Keycode::F) && m_CurrentState == State::UPDATE) {
        PerformQuickSave();
    }

    // Main State Machine routing
    switch (m_CurrentState) {
        case State::START:          break;
        case State::DIALOGUE:       ProcessDialogueState();       break;
        case State::START_MENU:     ProcessStartMenuState();      break;
        case State::INVENTORY_MENU: m_InventoryMenu->Update();    break;
        case State::POKEMON_MENU:   ProcessPokemonMenuState();    break;
        case State::UPDATE:         ProcessOverworldUpdateState(); break;
        case State::BATTLE:         ProcessBattleState();         break;
        case State::END:            break;
    }

    m_Map->Update();
    m_Renderer->Update();
}

void App::End() {
    LOG_TRACE("End");
}

// ==========================================
// INITIALIZATION HELPERS
// ==========================================

void App::InitSystems() {
    MoveDatabase::Init();
    PokemonDatabase::Init();
    TrainerDatabase::Init();
    srand(static_cast<unsigned int>(time(nullptr)));
    
    m_Renderer = std::make_shared<Util::Renderer>();
    m_Map = std::make_shared<Map>();
    m_Map->SetRenderer(m_Renderer); 
    m_PokemonMenu = std::make_shared<PokemonMenu>(m_Renderer);
    m_Character = std::make_shared<Player>(0.0f, 0.0f);
    m_Renderer->AddChild(m_Character); 

    m_BattleUI = std::make_shared<BattleUI>(m_Renderer);  
    
    try {
        AnimationLibrary::Get().LoadFromJson(RES + "/data/TEST.json"); 
    } catch (const std::exception& e) {
        LOG_ERROR("JSON Error: {}", e.what());
    }

    m_InventoryMenu = std::make_shared<InventoryMenu>(m_Renderer);
    m_BattleUI->SetInventoryMenu(m_InventoryMenu);
    m_BattleUI->SetPlayer(m_Character);
}

void App::InitGameLoad() {
    SaveSystem::GameState loadedState;
    m_Map->LoadConnections(RESOURCE_DIR "/maps/connections.txt");
    
    if (SaveSystem::LoadGame(loadedState)) {
        GameConfig::LootedItems = loadedState.lootedItems;
        m_Map->LoadLevel(loadedState.mapPath);
        
        m_Character->SetGridPosition(loadedState.gridX, loadedState.gridY);
        m_Map->WarpTo(loadedState.gridX, loadedState.gridY);
        m_Character->SetDirection(static_cast<Character::Direction>(loadedState.direction));
        m_Character->SetInventory(loadedState.inventory);
        
        for (const auto& pkmn : loadedState.party) {
            m_Character->AddPokemon(pkmn);
        }
    } else {
        GameConfig::LootedItems.clear(); 
        m_Map->LoadLevel(MAP_DIR + "NTUT"); 
        m_Character->SetGridPosition(14, 53); 
        m_Map->WarpTo(14, 53);
        
        auto starter = std::make_shared<Pokemon>(
            "Charmander", 5, PokemonType::FIRE, PokemonType::NONE, 39, 52, 43, 60, 50, 65, 45
        );
        starter->LearnMove("Scratch");
        starter->LearnMove("Growl");
        m_Character->AddPokemon(starter);
    }
}

void App::InitUI() {
    m_DialogueBoxUI = std::make_shared<Util::GameObject>();
    auto boxImage = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/BWTextBox3.png");    
    m_DialogueBoxUI->SetDrawable(boxImage);
    m_DialogueBoxUI->SetZIndex(9.0f); 
    m_DialogueBoxUI->SetVisible(false);
    m_DialogueBoxUI->m_Transform.scale = {1.0f, 1.0f};          
    m_DialogueBoxUI->m_Transform.translation = {0.0f, -288.0f};

    m_DialogueUI = std::make_shared<Util::GameObject>();
    m_DialogueText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/micross.ttf", 24, "...", Util::Color(50, 50, 50)
    );
    m_DialogueUI->SetDrawable(m_DialogueText);
    m_DialogueUI->SetZIndex(10.0f);  
    m_DialogueUI->SetVisible(false);
    m_DialogueUI->m_Transform.translation = {-600.0f, -260.0f};

    m_StartMenu = std::make_shared<StartMenu>(m_Renderer);
    
    m_Renderer->AddChild(m_DialogueBoxUI);
    m_Renderer->AddChild(m_DialogueUI);
}

void App::PerformQuickSave() {
    SaveSystem::GameState current;
    current.mapPath = m_Map->GetCurrentLevelPath(); 
    current.gridX = m_Character->GetGridX();
    current.gridY = m_Character->GetGridY();
    current.direction = static_cast<int>(m_Character->GetFacingDirection());
    current.inventory = m_Character->GetInventory();
    current.lootedItems = GameConfig::LootedItems;
    current.party = m_Character->GetParty();
    
    SaveSystem::SaveGame(current);
    LOG_TRACE("Game Saved Successfully!");
}

// ==========================================
// STATE MACHINE EXECUTION FUNCTIONS
// ==========================================

void App::ProcessDialogueState() {
    if (!Util::Input::IsKeyDown(Util::Keycode::Z)) return;

    // ── Still more lines to show ─────────────────────────────────────────────
    if (!m_CurrentDialogueLines.empty() &&
        m_CurrentDialogueIndex < m_CurrentDialogueLines.size() - 1) {

        m_CurrentDialogueIndex++;
        m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);

        float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
        m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
        return;
    }

    // ── Last line confirmed — close dialogue UI ──────────────────────────────
    m_DialogueBoxUI->SetVisible(false);
    m_DialogueUI->SetVisible(false);
    m_Map->SetPaused(false);

    if (!m_ActiveNPC) {
        // Sign or ground item — no action to process
        m_CurrentState = State::UPDATE;
        return;
    }

    // Unpack NPC data before we clear the pointer
    const NPCAction action = m_ActiveNPC->GetActionType();
    const std::string data = m_ActiveNPC->GetActionData();
    const std::string flag = m_ActiveNPC->GetInteractFlag();
    const ItemCategory category = m_ActiveNPC->GetActionCategory();
    auto               npcParty = m_ActiveNPC->GetParty();

    m_ActiveNPC->SetLocked(false);
    m_ActiveNPC = nullptr;

    // ── Action dispatch ──────────────────────────────────────────────────────
    switch (action) {

        case NPCAction::HEAL: {
            for (auto& pokemon : m_Character->GetParty()) {
                if (pokemon) pokemon->SetCurrentHP(pokemon->GetMaxHP());
            }
            if (!flag.empty()) GameFlags::Set(flag, true);
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::SHOP: {
            // if (!flag.empty()) GameFlags::Set(flag, true);
            // m_ShopUI->Show(data);
            // m_CurrentState = State::SHOP;
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::GIVE_ITEM: {
            if (!data.empty()) {
                m_Character->AddItem(data, category, 1);
                LOG_INFO("Player received: {} (qty: 1)", data);
            }
            if (!flag.empty()) GameFlags::Set(flag, true);
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::BATTLE: {
            m_Character->SetVisible(false);
            m_Map->SetVisible(false);
            m_BattleUI->StartTrainerBattle(m_Character->GetParty(), npcParty);

            if (!flag.empty()) GameFlags::Set(flag, true);
            m_CurrentState = State::BATTLE;
            break;
        }

        case NPCAction::CHECK_ITEM: {
            if (!data.empty() && m_Character->GetItemCount(data) > 0) {
                if (!flag.empty()) {
                    GameFlags::Set(flag, true);
                    LOG_INFO("CHECK_ITEM passed: '{}' found, flag '{}' set.", data, flag);
                }
            } else {
                LOG_INFO("CHECK_ITEM failed: player does not have '{}'.", data);
            }
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::NONE:
        default: {
            // Ambient NPC or sign — no flag, no side effect
            m_CurrentState = State::UPDATE;
            break;
        }
    }
}

void App::ProcessStartMenuState() {
    StartMenu::MenuOption selection = m_StartMenu->Update();
    
    if (selection == StartMenu::MenuOption::POKEMON) {
        LOG_TRACE("Selected: POKEMON");
        m_CurrentState = State::POKEMON_MENU;
        m_StartMenu->SetVisible(false);
        m_PokemonMenu->Show(m_Character->GetParty());
    }
    else if (selection == StartMenu::MenuOption::BAG) {
        LOG_TRACE("Selected: BAG");
        m_CurrentState = State::INVENTORY_MENU; 
        m_StartMenu->SetVisible(false);
        
        std::map<ItemCategory, std::vector<std::pair<std::string, int>>> sortedInventory;
        sortedInventory[ItemCategory::GENERAL] = {};
        sortedInventory[ItemCategory::POKEBALLS] = {};
        sortedInventory[ItemCategory::KEY_ITEMS] = {};
        
        for (const auto& [itemName, invData] : m_Character->GetInventory()) {
            sortedInventory[invData.category].push_back({itemName, invData.quantity});
        }
        m_InventoryMenu->Show(sortedInventory);
    } 
    else if (selection == StartMenu::MenuOption::SAVE) {
        LOG_TRACE("Selected: SAVE");
        PerformQuickSave(); // Reused optimized method
        CloseAllMenus(); 
    } 
    else if (selection == StartMenu::MenuOption::EXIT) {
        LOG_TRACE("Selected: EXIT");
        m_CurrentState = State::END; 
    }
}

void App::ProcessPokemonMenuState() {
    if (m_PokemonMenu->Update()) {
        LOG_TRACE("Exited POKEMON menu");
        m_PokemonMenu->Hide();
        m_StartMenu->SetVisible(true);
        m_CurrentState = State::START_MENU;
        m_SwapIndex = -1; 
        return;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
        int selectedIdx = m_PokemonMenu->GetSelectedIndex();
        if (m_SwapIndex == -1) {
            m_SwapIndex = selectedIdx; 
            LOG_TRACE("Selected Pokemon at index %d to swap.", m_SwapIndex);
        } else {
            LOG_TRACE("Swapping index %d with index %d.", m_SwapIndex, selectedIdx);
            m_Character->SwapPokemon(m_SwapIndex, selectedIdx);
            m_SwapIndex = -1; 
            m_PokemonMenu->Show(m_Character->GetParty()); 
        }
    }
}

void App::ProcessOverworldUpdateState() {
    // A. INTERACTION
    if (Util::Input::IsKeyDown(Util::Keycode::Z) && !m_Character->IsMoving()) {
        int checkX = m_Character->GetGridX();
        int checkY = m_Character->GetGridY();

        Character::Direction facing = m_Character->GetFacingDirection();
        if (facing == Character::Direction::UP)    checkY -= 1;
        if (facing == Character::Direction::DOWN)  checkY += 1;
        if (facing == Character::Direction::LEFT)  checkX -= 1;
        if (facing == Character::Direction::RIGHT) checkX += 1;

        HandleOverworldInteraction(checkX, checkY);
    }

    // B. PROCESSING MOVEMENT
    glm::vec2 movement = m_Character->Update(m_Map);
    m_Map->Move(-movement.x, -movement.y);

    // C. DOORS / WARPING
    if (m_Character->HasHitDoor()) {
        HandleOverworldWarping();
    }

    // D. RANDOM ENCOUNTERS
    HandleOverworldEncounters();
}

void App::ProcessBattleState() {
    m_BattleUI->Update();
    
    if (m_BattleUI->IsBattleOver() || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            m_BattleUI->Hide();
        }
        m_Map->SetVisible(true);
        m_Character->SetVisible(true);
        m_CurrentState = State::UPDATE;
    }
}

// ==========================================
// OVERWORLD UPDATE DELEGATES
// ==========================================

void App::HandleOverworldInteraction(int checkX, int checkY) {
    auto targetNPC = m_Map->GetNPCAt(checkX, checkY);
    
    if (!targetNPC) {
        int propID = m_Map->GetPropType(checkX, checkY); 
        if (propID == 24) {
            float extendedX = checkX;
            float extendedY = checkY;
            Character::Direction playerDir = m_Character->GetFacingDirection();
            
            if (playerDir == Character::Direction::UP)         extendedY -= 1.0f; 
            else if (playerDir == Character::Direction::DOWN)  extendedY += 1.0f;
            else if (playerDir == Character::Direction::LEFT)  extendedX -= 1.0f;
            else if (playerDir == Character::Direction::RIGHT) extendedX += 1.0f;

            targetNPC = m_Map->GetNPCAt(extendedX, extendedY);
        }
    }

    if (targetNPC) {
    m_ActiveNPC = targetNPC;
    m_Character->StopMoving();
    m_CurrentState = State::DIALOGUE; 
    m_DialogueBoxUI->SetVisible(true);
    m_DialogueUI->SetVisible(true);

    Character::Direction playerDir = m_Character->GetFacingDirection();
    if (playerDir == Character::Direction::UP)         targetNPC->SetDirection(Character::Direction::DOWN);
    else if (playerDir == Character::Direction::DOWN)  targetNPC->SetDirection(Character::Direction::UP);
    else if (playerDir == Character::Direction::LEFT)  targetNPC->SetDirection(Character::Direction::RIGHT);
    else if (playerDir == Character::Direction::RIGHT) targetNPC->SetDirection(Character::Direction::LEFT);

    // ─── UPDATE THIS LINE ───────────────────────────────────────────
    // Dereference m_Character so the NPC can read the player's inventory
    m_CurrentDialogueLines = targetNPC->Interact(*m_Character);
    // ────────────────────────────────────────────────────────────────
    
    m_CurrentDialogueIndex = 0; 

    if (!m_CurrentDialogueLines.empty()) {
        m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);
        float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
        m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
    }
}
    else {
        std::string collectedItem = m_Map->CollectItemAt(checkX, checkY, *m_Character);
        if (!collectedItem.empty()) {
            m_Character->StopMoving();
            m_CurrentState = State::DIALOGUE; 
            m_DialogueBoxUI->SetVisible(true);
            m_DialogueUI->SetVisible(true);
            
            m_CurrentDialogueLines = { "Obtained 1x " + collectedItem + "!" };
            m_CurrentDialogueIndex = 0;
            m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
        }
    }
}

void App::HandleOverworldWarping() {
    std::string doorKey = m_Map->GetCurrentLevelPath() + "_" + std::to_string(m_Character->GetGridX()) + "_" + std::to_string(m_Character->GetGridY());
    auto it = GameConfig::DoorRouting.find(doorKey);
    
    if (it != GameConfig::DoorRouting.end()) {
        GameConfig::WarpDestination dest = it->second;
        
        if (dest.levelPath.find("GENERATED_CAVE") != std::string::npos) {
            auto generated = MapGenerator::GenerateCave(40, 40);
            std::string exitKey = "GENERATED_CAVE_" + std::to_string(generated.spawnX) + "_" + std::to_string(generated.spawnY);
            
            GameConfig::DoorRouting[exitKey] = { m_Map->GetCurrentLevelPath(), m_Character->GetGridX(), m_Character->GetGridY() };
            
            m_Map->LoadGeneratedLevel("GENERATED_CAVE", generated.ground, generated.props);
            m_Character->SetGridPosition(generated.spawnX, generated.spawnY);
            m_Map->WarpTo(generated.spawnX, generated.spawnY);
        } 
        else {
            m_Map->LoadLevel(dest.levelPath); 
            m_Character->SetGridPosition(dest.spawnX, dest.spawnY);
            m_Map->WarpTo(dest.spawnX, dest.spawnY);
        }
    }
    m_Character->StopMoving();
    m_Character->ClearDoorFlag(); 
}

void App::HandleOverworldEncounters() {
    static int lastGridX = -1;
    static int lastGridY = -1;
    int currentX = m_Character->GetGridX();
    int currentY = m_Character->GetGridY();

    if (!m_Character->IsMoving() && (currentX != lastGridX || currentY != lastGridY)) {
        lastGridX = currentX;
        lastGridY = currentY;

        int currentProp = m_Map->GetPropType(currentX, currentY);
        if (currentProp == GameConfig::PROP_TALLGRASS) {
            if (rand() % 100 < 10) { 
                m_CurrentState = State::BATTLE;
                m_Character->SetVisible(false);
                m_Map->SetVisible(false); 
                
                auto wildPokemon = GenerateWildPokemon(m_Map->GetCurrentLevelPath()); 
                m_BattleUI->Show(m_Character->GetParty(), wildPokemon);
            }
        } 
    }
}

// ==========================================
// SYSTEM SHORTCUT / MENU ASSISTANCE HELPERS
// ==========================================

void App::HandleGlobalShortcuts() {
    if (Util::Input::IsKeyDown(Util::Keycode::I)) {
        if (m_CurrentState == State::UPDATE) {
            OpenStartMenu();
        }
        else if (m_CurrentState == State::START_MENU) {
            CloseAllMenus();
        } 
    }

    if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        if (m_CurrentState == State::START_MENU) {
            CloseAllMenus();
        } 
        else if (m_CurrentState == State::POKEMON_MENU || m_CurrentState == State::INVENTORY_MENU) {
            ReturnToStartMenu();
        }
    }
}

void App::OpenStartMenu() {
    m_StartMenu->SetVisible(true);
    m_CurrentState = State::START_MENU;
}

void App::CloseAllMenus() {
    m_StartMenu->SetVisible(false);
    if (m_PokemonMenu) m_PokemonMenu->Hide();
    if (m_InventoryMenu) m_InventoryMenu->Hide();
    m_CurrentState = State::UPDATE;
}

void App::ReturnToStartMenu() {
    if (m_PokemonMenu) m_PokemonMenu->Hide();
    if (m_InventoryMenu) m_InventoryMenu->Hide();
    m_StartMenu->SetVisible(true);
    m_CurrentState = State::START_MENU;
}

std::shared_ptr<Pokemon> App::GenerateWildPokemon(const std::string& mapPath) {
    auto it = RandomEncounters::MapEncounters.find(mapPath);
    if (it == RandomEncounters::MapEncounters.end()) {
        return PokemonDatabase::CreatePokemon("Rattata", 2); 
    }

    const auto& encounterList = it->second;
    int totalWeight = 0;
    for (const auto& entry : encounterList) {
        totalWeight += entry.weight;
    }

    int roll = rand() % totalWeight;
    int currentWeight = 0;
    for (const auto& entry : encounterList) {
        currentWeight += entry.weight;
        if (roll < currentWeight) {
            int levelRange = (entry.maxLevel - entry.minLevel) + 1;
            int randomLevel = entry.minLevel + (rand() % levelRange);
            return PokemonDatabase::CreatePokemon(entry.speciesName, randomLevel);
        }
    }
    return PokemonDatabase::CreatePokemon("Rattata", 2);
}