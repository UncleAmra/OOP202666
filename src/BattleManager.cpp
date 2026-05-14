#include "BattleManager.hpp"
#include "MoveDatabase.hpp"
#include "Util/Logger.hpp"
#include <cstdlib>
#include <cmath>
#include <algorithm>

BattleManager::BattleManager(std::shared_ptr<Pokemon> playerPokemon, std::shared_ptr<Pokemon> enemyPokemon, bool isWild)
    : m_PlayerPokemon(playerPokemon)
    , m_IsWildBattle(isWild)
    , m_EnemyPokemon(enemyPokemon)
    , m_State(BattleState::SELECTING_ACTION) {
}

// ==========================================
// TYPE EFFECTIVENESS TABLE
// ==========================================
static float GetTypeEffectiveness(PokemonType attackType, PokemonType defenderType) {
    // Returns 0 (immune), 0.5 (not very effective), 1 (normal), 2 (super effective)
    // Only the non-1.0 matchups are listed — everything else defaults to 1.0

    using T = PokemonType;

    static const std::unordered_map<PokemonType,
        std::unordered_map<PokemonType, float>> chart = {
        { T::FIRE, {
            { T::GRASS, 2.0f }, { T::ICE, 2.0f }, { T::BUG, 2.0f },
            { T::FIRE, 0.5f },  { T::WATER, 0.5f }, { T::ROCK, 0.5f },
            { T::DRAGON, 0.5f }
        }},
        { T::WATER, {
            { T::FIRE, 2.0f }, { T::GROUND, 2.0f }, { T::ROCK, 2.0f },
            { T::WATER, 0.5f }, { T::GRASS, 0.5f }, { T::DRAGON, 0.5f }
        }},
        { T::GRASS, {
            { T::WATER, 2.0f }, { T::GROUND, 2.0f }, { T::ROCK, 2.0f },
            { T::FIRE, 0.5f }, { T::GRASS, 0.5f }, { T::POISON, 0.5f },
            { T::FLYING, 0.5f }, { T::BUG, 0.5f }, { T::DRAGON, 0.5f }
        }},
        { T::ELECTRIC, {
            { T::WATER, 2.0f }, { T::FLYING, 2.0f },
            { T::ELECTRIC, 0.5f }, { T::GRASS, 0.5f }, { T::DRAGON, 0.5f },
            { T::GROUND, 0.0f }
        }},
        { T::ICE, {
            { T::GRASS, 2.0f }, { T::GROUND, 2.0f },
            { T::FLYING, 2.0f }, { T::DRAGON, 2.0f },
            { T::FIRE, 0.5f }, { T::WATER, 0.5f }, { T::ICE, 0.5f }
        }},
        { T::FIGHTING, {
            { T::NORMAL, 2.0f }, { T::ICE, 2.0f }, { T::ROCK, 2.0f },
            { T::POISON, 0.5f }, { T::BUG, 0.5f }, { T::FLYING, 0.5f },
            { T::PSYCHIC, 0.5f },
            { T::GHOST, 0.0f }
        }},
        { T::POISON, {
            { T::GRASS, 2.0f }, { T::BUG, 2.0f },
            { T::POISON, 0.5f }, { T::GROUND, 0.5f },
            { T::ROCK, 0.5f }, { T::GHOST, 0.5f }
        }},
        { T::GROUND, {
            { T::FIRE, 2.0f }, { T::ELECTRIC, 2.0f }, { T::POISON, 2.0f },
            { T::ROCK, 2.0f },
            { T::GRASS, 0.5f }, { T::BUG, 0.5f },
            { T::FLYING, 0.0f }
        }},
        { T::FLYING, {
            { T::GRASS, 2.0f }, { T::FIGHTING, 2.0f }, { T::BUG, 2.0f },
            { T::ELECTRIC, 0.5f }, { T::ROCK, 0.5f }
        }},
        { T::PSYCHIC, {
            { T::FIGHTING, 2.0f }, { T::POISON, 2.0f },
            { T::PSYCHIC, 0.5f },
            { T::GHOST, 0.0f }
        }},
        { T::BUG, {
            { T::GRASS, 2.0f }, { T::POISON, 2.0f }, { T::PSYCHIC, 2.0f },
            { T::FIRE, 0.5f }, { T::FIGHTING, 0.5f }, { T::FLYING, 0.5f }
        }},
        { T::ROCK, {
            { T::FIRE, 2.0f }, { T::ICE, 2.0f },
            { T::FLYING, 2.0f }, { T::BUG, 2.0f },
            { T::FIGHTING, 0.5f }, { T::GROUND, 0.5f }
        }},
        { T::GHOST, {
            { T::GHOST, 2.0f },
            { T::PSYCHIC, 0.0f },
            { T::NORMAL, 0.0f }
        }},
        { T::DRAGON, {
            { T::DRAGON, 2.0f }
        }}
    };

    auto outer = chart.find(attackType);
    if (outer == chart.end()) return 1.0f;

    auto inner = outer->second.find(defenderType);
    if (inner == outer->second.end()) return 1.0f;

    return inner->second;
}

