#ifndef MAP_HPP
#define MAP_HPP

#include "pch.hpp"
#include "NPC.hpp"
#include "Character.hpp"
#include "Item.hpp"
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Animation.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map> 

// ADDED: The blueprint for every tile!
struct TileProperties {
    std::shared_ptr<Util::Image> texture;
    float zIndex = 0.0f;
    float yOffset = 0.0f;
    bool isWalkable = false; 
};

enum class PropAnimMode {
    STATIC,       // Default — never changes frame automatically
    LOOP,         // Cycles through all frames continuously  
    PING_PONG     // Plays forward then backward (good for flickering effects)
};

struct PropProperties {
    std::vector<std::string> texturePaths;
    float zIndex = 0.8f;
    bool dynamicZ = true; 
    bool isWalkable = false;
    float visualOffsetX = 0.0f; 
    float visualOffsetY = 0.0f; 
    PropAnimMode animMode = PropAnimMode::STATIC;  
    int animFrameDelay = 8; 
 
};

struct PatrolPoint {
    int gridX;
    int gridY;
};

struct NPCProperties {
    // ---- Existing ----
    std::string  texturePath;
    float        visualOffsetY;
    float        zIndex;
    bool         dynamicZ;
    std::string  dialogueFilePath;
    NPCAction    actionType     = NPCAction::NONE;
    std::string  actionData     = "";
    ItemCategory itemCategory   = ItemCategory::GENERAL; // ← was missing

    // ---- New: movement ----
    MovementType             movementType  = MovementType::STILL;
    float                    moveInterval  = 2.0f;
    int                      wanderRadius  = 3;
    std::vector<PatrolPoint> patrolPoints  = {};
    std::string flagOnInteract;
    
};

struct ItemProperties {
    std::string  texturePath;
    std::string  name;
    ItemCategory category;
    float zIndex;
};

class Item;
class Prop;
class Player; // Forward declaration

class Map : public Util::GameObject, public std::enable_shared_from_this<Map> {
public:
    Map(); 
    void WarpTo(int gridX, int gridY);
    int GetPropType(int gridX, int gridY);
    void Move(float dx, float dy);
    void Draw(); 
    bool IsWalkable(int x, int y);
    
    // ONLY ONE Update signature now!
    void Update();
    
    int GetTileType(int gridX, int gridY);
    void LoadLevel(const std::string& filepath);
    std::shared_ptr<NPC> GetNPCAt(int gridX, int gridY);
    std::string GetCurrentLevelPath() const { return m_CurrentLevelPath; }
    std::string CollectItemAt(int gridX, int gridY, Character& player);
    void SetRenderer(std::weak_ptr<Util::Renderer> renderer);
    void UpdateSteppedProps(int playerGridX, int playerGridY);
    void UpdatePropOverlap(int playerGridX, int playerGridY, float playerFootY);

    void SetVisible(bool visible);
    void LoadConnections(const std::string& filepath);
    // New overload — accepts generated data directly, skips CSV loading
    void LoadGeneratedLevel(const std::string& mapName,
                        std::vector<std::vector<int>> groundData,
                        std::vector<std::vector<int>> propData);
    
private:
    // --- MAP DATA ---
    std::vector<std::shared_ptr<NPC>> m_NPCs;
    std::vector<std::vector<int>> m_PropData;
    std::vector<std::vector<int>> m_LevelData;
    std::vector<std::shared_ptr<Util::GameObject>> m_Tiles;
    std::vector<std::shared_ptr<Prop>> m_Props;
    std::vector<std::shared_ptr<Item>> m_Items;
    std::string m_CurrentLevelPath;
    std::weak_ptr<Util::Renderer> m_Renderer;

    // --- THE REGISTRY ---
    std::unordered_map<int, TileProperties> m_TileRegistry;
    std::unordered_map<int, NPCProperties> m_NPCRegistry;
    std::unordered_map<int, PropProperties> m_PropRegistry; 
    std::unordered_map<int, ItemProperties> m_ItemRegistry;
    
    void InitTileRegistry(); 
    void InitPropRegistry();
    void InitNPCRegistry(); 
    void InitItemRegistry();
    void AddToRenderer(std::shared_ptr<Util::GameObject> obj);
    void SpawnTilesAndProps();

    // --- ANIMATIONS ---
    std::shared_ptr<Util::Animation> m_LeaderWater;
    std::shared_ptr<Util::Animation> m_FollowerWater;

    // --- HELPER FUNCTIONS ---
    std::vector<std::vector<int>> LoadCSV(const std::string& filepath);
    
    void ClearMap();

    int m_OutOfBoundsPropID = -1;

    void UpdateVisibleTiles(float cameraX, float cameraY, int screenW, int screenH);

    // Track which tiles are currently in the renderer
    std::vector<bool> m_TileVisible; // parallel to m_Tiles
    void LoadNPCsFromJSON(const std::string& path);
    static NPCAction    StringToAction  (const std::string& s);
    static MovementType StringToMovement(const std::string& s);
    static ItemCategory StringToCategory(const std::string& s);   
};

#endif