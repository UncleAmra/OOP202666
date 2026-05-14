#include "Player.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Map.hpp"

Player::Player(float x, float y) : Character(x, y) {
    // 1. Load our specific Red sprites
    m_BaseZIndex = 0.5f;
    LoadSprites();
    //m_FootOffsetY += 8.0f;
    m_Transform.translation.y -= 12.0f;
    // 2. Snap to the first frame
    UpdateSprite(); 
    
}

void Player::LoadSprites() {
    const std::string CHAR_DIR = std::string(RESOURCE_DIR) + "/player/";

    std::vector<std::string> downFrames  = {
        CHAR_DIR + "character_000.png",
        CHAR_DIR + "character_001.png",
        CHAR_DIR + "character_000.png",
        CHAR_DIR + "character_002.png"
    };

    std::vector<std::string> upFrames = {
        CHAR_DIR + "character_003.png",
        CHAR_DIR + "character_004.png",
        CHAR_DIR + "character_003.png",
        CHAR_DIR + "character_005.png"
    };

    std::vector<std::string> leftFrames = {
        CHAR_DIR + "character_006.png",
        CHAR_DIR + "character_007.png",
        CHAR_DIR + "character_006.png",
        CHAR_DIR + "character_008.png"
    };

    std::vector<std::string> rightFrames = {
        CHAR_DIR + "character_009.png",
        CHAR_DIR + "character_010.png",
        CHAR_DIR + "character_009.png",
        CHAR_DIR + "character_011.png"
    };

        std::vector<std::string> RundownFrames  = {
        CHAR_DIR + "run1.png",
        CHAR_DIR + "run2.png",
        CHAR_DIR + "run1.png",
        CHAR_DIR + "run3.png"
    };

    std::vector<std::string> RunupFrames = {
        CHAR_DIR + "run4.png",
        CHAR_DIR + "run5.png",
        CHAR_DIR + "run4.png",
        CHAR_DIR + "run6.png"
    };

    std::vector<std::string> RunleftFrames = {
        CHAR_DIR + "run7.png",
        CHAR_DIR + "run8.png",
        CHAR_DIR + "run7.png",
        CHAR_DIR + "run9.png"
    };

    std::vector<std::string> RunrightFrames = {
        CHAR_DIR + "run10.png",
        CHAR_DIR + "run11.png",
        CHAR_DIR + "run10.png",
        CHAR_DIR + "run12.png"
    };

        // Fast version of walk anims (80ms vs 150ms = noticeably snappier)
    m_AnimRunDown  = std::make_shared<Util::Animation>(RundownFrames,  false, 80, true, 0);
    m_AnimRunUp    = std::make_shared<Util::Animation>(RunupFrames,    false, 80, true, 0);
    m_AnimRunLeft  = std::make_shared<Util::Animation>(RunleftFrames,  false, 80, true, 0);
    m_AnimRunRight = std::make_shared<Util::Animation>(RunrightFrames, false, 80, true, 0);

    m_AnimDown  = std::make_shared<Util::Animation>(downFrames, false, 150, true, 0);
    m_AnimUp    = std::make_shared<Util::Animation>(upFrames, false, 150, true, 0);
    m_AnimLeft  = std::make_shared<Util::Animation>(leftFrames, false, 150, true, 0);
    m_AnimRight = std::make_shared<Util::Animation>(rightFrames, false, 150, true, 0);

    m_CurrentAnimation = m_AnimDown;
    m_Drawable = m_CurrentAnimation;
}

void Player::HandleInput(std::shared_ptr<Map> map) {
    if (IsMoving()) return;

    // Read shift state before anything else
    m_IsRunning = Util::Input::IsKeyPressed(Util::Keycode::LSHIFT) ||
                  Util::Input::IsKeyPressed(Util::Keycode::RSHIFT);

    int dx = 0, dy = 0;

    if      (Util::Input::IsKeyPressed(Util::Keycode::W)   || Util::Input::IsKeyPressed(Util::Keycode::UP))    { dy = -1; m_Direction = Direction::UP; }
    else if (Util::Input::IsKeyPressed(Util::Keycode::S)   || Util::Input::IsKeyPressed(Util::Keycode::DOWN))  { dy =  1; m_Direction = Direction::DOWN; }
    else if (Util::Input::IsKeyPressed(Util::Keycode::A)   || Util::Input::IsKeyPressed(Util::Keycode::LEFT))  { dx = -1; m_Direction = Direction::LEFT; }
    else if (Util::Input::IsKeyPressed(Util::Keycode::D)   || Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) { dx =  1; m_Direction = Direction::RIGHT; }

    if (dx != 0 || dy != 0) {
        int targetX = m_GridX + dx;
        int targetY = m_GridY + dy;

        int Prop = map->GetPropType(targetX, targetY);
        if (!map->IsWalkable(targetX, targetY) && Prop == GameConfig::PROP_INVISIBLE_DOOR) {
            m_HitDoor = true;
        }

        m_SpeedMultiplier = m_IsRunning ? 2.0f : 1.0f;  // <-- speed injected here
        m_CurrentDirection = {dx, dy};
        TryMove(dx, dy, map);
    } else {
        m_State    = State::IDLE;
        m_IsRunning = false;
    }
}

glm::vec2 Player::Update(std::shared_ptr<Map> map) {
    HandleInput(map);
    
    // Snapshot moving state BEFORE the base update runs
    bool wasMoving = m_IsMoving;
    
    glm::vec2 movement = Character::Update(map);
    
    map->UpdateSteppedProps(m_GridX, m_GridY);
    
    // If we JUST finished a step this frame, check the tile we landed on
    if (wasMoving && !m_IsMoving) {
        int propID = map->GetPropType(m_GridX, m_GridY);
        
        if (propID == GameConfig::PROP_TALLGRASS) {
            // 15% encounter rate per step — adjust to taste
            if (rand() % 100 < 15) {
                m_WildEncounterTriggered = true;
                LOG_TRACE("Wild encounter triggered at ({}, {})!", m_GridX, m_GridY);
            }
        }
    }
    
    return movement;
}

void Player::UpdateSprite() {
    // Pick run or walk set based on shift state
    auto& animDown  = m_IsRunning ? m_AnimRunDown  : m_AnimDown;
    auto& animUp    = m_IsRunning ? m_AnimRunUp    : m_AnimUp;
    auto& animLeft  = m_IsRunning ? m_AnimRunLeft  : m_AnimLeft;
    auto& animRight = m_IsRunning ? m_AnimRunRight : m_AnimRight;

    switch (m_Direction) {
        case Direction::DOWN:  m_CurrentAnimation = animDown;  break;
        case Direction::UP:    m_CurrentAnimation = animUp;    break;
        case Direction::LEFT:  m_CurrentAnimation = animLeft;  break;
        case Direction::RIGHT: m_CurrentAnimation = animRight; break;
    }
    m_Drawable = m_CurrentAnimation;

    // Preserve the exact play/pause logic from the base class
    if (m_State == State::MOVING) {
        m_CurrentAnimation->Play();
    } else {
        m_CurrentAnimation->Pause();
        m_CurrentAnimation->SetCurrentFrame(0);
    }
}