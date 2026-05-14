#include <algorithm>
#include "Map.hpp"
#include "Prop.hpp"
#include "GameConfig.hpp" 
#include "Item.hpp"
#include <fstream>  
#include <sstream>  
#include <iostream> 
#include "TrainerDatabase.hpp"
#include "ResourceManager.hpp"

//paths to assets
const std::string RES        = std::string(RESOURCE_DIR);
const std::string TILE_DIR   = RES + "/tiles/";
const std::string MAP_DIR    = RES + "/maps/";
const std::string PROP_DIR   = RES + "/props/";
const std::string NPC_DIR    = RES + "/npcs/";
const std::string DIALOGUE_DIR = RES + "/dialogue/";

// Half-screen dimensions — match your actual window size
static constexpr float HALF_W = 640.0f;
static constexpr float HALF_H = 360.0f;

Map::Map() { 
    InitTileRegistry();
    InitNPCRegistry();
    InitPropRegistry();
    InitItemRegistry();
}

// ─────────────────────────────────────────────
//  REGISTRIES
// ─────────────────────────────────────────────

void Map::InitTileRegistry() {
    // ID = { texture, zIndex, yOffset, isWalkable }
    m_TileRegistry[GameConfig::TILE_GRASS]         = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Grass1.png"),        0.0f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_WATER_SOLID]   = { nullptr,                                                                0.0f, 0.0f, false };
    m_TileRegistry[GameConfig::TILE_DIRT]          = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Dirt1.png"),         0.0f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_SAND]          = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Sand.png"),          0.0f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_CONCRETE]      = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Concrete.png"),      0.0f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_DOOR]          = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Dirt1.png"),         0.0f, 0.0f, false };
    m_TileRegistry[GameConfig::TILE_PC_FLOOR]      = { ResourceManager::GetImageStore().Get(TILE_DIR + "/PCFloorTile.png"),   0.0f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_PM_FLOOR]      = { ResourceManager::GetImageStore().Get(TILE_DIR + "/PokeMartTile.png"),  0.0f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_PC_WALL]       = { ResourceManager::GetImageStore().Get(TILE_DIR + "/PCWall1.png"),       0.1f, 0.0f, false };
    m_TileRegistry[GameConfig::TILE_INSIDE_CHURCH] = { ResourceManager::GetImageStore().Get(TILE_DIR + "/church_inside.png"),0.1f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_GRAVEL]        = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Gravel.png"),        0.1f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_ROAD]          = { ResourceManager::GetImageStore().Get(TILE_DIR + "/Road.png"),          0.1f, 0.0f, true  };
    m_TileRegistry[GameConfig::TILE_CALM_WATER]    = { ResourceManager::GetImageStore().Get(TILE_DIR + "/CalmWater.png"),     0.1f, 0.0f, false };
    m_TileRegistry[GameConfig::TILE_RED_BRICK]    =  { ResourceManager::GetImageStore().Get(TILE_DIR + "/RedBrick.png"),     0.1f, 0.0f, true };
    m_TileRegistry[GameConfig::TILE_GREY_BRICK]    = { ResourceManager::GetImageStore().Get(TILE_DIR + "/GreyBrick.png"),     0.1f, 0.0f, true };

}

void Map::InitNPCRegistry() {
    // ID = { spritePath, visualOffsetY, zIndex, dynamicZ, dialoguePath, NPCAction, actionData }
    m_NPCRegistry[GameConfig::NPC_NURSE]   = NPCProperties{ NPC_DIR + "Nurse",      12.0f, 0.2f, false, DIALOGUE_DIR + "nurse.txt", NPCAction::HEAL,   ""            };
    m_NPCRegistry[GameConfig::NPC_TA1]     = NPCProperties{ NPC_DIR + "TA0",       -12.0f, 0.5f, true,  DIALOGUE_DIR + "ta.txt",    NPCAction::BATTLE, "Trainer_TA"  };
    m_NPCRegistry[GameConfig::SHOP_KEEPER] = NPCProperties{ NPC_DIR + "ShopKeeper",-12.0f, 0.5f, true,  DIALOGUE_DIR + "ta.txt",    NPCAction::SHOP,   "Mart_Potions"};
}

