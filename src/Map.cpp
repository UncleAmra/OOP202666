#include <algorithm>
#include "Map.hpp"
#include "Prop.hpp"
#include "GameConfig.hpp" 
#include <cstdio> 
#include "Item.hpp"
#include <fstream>  
#include <sstream>  
#include <iostream> 
#include "TrainerDatabase.hpp"
#include "ResourceManager.hpp"
#include "GameFlags.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

const std::string RES        = std::string(RESOURCE_DIR);
const std::string TILE_DIR   = RES + "/tiles/";
const std::string MAP_DIR    = RES + "/maps/";
const std::string PROP_DIR   = RES + "/props/";
const std::string NPC_DIR    = RES + "/npcs/";
const std::string DIALOGUE_DIR = RES + "/dialogue/";
const std::string DATA_DIR   = RES + "/data/";

static constexpr float HALF_W = 640.0f;
static constexpr float HALF_H = 360.0f;

// ─────────────────────────── string‑to‑enum helpers ───────────────────────────

static PropAnimMode ParseAnimMode(const std::string& s) {
    if (s == "LOOP")   return PropAnimMode::LOOP;
    if (s == "STATIC") return PropAnimMode::STATIC;
    LOG_WARN("PropRegistry: unknown animMode '{}', defaulting to STATIC", s);
    return PropAnimMode::STATIC;
}

MovementType Map::StringToMovement(const std::string& s) {
    if (s == "LOOK_AROUND") return MovementType::LOOK_AROUND;
    if (s == "WANDER")      return MovementType::WANDER;
    if (s == "PATROL")      return MovementType::PATROL;
    if (s != "STILL" && !s.empty())
        LOG_WARN("NPCRegistry: unknown movement '{}', defaulting to STILL", s);
    return MovementType::STILL;
}
 
ItemCategory Map::StringToCategory(const std::string& s) {
    if (s == "POKEBALLS") return ItemCategory::POKEBALLS;
    if (s == "KEY_ITEMS") return ItemCategory::KEY_ITEMS;
    if (s != "GENERAL" && !s.empty())
        LOG_WARN("NPCRegistry: unknown itemCategory '{}', defaulting to GENERAL", s);
    return ItemCategory::GENERAL;
}
NPCAction Map::StringToAction(const std::string& s) {
    LOG_INFO("StringToAction: '{}' len={}", s, s.size());
    if (s == "HEAL")       return NPCAction::HEAL;
    if (s == "SHOP")       return NPCAction::SHOP;
    if (s == "GIVE_ITEM")  return NPCAction::GIVE_ITEM;
    if (s == "BATTLE")     return NPCAction::BATTLE;
    if (s == "CHECK_ITEM") return NPCAction::CHECK_ITEM;
    if (s != "NONE" && !s.empty())
        LOG_WARN("NPCRegistry: unknown action '{}', defaulting to NONE", s);
    return NPCAction::NONE;
}

static std::vector<std::string> ParseTextureList(const json& entry, const std::string& baseDir) {
    std::vector<std::string> paths;
    if (!entry.contains("textures")) return paths;

    const json& tex = entry["textures"];
    if (tex.is_string()) {
        paths.push_back(baseDir + tex.get<std::string>());
    }
    else if (tex.is_array()) {
        for (const auto& t : tex)
            paths.push_back(baseDir + t.get<std::string>());
    }
    else if (tex.is_object()) {
        if (tex.value("type", "") == "sequence") {
            std::string prefix = tex["prefix"];
            int count = tex["count"];
            for (int i = 1; i <= count; ++i)
                paths.push_back(baseDir + prefix + std::to_string(i) + ".png");
        }
        else {
            LOG_ERROR("PropRegistry: unknown textures object type '{}'", tex["type"]);
        }
    }
    return paths;
}

// ──────────────────────────── JSON LOADERS ─────────────────────────────────────

void Map::LoadTilesFromJSON(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("TileRegistry: could not open '{}'", path);
        return;
    }
    json root;
    try { file >> root; }
    catch (const json::parse_error& e) {
        LOG_ERROR("TileRegistry: JSON parse error in '{}': {}", path, e.what());
        return;
    }

    if (!root.contains("tiles") || !root["tiles"].is_array()) {
        LOG_ERROR("TileRegistry: '{}' missing \"tiles\" array", path);
        return;
    }

    int loaded = 0;
    for (const auto& entry : root["tiles"]) {
        if (!entry.contains("id")) { LOG_WARN("TileRegistry: entry missing id"); continue; }
        const int id = entry["id"];

        std::string texName = entry.value("texture", "");
        std::shared_ptr<Util::Image> texture = nullptr;
        if (!texName.empty() && texName != "null") {
            texture = ResourceManager::GetImageStore().Get(TILE_DIR + texName);
        }

        m_TileRegistry[id] = {
            texture,
            entry.value("zIndex", 0.0f),
            entry.value("yOffset", 0.0f),
            entry.value("walkable", true)
        };
        ++loaded;
    }
    LOG_INFO("TileRegistry: loaded {} tiles from '{}'", loaded, path);
}

