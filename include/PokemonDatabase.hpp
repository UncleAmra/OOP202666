#pragma once
#include "Pokemon.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

struct PokemonSpeciesData {
    int id;
    std::string name;
    PokemonType type1;
    PokemonType type2;
    int baseHP, baseAttack, baseDefense;
    int baseSpAtk, baseSpDef, baseSpeed;
    int catchRate;
    int baseExp;
    std::vector<std::pair<int, std::string>> levelUpMoves; // (level, move name)
};

class PokemonDatabase {
public:
    static void Init();
    static std::shared_ptr<Pokemon> CreatePokemon(const std::string& speciesName, int level);
    static const PokemonSpeciesData* GetSpecies(const std::string& name);

private:
    static std::unordered_map<std::string, PokemonSpeciesData> s_Species;
};