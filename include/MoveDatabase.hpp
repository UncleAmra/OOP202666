#pragma once

#include <string>
#include <unordered_map>
#include "Pokemon.hpp" // Assuming this is where PokemonType is defined

// Defines how the move calculates damage
enum class MoveCategory {
    PHYSICAL, // Uses Attacker's Attack vs Defender's Defense
    SPECIAL,  // Uses Attacker's Sp. Atk vs Defender's Sp. Def
    STATUS    // Does not deal direct damage (e.g., Growl, Tail Whip)
};

struct MoveData {
    std::string name;
    PokemonType type;
    MoveCategory category;
    int power;
    int accuracy;      // 1 to 100
    int maxPP;
    std::string animation_key;
};

class MoveDatabase {
public:
    // Called once when the game starts to load all moves
    static void Init();
    
    // Quick lookup for move stats
    static const MoveData& GetMove(const std::string& moveName);
    
    // Safety check
    static bool HasMove(const std::string& moveName);

private:
    static std::unordered_map<std::string, MoveData> s_Moves;
};