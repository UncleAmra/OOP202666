// PokemonMenu.hpp
#pragma once
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include "Pokemon.hpp"
#include <memory>
#include <vector>

class PokemonMenu {
public:
    PokemonMenu(std::shared_ptr<Util::Renderer> renderer);
    
    // Pass the party in to display it
    void Show(const std::vector<std::shared_ptr<Pokemon>>& party);
    void Hide();
    
    // Returns true when the player presses back
    bool Update();
    int GetSelectedIndex() const { return m_CursorIndex; }

private:
    std::shared_ptr<Util::GameObject> m_BoxUI;
    std::shared_ptr<Util::GameObject> m_TextUI;
    std::shared_ptr<Util::Text> m_Text;
    std::shared_ptr<Util::GameObject> m_CursorUI;

    int m_CursorIndex   = 0;
    int m_PartySize     = 0;
    
    void UpdateCursorPosition();
    void BuildPartyText(const std::vector<std::shared_ptr<Pokemon>>& party);
};