// PokemonDatabase.cpp
#include "PokemonDatabase.hpp"
#include "MoveDatabase.hpp"
#include "Util/Logger.hpp"
#include <cmath>

std::unordered_map<std::string, PokemonSpecies> PokemonDatabase::s_Species;

int PokemonDatabase::CalcHP(int baseHP, int level) {
    // Simplified Gen 1 HP formula
    return static_cast<int>((baseHP * 2 * level) / 100.0f) + level + 10;
}

int PokemonDatabase::CalcStat(int baseStat, int level) {
    return static_cast<int>((baseStat * 2 * level) / 100.0f) + 5;
}

std::shared_ptr<Pokemon> PokemonDatabase::CreatePokemon(
    const std::string& name, int level) {
    
    if (!HasSpecies(name)) {
        LOG_ERROR("Unknown Pokemon species: {}", name);
        return nullptr;
    }

    const PokemonSpecies& species = GetSpecies(name);

    int hp  = CalcHP  (species.baseHP,          level);
    int atk = CalcStat(species.baseAttack,      level);
    int def = CalcStat(species.baseDefense,     level);
    int spa = CalcStat(species.baseSpAttack,    level);
    int spd = CalcStat(species.baseSpDefense,   level);
    int spe = CalcStat(species.baseSpeed,       level);

    auto pokemon = std::make_shared<Pokemon>(
        name, level,
        species.type1, species.type2,
        hp, atk, def, spa, spd, spe, species.catchRate);

    // Give all moves learnable at or below this level
    for (const auto& lm : species.levelUpMoves) {
        if (lm.level <= level) {
            pokemon->LearnMove(lm.moveName);
        }
    }
    return pokemon;
}