void Map::InitPropRegistry() {
    // FORMAT: { {"Frame1", ...}, zIndex, dynamicZ, isWalkable, offsetX, offsetY }

    auto generateFrames = [](const std::string& prefix, int frameCount) {
        std::vector<std::string> frames;
        for (int i = 1; i <= frameCount; ++i) {
            frames.push_back(prefix + std::to_string(i) + ".png");
        }
        return frames;
    };

    // Invisible/logic props (no textures)
    m_PropRegistry[GameConfig::PROP_INVISIBLE_DOOR]    = { {}, 0.0f, false, false, 0.0f,  0.0f };
    m_PropRegistry[GameConfig::PROP_INVISIBLE_WALL]    = { {}, 0.0f, false, false, 0.0f,  0.0f };
    m_PropRegistry[GameConfig::PROP_INTERACTABLE_WALL] = { {}, 0.0f, false, false, 0.0f,  0.0f };

    // Buildings
    m_PropRegistry[GameConfig::PROP_POKECENTER]          = { {PROP_DIR + "/PokeCentre.png"},        0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_POKEMONGYM]          = { {PROP_DIR + "/PokemonGym.png"},         0.8f, true,  false, 0.0f,  22.0f };
    m_PropRegistry[GameConfig::PROP_CHURCH]              = { {PROP_DIR + "/Church.png"},             0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::WOODEN_HOUSE]             = { {PROP_DIR + "/wood_house.png"},         0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::POKEMART]                 = { {PROP_DIR + "/PokeMart.png"},           0.8f, true,  false, 24.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING1]      = { {PROP_DIR + "/NTUT_Building1.png"},     0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_MOSS_BUILDING]  = { {PROP_DIR + "/ntut_build_1.png"}, 0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING2]      = { {PROP_DIR + "/Building2.png"},          0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING3]      = { {PROP_DIR + "/Building3.png"},          0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING4]      = { {PROP_DIR + "/Building4.png"},          0.8f, true,  false, 24.0f,  0.0f  };
    //558
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING5]      = { {PROP_DIR + "/Building5.png"},          0.8f, true,  false, -16.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING6]      = { {PROP_DIR + "/Building6.png"},          0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING7]      = { {PROP_DIR + "/ntut_build_2.png"},       0.8f, true,  false, 0.0f,  0.0f  };    
    m_PropRegistry[GameConfig::PROP_NTUT_BUILDING8]      = { {PROP_DIR + "/ntut_build_3.png"},       0.8f, true,  false, 0.0f,  0.0f  };    
    m_PropRegistry[GameConfig::PROP_NTUT_TECH_BUILDING2]      = { {PROP_DIR + "/TechBuilding12.png"},          0.8f, true,  false, 0.0f,  0.0f};
    m_PropRegistry[GameConfig::PROP_NTUT_CAFETERIA_BUILDING]      = { {PROP_DIR + "/CafeteriaBuilding.png"},          0.8f, true,  false, 0.0f,  0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_TECH_BUILDING]      = { {PROP_DIR + "/TechBuilding.png"},          0.8f, true,  false, 0.0f,  -144.0f};



    // Checkpoints / gates
    m_PropRegistry[GameConfig::PROP_CHECKPOINT]    = { {PROP_DIR + "/Checkpoint2.png"}, 0.8f, true,  false, 0.0f, 96.0f };
    m_PropRegistry[GameConfig::PROP_CHECKPOINT2]   = { {PROP_DIR + "/Checkpoint3.png"}, 0.8f, true,  false, 0.0f, 96.0f };
    m_PropRegistry[GameConfig::PROP_GATE_TOP]      = { {PROP_DIR + "/GateTopEnd.png"},  0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_GATE_MIDDLE]   = { {PROP_DIR + "/GateMiddle.png"},  0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_GATE_MIDDLE2]  = { {PROP_DIR + "/GateMiddle2.png"}, 0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_GATE_MIDDLE3]  = { {PROP_DIR + "/GateMiddle3.png"}, 0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_GATE_END]      = { {PROP_DIR + "/GateBotEnd.png"},  0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_GATE_TOP2]     = { {PROP_DIR + "/GateTopEnd2.png"}, 0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_GATE_END2]     = { {PROP_DIR + "/GateBotEnd2.png"}, 0.1f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_STAIRS_NORTH]     = { {PROP_DIR + "/Stairs_North.png"}, 0.4f, false,  true, 0.0f, 0.0f  };

    //Decoration
    //m_PropRegistry[GameConfig::PROP_NTUT_SCREEN]     = { {PROP_DIR + "/NTUT_Screen.png"}, 0.7f, true,  true, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_NTUT_BALL_STATUE]     = { {PROP_DIR + "/NTUT_Ball.png"}, 1.0f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_TRUCK1]     = { {PROP_DIR + "/Truck.png"}, 0.8f, true,  false, 0.0f, 10.0f  };
    m_PropRegistry[GameConfig::PROP_UMBRELLA_STAND]     = { {PROP_DIR + "/UmbrellaStand.png"}, 0.4f, true,  false, 0.0f, 0.0f  };
    m_PropRegistry[GameConfig::PROP_WHITE_PILLAR]     = { {PROP_DIR + "/WhitePillar.png"}, 0.4f, true,  false, 0.0f, 0.0f  };



    // Interior props
    m_PropRegistry[GameConfig::PROP_DOORMAT]       = { {PROP_DIR + "/PC_doormat.png"},   0.1f, false, true,  0.0f,  -20.0f };
    m_PropRegistry[GameConfig::PROP_PC_DESK]       = { {PROP_DIR + "/PCDesk1.png"},      0.4f, false, false, 0.0f,  0.0f   };
    m_PropRegistry[GameConfig::PROP_PM_DESK]       = { {PROP_DIR + "/PokeMartDesk.png"}, 0.4f, false, false, 24.0f, 0.0f   };
    m_PropRegistry[GameConfig::PROP_PC_WALL_LEFT]  = { {PROP_DIR + "/PCWall2.png"},      0.3f, false, false, 0.0f,  0.0f   };
    m_PropRegistry[GameConfig::PROP_PC_WALL_RIGHT] = { {PROP_DIR + "/PCWall3.png"},      0.3f, false, false, 0.0f,  0.0f   };

    // Nature props
    m_PropRegistry[GameConfig::PROP_TREE]       = { {PROP_DIR + "/Tree.png"},      0.8f, true, true,  20.0f, -16.0f };
    m_PropRegistry[GameConfig::PROP_PALM_TREE]       = { {PROP_DIR + "/PalmTree.png"},      0.8f, true, true,  20.0f, -16.0f };
    m_PropRegistry[GameConfig::PROP_SMALL_TREE] = { {PROP_DIR + "/SmallTree.png"}, 0.8f, true, false, 0.0f,  0.0f   };
    m_PropRegistry[GameConfig::PROP_TALL_TREE] = { {PROP_DIR + "/TallTree.png"}, 0.9f, true, false, 0.0f,  0.0f   };

    m_PropRegistry[GameConfig::PROP_LAMP_POST]  = { {PROP_DIR + "/LampPost.png"},  0.8f, true, false, 0.0f,  0.0f   };

    // Log obstacles
    m_PropRegistry[GameConfig::PROP_LOG_DOWN1]  = { {PROP_DIR + "/wallstone_5.png"},  0.8f, true, false, 0.0f, 0.0f };
    m_PropRegistry[GameConfig::PROP_LOG_DOWN2]  = { {PROP_DIR + "/wallstone_3.png"},  0.8f, true, false, 0.0f, 0.0f };
    m_PropRegistry[GameConfig::PROP_LOG_DOWN3]  = { {PROP_DIR + "/wallstone_4.png"},  0.8f, true, false, 0.0f, 0.0f };
    m_PropRegistry[GameConfig::PROP_LOG_LEFT1]  = { {PROP_DIR + "/wallstone_2.png"},  0.8f, true, false, 0.0f, 0.0f };
    m_PropRegistry[GameConfig::PROP_LOG_LEFT2]  = { {PROP_DIR + "/wallstone_1.png"},  0.8f, true, false, 0.0f, 0.0f };

    m_PropRegistry[GameConfig::PROP_LOG_up1]    = { {PROP_DIR + "/wallstone_7.png"},  0.8f, true, false, 0.0f, 0.0f };
    m_PropRegistry[GameConfig::PROP_LOG_up2]    = { {PROP_DIR + "/wallstone_6.png"},  0.8f, true, false, 0.0f, 0.0f };
 
    // Animated doors
    m_PropRegistry[GameConfig::DOOR_OPENING_GYM] = { { PROP_DIR + "/door0.png", PROP_DIR + "/door1.png", PROP_DIR + "/door2.png", PROP_DIR + "/door3.png" }, 0.0f, true, true,  2.0f,  20.0f };
    m_PropRegistry[GameConfig::DOOR_OPENING_PC]  = { { PROP_DIR + "/door0.png", PROP_DIR + "/door1.png", PROP_DIR + "/door2.png", PROP_DIR + "/door3.png" }, 0.0f, true, true,  0.0f,  16.0f };
    m_PropRegistry[GameConfig::DOOR_OPENING_PM]  = { { PROP_DIR + "/door0.png", PROP_DIR + "/door1.png", PROP_DIR + "/door2.png", PROP_DIR + "/door3.png" }, 0.0f, true, true,  2.0f,  32.0f };

    // Tall grass (interactive)
    m_PropRegistry[GameConfig::PROP_TALLGRASS] = {
        { PROP_DIR + "/TallGrass2.png", PROP_DIR + "/TallGrass3.png", PROP_DIR + "/TallGrass4.png" },
        0.5f, true, true, 0.0f, 0.0f
    };

    // Animated signs / decorations
    m_PropRegistry[GameConfig::POKEMART_SIGN] = {
        { PROP_DIR + "/PokeMartSign1.png", PROP_DIR + "/PokeMartSign2.png",
          PROP_DIR + "/PokeMartSign3.png", PROP_DIR + "/PokeMartSign4.png" },
        0.9f, true, false, 16.0f, 32.0f,
        PropAnimMode::LOOP, 30
    };

    m_PropRegistry[GameConfig::PROP_FLOWER] = {
        { PROP_DIR + "/Flower1.png", PROP_DIR + "/Flower2.png", PROP_DIR + "/Flower3.png",
          PROP_DIR + "/Flower4.png", PROP_DIR + "/Flower5.png" },
        0.7f, true, true, 0.0f, 0.0f,
        PropAnimMode::LOOP, 45
    };

    m_PropRegistry[GameConfig::PROP_NTUT_SCREEN] = { 
        generateFrames(PROP_DIR + "NTUTScreen/NTUT_Screen-", 32), // Generates frames 1 through 32
        0.7f, true, true, 0.0f, 0.0f,
        PropAnimMode::LOOP, 10 // Added the animation mode and frame delay here!
        
    };

}