// ==========================================
// DAMAGE CALCULATION
// ==========================================
int BattleManager::CalculateDamage(Pokemon* attacker, Pokemon* defender,
                                    const std::string& moveName) {
    if (!MoveDatabase::HasMove(moveName)) {
        LOG_ERROR("Move '{}' not found in MoveDatabase!", moveName);
        return 0;
    }

    const MoveData& move = MoveDatabase::GetMove(moveName);
    LOG_INFO("[BattleManager] Move '{}' animation_key = '{}'", moveName, move.animation_key);
    if (move.category == MoveCategory::STATUS) return 0;
    if (move.power <= 0) return 0;

    // 1. Pick correct attack/defense stats based on move category
    int atk = (move.category == MoveCategory::PHYSICAL)
              ? attacker->GetAttack()
              : attacker->GetSpecialAttack();

    int def = (move.category == MoveCategory::PHYSICAL)
              ? defender->GetDefense()
              : defender->GetSpecialDefense();

    // 2. Gen 1 damage formula
    float base = ((2.0f * attacker->GetLevel() / 5.0f) + 2.0f)
                 * move.power * ((float)atk / (float)def)
                 / 50.0f + 2.0f;

    // 3. STAB (Same Type Attack Bonus) — 1.5x if move type matches attacker type
    float stab = 1.0f;
    if (move.type == attacker->GetType1() || move.type == attacker->GetType2()) {
        stab = 1.5f;
    }

    // 4. Type effectiveness against defender (handles dual types)
    float effectiveness = GetTypeEffectiveness(move.type, defender->GetType1());
    if (defender->GetType2() != PokemonType::NONE) {
        effectiveness *= GetTypeEffectiveness(move.type, defender->GetType2());
    }

    // 5. Random factor (217-255 / 255, same as Gen 1)
    float randomFactor = (float)(rand() % 39 + 217) / 255.0f;

    int damage = static_cast<int>(base * stab * effectiveness * randomFactor);

    // Always deal at least 1 damage if the move isn't immune
    if (effectiveness > 0.0f && damage < 1) damage = 1;

    return damage;
}

// ==========================================
// ACCURACY CHECK
// ==========================================
static bool AccuracyCheck(int moveAccuracy) {
    return (rand() % 100) < moveAccuracy;
}

// ==========================================
// EFFECTIVENESS MESSAGE
// ==========================================
static std::string EffectivenessMessage(PokemonType moveType,
                                         Pokemon* defender) {
    float e = GetTypeEffectiveness(moveType, defender->GetType1());
    if (defender->GetType2() != PokemonType::NONE) {
        e *= GetTypeEffectiveness(moveType, defender->GetType2());
    }

    if (e == 0.0f)  return "It doesn't affect " + defender->GetName() + "!";
    if (e < 1.0f)   return "It's not very effective...";
    if (e > 1.0f)   return "It's super effective!";
    return "";
}