void PokemonDatabase::Init() {

    // ==========================================
    // GEN 1 STARTERS & EVOLUTIONS
    // ==========================================

    s_Species["Bulbasaur"] = {
        1, "Bulbasaur",
        PokemonType::GRASS, PokemonType::POISON,
        45, 49, 49, 65, 65, 45, 45, 64,
        {{ 1,"Tackle"},{1,"Growl"},{7,"Vine Whip"},
         {13,"Leech Seed"},{22,"Razor Leaf"},{30,"Solar Beam"}},
        "bulbasaur"
    };
    s_Species["Ivysaur"] = {
        2, "Ivysaur",
        PokemonType::GRASS, PokemonType::POISON,
        60, 62, 63, 80, 80, 60, 45, 141,
        {{ 1,"Tackle"},{1,"Growl"},{7,"Vine Whip"},
         {13,"Leech Seed"},{22,"Razor Leaf"},{30,"Solar Beam"},{43,"Sweet Scent"}},
        "ivysaur"
    };
    s_Species["Venusaur"] = {
        3, "Venusaur",
        PokemonType::GRASS, PokemonType::POISON,
        80, 82, 83, 100, 100, 80, 45, 208,
        {{ 1,"Tackle"},{1,"Growl"},{7,"Vine Whip"},
         {13,"Leech Seed"},{22,"Razor Leaf"},{30,"Solar Beam"},{43,"Petal Dance"}},
        "venusaur"
    };

    s_Species["Charmander"] = {
        4, "Charmander",
        PokemonType::FIRE, PokemonType::NONE,
        39, 52, 43, 60, 50, 65, 45, 65,
        {{ 1,"Scratch"},{1,"Growl"},{7,"Ember"},
         {13,"Smokescreen"},{25,"Slash"},{31,"Flamethrower"},{38,"Fire Spin"}},
        "charmander"
    };
    s_Species["Charmeleon"] = {
        5, "Charmeleon",
        PokemonType::FIRE, PokemonType::NONE,
        58, 64, 58, 80, 65, 80, 45, 142,
        {{ 1,"Scratch"},{1,"Growl"},{7,"Ember"},
         {13,"Smokescreen"},{25,"Slash"},{31,"Flamethrower"},{38,"Fire Spin"}},
        "charmeleon"
    };
    s_Species["Charizard"] = {
        6, "Charizard",
        PokemonType::FIRE, PokemonType::FLYING,
        78, 84, 78, 109, 85, 100, 45, 209,
        {{ 1,"Scratch"},{1,"Growl"},{7,"Ember"},
         {25,"Slash"},{31,"Flamethrower"},{38,"Fire Spin"},{56,"Wing Attack"}},
        "charizard"
    };

    s_Species["Squirtle"] = {
        7, "Squirtle",
        PokemonType::WATER, PokemonType::NONE,
        44, 48, 65, 50, 64, 43, 45, 66,
        {{ 1,"Tackle"},{7,"Water Gun"},
         {13,"Withdraw"},{22,"Bubble"},{28,"Surf"}},
        "squirtle"
    };
    s_Species["Wartortle"] = {
        8, "Wartortle",
        PokemonType::WATER, PokemonType::NONE,
        59, 63, 80, 65, 80, 58, 45, 143,
        {{ 1,"Tackle"},{1,"Tail Whip"},{7,"Water Gun"},
         {13,"Withdraw"},{22,"Bubble"},{28,"Surf"},{40,"Hydro Pump"}},
        "wartortle"
    };
    s_Species["Blastoise"] = {
        9, "Blastoise",
        PokemonType::WATER, PokemonType::NONE,
        79, 83, 100, 85, 105, 78, 45, 210,
        {{ 1,"Tackle"},{1,"Tail Whip"},{7,"Water Gun"},
         {28,"Surf"},{40,"Hydro Pump"},{52,"Rain Dance"}},
        "blastoise"
    };

    // ==========================================
    // EARLY ROUTE BIRDS
    // ==========================================

    s_Species["Pidgey"] = {
        16, "Pidgey",
        PokemonType::NORMAL, PokemonType::FLYING,
        40, 45, 40, 35, 35, 56, 255, 55,
        {{ 1,"Tackle"},{1,"Growl"},{5,"Quick Attack"},{12,"Wing Attack"}},
        "pidgey"
    };
    s_Species["Pidgeotto"] = {
        17, "Pidgeotto",
        PokemonType::NORMAL, PokemonType::FLYING,
        63, 60, 55, 50, 50, 71, 120, 113,
        {{ 1,"Tackle"},{1,"Growl"},{5,"Quick Attack"},{12,"Wing Attack"},{21,"Agility"}},
        "pidgeotto"
    };
    s_Species["Pidgeot"] = {
        18, "Pidgeot",
        PokemonType::NORMAL, PokemonType::FLYING,
        83, 80, 75, 70, 70, 101, 45, 172,
        {{ 1,"Tackle"},{1,"Growl"},{5,"Quick Attack"},
         {12,"Wing Attack"},{21,"Agility"},{36,"Wing Attack"}},
        "pidgeot"
    };

    s_Species["Spearow"] = {
        21, "Spearow",
        PokemonType::NORMAL, PokemonType::FLYING,
        40, 60, 30, 31, 31, 70, 255, 58,
        {{ 1,"Tackle"},{1,"Growl"},{9,"Quick Attack"},{15,"Wing Attack"}},
        "spearow"
    };
    s_Species["Fearow"] = {
        22, "Fearow",
        PokemonType::NORMAL, PokemonType::FLYING,
        65, 90, 65, 61, 61, 100, 90, 162,
        {{ 1,"Tackle"},{1,"Growl"},{9,"Quick Attack"},
         {15,"Wing Attack"},{25,"Fury Attack"},{38,"Drill Peck"}},
        "fearow"
    };

    s_Species["Starly"] = {
        396, "Starly",
        PokemonType::NORMAL, PokemonType::FLYING,
        40, 55, 30, 30, 30, 60, 255, 54,
        {{ 1,"Tackle"},{1,"Growl"},{5,"Quick Attack"},{9,"Wing Attack"}},
        "starly"
    };

    // ==========================================
    // EARLY ROUTE RODENTS
    // ==========================================

    s_Species["Rattata"] = {
        19, "Rattata",
        PokemonType::NORMAL, PokemonType::NONE,
        30, 56, 35, 25, 35, 72, 255, 51,
        {{ 1,"Tackle"},{1,"Tail Whip"},{7,"Quick Attack"},{14,"Hyper Fang"}},
        "rattata"
    };
    s_Species["Raticate"] = {
        20, "Raticate",
        PokemonType::NORMAL, PokemonType::NONE,
        55, 81, 60, 50, 70, 97, 127, 116,
        {{ 1,"Tackle"},{1,"Tail Whip"},{7,"Quick Attack"},
         {14,"Hyper Fang"},{27,"Super Fang"},{41,"Double-Edge"}},
        "raticate"
    };

    // ==========================================
    // CATERPILLAR LINE
    // ==========================================

    s_Species["Caterpie"] = {
        10, "Caterpie",
        PokemonType::BUG, PokemonType::NONE,
        45, 30, 35, 20, 20, 45, 255, 53,
        {{ 1,"Tackle"},{1,"Growl"}},
        "caterpie"
    };
    s_Species["Metapod"] = {
        11, "Metapod",
        PokemonType::BUG, PokemonType::NONE,
        50, 20, 55, 25, 25, 30, 120, 72,
        {{ 1,"Tackle"},{1,"Growl"}},
        "metapod"
    };
    s_Species["Butterfree"] = {
        12, "Butterfree",
        PokemonType::BUG, PokemonType::FLYING,
        60, 45, 50, 80, 80, 70, 45, 160,
        {{ 1,"Confusion"},{10,"Poison Powder"},
         {13,"Gust"},{26,"Psybeam"},{32,"Wing Attack"}},
        "butterfree"
    };

    s_Species["Weedle"] = {
        13, "Weedle",
        PokemonType::BUG, PokemonType::POISON,
        40, 35, 30, 20, 20, 50, 255, 52,
        {{ 1,"Tackle"},{1,"Growl"}},
        "weedle"
    };
    s_Species["Kakuna"] = {
        14, "Kakuna",
        PokemonType::BUG, PokemonType::POISON,
        45, 25, 50, 25, 25, 35, 120, 71,
        {{ 1,"Tackle"},{1,"Growl"}},
        "kakuna"
    };
    s_Species["Beedrill"] = {
        15, "Beedrill",
        PokemonType::BUG, PokemonType::POISON,
        65, 80, 40, 45, 80, 75, 45, 159,
        {{ 1,"Tackle"},{1,"Growl"},{20,"Fury Attack"},
         {35,"Pin Missile"},{45,"Twineedle"}},
        "beedrill"
    };

    // ==========================================
    // ELECTRIC
    // ==========================================

    s_Species["Pikachu"] = {
        25, "Pikachu",
        PokemonType::ELECTRIC, PokemonType::NONE,
        35, 55, 30, 50, 40, 90, 190, 82,
        {{ 1,"Thunder Shock"},{1,"Growl"},
         {9,"Quick Attack"},{16,"Thunderbolt"},{26,"Agility"},{33,"Thunder"}},
        "pikachu"
    };
    s_Species["Raichu"] = {
        26, "Raichu",
        PokemonType::ELECTRIC, PokemonType::NONE,
        60, 90, 55, 90, 80, 110, 75, 122,
        {{ 1,"Thunder Shock"},{1,"Growl"},
         {9,"Quick Attack"},{16,"Thunderbolt"},{26,"Agility"},{33,"Thunder"}},
        "raichu"
    };

    s_Species["Magnemite"] = {
        81, "Magnemite",
        PokemonType::ELECTRIC, PokemonType::NONE,
        25, 35, 70, 95, 55, 45, 190, 89,
        {{ 1,"Thunder Shock"},{1,"Growl"},{21,"Thunderbolt"},{31,"Thunder"}},
        "magnemite"
    };
    s_Species["Magneton"] = {
        82, "Magneton",
        PokemonType::ELECTRIC, PokemonType::NONE,
        50, 60, 95, 120, 70, 70, 60, 161,
        {{ 1,"Thunder Shock"},{1,"Growl"},{21,"Thunderbolt"},{31,"Thunder"}},
        "magneton"
    };

    s_Species["Voltorb"] = {
        100, "Voltorb",
        PokemonType::ELECTRIC, PokemonType::NONE,
        40, 30, 50, 55, 55, 100, 190, 103,
        {{ 1,"Thunder Shock"},{9,"Thunderbolt"},{17,"Screech"},{25,"Thunder"}},
        "voltorb"
    };

    // ==========================================
    // WATER
    // ==========================================

    s_Species["Psyduck"] = {
        54, "Psyduck",
        PokemonType::WATER, PokemonType::NONE,
        50, 52, 48, 65, 50, 55, 190, 80,
        {{ 1,"Scratch"},{1,"Tail Whip"},{8,"Water Gun"},{16,"Confusion"},{26,"Psybeam"}},
        "psyduck"
    };
    s_Species["Golduck"] = {
        55, "Golduck",
        PokemonType::WATER, PokemonType::NONE,
        80, 82, 78, 95, 80, 85, 75, 174,
        {{ 1,"Scratch"},{1,"Tail Whip"},{8,"Water Gun"},
         {16,"Confusion"},{26,"Psybeam"},{38,"Surf"},{48,"Psychic"}},
        "golduck"
    };

    s_Species["Staryu"] = {
        120, "Staryu",
        PokemonType::WATER, PokemonType::NONE,
        30, 45, 55, 70, 55, 85, 225, 106,
        {{ 1,"Tackle"},{1,"Water Gun"},{17,"Swift"},{28,"Surf"},{36,"Psychic"}},
        "staryu"
    };

    s_Species["Magikarp"] = {
        129, "Magikarp",
        PokemonType::WATER, PokemonType::NONE,
        20, 10, 55, 15, 20, 80, 255, 20,
        {{ 1,"Tackle"},{15,"Flail"}},
        "magikarp"
    };
    s_Species["Gyarados"] = {
        130, "Gyarados",
        PokemonType::WATER, PokemonType::FLYING,
        95, 125, 79, 60, 100, 81, 45, 214,
        {{ 1,"Tackle"},{1,"Bite"},{20,"Dragon Rage"},
         {25,"Leer"},{32,"Hydro Pump"},{41,"Hyper Beam"}},
        "gyarados"
    };

    // ==========================================
    // PSYCHIC
    // ==========================================

    s_Species["Abra"] = {
        63, "Abra",
        PokemonType::PSYCHIC, PokemonType::NONE,
        25, 20, 15, 105, 55, 90, 200, 73,
        {{ 1,"Teleport"}},
        "abra"
    };
    s_Species["Kadabra"] = {
        64, "Kadabra",
        PokemonType::PSYCHIC, PokemonType::NONE,
        40, 35, 30, 120, 70, 105, 100, 145,
        {{ 1,"Confusion"},{16,"Disable"},{20,"Psybeam"},{27,"Psychic"},{38,"Kinesis"}},
        "kadabra"
    };
    s_Species["Alakazam"] = {
        65, "Alakazam",
        PokemonType::PSYCHIC, PokemonType::NONE,
        55, 50, 45, 135, 85, 120, 50, 186,
        {{ 1,"Confusion"},{16,"Disable"},
         {20,"Psybeam"},{27,"Psychic"},{38,"Recover"},{45,"Kinesis"}},
        "alakazam"
    };

    // ==========================================
    // ROCK / GROUND (BROCK'S TEAM)
    // ==========================================

    s_Species["Geodude"] = {
        74, "Geodude",
        PokemonType::ROCK, PokemonType::GROUND,
        40, 80, 100, 30, 30, 20, 255, 86,
        {{ 1,"Tackle"},{11,"Rock Throw"},{16,"Rock Slide"},{21,"Earthquake"}},
        "geodude"
    };
    s_Species["Graveler"] = {
        75, "Graveler",
        PokemonType::ROCK, PokemonType::GROUND,
        55, 95, 115, 45, 45, 35, 120, 134,
        {{ 1,"Tackle"},{11,"Rock Throw"},{16,"Rock Slide"},{21,"Earthquake"},{29,"Explosion"}},
        "graveler"
    };
    s_Species["Golem"] = {
        76, "Golem",
        PokemonType::ROCK, PokemonType::GROUND,
        80, 110, 130, 55, 65, 45, 45, 177,
        {{ 1,"Tackle"},{11,"Rock Throw"},{16,"Rock Slide"},
         {21,"Earthquake"},{29,"Explosion"},{36,"Double-Edge"}},
        "golem"
    };

    s_Species["Onix"] = {
        95, "Onix",
        PokemonType::ROCK, PokemonType::GROUND,
        35, 45, 160, 30, 45, 70, 45, 108,
        {{ 1,"Tackle"},{1,"Screech"},{15,"Rock Throw"},{19,"Rage"},{25,"Rock Slide"}},
        "onix"
    };

    s_Species["Sandshrew"] = {
        27, "Sandshrew",
        PokemonType::GROUND, PokemonType::NONE,
        50, 75, 85, 20, 30, 40, 255, 93,
        {{ 1,"Scratch"},{1,"Growl"},{10,"Rock Throw"},{17,"Slash"},{28,"Earthquake"}},
        "sandshrew"
    };
    s_Species["Sandslash"] = {
        28, "Sandslash",
        PokemonType::GROUND, PokemonType::NONE,
        75, 100, 110, 45, 55, 65, 90, 163,
        {{ 1,"Scratch"},{1,"Growl"},{10,"Rock Throw"},
         {17,"Slash"},{28,"Earthquake"},{38,"Sword Dance"}},
        "sandslash"
    };

    // ==========================================
    // POISON / GRASS
    // ==========================================

    s_Species["Oddish"] = {
        43, "Oddish",
        PokemonType::GRASS, PokemonType::POISON,
        45, 50, 55, 75, 65, 30, 255, 78,
        {{ 1,"Absorb"},{1,"Growl"},{15,"Acid"},{19,"Razor Leaf"},{26,"Solar Beam"}},
        "oddish"
    };
    s_Species["Gloom"] = {
        44, "Gloom",
        PokemonType::GRASS, PokemonType::POISON,
        60, 65, 70, 85, 75, 40, 120, 132,
        {{ 1,"Absorb"},{1,"Growl"},{15,"Acid"},{19,"Razor Leaf"},{26,"Solar Beam"},{33,"Petal Dance"}},
        "gloom"
    };
    s_Species["Vileplume"] = {
        45, "Vileplume",
        PokemonType::GRASS, PokemonType::POISON,
        75, 80, 85, 100, 90, 50, 45, 184,
        {{ 1,"Absorb"},{1,"Growl"},{15,"Acid"},
         {19,"Razor Leaf"},{26,"Solar Beam"},{33,"Petal Dance"},{42,"Stun Spore"}},
        "vileplume"
    };

    s_Species["Bellsprout"] = {
        69, "Bellsprout",
        PokemonType::GRASS, PokemonType::POISON,
        50, 75, 35, 70, 30, 40, 255, 84,
        {{ 1,"Vine Whip"},{1,"Growl"},{13,"Razor Leaf"},{15,"Acid"},{26,"Solar Beam"}},
        "bellsprout"
    };

    s_Species["Ekans"] = {
        23, "Ekans",
        PokemonType::POISON, PokemonType::NONE,
        35, 60, 44, 40, 54, 55, 255, 62,
        {{ 1,"Wrap"},{1,"Leer"},{9,"Poison Sting"},{14,"Bite"},{25,"Acid"}},
        "ekans"
    };
    s_Species["Arbok"] = {
        24, "Arbok",
        PokemonType::POISON, PokemonType::NONE,
        60, 85, 69, 65, 79, 80, 90, 147,
        {{ 1,"Wrap"},{1,"Leer"},{9,"Poison Sting"},
         {14,"Bite"},{25,"Acid"},{35,"Glare"},{48,"Crunch"}},
        "arbok"
    };

    // ==========================================
    // FIRE
    // ==========================================

    s_Species["Vulpix"] = {
        37, "Vulpix",
        PokemonType::FIRE, PokemonType::NONE,
        38, 41, 40, 50, 65, 65, 190, 63,
        {{ 1,"Ember"},{1,"Tail Whip"},{9,"Quick Attack"},{15,"Flamethrower"},{28,"Fire Spin"}},
        "vulpix"
    };
    s_Species["Ninetales"] = {
        38, "Ninetales",
        PokemonType::FIRE, PokemonType::NONE,
        73, 76, 75, 81, 100, 100, 75, 178,
        {{ 1,"Ember"},{1,"Tail Whip"},{9,"Quick Attack"},
         {15,"Flamethrower"},{28,"Fire Spin"},{50,"Confuse Ray"}},
        "ninetales"
    };

    s_Species["Growlithe"] = {
        58, "Growlithe",
        PokemonType::FIRE, PokemonType::NONE,
        55, 70, 45, 70, 50, 60, 190, 91,
        {{ 1,"Bite"},{1,"Roar"},{9,"Ember"},{18,"Leer"},{23,"Flamethrower"},{30,"Fire Blast"}},
        "growlithe"
    };
    s_Species["Arcanine"] = {
        59, "Arcanine",
        PokemonType::FIRE, PokemonType::NONE,
        90, 110, 80, 100, 80, 95, 75, 194,
        {{ 1,"Bite"},{1,"Roar"},{9,"Ember"},
         {18,"Leer"},{23,"Flamethrower"},{30,"Fire Blast"},{49,"Extreme Speed"}},
        "arcanine"
    };

    s_Species["Ponyta"] = {
        77, "Ponyta",
        PokemonType::FIRE, PokemonType::NONE,
        50, 85, 55, 65, 65, 90, 190, 152,
        {{ 1,"Tackle"},{1,"Growl"},{32,"Ember"},{39,"Flame Wheel"},{48,"Flamethrower"}},
        "ponyta"
    };

    // ==========================================
    // EEVEE FAMILY
    // ==========================================

    s_Species["Eevee"] = {
        133, "Eevee",
        PokemonType::NORMAL, PokemonType::NONE,
        55, 55, 50, 45, 65, 55, 45, 92,
        {{ 1,"Tackle"},{1,"Tail Whip"},{9,"Quick Attack"},{25,"Bite"},{33,"Double-Edge"}},
        "eevee"
    };
    s_Species["Vaporeon"] = {
        134, "Vaporeon",
        PokemonType::WATER, PokemonType::NONE,
        130, 65, 60, 110, 95, 65, 45, 196,
        {{ 1,"Tackle"},{1,"Tail Whip"},{9,"Quick Attack"},
         {25,"Bite"},{33,"Water Gun"},{41,"Surf"},{53,"Hydro Pump"}},
        "vaporeon"
    };
    s_Species["Jolteon"] = {
        135, "Jolteon",
        PokemonType::ELECTRIC, PokemonType::NONE,
        65, 65, 60, 110, 95, 130, 45, 197,
        {{ 1,"Tackle"},{1,"Tail Whip"},{9,"Quick Attack"},
         {25,"Bite"},{33,"Thunder Shock"},{41,"Thunderbolt"},{53,"Thunder"}},
        "jolteon"
    };
    s_Species["Flareon"] = {
        136, "Flareon",
        PokemonType::FIRE, PokemonType::NONE,
        65, 130, 60, 95, 110, 65, 45, 198,
        {{ 1,"Tackle"},{1,"Tail Whip"},{9,"Quick Attack"},
         {25,"Bite"},{33,"Ember"},{41,"Flamethrower"},{53,"Fire Blast"}},
        "flareon"
    };

    // ==========================================
    // GEN 2 STARTERS
    // ==========================================

    s_Species["Chikorita"] = {
        152, "Chikorita",
        PokemonType::GRASS, PokemonType::NONE,
        45, 49, 65, 49, 65, 45, 45, 64,
        {{ 1,"Tackle"},{1,"Growl"},{8,"Razor Leaf"},{15,"Reflect"},{22,"Synthesis"},{29,"Solar Beam"}},
        "chikorita"
    };
    s_Species["Cyndaquil"] = {
        155, "Cyndaquil",
        PokemonType::FIRE, PokemonType::NONE,
        39, 52, 43, 60, 50, 65, 45, 65,
        {{ 1,"Tackle"},{1,"Leer"},{6,"Smokescreen"},{12,"Ember"},{19,"Flamethrower"},{29,"Swift"}},
        "cyndaquil"
    };
    s_Species["Totodile"] = {
        158, "Totodile",
        PokemonType::WATER, PokemonType::NONE,
        50, 65, 64, 44, 48, 43, 45, 66,
        {{ 1,"Scratch"},{1,"Leer"},{7,"Water Gun"},{13,"Bite"},{20,"Surf"},{30,"Crunch"}},
        "totodile"
    };

    s_Species["Togepi"] = {
        175, "Togepi",
        PokemonType::NORMAL, PokemonType::NONE,
        35, 20, 65, 40, 65, 20, 190, 74,
        {{ 1,"Growl"},{1,"Charm"},{9,"Metronome"},{13,"Sweet Kiss"},{25,"Wish"}},
        "togepi"
    };

    s_Species["Marill"] = {
        183, "Marill",
        PokemonType::WATER, PokemonType::NONE,
        70, 20, 50, 20, 50, 40, 190, 88,
        {{ 1,"Tackle"},{1,"Bubble"},{9,"Water Gun"},{18,"Surf"},{25,"Double-Edge"}},
        "marill"
    };

    s_Species["Wooper"] = {
        200, "Wooper",
        PokemonType::WATER, PokemonType::GROUND,
        70, 20, 50, 20, 50, 40, 190, 88,
        {{ 1,"Tackle"},{1,"Bubble"},{9,"Water Gun"},{18,"Surf"},{25,"Double-Edge"}},
        "wooper"
    };

    s_Species["PaldeanWooper"] = {
        202, "PaldeanWooper",
        PokemonType::WATER, PokemonType::GROUND,
        70, 20, 50, 20, 50, 40, 190, 88,
        {{ 1,"Tackle"},{1,"Bubble"},{9,"Water Gun"},{18,"Surf"},{25,"Double-Edge"}},
        "paldeanwooper"
    };

    LOG_TRACE("PokemonDatabase initialised with {} species.", s_Species.size());
}

const PokemonSpecies& PokemonDatabase::GetSpecies(const std::string& name) {
    auto it = s_Species.find(name);
    if  (it == s_Species.end()) {
        LOG_ERROR("Species '{}' not found!", name);
        static PokemonSpecies empty{};
        return empty;
    }
    return it->second;
}

bool PokemonDatabase::HasSpecies(const std::string& name) {
    return s_Species.count(name) > 0;
}