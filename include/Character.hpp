#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "pch.hpp"
#include "Util/GameObject.hpp"
#include "Util/Animation.hpp" // We use Animation now instead of Image
#include <vector>
#include <memory>

class Character : public Util::GameObject {
public:
    enum class Direction { DOWN, UP, LEFT, RIGHT };
    enum class State { IDLE, MOVING };

    Character(float x, float y);

    void Update();

private:
    void HandleInput();
    void LoadSprites();
    void UpdateSprite();


    float m_Speed = 3.0f;

    Direction m_Direction = Direction::DOWN;
    State m_State = State::IDLE;

    std::shared_ptr<Util::Animation> m_AnimDown;
    std::shared_ptr<Util::Animation> m_AnimUp;
    std::shared_ptr<Util::Animation> m_AnimLeft;
    std::shared_ptr<Util::Animation> m_AnimRight;
    
    std::shared_ptr<Util::Animation> m_CurrentAnimation;
};

#endif