void Map::InitItemRegistry() {
    // ID = { texturePath, name, category, zIndex }
    m_ItemRegistry[GameConfig::ITEM_POTION]   = { PROP_DIR + "/PokeBall.png", "Potion",   ItemCategory::GENERAL,   0.5f };
    m_ItemRegistry[GameConfig::ITEM_POKEBALL] = { PROP_DIR + "/PokeBall.png", "Pokeball", ItemCategory::POKEBALLS, 0.5f };
}

// ─────────────────────────────────────────────
//  FILE LOADING
// ─────────────────────────────────────────────

std::vector<std::vector<int>> Map::LoadCSV(const std::string& filepath) {
    std::vector<std::vector<int>> data;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open map file at " << filepath << "!\n";
        return data;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> row;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            try {
                row.push_back(std::stoi(cell));
            } catch (const std::exception& e) {
                std::cerr << "Bad CSV value '" << cell << "' at row "
                          << data.size() << ": " << e.what() << "\n";
                row.push_back(0);
            }
        }
        data.push_back(row);
    }
    return data;
}

void Map::LoadConnections(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open connections file at " << filepath << "!\n";
        return;
    }

    const std::string BASE = std::string(RESOURCE_DIR) + "/maps/";
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line.rfind("//", 0) == 0) continue;

        std::istringstream ss(line);
        std::string srcMap, arrow, destMap;
        int srcX, srcY, dstX, dstY;

        if (ss >> srcMap >> srcX >> srcY >> arrow >> destMap >> dstX >> dstY) {
            std::string key = BASE + srcMap + "_" + std::to_string(srcX) + "_" + std::to_string(srcY);
            GameConfig::DoorRouting[key] = { BASE + destMap, dstX, dstY };
        }
    }
}

