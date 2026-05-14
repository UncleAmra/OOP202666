#pragma once
#include "Pokemon.hpp"
#include "Character.hpp"
#include <memory>
#include <string>
#include <vector>

class BattleManager {
public:
    enum class BattleState {
        SELECTING_ACTION,
        SELECTING_MOVE,
        EXECUTING_PLAYER_TURN,
        EXECUTING_ENEMY_TURN,
        SHOWING_MESSAGE,
        BATTLE_WON,
        BATTLE_LOST,
        CATCHING,
        BATTLE_ESCAPED
    };


    enum class Action { FIGHT, BAG, POKEMON, RUN };

    struct TurnResult {
        std::string message;      // "Charmander used Scratch! It dealt 12 damage!"
        bool playerFainted;
        bool enemyFainted;
        int expGained;
    };

    BattleManager(std::shared_ptr<Pokemon> playerPokemon,
                  std::shared_ptr<Pokemon> enemyPokemon,
                  bool isWildBattle);

    // Called by App each frame when in BATTLE state
    BattleState GetState() const { return m_State; }
    void SetState(BattleState state) {m_State = state; }
     void SetPlayerPokemon(std::shared_ptr<Pokemon> newPokemon) {
         m_PlayerPokemon = newPokemon; 
    }

    std::shared_ptr<Pokemon> GetPlayerPokemon() { return m_PlayerPokemon; }
    std::shared_ptr<Pokemon> GetEnemyPokemon()  { return m_EnemyPokemon; }

    // Player input handlers — called by BattleUI/App
    TurnResult SelectAction(Action action);
    TurnResult SelectMove(int moveIndex);
    TurnResult ThrowBall();
       //battle items and pokeball logic
    void UseItem(std::shared_ptr<Character> player, const std::string& itemName);
    int CalculateCatchRate();
    bool TryCatchPokemon(std::shared_ptr<Pokemon> target, float ballMultiplier);
    TurnResult ExecuteEnemyMove();
    TurnResult ProcessEnemyTurn();
    std::string GetLastEnemyMove() const { return m_LastEnemyMove; }



private:
    bool m_IsWildBattle;
    std::string m_LastEnemyMove;
    std::shared_ptr<Pokemon> m_PlayerPokemon;
    std::shared_ptr<Pokemon> m_EnemyPokemon;
    BattleState m_State = BattleState::SELECTING_ACTION;
    

    TurnResult ExecutePlayerMove(int moveIndex);
    int CalculateDamage(Pokemon* attacker, Pokemon* defender, 
                        const std::string& moveName);

};