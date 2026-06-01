#pragma once

#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include <memory>
#include <vector>
#include <string>

class StartMenu {
public:
    enum class Option {
        NONE = -1,
        POKEMON,
        BAG,
        SAVE,
        EXIT,
        CANCEL       // returned when Escape is pressed
    };

    struct Item {
        std::string label;
        Option      value;      // which enum this line corresponds to
    };

    explicit StartMenu(std::shared_ptr<Util::Renderer> renderer);
    ~StartMenu() = default;

    void SetVisible(bool visible);
    Option Update();           // returns selected option, or NONE/CANCEL

private:
    void BuildMenuGraphics();  // (re)create text objects from m_Items
    void UpdateCursorPosition();
    Option ProcessInput();     // handles keys, returns chosen option

    // ── Configurable layout ────────────────────────────────────────
    static constexpr float BOX_SCALE_X      = 1.0f;
    static constexpr float BOX_POS_X        = 331.5f;
    static constexpr float BOX_POS_Y        = 0.0f;
    static constexpr float TEXT_START_X     = 170.0f;
    static constexpr float TEXT_START_Y     = 220.0f;   // top of first line
    static constexpr float CURSOR_X         = 60.0f;
    static constexpr float CURSOR_OFFSET_X  = -20.0f;   // fine‑tune relative to text
    static constexpr float LINE_SPACING     = 40.0f;
    static constexpr int   INPUT_COOLDOWN   = 10;       // frames between moves
    static constexpr float TEXT_LEFT_MARGIN = 100.0f;   // change to whatever you like

    std::shared_ptr<Util::Renderer> m_Renderer;

    // Master list – add/remove/reorder items here and nothing else needs to change
    const std::vector<Item> m_Items = {
        {"POKEMON", Option::POKEMON},
        {"BAG",     Option::BAG},
        {"SAVE",    Option::SAVE},
        {"EXIT",    Option::EXIT}
    };

    int m_CursorIndex = 0;
    int m_InputTimer  = 0;

    // UI elements
    std::shared_ptr<Util::GameObject> m_BoxUI;
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemTexts;
    std::shared_ptr<Util::GameObject> m_CursorUI;
};