// ─────────────────────────────────────────────
//  LEVEL LOADING
// ─────────────────────────────────────────────

void Map::SpawnTilesAndProps() {
     // Water animation: one leader drives timing, all followers sync to it
    std::vector<std::string> waterPaths = {
        TILE_DIR + "/Water1.png", TILE_DIR + "/Water2.png", TILE_DIR + "/Water3.png",
        TILE_DIR + "/Water4.png", TILE_DIR + "/Water5.png", TILE_DIR + "/Water6.png",
        TILE_DIR + "/Water7.png"
    };
    m_LeaderWater   = std::make_shared<Util::Animation>(waterPaths, true,  500, true, 0);
    m_FollowerWater = std::make_shared<Util::Animation>(waterPaths, false, 500, true, 0);
    bool leaderAssigned = false;

    // ── PHASE 1: Ground tiles ──────────────────
    for (size_t y = 0; y < m_LevelData.size(); y++) {
        for (size_t x = 0; x < m_LevelData[y].size(); x++) {
            int tileID = m_LevelData[y][x];
            if (m_TileRegistry.count(tileID) == 0) continue;

            const TileProperties& props = m_TileRegistry[tileID];
            auto newTile = std::make_shared<Util::GameObject>();

            newTile->m_Transform.scale         = { GameConfig::SCALE, GameConfig::SCALE };
            newTile->m_Transform.translation.x = GameConfig::CAMERA_START_X + (x * GameConfig::EFFECTIVE_TILE_SIZE);
            newTile->m_Transform.translation.y = GameConfig::CAMERA_START_Y - (y * GameConfig::EFFECTIVE_TILE_SIZE) + props.yOffset;
            newTile->SetZIndex(props.zIndex);

            if (tileID == GameConfig::TILE_WATER_SOLID) {
                if (!leaderAssigned) { newTile->SetDrawable(m_LeaderWater); leaderAssigned = true; }
                else                 { newTile->SetDrawable(m_FollowerWater); }
            } else {
                newTile->SetDrawable(props.texture);
            }

            m_Tiles.push_back(newTile);
            m_TileVisible.push_back(true); // culling corrects this on first Update()
            AddToRenderer(newTile);
        }
    }

    // ── PHASE 2: Props, NPCs, Items ───────────
    if (m_PropData.empty()) return;
    
    for (size_t y = 0; y < m_PropData.size(); y++) {
        for (size_t x = 0; x < m_PropData[y].size(); x++) {
            int propID = m_PropData[y][x];
            if (propID <= 0) continue;
            float worldX = GameConfig::CAMERA_START_X + (x * GameConfig::EFFECTIVE_TILE_SIZE);
            float worldY = GameConfig::CAMERA_START_Y - (y * GameConfig::EFFECTIVE_TILE_SIZE);

            // NPCs
            if (m_NPCRegistry.count(propID) > 0) {
                const NPCProperties& npcProps = m_NPCRegistry[propID];
                auto npc = std::make_shared<NPC>(
                    worldX, worldY + npcProps.visualOffsetY * GameConfig::SCALE/3.0f,
                    npcProps.texturePath, npcProps.dialogueFilePath, "", ""
                );
                npc->SetGridPosition(x, y);
                npc->SetZIndex(npcProps.zIndex);
                npc->SetBaseZIndex(npcProps.zIndex);
                npc->SetDynamicZ(npcProps.dynamicZ);
                npc->SetAction(npcProps.actionType, npcProps.actionData);

                if (npcProps.actionType == NPCAction::BATTLE) {
                    auto loadedParty = TrainerDatabase::CreateTrainerParty(npcProps.actionData);
                    for (const auto& p : loadedParty) npc->GetParty().push_back(p);
                }

                m_NPCs.push_back(npc);
                AddToRenderer(npc);
            }

            // Props
            if (m_PropRegistry.count(propID) > 0) {
                const PropProperties& props = m_PropRegistry[propID];
                glm::vec2 spawnPos(
                worldX + (props.visualOffsetX * GameConfig::SCALE/3.0f), 
                worldY + (props.visualOffsetY * GameConfig::SCALE/3.0f)
                );
                auto prop = std::make_shared<Prop>(
                    props.texturePaths, spawnPos, GameConfig::SCALE, props.zIndex, x, y
                );
                prop->SetDynamicZ(props.dynamicZ);
                prop->SetZIndex(props.zIndex);
                prop->SetAnimMode(props.animMode, props.animFrameDelay);
                m_Props.push_back(prop);
                if (!props.texturePaths.empty()) AddToRenderer(prop);
            }

            // Items
            if (m_ItemRegistry.count(propID) > 0) {
                std::string uniqueID = m_CurrentLevelPath + "_" + std::to_string(x) + "_" + std::to_string(y);
                if (GameConfig::LootedItems.count(uniqueID) > 0) {
                    m_PropData[y][x] = 0; // already looted — clear so tile is walkable
                } else {
                    const ItemProperties& itemProps = m_ItemRegistry[propID];
                    if (!itemProps.texturePath.empty()) {
                        auto item = std::make_shared<Item>(
                            itemProps.texturePath, glm::vec2(worldX, worldY),
                            itemProps.name, itemProps.category, x, y
                        );
                        m_Items.push_back(item);
                        AddToRenderer(item);
                    }
                }
            }
        }
    }
}
void Map::LoadGeneratedLevel(const std::string& mapName, 
                             std::vector<std::vector<int>> groundData, 
                             std::vector<std::vector<int>> propData) {
    ClearMap();
    m_CurrentLevelPath = mapName;
    m_LevelData = std::move(groundData);
    m_PropData  = std::move(propData);
    
    if (m_LevelData.empty()) return;
    
    SpawnTilesAndProps();
}

