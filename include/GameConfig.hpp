#ifndef GAMECONFIG_HPP
#define GAMECONFIG_HPP
#include "config.hpp"
#include <string> 
#include <unordered_set> 
#include <unordered_map>

namespace GameConfig {
    constexpr float TILE_SIZE   = 16.0f;
    constexpr float SCALE       = 3.0f;
    constexpr float SCALED_TILE_SIZE    = TILE_SIZE * SCALE; // 48.0f
    constexpr float EFFECTIVE_TILE_SIZE = SCALED_TILE_SIZE - 0.1f; 

    //Camera
    constexpr float CAMERA_START_X = -288.0f; 
    constexpr float CAMERA_START_Y =  288.0f;

    // Tile IDs
    constexpr int TILE_GRASS        = 0;
    constexpr int TILE_WATER_SOLID  = 1;
    constexpr int TILE_DIRT         = 2;
    constexpr int TILE_CONCRETE     = 4;
    constexpr int TILE_GRAVEL       = 5;
    constexpr int TILE_DOOR         = 6;      
    constexpr int TILE_PC_FLOOR     = 9;
    constexpr int TILE_SAND         = 10;
    constexpr int TILE_PC_WALL      = 11;
    constexpr int TILE_ROAD         = 12;
    constexpr int TILE_PM_FLOOR     = 20;
    constexpr int TILE_EXIT         = 99;
    constexpr int TILE_INSIDE_CHURCH= 67;
    constexpr int TILE_CALM_WATER   = 14;
    constexpr int TILE_RED_BRICK    = 15;
    constexpr int TILE_GREY_BRICK   = 16;

    // Prop IDs 
    constexpr int PROP_POKECENTER       = 3;
    constexpr int PROP_CHURCH           = 5;
    constexpr int PROP_DOORMAT          = 7;
    constexpr int DOOR_OPENING_GYM      = 420;
    constexpr int DOOR_OPENING_PC       = 421;
    constexpr int DOOR_OPENING_PM       = 422;
    constexpr int PROP_INVISIBLE_DOOR   = 990;
    constexpr int PROP_INVISIBLE_WALL   = 999;
    constexpr int PROP_GATE_TOP         = 500;
    constexpr int PROP_GATE_MIDDLE      = 501;
    constexpr int PROP_GATE_END         = 502;
    constexpr int PROP_GATE_TOP2        = 503;
    constexpr int PROP_GATE_END2        = 504;
    constexpr int PROP_SMALL_TREE       = 505;
    constexpr int PROP_GATE_MIDDLE2     = 506;
    constexpr int PROP_LAMP_POST        = 507;
    constexpr int PROP_TALL_TREE        = 508;
    constexpr int PROP_GATE_MIDDLE3     = 509;
    constexpr int PROP_PC_DESK          = 10;
    constexpr int PROP_PM_DESK          = 11;
    constexpr int PROP_PC_WALL_LEFT     = 12;
    constexpr int PROP_PC_WALL_RIGHT    = 13;
    constexpr int PROP_CHECKPOINT       = 15;
    constexpr int PROP_CHECKPOINT2      = 16;
    constexpr int PROP_INTERACTABLE_WALL= 24;
    constexpr int PROP_TREE             = 60;
    constexpr int PROP_TALLGRASS        = 61;
    constexpr int PROP_POKEMONGYM       = 62;
    constexpr int PROP_LOG_DOWN1        = 63;
    constexpr int PROP_LOG_DOWN2        = 64;
    constexpr int PROP_LOG_DOWN3        = 65;
    constexpr int PROP_LOG_LEFT1        = 66;
    constexpr int PROP_LOG_LEFT2        = 68;
    constexpr int PROP_LOG_LEFT3        = 69;
    constexpr int PROP_FLOWER           = 70;
    constexpr int PROP_PLANT            = 72;
    constexpr int PROP_TRUCK1           = 73;
    constexpr int PROP_TRUCK2           = 74;
    constexpr int PROP_CAR              = 75;
    constexpr int PROP_STAIRS_NORTH     = 76;
    constexpr int PROP_PALM_TREE        = 77;
    constexpr int PROP_UMBRELLA_STAND   = 78;
    constexpr int PROP_WHITE_PILLAR     = 79;
    constexpr int PROP_LOG_up1          = 80;
    constexpr int PROP_LOG_up2          = 81;

    constexpr int PROP_NTUT_BUILDING1       = 550;
    constexpr int PROP_NTUT_MOSS_BUILDING   = 551;
    constexpr int PROP_NTUT_BUILDING2       = 552;
    constexpr int PROP_NTUT_BUILDING3       = 553;
    constexpr int PROP_NTUT_TECH_BUILDING   = 554;
    constexpr int PROP_NTUT_BUILDING4       = 555;
    constexpr int PROP_NTUT_SCREEN          = 556;
    constexpr int PROP_NTUT_BALL_STATUE     = 557;
    constexpr int PROP_NTUT_BUILDING5       = 558;
    constexpr int PROP_NTUT_TECH_BUILDING2  = 559;
    constexpr int PROP_NTUT_BUILDING6       = 560;
    constexpr int PROP_NTUT_CAFETERIA_BUILDING  = 561;
    constexpr int PROP_NTUT_BUILDING7       = 562;
    constexpr int PROP_NTUT_BUILDING8       = 563;