void Map::LoadPropsFromJSON(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("PropRegistry: could not open '{}'", path);
        return;
    }
    json root;
    try { file >> root; }
    catch (const json::parse_error& e) {
        LOG_ERROR("PropRegistry: JSON parse error in '{}': {}", path, e.what());
        return;
    }

    if (!root.contains("props") || !root["props"].is_array()) {
        LOG_ERROR("PropRegistry: '{}' missing \"props\" array", path);
        return;
    }

    int loaded = 0;
    for (const auto& entry : root["props"]) {
        if (!entry.contains("id")) { LOG_WARN("PropRegistry: entry missing id"); continue; }
        const int id = entry["id"];

        PropProperties props;
        props.texturePaths   = ParseTextureList(entry, PROP_DIR);
        props.zIndex         = entry.value("zIndex", 0.0f);
        props.dynamicZ       = entry.value("dynamicZ", true);
        props.isWalkable     = entry.value("walkable", true);
        props.visualOffsetX  = entry.value("offsetX", 0.0f);
        props.visualOffsetY  = entry.value("offsetY", 0.0f);
        props.animMode       = ParseAnimMode(entry.value("animMode", "STATIC"));
        props.animFrameDelay = entry.value("frameDelay", 100);

        m_PropRegistry[id] = std::move(props);
        ++loaded;
    }
    LOG_INFO("PropRegistry: loaded {} props from '{}'", loaded, path);
}

void Map::LoadItemsFromJSON(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("ItemRegistry: could not open '{}'", path);
        return;
    }
    json root;
    try { file >> root; }
    catch (const json::parse_error& e) {
        LOG_ERROR("ItemRegistry: JSON parse error in '{}': {}", path, e.what());
        return;
    }

    if (!root.contains("items") || !root["items"].is_array()) {
        LOG_ERROR("ItemRegistry: '{}' missing \"items\" array", path);
        return;
    }

    int loaded = 0;
    for (const auto& entry : root["items"]) {
        if (!entry.contains("id")) { LOG_WARN("ItemRegistry: entry missing id"); continue; }
        const int id = entry["id"];

        m_ItemRegistry[id] = {
            PROP_DIR + entry["texture"].get<std::string>(),
            entry["name"].get<std::string>(),
            StringToCategory(entry.value("category", "GENERAL")),
            entry.value("zIndex", 0.5f)
        };
        ++loaded;
    }
    LOG_INFO("ItemRegistry: loaded {} items from '{}'", loaded, path);
}

void Map::InitTileRegistry()   { LoadTilesFromJSON(DATA_DIR + "tiles.json"); }
void Map::InitNPCRegistry()    { LoadNPCsFromJSON(DATA_DIR + "npcs.json"); }
void Map::InitPropRegistry()   { LoadPropsFromJSON(DATA_DIR + "props.json"); }
void Map::InitItemRegistry()   { LoadItemsFromJSON(DATA_DIR + "items.json"); }

Map::Map() { 
    InitTileRegistry();
    InitNPCRegistry();
    InitPropRegistry();
    InitItemRegistry();
}