// Map.cpp — refactor LoadLevel to call this:
void Map::LoadLevel(const std::string& mapName) {
    ClearMap();
    m_CurrentLevelPath = mapName;
    m_LevelData = LoadCSV(mapName + "_ground.csv");
    m_PropData  = LoadCSV(mapName + "_props.csv");
    if (m_LevelData.empty()) return;
    SpawnTilesAndProps();
}


void Map::Update() {
    // 1. Keep all water tiles in sync with the leader's frame
    if (m_LeaderWater && m_FollowerWater) {
        m_FollowerWater->SetCurrentFrame(m_LeaderWater->GetCurrentFrameIndex());
    }

    // 2. Tile culling — only render what's on screen
    // Tiles outside the viewport are hidden, saving the renderer from
    // Z-sorting thousands of invisible objects every frame.
    if (!m_Tiles.empty()) {
        const float margin = GameConfig::EFFECTIVE_TILE_SIZE * 2.0f;
        const float minX = -(HALF_W + margin);
        const float maxX =  (HALF_W + margin);
        const float minY = -(HALF_H + margin);
        const float maxY =  (HALF_H + margin);

        for (size_t i = 0; i < m_Tiles.size(); i++) {
            float tx = m_Tiles[i]->m_Transform.translation.x;
            float ty = m_Tiles[i]->m_Transform.translation.y;
            bool inView = tx > minX && tx < maxX && ty > minY && ty < maxY;

            // Only call SetVisible when state actually changes — avoids
            // redundant renderer notifications every frame
            if (inView != m_TileVisible[i]) {
                m_TileVisible[i] = inView;
                m_Tiles[i]->SetVisible(inView);
            }
        }
    }

        // 3. Prop culling + update
    // Props use a larger margin since large buildings extend well beyond
    // their anchor tile — EFFECTIVE_TILE_SIZE * 6 covers the biggest sprites
    const float propMargin = GameConfig::EFFECTIVE_TILE_SIZE * 6.0f;
    const float propMinX   = -(HALF_W + propMargin);
    const float propMaxX   =  (HALF_W + propMargin);
    const float propMinY   = -(HALF_H + propMargin);
    const float propMaxY   =  (HALF_H + propMargin);

    for (auto& prop : m_Props) {
        float px = prop->m_Transform.translation.x;
        float py = prop->m_Transform.translation.y;
        bool inView = px > propMinX && px < propMaxX &&
                      py > propMinY && py < propMaxY;

        prop->SetVisible(inView);

        // Only run animation/Z-sort logic for visible props —
        // invisible props don't need to animate or sort
        if (inView) prop->Update();
    }

    // 3. Props and NPCs (few enough to not need culling)
    for (auto& npc  : m_NPCs)  npc->Update(shared_from_this());
}

