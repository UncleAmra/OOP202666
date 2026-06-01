#include "PokemonDatabase.hpp"
#include "Util/Logger.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::unordered_map<std::string, PokemonSpeciesData> PokemonDatabase::s_Species;

// Helper to convert string -> PokemonType
static PokemonType StringToType(const std::string& s) {
    static const std::unordered_map<std::string, PokemonType> map = {
        {"NONE", PokemonType::NONE},
        {"NORMAL", PokemonType::NORMAL},
        {"FIRE", PokemonType::FIRE},
        {"WATER", PokemonType::WATER},
        {"GRASS", PokemonType::GRASS},
        {"ELECTRIC", PokemonType::ELECTRIC},
        {"ICE", PokemonType::ICE},
        {"FIGHTING", PokemonType::FIGHTING},
        {"POISON", PokemonType::POISON},
        {"GROUND", PokemonType::GROUND},
        {"FLYING", PokemonType::FLYING},
        {"PSYCHIC", PokemonType::PSYCHIC},
        {"BUG", PokemonType::BUG},
        {"ROCK", PokemonType::ROCK},
        {"GHOST", PokemonType::GHOST},
        {"DRAGON", PokemonType::DRAGON},
        {"DARK", PokemonType::DARK},
        {"STEEL", PokemonType::STEEL},
        {"FAIRY", PokemonType::FAIRY}
    };
    auto it = map.find(s);
    return (it != map.end()) ? it->second : PokemonType::NONE;
}

void PokemonDatabase::Init() {
    std::string path = std::string(RESOURCE_DIR) + "/data/pokemon.json";
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Cannot open {}", path);
        return;
    }

    json data;
    file >> data;

    for (const auto& sp : data["species"]) {
        PokemonSpeciesData species;
        species.id          = sp["id"];
        species.name        = sp["name"];
        species.type1       = StringToType(sp["type1"]);
        species.type2       = StringToType(sp.value("type2", "NONE"));
        species.baseHP      = sp["baseHP"];
        species.baseAttack  = sp["baseAttack"];
        species.baseDefense = sp["baseDefense"];
        species.baseSpAtk   = sp["baseSpAtk"];
        species.baseSpDef   = sp["baseSpDef"];
        species.baseSpeed   = sp["baseSpeed"];
        species.catchRate   = sp.value("catchRate", 45);
        species.baseExp     = sp.value("baseExp", 64);

        // Learnset
        if (sp.contains("moves")) {
            for (const auto& moveEntry : sp["moves"]) {
                species.levelUpMoves.emplace_back(
                    moveEntry["level"].get<int>(),
                    moveEntry["move"].get<std::string>()
                );
            }
        }

        s_Species[species.name] = species;
    }

    LOG_INFO("Loaded {} Pokémon species from JSON.", s_Species.size());
}

std::shared_ptr<Pokemon> PokemonDatabase::CreatePokemon(const std::string& speciesName, int level) {
    auto it = s_Species.find(speciesName);
    if (it == s_Species.end()) {
        LOG_ERROR("Pokemon species not found: {}", speciesName);
        return nullptr;
    }

    const auto& data = it->second;

    auto pkmn = std::make_shared<Pokemon>(
        data.name, level, data.type1, data.type2,
        data.baseHP, data.baseAttack, data.baseDefense,
        data.baseSpAtk, data.baseSpDef, data.baseSpeed,
        data.catchRate
    );

    // Teach moves that are available at this level
    for (const auto& [learnLevel, moveName] : data.levelUpMoves) {
        if (learnLevel <= level) {
            pkmn->LearnMove(moveName);
        }
    }

    return pkmn;
}

const PokemonSpeciesData* PokemonDatabase::GetSpecies(const std::string& name) {
    auto it = s_Species.find(name);
    return (it != s_Species.end()) ? &it->second : nullptr;
}