// ==========================================
// SELECT ACTION (called by BattleUI)
// ==========================================
BattleManager::TurnResult BattleManager::SelectAction(Action action) {
    TurnResult result;
    result.playerFainted = false;
    result.enemyFainted = false;
    result.expGained = 0;

    switch (action) {
        case Action::FIGHT:
            m_State = BattleState::SELECTING_MOVE;
            result.message = "Choose a move!";
            break;

        case Action::RUN:
            if (m_IsWildBattle) {
                // Simple run formula — always succeeds for now
                m_State = BattleState::BATTLE_WON;
                result.message = "Got away safely!";
            } else {
                result.message = "Can't escape from a trainer battle!";
            }
            break;

        case Action::BAG:
            result.message = "Choose an item!";
            // BattleUI handles switching to bag screen
            break;

        case Action::POKEMON:
            result.message = "Choose a Pokemon!";
            // BattleUI handles switching to party screen
            break;
    }

    return result;
}

// ==========================================
// SELECT MOVE (called by BattleUI after player picks)
// ==========================================
BattleManager::TurnResult BattleManager::SelectMove(int moveIndex) {
    TurnResult result;
    result.playerFainted = false;
    result.enemyFainted = false;
    result.expGained = 0;

    auto moves = m_PlayerPokemon->GetMoves();
    if (moveIndex < 0 || moveIndex >= (int)moves.size()) {
        result.message = "No move in that slot!";
        return result;
    }

    // --- SPEED CHECK: faster Pokemon attacks first ---
    bool playerGoesFirst = m_PlayerPokemon->GetSpeed() >= m_EnemyPokemon->GetSpeed();

    if (playerGoesFirst) {
        result = ExecutePlayerMove(moveIndex);
        // INJECT TAG: Tell the UI what the enemy's HP is AFTER the player's attack!
        result.message += "\n[SYNC_ENEMY]" + std::to_string(m_EnemyPokemon->GetCurrentHP());

        if (!result.enemyFainted && !m_EnemyPokemon->IsFainted()) {
            TurnResult enemyResult = ExecuteEnemyMove();
            result.message += "\n" + enemyResult.message;
            // INJECT TAG: Tell the UI what the player's HP is AFTER the enemy's attack!
            result.message += "\n[SYNC_PLAYER]" + std::to_string(m_PlayerPokemon->GetCurrentHP());
            
            result.playerFainted = enemyResult.playerFainted;
        }
    } else {
        TurnResult enemyResult = ExecuteEnemyMove();
        result.message = enemyResult.message;
        // INJECT TAG: Tell the UI what the player's HP is AFTER the enemy's attack!
        result.message += "\n[SYNC_PLAYER]" + std::to_string(m_PlayerPokemon->GetCurrentHP());
        
        result.playerFainted = enemyResult.playerFainted;

        if (!result.playerFainted && !m_PlayerPokemon->IsFainted()) {
            TurnResult playerResult = ExecutePlayerMove(moveIndex);
            result.message += "\n" + playerResult.message;
            // INJECT TAG: Tell the UI what the enemy's HP is AFTER the player's attack!
            result.message += "\n[SYNC_ENEMY]" + std::to_string(m_EnemyPokemon->GetCurrentHP());
            
            result.enemyFainted = playerResult.enemyFainted;
            result.expGained = playerResult.expGained;
        }
    }

    // Update battle state
    if (result.playerFainted) {
        m_State = BattleState::BATTLE_LOST;
    } else if (result.enemyFainted) {
        m_State = BattleState::BATTLE_WON;
    } else {
        m_State = BattleState::SELECTING_ACTION;
    }

    return result;
}

