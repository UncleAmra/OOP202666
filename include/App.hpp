#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "Character.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End();

private:
    void ValidTask();

private:
    State m_CurrentState = State::START;
    Character m_Character{0.0f, 0.0f};
};

#endif