// ─────────────────────────────────────────────
//  MOVEMENT & CAMERA
// ─────────────────────────────────────────────

void Map::Move(float dx, float dy) {
    for (auto& tile : m_Tiles)  { tile->m_Transform.translation.x += dx; tile->m_Transform.translation.y += dy; }
    for (auto& prop : m_Props)  { prop->m_Transform.translation.x += dx; prop->m_Transform.translation.y += dy; }
    for (auto& npc  : m_NPCs)   { npc->m_Transform.translation.x  += dx; npc->m_Transform.translation.y  += dy; }
    for (auto& item : m_Items)  { item->m_Transform.translation.x += dx; item->m_Transform.translation.y += dy; }
}

void Map::WarpTo(int gridX, int gridY) {
    float shiftX = GameConfig::CAMERA_START_X + (gridX * GameConfig::EFFECTIVE_TILE_SIZE);
    float shiftY = GameConfig::CAMERA_START_Y - (gridY * GameConfig::EFFECTIVE_TILE_SIZE);
    Move(-shiftX, -shiftY);
}

// ─────────────────────────────────────────────
//  QUERIES
// ─────────────────────────────────────────────

#include <cstdio> // Make sure this is at the top of your file for printf

bool Map::IsWalkable(int x, int y) {
    // 1. Check Bounds
    if (x < 0 || x >= (int)m_LevelData[0].size() ||
        y < 0 || y >= (int)m_LevelData.size()) {
        printf("WALK FAILED: Tile (%d, %d) is out of bounds!\n", x, y);
        return false;
    }

    // 2. Check Ground Tile
    int tileID = m_LevelData[y][x];
    if (m_TileRegistry.count(tileID) == 0) {
        printf("WALK FAILED: Ground Tile ID %d is missing from registry!\n", tileID);
        return false;
    }
    if (!m_TileRegistry[tileID].isWalkable) {
        printf("WALK FAILED: Ground Tile ID %d is marked as solid!\n", tileID);
        return false;
    }

    // 3. Check Props and Items
    int propID = m_PropData[y][x];
    if (m_PropRegistry.count(propID) > 0 && !m_PropRegistry[propID].isWalkable) {
        printf("WALK FAILED: Blocked by solid Prop ID %d!\n", propID);
        return false;
    }
    if (m_ItemRegistry.count(propID) > 0) {
        printf("WALK FAILED: Blocked by unpicked Item ID %d!\n", propID);
        return false;
    }

    // 4. Check NPCs
    for (const auto& npc : m_NPCs) {
        if (npc->GetGridX() == x && npc->GetGridY() == y) {
            printf("WALK FAILED: Blocked by an NPC at (%d, %d)!\n", x, y);
            return false;
        }
    }

    return true;
}