// ==========================================
// EXECUTE PLAYER MOVE
// ==========================================
// ==========================================
// EXECUTE PLAYER MOVE
// ==========================================
BattleManager::TurnResult BattleManager::ExecutePlayerMove(int moveIndex) {
    TurnResult result;
    result.playerFainted = false;
    result.enemyFainted = false;
    result.expGained = 0;

    auto moves = m_PlayerPokemon->GetMoves();
    const std::string& moveName = moves[moveIndex];
    const MoveData& moveData = MoveDatabase::GetMove(moveName);

    result.message = m_PlayerPokemon->GetName() + " used " + moveName + "!";

    // 1. Accuracy Check
    if (!AccuracyCheck(moveData.accuracy)) {
        result.message += "\n" + m_PlayerPokemon->GetName() + "'s attack missed!";
        return result;
    }

    // 2. Animation Tag
    //result.message += "\n[ANIM:" + moveData.animation_key + ":TARGET_ENEMY]";

    // 3. Damage Calculation & Application
    int damage = CalculateDamage(m_PlayerPokemon.get(), m_EnemyPokemon.get(), moveName);
    m_EnemyPokemon->TakeDamage(damage);

    // 4. THE MISSING LINK: Sync Tag
    // We send the NEW current HP to the UI so it can animate the bar and check for 0
    result.message += "\n[SYNC_ENEMY]" + std::to_string(m_EnemyPokemon->GetCurrentHP());

    // 5. Effectiveness
    std::string effMsg = EffectivenessMessage(moveData.type, m_EnemyPokemon.get());
    if (!effMsg.empty()) result.message += "\n" + effMsg;

    // 6. Faint Check
    if (m_EnemyPokemon->IsFainted()) {
        result.enemyFainted = true;
        result.message += "\n" + m_EnemyPokemon->GetName() + " fainted!";

        result.expGained = (m_EnemyPokemon->GetLevel() * 50) / 7;
        result.message += "\n" + m_PlayerPokemon->GetName() + " gained " + std::to_string(result.expGained) + " EXP!";
        m_PlayerPokemon->GainExp(result.expGained);
    }

    return result;
}

// ==========================================
// EXECUTE ENEMY MOVE (simple random AI)
// ==========================================
// ==========================================
// EXECUTE ENEMY MOVE (simple random AI)
// ==========================================
BattleManager::TurnResult BattleManager::ExecuteEnemyMove() {
    TurnResult result;
    result.playerFainted = false;
    result.enemyFainted = false;

    auto moves = m_EnemyPokemon->GetMoves();
    int moveIndex = rand() % (int)moves.size();
    const std::string& moveName = moves[moveIndex];
    const MoveData& move = MoveDatabase::GetMove(moveName);

    result.message = m_EnemyPokemon->GetName() + " used " + moveName + "!";

    if (!AccuracyCheck(move.accuracy)) {
        result.message += "\n" + m_EnemyPokemon->GetName() + "'s attack missed!";
        return result;
    }

    // Animation Tag
    result.message += "\n[ANIM:" + move.animation_key + ":TARGET_PLAYER]";

    // Damage & Sync
    int damage = CalculateDamage(m_EnemyPokemon.get(), m_PlayerPokemon.get(), moveName);
    m_PlayerPokemon->TakeDamage(damage);

    // INJECT SYNC TAG: Updates player's HP bar and triggers player faint if HP <= 0
    result.message += "\n[SYNC_PLAYER]" + std::to_string(m_PlayerPokemon->GetCurrentHP());

    std::string effMsg = EffectivenessMessage(move.type, m_PlayerPokemon.get());
    if (!effMsg.empty()) result.message += "\n" + effMsg;

    if (m_PlayerPokemon->IsFainted()) {
        result.playerFainted = true;
        result.message += "\n" + m_PlayerPokemon->GetName() + " fainted!";
    }

    return result;
}
// ==========================================
// THROW POKEBALL
// ==========================================
BattleManager::TurnResult BattleManager::ThrowBall() {
    TurnResult result;
    result.playerFainted = false;
    result.enemyFainted = false;
    result.expGained = 0;

    if (!m_IsWildBattle) {
        result.message = "You can't catch a trainer's Pokemon!";
        return result;
    }

    int catchChance = CalculateCatchRate();

    if (rand() % 255 < catchChance) {
        m_State = BattleState::BATTLE_WON;
        result.message = m_EnemyPokemon->GetName() + " was caught!";
        result.enemyFainted = true; // Reuse this flag to signal "battle over"
    } else {
        result.message = m_EnemyPokemon->GetName() + " broke free!";
        // Enemy gets a free attack after breaking out
        TurnResult enemyTurn = ExecuteEnemyMove();
        result.message += "\n" + enemyTurn.message;
        result.playerFainted = enemyTurn.playerFainted;
    }

    return result;
}

