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

void App::Start() {
    LOG_TRACE("Start");
    
    // ==========================================
    // 1. INITIALIZE ARCHITECTURE
    // ==========================================
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
    
    // Use the Singleton to load the JSON directly!
    try {
        AnimationLibrary::Get().LoadFromJson(RES + "/data/TEST.json"); 
        
    } catch (const std::exception& e) {
        LOG_ERROR("JSON Error: {}", e.what());
    }

    m_InventoryMenu = std::make_shared<InventoryMenu>(m_Renderer);
    m_BattleUI->SetInventoryMenu(m_InventoryMenu);

    m_BattleUI->SetPlayer(m_Character);
 

    

    // ==========================================
    // 2. LOAD GAME OR START FRESH
    // ==========================================
    SaveSystem::GameState loadedState;
    m_Map->LoadConnections(RESOURCE_DIR "/maps/connections.txt");
    if (SaveSystem::LoadGame(loadedState)) {
        GameConfig::LootedItems = loadedState.lootedItems;
        m_Map->LoadLevel(loadedState.mapPath);
        
        m_Character->SetGridPosition(loadedState.gridX, loadedState.gridY);
        m_Map->WarpTo(loadedState.gridX, loadedState.gridY);
        m_Character->SetDirection(static_cast<Character::Direction>(loadedState.direction));
        m_Character->SetInventory(loadedState.inventory);
        //load pokemon party from savefile
        for (const auto& pkmn : loadedState.party) {
            m_Character->AddPokemon(pkmn);
        }
    } else {
        GameConfig::LootedItems.clear(); 
        m_Map->LoadLevel(MAP_DIR + "NTUT"); 
        m_Character->SetGridPosition(14, 53); 
        m_Map->WarpTo(14, 53);
        // Give starter for testing
        auto starter = std::make_shared<Pokemon>(
        "Charmander", 5,
        PokemonType::FIRE, PokemonType::NONE,
        39, 52, 43, 60, 50, 65, 45
        );
        starter->LearnMove("Scratch");
        starter->LearnMove("Growl");
        m_Character->AddPokemon(starter);
    }
    
    // ==========================================
    // 3. BUILD THE DIALOGUE UI
    // ==========================================
    m_DialogueBoxUI = std::make_shared<Util::GameObject>();
    auto boxImage = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/BWTextBox3.png");    
    m_DialogueBoxUI->SetDrawable(boxImage);
    m_DialogueBoxUI->SetZIndex(9.0f); 
    m_DialogueBoxUI->SetVisible(false);
    m_DialogueBoxUI->m_Transform.scale = {1.0f, 1.0f};          
    m_DialogueBoxUI->m_Transform.translation = {0.0f, -288.0f};

    m_DialogueUI = std::make_shared<Util::GameObject>();
    m_DialogueText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/micross.ttf", 
        24,                              
        "...",                           
        Util::Color(50, 50, 50) 
    );
    m_DialogueUI->SetDrawable(m_DialogueText);
    m_DialogueUI->SetZIndex(10.0f);  
    m_DialogueUI->SetVisible(false);
    m_DialogueUI->m_Transform.translation = {-600.0f, -260.0f};

    // ==========================================
    // 4. BUILD THE INVENTORY UI
    // ==========================================
    m_InventoryMenu = std::make_shared<InventoryMenu>(m_Renderer);
    
    // ==========================================
    // 5. BUILD THE START MENU UI
    // ==========================================
    m_StartMenu = std::make_shared<StartMenu>(m_Renderer);
    
    m_Renderer->AddChild(m_DialogueBoxUI);
    m_Renderer->AddChild(m_DialogueUI);
    // Note: InventoryBoxUI and InventoryTextUI are no longer added here! 
    // InventoryMenu handles its own rendering registration internally.

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    // 1. HARD QUIT (Window X button)
    if (Util::Input::IfExit()) {
        m_CurrentState = State::END;
        return;
    }

    // 2. INTERCEPT GLOBAL SHORTCUTS
    HandleGlobalShortcuts();

    // 3. OPTIONAL QUICKSAVE ('F' Key)
    if (Util::Input::IsKeyDown(Util::Keycode::F) && m_CurrentState == State::UPDATE) {
        SaveSystem::GameState current;
        current.mapPath = m_Map->GetCurrentLevelPath(); 
        current.gridX = m_Character->GetGridX();
        current.gridY = m_Character->GetGridY();
        current.direction = static_cast<int>(m_Character->GetFacingDirection());
        current.inventory = m_Character->GetInventory();
        current.lootedItems = GameConfig::LootedItems;
        current.party = m_Character->GetParty();
        SaveSystem::SaveGame(current);
        LOG_TRACE("Quick Saved!");
    }
    // ==========================================
    // 4. MAIN STATE MACHINE
    // ==========================================
    switch (m_CurrentState) {
        case State::START:
            // Do nothing for now
            break;

        case State::DIALOGUE: {
            if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
                if (!m_CurrentDialogueLines.empty() && m_CurrentDialogueIndex < m_CurrentDialogueLines.size() - 1) {
                    // Still reading text, go to next line
                    m_CurrentDialogueIndex++;
                    m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);
                    
                    // X-SHIFT ALIGNMENT
                    float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
                    m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth; 
                } 
                else {
                    // --- DIALOGUE HAS ENDED ---
                    // 1. Hide the UI
                    m_DialogueBoxUI->SetVisible(false);
                    m_DialogueUI->SetVisible(false);

                    // 2. Determine what happens next based on the Active NPC
                    if (m_ActiveNPC && !m_ActiveNPC->GetInteractFlag().empty()) {
                        GameFlags::Set(m_ActiveNPC->GetInteractFlag(), true);

                        NPCAction action = m_ActiveNPC->GetActionType();
                        std::string data = m_ActiveNPC->GetActionData();

                        if (action == NPCAction::SHOP) {
                            //LOG_INFO("TEST SUCCESS: Opening Shop with data: {}", data);
                            
                            m_CurrentState = State::UPDATE; // Switch to State::SHOP later!
                            // m_ShopUI->Show(data);
                        }
                        else if (action == NPCAction::HEAL) {
                            //LOG_INFO("TEST SUCCESS: Healing");
                            auto party = m_Character->GetParty();
                            
                            // 2. Loop through every Pokemon in the party
                            for (auto& pokemon : party) {
                                if (pokemon) { // Safety null-check
                                    // Restore HP to maximum
                                    pokemon->SetCurrentHP(pokemon->GetMaxHP());
                                    
                                    // --- FUTURE PROOFING ---
                                    // If you eventually add Status Conditions (Paralysis, Poison) 
                                    // or Move PP (Power Points), you should reset them here too!
                                    // pokemon->SetStatus("NONE"); 
                                    // pokemon->RestoreAllPP();
                                }
                            }
                            // Trigger your heal logic here
                            m_CurrentState = State::UPDATE;
                        }
                        else if (action == NPCAction::BATTLE) {
                            
                            m_Character->SetVisible(false);
                            m_Map->SetVisible(false);
                            m_BattleUI->StartTrainerBattle(
                                m_Character->GetParty(), 
                                m_ActiveNPC->GetParty()
                            );
                            m_CurrentState = State::BATTLE;
                            // Setup your battle here using the 'data' string
                        }
                        else if (action == NPCAction::GIVE_ITEM) {
                            if (!data.empty()) {
                                ItemCategory category = m_ActiveNPC->GetActionCategory();
                                m_Character->AddItem(data, category, 1);
                                LOG_INFO("Player received: {} (qty: 1)", data);
                            }
                            m_CurrentState = State::UPDATE;
                        }
                        else {
                            // NONE or GIVE_ITEM falls back to the overworld
                            m_CurrentState = State::UPDATE;
                        }

                        // We are done with the NPC, clear the pointer!
                        m_ActiveNPC = nullptr;
                    } 
                    else {
                        // Fallback: This was just a sign or an item on the ground
                        m_CurrentState = State::UPDATE;
                    }
                }
            }
            break;
        }

        case State::START_MENU: {
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
                // Trigger the 'F' key save logic manually here
                SaveSystem::GameState current;
                current.mapPath = m_Map->GetCurrentLevelPath(); 
                current.gridX = m_Character->GetGridX();
                current.gridY = m_Character->GetGridY();
                current.direction = static_cast<int>(m_Character->GetFacingDirection());
                current.inventory = m_Character->GetInventory();
                current.lootedItems = GameConfig::LootedItems;
                current.party = m_Character->GetParty();
                SaveSystem::SaveGame(current);
                
                CloseAllMenus(); // Close menus after saving
                LOG_TRACE("Game Saved Successfully!");
            } 
            else if (selection == StartMenu::MenuOption::EXIT) {
                LOG_TRACE("Selected: EXIT");
                m_CurrentState = State::END; 
            }
            break;
        }

        case State::INVENTORY_MENU: {
            m_InventoryMenu->Update();
            break; // Input mapping handles going back now!
        }

        case State::POKEMON_MENU: {
            // 1. Let the menu handle UP/DOWN scrolling
            // If Update() returns true, the player pressed 'X' or 'Escape' to back out
            if (m_PokemonMenu->Update()) {
                LOG_TRACE("Exited POKEMON menu");
                m_PokemonMenu->Hide();
                m_StartMenu->SetVisible(true);
                m_CurrentState = State::START_MENU;
                m_SwapIndex = -1; // Reset the swap tracker just in case!
                break;
            }

            // 2. Handle 'Z' to select and swap Pokemon
            if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
                int selectedIdx = m_PokemonMenu->GetSelectedIndex();
                
                if (m_SwapIndex == -1) {
                    // FIRST CLICK: Remember this index!
                    m_SwapIndex = selectedIdx; 
                    LOG_TRACE("Selected Pokemon at index %d to swap.", m_SwapIndex);
                    
                    // (Optional: You could change the text color or add a little arrow graphic 
                    // in PokemonMenu.cpp here to visually show it's selected!)
                    
                } else {
                    // SECOND CLICK: Swap them!
                    LOG_TRACE("Swapping index %d with index %d.", m_SwapIndex, selectedIdx);
                    m_Character->SwapPokemon(m_SwapIndex, selectedIdx);
                    
                    // Reset the tracker so they can swap again
                    m_SwapIndex = -1; 
                    
                    // Refresh the visual text so the new order shows up immediately
                    m_PokemonMenu->Show(m_Character->GetParty()); 
                }
            }
            break;
        }

        case State::UPDATE: {
            // A. INTERACTION ('Z' KEY)
            if (Util::Input::IsKeyDown(Util::Keycode::Z) && !m_Character->IsMoving()) {
                int checkX = m_Character->GetGridX();
                int checkY = m_Character->GetGridY();

                Character::Direction facing = m_Character->GetFacingDirection();
                if (facing == Character::Direction::UP)    checkY -= 1;
                if (facing == Character::Direction::DOWN)  checkY += 1;
                if (facing == Character::Direction::LEFT)  checkX -= 1;
                if (facing == Character::Direction::RIGHT) checkX += 1;

// 1. Initial check directly in front of the player
                auto targetNPC = m_Map->GetNPCAt(checkX, checkY);
                
                // 2. --- NEW: THE COUNTER LOGIC ---
                if (!targetNPC) {
                    // Get the ID of the tile from your props layer
                    int propID = m_Map->GetPropType(checkX, checkY); // <-- Adjust this method name!
                    
                    // Is it the desk? (IDs 11 and 102 based on your CSV)
                    if (propID == 24) {
                        
                        float extendedX = checkX;
                        float extendedY = checkY;
                        
                        // Look one tile further in the same direction
                        // Note: Change the + and - below if your grid's Y-axis is inverted!
                        Character::Direction playerDir = m_Character->GetFacingDirection();
                        if (playerDir == Character::Direction::UP)         extendedY -= 1.0f; 
                        else if (playerDir == Character::Direction::DOWN)  extendedY += 1.0f;
                        else if (playerDir == Character::Direction::LEFT)  extendedX -= 1.0f;
                        else if (playerDir == Character::Direction::RIGHT) extendedX += 1.0f;

                        // Try fetching the NPC again at this new extended coordinate!
                        targetNPC = m_Map->GetNPCAt(extendedX, extendedY);
                    }
                }
                // ---------------------------------

                // 3. Your existing dialogue logic! (Executes if an NPC was found directly OR behind a desk)
                if (targetNPC) {
                    m_ActiveNPC = targetNPC;
                    m_Character->StopMoving();
                    m_CurrentState = State::DIALOGUE; // Replaces m_IsInDialogue
                    m_DialogueBoxUI->SetVisible(true);
                    m_DialogueUI->SetVisible(true);

                    // NPC Face Player
                    Character::Direction playerDir = m_Character->GetFacingDirection();
                    if (playerDir == Character::Direction::UP)         targetNPC->SetDirection(Character::Direction::DOWN);
                    else if (playerDir == Character::Direction::DOWN)  targetNPC->SetDirection(Character::Direction::UP);
                    else if (playerDir == Character::Direction::LEFT)  targetNPC->SetDirection(Character::Direction::RIGHT);
                    else if (playerDir == Character::Direction::RIGHT) targetNPC->SetDirection(Character::Direction::LEFT);

                    m_CurrentDialogueLines = targetNPC->Interact();
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

            glm::vec2 movement = m_Character->Update(m_Map);
            m_Map->Move(-movement.x, -movement.y);

            // C. DOORS AND WARPING
            if (m_Character->HasHitDoor()) {

                std::string doorKey = m_Map->GetCurrentLevelPath() + "_" + std::to_string(m_Character->GetGridX()) + "_" + std::to_string(m_Character->GetGridY());
                printf("Attempting to find Door Key: [%s]\n", doorKey.c_str());
                auto it = GameConfig::DoorRouting.find(doorKey);
                
                if (it != GameConfig::DoorRouting.end()) {
                    GameConfig::WarpDestination dest = it->second;
                    printf("Door found! Destination is: [%s]\n", dest.levelPath.c_str());
                    // --- NEW: Intercept procedural generation triggers ---
                    if (dest.levelPath.find("GENERATED_CAVE") != std::string::npos) {
                        printf("GENERATED CAVE LOGIC TRIGGERED!\n");
                        auto generated = MapGenerator::GenerateCave(40, 40);
                        
                        // Register the exit door to lead back to where the player just came from
                        std::string exitKey = "GENERATED_CAVE_" 
                                            + std::to_string(generated.spawnX) + "_" 
                                            + std::to_string(generated.spawnY);
                        GameConfig::DoorRouting[exitKey] = { m_Map->GetCurrentLevelPath(), 
                                                             m_Character->GetGridX(), 
                                                             m_Character->GetGridY() };
                        
                        m_Map->LoadGeneratedLevel("GENERATED_CAVE", generated.ground, generated.props);
                        m_Character->SetGridPosition(generated.spawnX, generated.spawnY);
                        m_Map->WarpTo(generated.spawnX, generated.spawnY);
                    } 
                    // --- EXISTING: Standard CSV level loading ---
                    else {
                        m_Map->LoadLevel(dest.levelPath); 
                        m_Character->SetGridPosition(dest.spawnX, dest.spawnY);
                        m_Map->WarpTo(dest.spawnX, dest.spawnY);
                    }

                    // Shared cleanup for both warp types
                    m_Character->StopMoving();
                    m_Character->ClearDoorFlag(); 

                } else {
                    // No door routing found for this tile
                    m_Character->ClearDoorFlag(); 
                }
            }
            // Check for encounters right after the player finishes moving to a new grid tile
            // D. ENCOUNTERS (Bulletproof Version)
            static int lastGridX = -1;
            static int lastGridY = -1;
            int currentX = m_Character->GetGridX();
            int currentY = m_Character->GetGridY();

            // If player is standing still, but their grid position changed since we last checked:
            if (!m_Character->IsMoving() && (currentX != lastGridX || currentY != lastGridY)) {
                
                // Update the tracker so we don't check this same tile twice
                lastGridX = currentX;
                lastGridY = currentY;

                int currentProp = m_Map->GetPropType(currentX, currentY);
                
                //LOG_INFO("STEPPED ON NEW TILE: ({}, {}) | PROP ID: {}", currentX, currentY, currentProp);

                if (currentProp == GameConfig::PROP_TALLGRASS) {
                    if (rand() % 100 < 10) { // Still keeping at 100% chance for testing!
                        
                        // 1. Tell the switch statement to go to State::BATTLE on the NEXT frame
                        m_CurrentState = State::BATTLE;
                        
                        // 2. Hide Overworld
                        m_Character->SetVisible(false);
                        m_Map->SetVisible(false); 
                        
                        // 3. Setup the battle UI (Do NOT put Update() or ESCAPE here!)
                        auto wildPokemon = GenerateWildPokemon(m_Map->GetCurrentLevelPath()); 
                        m_BattleUI->Show(m_Character->GetParty(), wildPokemon);
                    }
                } 
            }
          break; // <-- This break ends case State::UPDATE:
        }

        // ==========================================
        // ADD THIS BRAND NEW CASE!
        // ==========================================
        case State::BATTLE: {
            // This runs every single frame while you are in a battle
            m_BattleUI->Update();
            if (m_BattleUI->IsBattleOver()) {
                
                // (Note: m_BattleUI->Hide() is now automatically called inside 
                // BattleUI::Update() when the battle ends, so we don't need to call it here)
                
                // 3. Bring the Overworld back!
                m_Map->SetVisible(true);
                m_Character->SetVisible(true);
                
                // 4. Return to the Overworld state
                m_CurrentState = State::UPDATE;
            }
            // TEMP: Press ESCAPE to run away and return to the overworld
            if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
                m_BattleUI->Hide();
                m_Map->SetVisible(true);
                m_Character->SetVisible(true);
                
                // Go back to the overworld logic
                m_CurrentState = State::UPDATE;
            }
            break; // <-- Ends case State::BATTLE:
        }
        
        case State::END:
            break;
    }

    // ==========================================
    // 5. RENDER PHASE
    // ==========================================
    m_Map->Update();
    m_Renderer->Update();
}