void Map::LoadNPCsFromJSON(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("NPCRegistry: could not open '{}'", path);
        return;
    }

    json root;
    try { file >> root; }
    catch (const json::parse_error& e) {
        LOG_ERROR("NPCRegistry: JSON parse error in '{}': {}", path, e.what());
        return;
    }

    if (!root.contains("npcs") || !root["npcs"].is_array()) {
        LOG_ERROR("NPCRegistry: '{}' is missing a top-level \"npcs\" array", path);
        return;
    }

    int loaded = 0;
    for (const auto& entry : root["npcs"]) {
        if (!entry.contains("id")) {
            LOG_WARN("NPCRegistry: entry missing \"id\", skipping");
            continue;
        }
        const int id = entry["id"].get<int>();
        NPCProperties props;

        props.texturePath    = NPC_DIR + entry.value("texture",      "");
        props.visualOffsetY  = entry.value("visualOffsetY", 0.0f);
        props.zIndex         = entry.value("zIndex",        0.5f);
        props.dynamicZ       = entry.value("dynamicZ",      true);
        props.actionType     = StringToAction  (entry.value("action",       "NONE"));
        props.actionData     = entry.value("actionData",    "");
        props.itemCategory   = StringToCategory(entry.value("itemCategory", "GENERAL"));
        props.flagOnInteract = entry.value("flagOnInteract","");
        props.flagToHide     = entry.value("flagToHide",    "");

        // ── Dialogue ────────────────────────────────────────────────────────
        if (entry.contains("dialogue") && entry["dialogue"].is_object()) {
            const auto& d = entry["dialogue"];

            // Default lines — shown when no flag condition is met
            if (d.contains("default") && d["default"].is_array())
                for (const auto& line : d["default"])
                    props.defaultDialogue.push_back(line.get<std::string>());

            // Flag-conditional lines — checked top-to-bottom, first match wins
            if (d.contains("flags") && d["flags"].is_array()) {
                for (const auto& flag : d["flags"]) {
                    if (!flag.contains("condition") || !flag.contains("lines")) {
                        LOG_WARN("NPCRegistry: NPC id={} dialogue flag entry missing "
                                 "\"condition\" or \"lines\", skipping", id);
                        continue;
                    }
                    NPCDialogueEntry e;
                    e.condition = flag["condition"].get<std::string>();
                    for (const auto& line : flag["lines"])
                        e.lines.push_back(line.get<std::string>());
                    props.conditionalDialogue.push_back(std::move(e));
                }
            }
        } else {
            LOG_WARN("NPCRegistry: NPC id={} has no \"dialogue\" block — "
                     "will be silent on interaction", id);
        }

        // ── Movement ────────────────────────────────────────────────────────
        if (entry.contains("movement") && entry["movement"].is_object()) {
            const auto& mov = entry["movement"];
            props.movementType = StringToMovement(mov.value("type",         "STILL"));
            props.moveInterval = mov.value("moveInterval", 2.0f);
            props.wanderRadius = mov.value("wanderRadius", 3);

            if (mov.contains("patrolPoints") && mov["patrolPoints"].is_array()) {
                for (const auto& pt : mov["patrolPoints"]) {
                    if (!pt.contains("x") || !pt.contains("y")) {
                        LOG_WARN("NPCRegistry: NPC id={} patrol point missing x or y, "
                                 "skipping point", id);
                        continue;
                    }
                    props.patrolPoints.push_back({ pt["x"].get<int>(), pt["y"].get<int>() });
                }
            }
        }

        m_NPCRegistry[id] = std::move(props);
        ++loaded;
    }
    LOG_INFO("NPCRegistry: loaded {} NPCs from '{}'", loaded, path);
}

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
                std::cerr << "Bad CSV value '" << cell << "' at row " << data.size() << ": " << e.what() << "\n";
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

