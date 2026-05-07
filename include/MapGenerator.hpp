#pragma once
#include <vector>
#include <random>
#include "GameConfig.hpp"

struct GeneratedMap {
    std::vector<std::vector<int>> ground;
    std::vector<std::vector<int>> props;
    int spawnX = 1;
    int spawnY = 1;
    int seed   = 0; // stored so player can share/debug it
};

class MapGenerator {
public:
    static GeneratedMap GenerateCave(int width, int height, int seed = -1);

private:
    using Grid = std::vector<std::vector<int>>;
    using RNG  = std::mt19937;

    static void FillRandom      (Grid& grid, int w, int h, float wallChance, RNG& rng);
    static void Smooth          (Grid& grid, int passes);
    static int  CountWallNeighbours(const Grid& grid, int x, int y);
    static void FloodFillKeepLargest(Grid& grid, int w, int h);  // fixes disconnected caves
    static void PlaceProps      (const Grid& ground, Grid& props, int w, int h, RNG& rng);
    static void EnsureSpawnClear(Grid& ground, Grid& props, int spawnX, int spawnY, int radius = 2);
    static bool PlaceExit       (Grid& props, const Grid& ground, int w, int h); // returns false if no spot found
};