#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

Character::Character(float x, float y) {
    m_Transform.translation = {x, y};
    m_ZIndex = 1.0f;
    m_Visible = true;
    m_State = State::IDLE;
    m_Direction = Direction::DOWN;
    m_Transform.scale = {3.0f, 3.0f};
    
    LoadSprites();
    UpdateSprite();
}

void Character::LoadSprites() {
    // Define the sequences using your 12 new files
    // The pattern is: Left foot (0), Stand (1), Right foot (2), Stand (1)
    std::vector<std::string> downFrames  = {RESOURCE_DIR "/character_000.png",  RESOURCE_DIR "/character_001.png",  RESOURCE_DIR "/character_002.png",  RESOURCE_DIR "/character_000.png"};
    std::vector<std::string> upFrames    = {RESOURCE_DIR "/character_003.png",    RESOURCE_DIR "/character_004.png",    RESOURCE_DIR "/character_005.png",    RESOURCE_DIR "/character_003.png"};
    std::vector<std::string> leftFrames  = {RESOURCE_DIR "/character_006.png",  RESOURCE_DIR "/character_007.png",  RESOURCE_DIR "/character_008.png",  RESOURCE_DIR "/character_006.png"};
    std::vector<std::string> rightFrames = {RESOURCE_DIR "/character_009.png", RESOURCE_DIR "/character_010.png", RESOURCE_DIR "/character_011.png", RESOURCE_DIR "/character_009.png"};

    // Initialize the animations (Paths, Play=false, Interval=150ms, Looping=true, Cooldown=0)
    m_AnimDown  = std::make_shared<Util::Animation>(downFrames, false, 150, true, 0);
    m_AnimUp    = std::make_shared<Util::Animation>(upFrames, false, 150, true, 0);
    m_AnimLeft  = std::make_shared<Util::Animation>(leftFrames, false, 150, true, 0);
    m_AnimRight = std::make_shared<Util::Animation>(rightFrames, false, 150, true, 0);

    // Set default starting state
    m_CurrentAnimation = m_AnimDown;
    m_Drawable = m_CurrentAnimation;
}

void Character::HandleInput() {
    m_State = State::IDLE;

    if (Util::Input::IsKeyPressed(Util::Keycode::DOWN) || Util::Input::IsKeyPressed(Util::Keycode::S)) {
        m_Transform.translation.y -= m_Speed;
        m_Direction = Direction::DOWN;
        m_State = State::MOVING;
    } else if (Util::Input::IsKeyPressed(Util::Keycode::UP) || Util::Input::IsKeyPressed(Util::Keycode::W)) {
        m_Transform.translation.y += m_Speed;
        m_Direction = Direction::UP;
        m_State = State::MOVING;
    } else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT) || Util::Input::IsKeyPressed(Util::Keycode::A)) {
        m_Transform.translation.x -= m_Speed;
        m_Direction = Direction::LEFT;
        m_State = State::MOVING;
    } else if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT) || Util::Input::IsKeyPressed(Util::Keycode::D)) {
        m_Transform.translation.x += m_Speed;
        m_Direction = Direction::RIGHT;
        m_State = State::MOVING;
    }
}

void Character::UpdateSprite() {
    // 1. Pick the animation based on the LAST KNOWN direction
    switch (m_Direction) {
        case Direction::DOWN:  m_CurrentAnimation = m_AnimDown;  break;
        case Direction::UP:    m_CurrentAnimation = m_AnimUp;    break;
        case Direction::LEFT:  m_CurrentAnimation = m_AnimLeft;  break;
        case Direction::RIGHT: m_CurrentAnimation = m_AnimRight; break;
    }
    
    m_Drawable = m_CurrentAnimation;

    // 2. Play if moving, or Snap to Stand if idle
    if (m_State == State::MOVING) {
        m_CurrentAnimation->Play(); 
    } else {
        m_CurrentAnimation->Pause();
        // FORCE the flipbook to turn to page 1 (the standing frame)
        m_CurrentAnimation->SetCurrentFrame(0); 
    }
}

void Character::Update() {
    // Notice how clean this is now! No more AdvanceFrame().
    HandleInput();
    UpdateSprite();
    Draw(); 
}