void Map::SpawnTilesAndProps() {
    std::vector<std::string> waterPaths = {
        TILE_DIR + "/Water1.png", TILE_DIR + "/Water2.png", TILE_DIR + "/Water3.png",
        TILE_DIR + "/Water4.png", TILE_DIR + "/Water5.png", TILE_DIR + "/Water6.png",
        TILE_DIR + "/Water7.png"
    };
    m_LeaderWater   = std::make_shared<Util::Animation>(waterPaths, true,  500, true, 0);
    m_FollowerWater = std::make_shared<Util::Animation>(waterPaths, false, 500, true, 0);
    bool leaderAssigned = false;

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
            m_TileVisible.push_back(true);
            AddToRenderer(newTile);
        }
    }

    if (m_PropData.empty()) return;
    
    for (size_t y = 0; y < m_PropData.size(); y++) {
        for (size_t x = 0; x < m_PropData[y].size(); x++) {
            int propID = m_PropData[y][x];
            if (propID <= 0) continue;
            float worldX = GameConfig::CAMERA_START_X + (x * GameConfig::EFFECTIVE_TILE_SIZE);
            float worldY = GameConfig::CAMERA_START_Y - (y * GameConfig::EFFECTIVE_TILE_SIZE);

            // Inside SpawnTilesAndProps — NPC spawn block (replaces the existing one)
            if (m_NPCRegistry.count(propID) > 0) {
                const NPCProperties& npcProps = m_NPCRegistry[propID];

                auto npc = std::make_shared<NPC>(
                    worldX, worldY + npcProps.visualOffsetY * GameConfig::SCALE / 3.0f,
                    npcProps.texturePath
                );

                npc->SetGridPosition(x, y);
                npc->SetSpawnPoint(x, y);
                npc->SetZIndex(npcProps.zIndex);
                npc->SetBaseZIndex(npcProps.zIndex);
                npc->SetDynamicZ(npcProps.dynamicZ);

                // Inline dialogue — no file paths, data comes straight from the registry
                npc->SetDialogue(npcProps.defaultDialogue, npcProps.conditionalDialogue);

                npc->SetAction(npcProps.actionType, npcProps.actionData, npcProps.itemCategory);
                if (npcProps.actionType == NPCAction::BATTLE) {
                    auto loadedParty = TrainerDatabase::CreateTrainerParty(npcProps.actionData);
                    for (const auto& p : loadedParty) npc->GetParty().push_back(p);
                }

                npc->SetMovementType(npcProps.movementType);
                npc->SetMoveInterval(npcProps.moveInterval);
                npc->SetWanderRadius(npcProps.wanderRadius);
                for (const auto& point : npcProps.patrolPoints)
                    npc->AddPatrolPoint(point.gridX, point.gridY);

                npc->SetInteractFlag(npcProps.flagOnInteract);
                if (!npcProps.flagToHide.empty()) {
                    npc->SetFlagToHide(npcProps.flagToHide);
                    npc->SetVisible(!GameFlags::Get(npcProps.flagToHide));
                }

                LOG_INFO("Spawning NPC id={} action={} data='{}'",
                        propID, (int)npcProps.actionType, npcProps.actionData);

                m_NPCs.push_back(npc);
                AddToRenderer(npc);
            }

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

            if (m_ItemRegistry.count(propID) > 0) {
                std::string uniqueID = m_CurrentLevelPath + "_" + std::to_string(x) + "_" + std::to_string(y);
                if (GameConfig::LootedItems.count(uniqueID) > 0) {
                    m_PropData[y][x] = 0;
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

void Map::LoadLevel(const std::string& mapName) {
    ClearMap();
    m_CurrentLevelPath = mapName;
    m_LevelData = LoadCSV(mapName + "_ground.csv");
    m_PropData  = LoadCSV(mapName + "_props.csv");
    if (m_LevelData.empty()) return;
    SpawnTilesAndProps();
}

void Map::Update() {
    // Water sync — always runs, pause doesn't affect it.
    // Stopping mid-ripple during dialogue would look wrong.
    if (m_LeaderWater && m_FollowerWater) {
        m_FollowerWater->SetCurrentFrame(m_LeaderWater->GetCurrentFrameIndex());
    }
 
    // Tile culling — pure bookkeeping, no visible gameplay effect.
    if (!m_Tiles.empty()) {
        const float margin = GameConfig::EFFECTIVE_TILE_SIZE * 2.0f;
        const float minX = -(HALF_W + margin), maxX = (HALF_W + margin);
        const float minY = -(HALF_H + margin), maxY = (HALF_H + margin);
 
        for (size_t i = 0; i < m_Tiles.size(); i++) {
            float tx = m_Tiles[i]->m_Transform.translation.x;
            float ty = m_Tiles[i]->m_Transform.translation.y;
            bool inView = tx > minX && tx < maxX && ty > minY && ty < maxY;
            if (inView != m_TileVisible[i]) {
                m_TileVisible[i] = inView;
                m_Tiles[i]->SetVisible(inView);
            }
        }
    }
 
    // Prop culling — visibility always updates, but animation pauses
    // during dialogue so spinning signs don't advance mid-conversation.
    const float propMargin = GameConfig::EFFECTIVE_TILE_SIZE * 6.0f;
    const float propMinX = -(HALF_W + propMargin), propMaxX = (HALF_W + propMargin);
    const float propMinY = -(HALF_H + propMargin), propMaxY = (HALF_H + propMargin);
 
    for (auto& prop : m_Props) {
        float px = prop->m_Transform.translation.x;
        float py = prop->m_Transform.translation.y;
        bool inView = px > propMinX && px < propMaxX &&
                      py > propMinY && py < propMaxY;
        prop->SetVisible(inView);
        if (inView && !m_Paused) prop->Update();
    }
 
    // NPCs freeze entirely during dialogue — no movement decisions,
    // no animation advancement, no Z-sort updates.
    if (!m_Paused) {
        for (auto& npc : m_NPCs) npc->Update(shared_from_this());
    }
}

void Map::Move(float dx, float dy) {
    for (auto& tile : m_Tiles) { tile->m_Transform.translation.x += dx; tile->m_Transform.translation.y += dy; }
    for (auto& prop : m_Props) { prop->m_Transform.translation.x += dx; prop->m_Transform.translation.y += dy; }
    for (auto& npc  : m_NPCs)  { npc->m_Transform.translation.x  += dx; npc->m_Transform.translation.y  += dy; }
    for (auto& item : m_Items) { item->m_Transform.translation.x += dx; item->m_Transform.translation.y += dy; }
}

void Map::WarpTo(int gridX, int gridY) {
    float shiftX = GameConfig::CAMERA_START_X + (gridX * GameConfig::EFFECTIVE_TILE_SIZE);
    float shiftY = GameConfig::CAMERA_START_Y - (gridY * GameConfig::EFFECTIVE_TILE_SIZE);
    Move(-shiftX, -shiftY);
}

bool Map::IsWalkable(int x, int y) {
    // 1. Bounds
    if (x < 0 || x >= (int)m_LevelData[0].size() ||
        y < 0 || y >= (int)m_LevelData.size()) {
        printf("WALK FAILED: Tile (%d, %d) is out of bounds!\n", x, y);
        return false;
    }
 
    // 2. Ground tile
    int tileID = m_LevelData[y][x];
    if (m_TileRegistry.count(tileID) == 0) {
        printf("WALK FAILED: Ground Tile ID %d is missing from registry!\n", tileID);
        return false;
    }
    if (!m_TileRegistry[tileID].isWalkable) {
        printf("WALK FAILED: Ground Tile ID %d is marked as solid!\n", tileID);
        return false;
    }
 
    // 3. Props and items
    int propID = m_PropData[y][x];
    if (m_PropRegistry.count(propID) > 0 && !m_PropRegistry[propID].isWalkable) {
        printf("WALK FAILED: Blocked by solid Prop ID %d!\n", propID);
        return false;
    }
    if (m_ItemRegistry.count(propID) > 0) {
        printf("WALK FAILED: Blocked by unpicked Item ID %d!\n", propID);
        return false;
    }
 
    // 4. Active NPCs — inactive NPCs (defeated trainers, hidden by flag)
    //    are skipped so the player and other NPCs can pass through them.
    for (const auto& npc : m_NPCs) {
        if (!npc->IsActive()) continue;
        if (npc->GetGridX() == x && npc->GetGridY() == y) {
            printf("WALK FAILED: Blocked by an active NPC at (%d, %d)!\n", x, y);
            return false;
        }
    }
 
    // 5. Player tile — NPCs cannot walk through the player.
    //    Sentinel -1,-1 is never a valid tile so this is a no-op until
    //    SetPlayerGridPosition() has been called at least once.
    if (m_PlayerGridX == x && m_PlayerGridY == y) {
        return false;
    }
 
    return true;
}

int Map::GetTileType(int gridX, int gridY) {
    if (gridY < 0 || gridY >= (int)m_LevelData.size() || gridX < 0 || gridX >= (int)m_LevelData[0].size()) return -1;
    return m_LevelData[gridY][gridX];
}

int Map::GetPropType(int gridX, int gridY) {
    if (gridY < 0 || gridY >= (int)m_PropData.size() || gridX < 0 || gridX >= (int)m_PropData[0].size()) return 0;
    return m_PropData[gridY][gridX];
}

std::shared_ptr<NPC> Map::GetNPCAt(int x, int y) {
    for (auto& npc : m_NPCs) {
        if (npc->IsActive() && npc->GetGridX() == x && npc->GetGridY() == y) {
            return npc;
        }
    }
    return nullptr;
}

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
    m_TileVisible.clear(); 
    m_LevelData.clear();
    m_PropData.clear();
    m_Props.clear();
    m_NPCs.clear();
    m_Items.clear();
}

void Map::SetVisible(bool visible) {
    for (size_t i = 0; i < m_Tiles.size(); ++i) {
        if (m_Tiles[i]) {
            m_Tiles[i]->SetVisible(visible);
            m_TileVisible[i] = visible;   
        }
    }
    for (auto& prop : m_Props) if (prop) prop->SetVisible(visible);
    for (auto& npc  : m_NPCs)  if (npc)  npc->SetVisible(visible);
    for (auto& item : m_Items) if (item) item->SetVisible(visible);
}

void Map::Draw() {
    // Intentionally empty — the Renderer handles all drawing.
}