int Map::GetTileType(int gridX, int gridY) {
    if (gridY < 0 || gridY >= (int)m_LevelData.size() ||
        gridX < 0 || gridX >= (int)m_LevelData[0].size()) return -1;
    return m_LevelData[gridY][gridX];
}

int Map::GetPropType(int gridX, int gridY) {
    if (gridY < 0 || gridY >= (int)m_PropData.size() ||
        gridX < 0 || gridX >= (int)m_PropData[0].size()) return 0;
    return m_PropData[gridY][gridX];
}

std::shared_ptr<NPC> Map::GetNPCAt(int gridX, int gridY) {
    for (auto& npc : m_NPCs) {
        if (npc->GetGridX() == gridX && npc->GetGridY() == gridY) return npc;
    }
    return nullptr;
}

// ─────────────────────────────────────────────
//  INTERACTIONS
// ─────────────────────────────────────────────

std::string Map::CollectItemAt(int gridX, int gridY, Character& player) {
    int propID = GetPropType(gridX, gridY);
    if (m_ItemRegistry.count(propID) == 0) return "";

    std::string  itemName = m_ItemRegistry[propID].name;
    ItemCategory itemCat  = m_ItemRegistry[propID].category;

    player.AddItem(itemName, itemCat, 1);
    m_PropData[gridY][gridX] = 0;

    for (auto& item : m_Items) {
        if (item->GetGridX() == gridX && item->GetGridY() == gridY && !item->IsCollected()) {
            item->Collect();
            break;
        }
    }

    std::string uniqueID = m_CurrentLevelPath + "_" + std::to_string(gridX) + "_" + std::to_string(gridY);
    GameConfig::LootedItems.insert(uniqueID);

    return itemName;
}