// ==========================================
// CATCH RATE FORMULA (simplified Gen 1)
// ==========================================
int BattleManager::CalculateCatchRate() {
    // Formula: (maxHP * 255 * 4) / (currentHP * catchRate)
    // Using catchRate = 45 (Rattata level) as default
    // Higher result = easier to catch
    int maxHP = m_EnemyPokemon->GetMaxHP();
    int currentHP = std::max(1, m_EnemyPokemon->GetCurrentHP());
    int baseCatchRate = 45; // Standard wild Pokemon rate

    int catchValue = (maxHP * 255 * 4) / (currentHP * baseCatchRate);
    return std::min(catchValue, 255); // Cap at 255
}

bool BattleManager::TryCatchPokemon(std::shared_ptr<Pokemon> target, float ballMultiplier) {
    float maxHP = target->GetMaxHP();
    float currentHP = target->GetCurrentHP();
    int catchRate = target->GetCatchRate(); 

    // The Gen 3/4 Catch Formula
    float hpFactor = ((3.0f * maxHP) - (2.0f * currentHP)) / (3.0f * maxHP);
    int finalCatchValue = static_cast<int>(hpFactor * catchRate * ballMultiplier);

    if (finalCatchValue >= 255) return true;

    int randomRoll = std::rand() % 256;
    return randomRoll <= finalCatchValue;
}

void BattleManager::UseItem(std::shared_ptr<Character> player, const std::string& itemName) {
    // 1. Verify the player has the item
    if (player->GetItemCount(itemName) <= 0) {
        LOG_INFO("Player does not have any %s left!", itemName.c_str());
        return;
    }

    // 2. Consume the item
    player->RemoveItem(itemName, 1);

    // ==========================================
    // POKEBALL LOGIC
    // ==========================================
    if (itemName == "Pokeball" || itemName == "Great Ball" || itemName == "Ultra Ball") {
        if (!m_IsWildBattle) {
            LOG_INFO("The trainer blocked the ball! Don't be a thief!");
            m_State = BattleState::EXECUTING_ENEMY_TURN; // You lose your turn!
            return;
        }

        // Determine multiplier based on name
        float multiplier = 1.0f;
        if (itemName == "Great Ball") multiplier = 1.5f;
        if (itemName == "Ultra Ball") multiplier = 2.0f;

        LOG_INFO("You threw a %s!", itemName.c_str());
        bool caught = TryCatchPokemon(m_EnemyPokemon, multiplier);

        if (caught) {
            LOG_INFO("Gotcha! %s was caught!", m_EnemyPokemon->GetName().c_str());
            
            // Stamp the Pokemon with its new home
            m_EnemyPokemon->SetCaughtBall(itemName);

            // Add to party
            if (player->AddPokemon(m_EnemyPokemon)) {
                LOG_INFO("Added %s to your party!", m_EnemyPokemon->GetName().c_str());
            } else {
                LOG_INFO("Party full! %s was sent to the PC!", m_EnemyPokemon->GetName().c_str());
            }
            
            m_State = BattleState::BATTLE_WON; // End the battle!
        } else {
            LOG_INFO("Oh no! The Pokemon broke free!");
            m_State = BattleState::EXECUTING_ENEMY_TURN; // Enemy gets to attack
        }
        return;
    }

    // ==========================================
    // HEALING ITEM LOGIC (Example)
    // ==========================================
    if (itemName == "Potion") {
        m_PlayerPokemon->Heal(20);
        LOG_INFO("You used a Potion! %s recovered 20 HP.", m_PlayerPokemon->GetName().c_str());
        m_State = BattleState::EXECUTING_ENEMY_TURN;
        return;
    }
}


BattleManager::TurnResult BattleManager::ProcessEnemyTurn() {
    return ExecuteEnemyMove();
}