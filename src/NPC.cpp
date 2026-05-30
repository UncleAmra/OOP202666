#include "NPC.hpp"
#include "Map.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include "GameFlags.hpp"
#include "Util/Animation.hpp"

#include <fstream>
#include <cstdlib>   // rand()
#include <cmath>     // std::abs()
#include <algorithm> // std::swap

// ============================================================
//  Constructor
// ============================================================
NPC::NPC(float x, float y, const std::string& spritePath)
    : Character(x, y)
    , m_SpritePath(spritePath)
{
    m_Speed = 100.0f;

    // NOTE: m_SpawnGridX/Y are intentionally NOT set here.
    // Character's grid position is still the default at this point.
    // The spawner must call SetGridPosition() then SetSpawnPoint() after construction.

    // Stagger first decision so grouped NPCs don't all step simultaneously.
    m_MoveTimer = m_MoveInterval * (0.5f + (rand() % 100) / 100.0f);

    LoadSprites();
}

// ============================================================
//  Active state — driven by flagToHide
// ============================================================
bool NPC::IsActive() const {
    // If the NPC has a hide flag and it's set, the NPC should vanish.
    if (!m_FlagToHide.empty()) {
        return !GameFlags::Get(m_FlagToHide);
    }
    return true;
}

// ============================================================
//  Update
// ============================================================
glm::vec2 NPC::Update(std::shared_ptr<Map> map) {
    if (!IsActive()) {
        SetVisible(false);
        return glm::vec2(0.0f, 0.0f);
    }
    SetVisible(true);

    // 1. Always finish any in-progress tile movement first.
    if (m_IsMoving) {
        glm::vec2 movement = Character::Update(map);
        m_Transform.translation += movement;
        return movement;
    }

    // 2. Freeze decisions during dialogue — NPC finishes its current step
    //    then holds still until SetLocked(false) is called.
    if (m_Locked) {
        return Character::Update(map);
    }

    // 3. Tick the decision timer.
    float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_MoveTimer -= dt;

    if (m_MoveTimer > 0.0f) {
        glm::vec2 movement = Character::Update(map);
        m_Transform.translation += movement;
        return movement;
    }

    // 4. Timer expired — make a movement decision.
    switch (m_MovementType) {
        case MovementType::LOOK_AROUND: DoLookAround(); break;
        case MovementType::WANDER:      DoWander(map);  break;
        case MovementType::PATROL:      DoPatrol(map);  break;
        case MovementType::STILL:       break;
    }

    // 5. Reset timer with jitter so NPCs feel organic, not mechanical.
    float jitter = (rand() % 100) / 100.0f * m_MoveInterval * 0.4f;
    m_MoveTimer = m_MoveInterval + jitter;

    // 6. Apply the first frame of any movement started this decision tick.
    glm::vec2 movement = Character::Update(map);
    m_Transform.translation += movement;
    return movement;
}

// ============================================================
//  UpdateSprite — NPC override
//  Does NOT call SetCurrentFrame(0) on idle so the walk cycle
//  isn't pinned to frame 0 every frame between steps.
// ============================================================


// ============================================================
//  Interaction
// ============================================================
void NPC::FaceToward(int playerGridX, int playerGridY) {
    int dx = playerGridX - m_GridX;
    int dy = playerGridY - m_GridY;

    if (std::abs(dx) >= std::abs(dy)) {
        SetDirection(dx > 0 ? Direction::RIGHT : Direction::LEFT);
    } else {
        SetDirection(dy > 0 ? Direction::DOWN : Direction::UP);
    }
}

std::vector<std::string> NPC::Interact(const Character& player) {
    FaceToward(player.GetGridX(), player.GetGridY());
    SetLocked(true);

    // Check flag-conditional dialogue top-to-bottom — first match wins.
    // This lets you express NPC story progression by ordering entries
    // from most-progressed to least (e.g. beat_rival_2 before beat_rival_1).
    for (const auto& entry : m_ConditionalDialogue) {
        if (!entry.condition.empty() && GameFlags::Get(entry.condition))
            return entry.lines;
    }

    return m_DefaultDialogue;
}

// ============================================================
//  Action API
// ============================================================
void NPC::SetAction(NPCAction type,
                    const std::string& data,
                    ItemCategory itemCategory) {
    m_ActionType     = type;
    m_ActionData     = data;
    m_ActionCategory = itemCategory;
}

// ============================================================
//  Movement helpers
// ============================================================

// Converts a grid delta (dx, dy) to the matching facing Direction.
// Grid convention: dy=+1 is a higher row index = visually downward.
static Character::Direction DeltaToDirection(int dx, int dy) {
    if (dx > 0) return Character::Direction::RIGHT;
    if (dx < 0) return Character::Direction::LEFT;
    if (dy > 0) return Character::Direction::DOWN;
    return Character::Direction::UP;
}

// Wrapper around TryMove that also updates the facing direction.
// TryMove only sets m_CurrentDirection (movement vector) — it never calls
// SetDirection, so the sprite would always face DOWN without this wrapper.
bool NPC::TryMoveInDirection(int dx, int dy, std::shared_ptr<Map> map) {
    SetDirection(DeltaToDirection(dx, dy));
    return TryMove(dx, dy, map);
}