void Map::UpdateSteppedProps(int playerGridX, int playerGridY) {
    for (auto& prop : m_Props) {
        if (prop->GetGridX() == -1 || prop->GetGridY() == -1) continue;
        prop->SetSteppedOn(prop->GetGridX() == playerGridX && prop->GetGridY() == playerGridY);
    }
}

// ─────────────────────────────────────────────
//  RENDERER HELPERS
// ─────────────────────────────────────────────

void Map::SetRenderer(std::weak_ptr<Util::Renderer> renderer) {
    m_Renderer = renderer;
}

void Map::AddToRenderer(std::shared_ptr<Util::GameObject> obj) {
    if (auto r = m_Renderer.lock()) r->AddChild(obj);
}

void Map::ClearMap() {
    if (auto r = m_Renderer.lock()) {
        for (auto& tile : m_Tiles) r->RemoveChild(tile);
        for (auto& prop : m_Props) r->RemoveChild(prop);
        for (auto& npc  : m_NPCs)  r->RemoveChild(npc);
        for (auto& item : m_Items) r->RemoveChild(item);
    }
    m_Tiles.clear();
    m_TileVisible.clear(); // must stay in sync with m_Tiles
    m_LevelData.clear();
    m_PropData.clear();
    m_Props.clear();
    m_NPCs.clear();
    m_Items.clear();
}

void Map::SetVisible(bool visible) {
    for (auto& tile : m_Tiles) if (tile) tile->SetVisible(visible);
    for (auto& prop : m_Props) if (prop) prop->SetVisible(visible);
    for (auto& npc  : m_NPCs)  if (npc)  npc->SetVisible(visible);
    for (auto& item : m_Items) if (item) item->SetVisible(visible);
}

void Map::Draw() {
    // Intentionally empty — the Renderer handles all drawing.
}
