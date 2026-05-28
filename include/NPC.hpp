#ifndef NPC_HPP
#define NPC_HPP

#include "Character.hpp"
#include "Item.hpp"
#include <string>
#include <vector>
#include <utility> // std::pair

// ============================================================
//  NPCAction — what happens after dialogue closes
// ============================================================
enum class NPCAction {
    NONE,       // Dialogue only (signs, ambient NPCs)
    HEAL,       // Restores the player's entire party
    SHOP,       // Opens the Pokemart UI
    GIVE_ITEM,  // Adds one item to the player's bag
    BATTLE      // Starts a trainer battle
};

// ============================================================
//  MovementType — how the NPC behaves between interactions
// ============================================================
enum class MovementType {
    STILL,       // Faces one direction forever; never moves
    LOOK_AROUND, // Pivots to a new direction on a timer; never leaves spawn tile
    WANDER,      // Steps randomly within m_WanderRadius tiles of spawn
    PATROL       // Walks a fixed list of waypoints, ping-ponging at each end
};

// ============================================================
//  NPC
// ============================================================
class NPC : public Character {
public:
    NPC(float x, float y,
        const std::string& spritePath,
        const std::string& dialoguePath,
        const std::string& altDialoguePath = "",
        const std::string& flagCondition   = "");

    // --------------------------------------------------------
    //  Core overrides
    // --------------------------------------------------------
    glm::vec2 Update(std::shared_ptr<Map> map) override;

    // --------------------------------------------------------
    //  Interaction
    // --------------------------------------------------------
    // Returns the correct dialogue lines and suppresses re-triggering
    // actions via m_FlagCondition. Call FaceToward() before this.
    std::vector<std::string> Interact();

    // Snaps the NPC to face the player tile before dialogue opens.
    // Call this from the game loop immediately before showing dialogue.
    void FaceToward(int playerGridX, int playerGridY);

    // Freeze/unfreeze movement (set true when dialogue opens, false when it closes)
    void SetLocked(bool locked) { m_Locked = locked; }
    bool IsLocked()       const { return m_Locked; }

    // --------------------------------------------------------
    //  Action API (what happens after dialogue)
    // --------------------------------------------------------
    void SetAction(NPCAction           type,
                   const std::string&  data         = "",
                   ItemCategory        itemCategory  = ItemCategory::GENERAL);

    NPCAction    GetActionType()     const { return m_ActionType; }
    std::string  GetActionData()     const { return m_ActionData; }
    ItemCategory GetActionCategory() const { return m_ActionCategory; }

    // --------------------------------------------------------
    //  Movement API
    // --------------------------------------------------------
    void SetMovementType(MovementType type) { m_MovementType = type; }
    void SetMoveInterval(float seconds)     { m_MoveInterval = seconds; }
    void SetWanderRadius(int radius)        { m_WanderRadius = radius; }

    // Must be called after SetGridPosition() in the spawner so wander/patrol
    // NPCs measure their radius from the correct tile, not Character's default (6,6).
    void SetSpawnPoint(int x, int y) { m_SpawnGridX = x; m_SpawnGridY = y; }

    // Appends one waypoint to the patrol route (PATROL only).
    // Points are visited in insertion order, then reversed (ping-pong).
    void AddPatrolPoint(int gridX, int gridY) {
        m_PatrolPoints.emplace_back(gridX, gridY);
    }

    // --------------------------------------------------------
    //  Misc
    // --------------------------------------------------------
    void SetDynamicZ(bool dynamic) { m_UseDynamicZ = dynamic; }
    

    // flag check logic
    void SetInteractFlag(const std::string& flag) { m_InteractFlag = flag; }
    const std::string& GetInteractFlag() const { return m_InteractFlag; }
    std::string m_InteractFlag;
protected:
    void LoadSprites() override;

private:
    // ---- Sprite ----
    std::string m_SpritePath;

    // ---- Dialogue ----
    std::vector<std::string> m_DialogueLines;
    std::vector<std::string> m_AltDialogueLines;
    std::string              m_FlagCondition;

    // ---- Action (fires after dialogue closes) ----
    NPCAction    m_ActionType     = NPCAction::NONE;
    std::string  m_ActionData     = "";
    ItemCategory m_ActionCategory = ItemCategory::GENERAL;

    // ---- Movement ----
    MovementType m_MovementType = MovementType::STILL;
    bool         m_Locked       = false;
    float        m_MoveTimer    = 0.0f;
    float        m_MoveInterval = 2.0f;

    // Wander
    int m_SpawnGridX   = 0;  // Set via SetSpawnPoint() after construction
    int m_SpawnGridY   = 0;
    int m_WanderRadius = 3;

    // Patrol
    std::vector<std::pair<int, int>> m_PatrolPoints;
    int  m_PatrolIndex   = 0;
    bool m_PatrolReverse = false;

    // ---- Private helpers ----
    void LoadDialogue(const std::string& path, std::vector<std::string>& out);

    // Probes the filesystem for _Base, _Base2, _Base3 frames and returns
    // only the paths that actually exist, arranged as a walk cycle.
    // Safe to call for any NPC — missing frames are silently skipped.
    std::vector<std::string> BuildWalkCycle(const std::string& baseFramePath) const;

    // Returns true if the file at 'path' can be opened (cross-platform,
    // no C++17 filesystem dependency required).
    static bool FileExists(const std::string& path);

    bool TryMoveInDirection(int dx, int dy, std::shared_ptr<Map> map);
    void DoLookAround();
    void DoWander(std::shared_ptr<Map> map);
    void DoPatrol(std::shared_ptr<Map> map);
};

#endif // NPC_HPP