// LOOK_AROUND — pivot to a different direction on a timer; no tile movement.
void NPC::DoLookAround() {
    static const Direction dirs[] = {
        Direction::DOWN, Direction::UP,
        Direction::LEFT, Direction::RIGHT
    };
    Direction next = m_Direction;
    int attempts = 0;
    while (next == m_Direction && attempts < 10) {
        next = dirs[rand() % 4];
        ++attempts;
    }
    SetDirection(next);
}

// WANDER — shuffle randomly within m_WanderRadius tiles of spawn.
void NPC::DoWander(std::shared_ptr<Map> map) {
    // 30% chance to pause and glance around — matches GBA NPC personality.
    if (rand() % 10 < 3) {
        DoLookAround();
        return;
    }

    // Four candidate deltas in random order (Fisher-Yates).
    int dx[] = { 0,  0, -1, 1 };
    int dy[] = { -1, 1,  0, 0 };

    for (int i = 3; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(dx[i], dx[j]);
        std::swap(dy[i], dy[j]);
    }

    for (int i = 0; i < 4; ++i) {
        int nextX = m_GridX + dx[i];
        int nextY = m_GridY + dy[i];

        // Never leave the home radius.
        if (std::abs(nextX - m_SpawnGridX) > m_WanderRadius) continue;
        if (std::abs(nextY - m_SpawnGridY) > m_WanderRadius) continue;

        if (TryMoveInDirection(dx[i], dy[i], map)) return;
    }

    // All directions blocked or out of radius — at least look active.
    DoLookAround();
}

// PATROL — step toward the current waypoint; ping-pong at each end.
void NPC::DoPatrol(std::shared_ptr<Map> map) {
    // Need at least 2 points to form a route worth patrolling.
    if (m_PatrolPoints.size() < 2) return;

    auto [targetX, targetY] = m_PatrolPoints[m_PatrolIndex];

    // One-axis step toward waypoint (X preferred over Y).
    int dx = 0, dy = 0;
    if      (targetX > m_GridX) dx =  1;
    else if (targetX < m_GridX) dx = -1;
    else if (targetY > m_GridY) dy =  1;
    else if (targetY < m_GridY) dy = -1;

    if (dx == 0 && dy == 0) {
        // Arrived at waypoint — advance index with ping-pong logic.
        if (!m_PatrolReverse) {
            ++m_PatrolIndex;
            if (m_PatrolIndex >= static_cast<int>(m_PatrolPoints.size())) {
                m_PatrolIndex   = static_cast<int>(m_PatrolPoints.size()) - 2;
                m_PatrolReverse = true;
            }
        } else {
            --m_PatrolIndex;
            if (m_PatrolIndex < 0) {
                m_PatrolIndex   = 1;
                m_PatrolReverse = false;
            }
        }
        return;
    }

    if (!TryMoveInDirection(dx, dy, map)) {
        // Temporarily blocked (player / another NPC in path) — wait and retry.
        DoLookAround();
    }
}

// ============================================================
//  Sprite loading
// ============================================================
bool NPC::FileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

// Probes for base + up to two walk frames per direction.
// Cycle layout: [base, frame2, base, frame3] when all three exist.
// Missing frames are silently skipped — safe for every NPC sprite set.
std::vector<std::string> NPC::BuildWalkCycle(const std::string& base) const {
    const std::string f0 = base + ".png";
    const std::string f2 = base + "2.png";
    const std::string f3 = base + "3.png";

    if (!FileExists(f0)) {
        LOG_WARN("NPC::BuildWalkCycle — base frame missing: '{}'", f0);
        return {};
    }

    const bool has2 = FileExists(f2);
    const bool has3 = FileExists(f3);

    if ( has2 &&  has3) return { f0, f2, f0, f3 }; // Full 4-frame walk cycle
    if ( has2 && !has3) return { f0, f2 };
    if (!has2 &&  has3) return { f0, f3 };
    return { f0 };                                   // Static sprite
}

void NPC::LoadSprites() {
    const auto downFrames  = BuildWalkCycle(m_SpritePath + "_Down");
    const auto upFrames    = BuildWalkCycle(m_SpritePath + "_Up");
    const auto leftFrames  = BuildWalkCycle(m_SpritePath + "_Left");
    const auto rightFrames = BuildWalkCycle(m_SpritePath + "_Right");

    // Constructed with play=false — NPCs start idle, Play() is called by
    // UpdateSprite() only when m_State == MOVING.
    if (!downFrames.empty())
        m_AnimDown  = std::make_shared<Util::Animation>(downFrames,  false, 150, true, 0);
    if (!upFrames.empty())
        m_AnimUp    = std::make_shared<Util::Animation>(upFrames,    false, 150, true, 0);
    if (!leftFrames.empty())
        m_AnimLeft  = std::make_shared<Util::Animation>(leftFrames,  false, 150, true, 0);
    if (!rightFrames.empty())
        m_AnimRight = std::make_shared<Util::Animation>(rightFrames, false, 150, true, 0);

    m_CurrentAnimation = m_AnimDown;
    m_Drawable = m_CurrentAnimation;
}

// ============================================================
//  LoadDialogue — kept to satisfy the declaration in NPC.hpp.
//  Dialogue is now loaded from JSON via SetDialogue(); this
//  method is a no-op and can be removed once NPC.hpp is cleaned up.
// ============================================================
void NPC::LoadDialogue(const std::string& /*path*/, std::vector<std::string>& /*out*/) {
    // Intentionally empty — dialogue comes from JSON, not flat files.
}