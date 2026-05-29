#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "Player.hpp"
#include "StartMenu.hpp"
#include "BattleUI.hpp"
#include "InventoryMenu.hpp"
#include "PokemonMenu.hpp"
#include "Map.hpp"
#include "GameConfig.hpp"
#include "util/Text.hpp"

struct InteractionResult {
    std::vector<std::string> dialogueLines; 
    std::string specialAction;              
};

class App {
public:
    enum class State {
        START,
        UPDATE,
        DIALOGUE,         
        START_MENU,       
        POKEMON_MENU,     
        INVENTORY_MENU,
        BATTLE,   
        END
    };

    State GetCurrentState() const { return m_CurrentState; }
    
    std::shared_ptr<Util::GameObject> m_StartMenuBoxUI;
    std::shared_ptr<Util::GameObject> m_StartMenuCursorUI;
    std::shared_ptr<Util::GameObject> m_StartMenuTextUI;

    void Start();
    void Update();
    void End();

    std::vector<std::string> m_CurrentDialogueLines;
    size_t m_CurrentDialogueIndex = 0;
    std::shared_ptr<StartMenu> m_StartMenu;
    bool JustFinishedMoving() const { return m_JustFinishedMoving; }


private:
    void ValidTask();
    int m_SwapIndex = -1;
    std::shared_ptr<Util::Renderer> m_Renderer;
    std::shared_ptr<Map> m_Map;
    std::shared_ptr<Util::GameObject> m_DialogueBoxUI;
    
    State m_CurrentState = State::START;
    bool m_JustFinishedMoving = false;
    std::shared_ptr<Player> m_Character;
    std::shared_ptr<NPC> m_ActiveNPC = nullptr;

    // --- Your UI components ---
    std::shared_ptr<Util::GameObject> m_DialogueUI; // Confirmed GameObject
    std::shared_ptr<Util::Text> m_DialogueText;
    
    // Submenus
    std::shared_ptr<InventoryMenu> m_InventoryMenu;
    std::shared_ptr<PokemonMenu> m_PokemonMenu;
    std::shared_ptr<BattleUI> m_BattleUI;

    // ==========================================
    // RESTRUCTURING HELPER FUNCTIONS
    // ==========================================
    void InitSystems();
    void InitGameLoad();
    void InitUI();
    void PerformQuickSave();

    // State Processing Delegates
    void ProcessDialogueState();
    void ProcessStartMenuState();
    void ProcessPokemonMenuState();
    void ProcessOverworldUpdateState();
    void ProcessBattleState();

    // Overworld sub-helpers
    void HandleOverworldInteraction(int checkX, int checkY);
    void HandleOverworldWarping();
    void HandleOverworldEncounters();

    std::shared_ptr<Pokemon> GenerateWildPokemon(const std::string& mapPath);
    
    void HandleGlobalShortcuts();
    void OpenStartMenu();
    void CloseAllMenus();
    void ReturnToStartMenu();
};

#endif