void App::HandleGlobalShortcuts() {
    // Universal Menu Open (I)
    if (Util::Input::IsKeyDown(Util::Keycode::I)) {
        if (m_CurrentState == State::UPDATE) {
            OpenStartMenu();
        }
        else if (m_CurrentState == State::START_MENU) {
            // In Start Menu -> Close entirely, back to gameplay
            CloseAllMenus();
        } 
    }

    // Universal Back/Close (X or ESC)
    if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        
        if (m_CurrentState == State::START_MENU) {
            // In Start Menu -> Close entirely, back to gameplay
            CloseAllMenus();
        } 
        else if (m_CurrentState == State::POKEMON_MENU || m_CurrentState == State::INVENTORY_MENU) {
            // In a Submenu -> Jump back to Start Menu
            ReturnToStartMenu();
        }
        // (If you add a PC menu or Shop menu later, just add them to the else-if above!)
    }
}

void App::OpenStartMenu() {
    m_StartMenu->SetVisible(true);
    m_CurrentState = State::START_MENU;
}

void App::CloseAllMenus() {
    m_StartMenu->SetVisible(false);
    if (m_PokemonMenu) m_PokemonMenu->Hide();
    if (m_InventoryMenu) m_InventoryMenu->Hide(); // Uncomment or adjust based on your Inventory API

    m_CurrentState = State::UPDATE;
}

