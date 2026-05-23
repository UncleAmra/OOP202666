#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include "GameConfig.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using Grid = std::vector<std::vector<int>>;
using RNG  = std::mt19937;

// ── Data types ────────────────────────────────────────────────────────────────

struct GeneratedMap {
    int seed      = 0;
    int spawnX    = 0;
    int spawnY    = 0;
    Grid ground;
    Grid props;
    int floorTile = GameConfig::TILE_DIRT;
    int wallTile  = GameConfig::TILE_WATER_SOLID;
};

struct Stamp {
    std::string name;
    int width   = 0;
    int height  = 0;
    int spawnX  = -1;
    int spawnY  = -1;
    int anchorX = -1;
    int anchorY = -1;
    int weight  = 1;
    std::vector<std::vector<int>> ground;
    std::vector<std::vector<int>> props;
};

struct StampCollection {
    std::vector<Stamp> guaranteed;
    std::vector<Stamp> random;
    std::string biome        = "cave";
    int floorTile            = GameConfig::TILE_DIRT;
    int wallTile             = GameConfig::TILE_WATER_SOLID; // cave only
    int roadTile             = GameConfig::TILE_ROAD;        // city only
    int blockSpacingX        = 8;                            // city only
    int blockSpacingY        = 6;                            // city only
};

// ── MapGenerator ──────────────────────────────────────────────────────────────

class MapGenerator {
public:
    static GeneratedMap GenerateCave(int width, int height, int seed = -1);

private:
    // ── City generation ──────────────────────────────────────────────────────

    // Lays a grid of roads (roadWidth=2) across the map
    static void GenerateRoadGrid(Grid& ground, int w, int h,
                                  int roadTile, int blockSpacingX, int blockSpacingY);

    // Randomly erases road segments between intersections (unused by default)
    static void EraseRandomRoads(Grid& ground, int w, int h,
                                  int roadTile, int floorTile,
                                  int spacingX, int spacingY, RNG& rng);

    // Fills each city block with a weighted random type (commercial/residential/park/empty)
    static void FillCityBlocks(Grid& ground, Grid& props, int w, int h,
                                int roadTile, int floorTile,
                                const StampCollection& stamps, RNG& rng);

    // Places lamp posts deterministically along road edges
    static void PlaceSidewalkProps(Grid& ground, Grid& props,
                                    int w, int h, int roadTile);

    // Thins props toward the map edge to create a denser downtown feel
    static void ApplyDistrictDensity(Grid& props, int w, int h, RNG& rng);

    // Helper — true if any 4-directional neighbour is a road tile
    static bool IsAdjacentToRoad(const Grid& ground, int x, int y,
                                  int roadTile, int w, int h);

    // ── Cave generation ──────────────────────────────────────────────────────

    static void CarvePath(Grid& grid,
                           int startX, int startY,
                           int endX,   int endY,
                           int floorTile, RNG& rng);

    static void CarveBranches(Grid& grid, int w, int h,
                               int numBranches, int branchLength,
                               int floorTile, int wallTile, RNG& rng);

    static void CarveCircle(Grid& grid, int cx, int cy,
                             int radius, int floorTile);

    static void GrowGrassPatches(const Grid& ground, Grid& props,
                                  int w, int h, int numSeeds,
                                  RNG& rng, int floorTile);

    static void PlaceRewardsInNooks(const Grid& ground, Grid& props,
                                     int w, int h, int floorTile);

    static void PlaceGuardNPCs(const Grid& ground, Grid& props,
                                int w, int h,
                                int floorTile, int wallTile);

    // ── Cellular automata / flood fill ───────────────────────────────────────

    static void FillRandom(Grid& grid, int w, int h, float wallChance,
                            RNG& rng, int floorTile, int wallTile);

    static void Smooth(Grid& grid, int passes, int floorTile, int wallTile);

    static int  CountWallNeighbours(const Grid& grid, int x, int y, int wallTile);

    static void FloodFillKeepLargest(Grid& grid, int w, int h,
                                      int floorTile, int wallTile);

    // ── Stamp system ─────────────────────────────────────────────────────────

    static StampCollection LoadStamps       (const std::string& filepath);
    static const Stamp&    PickWeightedStamp(const std::vector<Stamp>& stamps, RNG& rng);
    static void            ApplyStamp       (Grid& ground, Grid& props,
                                             const Stamp& stamp,
                                             int anchorX, int anchorY,
                                             int& outSpawnX, int& outSpawnY);

    // ── Utility ──────────────────────────────────────────────────────────────

    static void PlaceProps(const Grid& ground, Grid& props,
                            int w, int h, RNG& rng, int floorTile);

    static void EnsureSpawnClear(Grid& ground, Grid& props,
                                  int spawnX, int spawnY,
                                  int radius, int floorTile);

    // Kept for cave biome fallback — city uses the guaranteed exit stamp instead
    static bool PlaceExit(Grid& props, const Grid& ground,
                           int w, int h, int floorTile);
};