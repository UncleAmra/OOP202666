#include "MoveDatabase.hpp"
#include "Util/Logger.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

std::unordered_map<std::string, MoveData> MoveDatabase::s_Moves;

// --- Helper Parsers ---
static PokemonType ParseType(const std::string& typeStr) {
    static const std::unordered_map<std::string, PokemonType> typeMap = {
        {"NORMAL", PokemonType::NORMAL}, {"FIRE", PokemonType::FIRE},
        {"WATER", PokemonType::WATER}, {"GRASS", PokemonType::GRASS},
        {"ELECTRIC", PokemonType::ELECTRIC}, {"PSYCHIC", PokemonType::PSYCHIC},
        {"FIGHTING", PokemonType::FIGHTING}, {"ROCK", PokemonType::ROCK},
        {"POISON", PokemonType::POISON}, {"DARK", PokemonType::DARK},
        {"FLYING", PokemonType::FLYING}, {"GROUND", PokemonType::GROUND},
        {"DRAGON", PokemonType::DRAGON}, {"BUG", PokemonType::BUG},
        {"GHOST", PokemonType::GHOST}, {"FAIRY", PokemonType::FAIRY}
    };
    
    auto it = typeMap.find(typeStr);
    if (it != typeMap.end()) {
        return it->second;
    }
    
    LOG_ERROR("Unknown PokemonType: {}. Defaulting to NORMAL.", typeStr);
    return PokemonType::NORMAL;
}

static MoveCategory ParseCategory(const std::string& catStr) {
    if (catStr == "PHYSICAL") return MoveCategory::PHYSICAL;
    if (catStr == "SPECIAL")  return MoveCategory::SPECIAL;
    if (catStr == "STATUS")   return MoveCategory::STATUS;
    
    LOG_ERROR("Unknown MoveCategory: {}. Defaulting to STATUS.", catStr);
    return MoveCategory::STATUS;
}

// --- Main Init ---
void MoveDatabase::Init() {
    // Open the JSON file (Adjust path to your project's directory structure)
    const std::string RES        = std::string(RESOURCE_DIR);
    const std::string MOVES = RES + "/data/moves.json";
    std::ifstream file(MOVES);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open moves.json! Moves database is empty.");
        return;
    }

    try {
        json j;
        file >> j;

        for (auto& el : j.items()) {
            const std::string& moveName = el.key();
            const auto& data = el.value();

            MoveData move;
            move.name     = moveName;
            move.type     = ParseType(data.value("type", "NORMAL"));
            move.category = ParseCategory(data.value("category", "STATUS"));
            move.power    = data.value("power", 0);
            move.accuracy = data.value("accuracy", 100);
            move.maxPP    = data.value("pp", 0);
            
            // --- NEW LINE: Extract the animation key with a safe fallback ---
            move.animation_key = data.value("animation_key", "Move:TACKLE");

            s_Moves[moveName] = move;
        }

        LOG_TRACE("MoveDatabase successfully loaded {} moves from JSON.", s_Moves.size());

    } catch (const json::exception& e) {
        LOG_ERROR("JSON Parsing error in moves.json: {}", e.what());
    }
}

const MoveData& MoveDatabase::GetMove(const std::string& moveName) {
    auto it = s_Moves.find(moveName);
    if (it == s_Moves.end()) {
        LOG_ERROR("Move '{}' not found!", moveName);
        static MoveData empty = { "Unknown", PokemonType::NORMAL,
                                   MoveCategory::STATUS, 0, 0, 0 };
        return empty;
    }
    return it->second;
}

bool MoveDatabase::HasMove(const std::string& moveName) {
    return s_Moves.count(moveName) > 0;
}