#include "Character.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

Character::Character(float x, float y) {
    m_Transform.translation = {x, y};
    m_ZIndex = 1.0f;
    m_Visible = true;
    LoadSprites();
    UpdateSprite();
}

void Character::LoadSprites() {
    m_Sprites.resize(5);

    auto load = [](const std::string& path) {
        return std::make_shared<Util::Image>(path);
    };

    // idle — 7 frames
    m_Sprites[0].resize(7);
    for (int i = 0; i < 7; i++) {
        std::string num = "00" + std::to_string(i);
        m_Sprites[0][i] = load(RESOURCE_DIR "/idle_" + num + ".png");
    }

    // down & left — character_008 to 015
    m_Sprites[1].resize(8);
    m_Sprites[3].resize(8);
    for (int i = 0; i < 8; i++) {
        std::string num = (i + 8 < 10) ? "00" + std::to_string(i + 8) : "0" + std::to_string(i + 8);
        m_Sprites[1][i] = load(RESOURCE_DIR "/character_" + num + ".png");
        m_Sprites[3][i] = load(RESOURCE_DIR "/character_" + num + ".png");
    }

    // up & right — character_000 to 007
    m_Sprites[2].resize(8);
    m_Sprites[4].resize(8);
    for (int i = 0; i < 8; i++) {
        std::string num = "00" + std::to_string(i);
        m_Sprites[2][i] = load(RESOURCE_DIR "/character_" + num + ".png");
        m_Sprites[4][i] = load(RESOURCE_DIR "/character_" + num + ".png");
    }
}

void Character::UpdateSprite() {
    int spriteSet;
    if (m_State == State::IDLE) {
        spriteSet = 0;
    } else {
        switch (m_Direction) {
            case Direction::DOWN:  spriteSet = 1; break;
            case Direction::UP:    spriteSet = 2; break;
            case Direction::LEFT:  spriteSet = 3; break;
            case Direction::RIGHT: spriteSet = 4; break;
            default:               spriteSet = 1; break;
        }
    }
    m_Drawable = m_Sprites[spriteSet][m_Frame];
}

void Character::HandleInput() {
    State prevState = m_State;
    Direction prevDir = m_Direction;
    m_State = State::IDLE;

    if (Util::Input::IsKeyPressed(Util::Keycode::DOWN) ||
        Util::Input::IsKeyPressed(Util::Keycode::S)) {
        m_Transform.translation.y -= m_Speed;
        m_Direction = Direction::DOWN;
        m_State = State::MOVING;
    } else if (Util::Input::IsKeyPressed(Util::Keycode::UP) ||
               Util::Input::IsKeyPressed(Util::Keycode::W)) {
        m_Transform.translation.y += m_Speed;
        m_Direction = Direction::UP;
        m_State = State::MOVING;
    } else if (Util::Input::IsKeyPressed(Util::Keycode::LEFT) ||
               Util::Input::IsKeyPressed(Util::Keycode::A)) {
        m_Transform.translation.x -= m_Speed;
        m_Direction = Direction::LEFT;
        m_State = State::MOVING;
    } else if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT) ||
               Util::Input::IsKeyPressed(Util::Keycode::D)) {
        m_Transform.translation.x += m_Speed;
        m_Direction = Direction::RIGHT;
        m_State = State::MOVING;
    }

    // reset frame when state or direction changes
    if (m_State != prevState || m_Direction != prevDir) {
        m_Frame = 0;
        m_FrameTimer = 0;
    }
}

void Character::AdvanceFrame() {
    int totalFrames = (m_State == State::IDLE) ? 7 : 8;

    m_FrameTimer++;
    if (m_FrameTimer >= m_FrameDelay) {
        m_FrameTimer = 0;
        m_Frame = (m_Frame + 1) % totalFrames;
    }
}

void Character::Update() {
    HandleInput();
    AdvanceFrame();
    UpdateSprite();
    Draw();  // GameObject::Draw() handles all the matrix stuff
}