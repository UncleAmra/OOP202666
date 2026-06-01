#pragma once
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include "Pokemon.hpp"
#include <memory>
#include <vector>

class PokemonMenu {
public:
    explicit PokemonMenu(std::shared_ptr<Util::Renderer> renderer);
    ~PokemonMenu() = default;

    void Show(const std::vector<std::shared_ptr<Pokemon>>& party);
    void Hide();
    bool Update();                            // returns true when back is pressed
    int  GetSelectedIndex() const { return m_CursorIndex; }

private:
    void ClearSlots();                        // remove all per‑Pokemon objects
    void BuildSlots(const std::vector<std::shared_ptr<Pokemon>>& party);
    void UpdateCursorPosition();

    std::shared_ptr<Util::Renderer> m_Renderer;

    // --- Background ---
    std::shared_ptr<Util::GameObject> m_BoxUI;

    // --- Per‑Pokemon slots ---
    struct Slot {
        std::shared_ptr<Util::GameObject> sprite;   // small Pokémon image
        std::shared_ptr<Util::GameObject> text;     // "Name Lv.X HP: Y/Z"
    };
    std::vector<Slot> m_Slots;

    // --- Cursor ---
    std::shared_ptr<Util::GameObject> m_CursorUI;

    int m_CursorIndex   = 0;
    int m_PartySize     = 0;
    int m_InputCooldown = 0;

    // --- Layout constants (tweak to your liking) ---
    static constexpr float SPRITE_LEFT_X   = -480.0f;   // left edge of sprites
    static constexpr float SPRITE_SCALE    = 2.0f;
    static constexpr float TEXT_LEFT_MARGIN = -340.0f;  // where info text starts
    static constexpr float SLOT_START_Y    = 180.0f;    // top slot Y
    static constexpr float SLOT_SPACING    = 60.0f;     // vertical space between slots
    static constexpr float CURSOR_OFFSET_X = -50.0f;    // cursor distance left of sprite
    static constexpr float CURSOR_Y_ADJUST = 5.0f;      // fine‑tune vertical cursor pos
    static constexpr int   INPUT_DELAY     = 10;        // frames between moves
};