void App::ReturnToStartMenu() {
    // 1. Hide all submenus
    if (m_PokemonMenu) m_PokemonMenu->Hide();
    if (m_InventoryMenu) m_InventoryMenu->Hide();

    // 2. Show the Start Menu
    m_StartMenu->SetVisible(true);
    m_CurrentState = State::START_MENU;
}


std::shared_ptr<Pokemon> App::GenerateWildPokemon(const std::string& mapPath) {
    // 1. Check if the current map actually has an encounter table
    auto it = RandomEncounters::MapEncounters.find(mapPath);
    if (it == RandomEncounters::MapEncounters.end()) {
        // Fallback just in case you step in grass on a map with no table
        return PokemonDatabase::CreatePokemon("Rattata", 2); 
    }

    const auto& encounterList = it->second;

    // 2. Calculate the total weight of all Pokémon in this area
    int totalWeight = 0;
    for (const auto& entry : encounterList) {
        totalWeight += entry.weight;
    }

    // 3. Roll a random number between 0 and totalWeight - 1
    int roll = rand() % totalWeight;
    
    // 4. Figure out which Pokémon we rolled!
    int currentWeight = 0;
    for (const auto& entry : encounterList) {
        currentWeight += entry.weight;
        
        if (roll < currentWeight) {
            // WE FOUND OUR POKEMON! 
            // Calculate a random level between minLevel and maxLevel
            int levelRange = (entry.maxLevel - entry.minLevel) + 1;
            int randomLevel = entry.minLevel + (rand() % levelRange);
            
            // Ask the Database to build it!
            return PokemonDatabase::CreatePokemon(entry.speciesName, randomLevel);
        }
    }

    // Fallback if the math somehow fails
    return PokemonDatabase::CreatePokemon("Rattata", 2);
}
void App::End() {
    LOG_TRACE("End");
}