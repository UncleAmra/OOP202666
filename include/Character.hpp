#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "pch.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
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
    void AdvanceFrame();
    void LoadSprites();
    void UpdateSprite();

    float m_Speed = 3.0f;

    Direction m_Direction = Direction::DOWN;
    State m_State = State::IDLE;

    int m_Frame = 0;
    int m_FrameTimer = 0;
    int m_FrameDelay = 8;

    // [0]=idle(7), [1]=down(8), [2]=up(8), [3]=left(8), [4]=right(8)
    std::vector<std::vector<std::shared_ptr<Util::Image>>> m_Sprites;
};

#endif