    //


    // Item IDs
    constexpr int ITEM_POTION   = 50;
    constexpr int ITEM_POKEBALL = 51;
    constexpr int POKEMART      = 52;
    constexpr int POKEMART_SIGN = 53;
    constexpr int WOODEN_HOUSE  = 54;

    //NPC IDs
    constexpr int NPC_NURSE     = 100;
    constexpr int NPC_TA1       = 101;
    constexpr int SHOP_KEEPER   = 102;


    struct WarpDestination {
        std::string levelPath;
        int spawnX;
        int spawnY;
    };

    inline std::unordered_set<std::string> LootedItems; //registry for items that have already been looted so they don't respawn


    /* 
    //Teleporting coordinates of the main map on level_props 990
    inline const WarpDestination WARP_TOWN_OUTSIDE =         { RESOURCE_DIR "/maps/level", 12, 8 };
    inline const WarpDestination WARP_TOWN_FROM_NTUT =       { RESOURCE_DIR "/maps/level", 4, 4 };
    inline const WarpDestination WARP_TOWN_FROM_MAZE =       { RESOURCE_DIR "/maps/level", 51, 14};
    inline const WarpDestination WARP_TOWN_FROM_POKEMART =   { RESOURCE_DIR "/maps/level", 28, 8};
    inline const WarpDestination WARP_TOWN_FROM_CHURCH =     { RESOURCE_DIR "/maps/level", 21, 8};
    inline const WarpDestination WARP_TOWN_FROM_POKEMONGYM1 =     { RESOURCE_DIR "/maps/level", 47, 8};
    inline const WarpDestination WARP_TOWN_FROM_WOODEN =     { RESOURCE_DIR "/maps/level", 39, 12};
    //======================================================================================================
    inline const WarpDestination WARP_MAZE =            { RESOURCE_DIR "/maps/maze",        2, 5 };     //inside Maze cordinate 
    inline const WarpDestination WARP_PC_INSIDE =       { RESOURCE_DIR "/maps/inside",      7, 8 };     //inside MRT
    inline const WarpDestination WARP_POKEMART_INSIDE = { RESOURCE_DIR "/maps/PokeMart",    5, 9};      //inside mart
    inline const WarpDestination WARP_NTUT =            { RESOURCE_DIR "/maps/NTUT",        9,3 };
    inline const WarpDestination WARP_CHURCH_INSIDE =   { RESOURCE_DIR "/maps/church",      5,14};
    inline const WarpDestination WARP_POKEMONGYM1 =     { RESOURCE_DIR "/maps/maze",        2,15};
    inline const WarpDestination WARP_WOODEN_HOUSE =    { RESOURCE_DIR "/maps/wood",        7,9};


    //coordinates x and y
    inline std::unordered_map<std::string, WarpDestination> DoorRouting = {
        
        { RESOURCE_DIR "/maps/level_12_8",  WARP_PC_INSIDE },
        { RESOURCE_DIR "/maps/inside_7_8",  WARP_TOWN_OUTSIDE },
        { RESOURCE_DIR "/maps/level_4_4",   WARP_NTUT},
        { RESOURCE_DIR "/maps/NTUT_9_3",    WARP_TOWN_FROM_NTUT},
        { RESOURCE_DIR "/maps/maze_3_2",    WARP_TOWN_FROM_MAZE },
        { RESOURCE_DIR "/maps/level_50_14", WARP_MAZE },
        { RESOURCE_DIR "/maps/level_28_8",  WARP_POKEMART_INSIDE },
        { RESOURCE_DIR "/maps/PokeMart_5_8",WARP_TOWN_FROM_POKEMART},
        { RESOURCE_DIR "/maps/church_5_14", WARP_TOWN_FROM_CHURCH},
        { RESOURCE_DIR "/maps/level_21_8",  WARP_CHURCH_INSIDE},
        { RESOURCE_DIR "/maps/maze_2_15",   WARP_TOWN_FROM_POKEMONGYM1},
        { RESOURCE_DIR "/maps/level_47_7",  WARP_POKEMONGYM1},
        { RESOURCE_DIR "/maps/wood_7_9",    WARP_TOWN_FROM_WOODEN},
        { RESOURCE_DIR "/maps/level_38_12", WARP_WOODEN_HOUSE},
    };
    inline std::unordered_map<std::string, int> MapBorders = {
        { RESOURCE_DIR "/maps/level", PROP_TREE }, // Town gets surrounded by trees
        { RESOURCE_DIR "/maps/NTUT", PROP_TREE }   // NTUT gets surrounded by trees
        // We leave "/maps/inside" out of this list so it defaults to the void!
    };
    */

    inline std::unordered_map<std::string, WarpDestination> DoorRouting;
}
#endif