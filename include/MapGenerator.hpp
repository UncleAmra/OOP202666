#pragma once

#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <nlohmann/json.hpp> // Assuming you are using nlohmann/json
#include "GameConfig.hpp"    // Assuming this contains your PROP_ and TILE_ constants

// If RESOURCE_DIR isn't defined globally in your build system, define a fallback
#ifndef RESOURCE_DIR
#define RESOURCE_DIR "res"
#endif

using json = nlohmann::json;
using Grid = std::vector<std::vector<int>>;
using RNG  = std::mt19937;

// ─────────────────────────────────────────────────────────────────────────────
//  Data Structures
// ─────────────────────────────────────────────────────────────────────────────

struct GeneratedMap {
    int seed = 0;
    int floorTile = 0;
    int wallTile = 0;
    int spawnX = -1;
    int spawnY = -1;
    
    Grid ground;
    Grid props;
};

struct Stamp {
    std::string name;
    int width = 0;
    int height = 0;
    int spawnX = -1;
    int spawnY = -1;
    int anchorX = -1;
    int anchorY = -1;
    int weight = 1;
    
    Grid ground;
    Grid props;
};

struct StampCollection {
    std::string biome;
    int floorTile = 0;
    int wallTile = 0;
    
    // City specific variables
    int roadTile = 0;
    int blockSpacingX = 8;
    int blockSpacingY = 6;

    std::vector<Stamp> guaranteed;
    std::vector<Stamp> random;
};

// ─────────────────────────────────────────────────────────────────────────────
//  MapGenerator Class
// ─────────────────────────────────────────────────────────────────────────────

class MapGenerator {
public:
    // Top-level entry point (handles both Cave and City generation based on json biome)
    static GeneratedMap GenerateCave(int width, int height, int seed = -1);

private:
    // ── CITY GENERATION METHODS ──────────────────────────────────────────────
    static void GenerateRoadGrid(Grid& ground, int w, int h, int roadTile, 
                                 int blockSpacingX, int blockSpacingY);
    static void EraseRandomRoads(Grid& ground, int w, int h, int roadTile, int floorTile, 
                                 int spacingX, int spacingY, RNG& rng);
    static void FillCityBlocks(Grid& ground, Grid& props, int w, int h, 
                               int roadTile, int floorTile, RNG& rng);
    static void PlaceSidewalkProps(Grid& ground, Grid& props, int w, int h, int roadTile, RNG& rng);
    static void ApplyDistrictDensity(Grid& props, int w, int h, RNG& rng);
    static bool IsAdjacentToRoad(const Grid& ground, int x, int y, int roadTile, int w, int h);

    // ── CAVE GENERATION METHODS ──────────────────────────────────────────────
    static void CarvePath(Grid& grid, int startX, int startY, int endX, int endY, 
                          int floorTile, RNG& rng);
    static void CarveBranches(Grid& grid, int w, int h, int numBranches, int branchLength, 
                              int floorTile, int wallTile, RNG& rng);
    static void CarveCircle(Grid& grid, int cx, int cy, int radius, int floorTile);
    static void GrowGrassPatches(const Grid& ground, Grid& props, int w, int h, 
                                 int numSeeds, RNG& rng, int floorTile);
    static void PlaceRewardsInNooks(const Grid& ground, Grid& props, int w, int h, int floorTile);
    static void PlaceGuardNPCs(const Grid& ground, Grid& props, int w, int h, 
                               int floorTile, int wallTile);
    static void Smooth(Grid& grid, int passes, int floorTile, int wallTile);
    static int  CountWallNeighbours(const Grid& grid, int x, int y, int wallTile);
    static void FloodFillKeepLargest(Grid& grid, int w, int h, int floorTile, int wallTile);

    // ── SHARED / UTILITY METHODS ─────────────────────────────────────────────
    static void FillRandom(Grid& grid, int w, int h, float wallChance, RNG& rng, 
                           int floorTile, int wallTile);
    static void PlaceProps(const Grid& ground, Grid& props, int w, int h, RNG& rng, int floorTile);
    static void EnsureSpawnClear(Grid& ground, Grid& props, int spawnX, int spawnY, 
                                 int radius, int floorTile);
    static bool PlaceExit(Grid& props, const Grid& ground, int w, int h, int floorTile);

    // ── STAMP SYSTEM ─────────────────────────────────────────────────────────
    static StampCollection LoadStamps(const std::string& filepath);
    static const Stamp&    PickWeightedStamp(const std::vector<Stamp>& stamps, RNG& rng);
    static void            ApplyStamp(Grid& ground, Grid& props, const Stamp& stamp, 
                                      int anchorX, int anchorY, int& outSpawnX, int& outSpawnY);
};