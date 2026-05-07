#include "MapGenerator.hpp"
#include <queue>
#include <random>
#include <iostream>

using Grid = std::vector<std::vector<int>>;
using RNG  = std::mt19937;

GeneratedMap MapGenerator::GenerateCave(int width, int height, int seed) {
    // Seed persistence — store it so it can be displayed/shared
    if (seed < 0) seed = (int)std::random_device{}();
    RNG rng(seed);

    GeneratedMap result;
    result.seed = seed;
    result.ground.assign(height, std::vector<int>(width, GameConfig::TILE_WATER_SOLID));
    result.props.assign (height, std::vector<int>(width, 0));

    // Phase 1: random noise
    FillRandom(result.ground, width, height, 0.45f, rng);

    // Phase 2: smooth into organic cave shapes
    Smooth(result.ground, 4);

    // Phase 3: force solid border
    for (int x = 0; x < width;  x++) {
        result.ground[0][x]        = GameConfig::TILE_WATER_SOLID;
        result.ground[height-1][x] = GameConfig::TILE_WATER_SOLID;
    }
    for (int y = 0; y < height; y++) {
        result.ground[y][0]       = GameConfig::TILE_WATER_SOLID;
        result.ground[y][width-1] = GameConfig::TILE_WATER_SOLID;
    }

    // Phase 4: flood fill — keep only the largest connected floor region,
    // fill all disconnected pockets with walls so player is never trapped
    FloodFillKeepLargest(result.ground, width, height);

    // Phase 5: scatter props
    PlaceProps(result.ground, result.props, width, height, rng);

// Phase 6: Stamp the Top-Left Corner Entrance/Exit Checkpoint
    // Stamping exactly at the top-left corner (x = 0, y = 0)
    
    // 1. Force the ground underneath to be a walkable floor so the player doesn't get stuck
    for (int y = 0; y <= 4; y++) {
        for (int x = 0; x <= 4; x++) {
            result.ground[y][x] = GameConfig::TILE_DIRT; 
        }
    }

    // 2. Stamp the exact Prop matrix from your image
    // Row 0 (y=0) - Adding an extra 999 to match your image's top border
    result.props[0][0] = 999; result.props[0][1] = 999; result.props[0][2] = 999; result.props[0][3] = 999; 
    
    // Row 1 (y=1)
    result.props[1][0] = 999; result.props[1][1] = 999; result.props[1][2] = 999; result.props[1][3] = 999; 
    
    // Row 2 (y=2)
    result.props[2][0] = 999; result.props[2][1] = 999; result.props[2][2] = 999; result.props[2][3] = 990;
    
    // Row 3 (y=3) -> Door is at x=3, y=3
    result.props[3][0] = 999; result.props[3][1] = 999; result.props[3][2] = 15; result.props[3][3] = 999;
    
    // Row 4 (y=4) -> Visual marker (15) is at x=2, y=4
    // 3. Set the player to spawn directly on the visual marker (15)
    result.spawnX = 4; 
    result.spawnY = 2; 

    std::cout << "Cave generated | seed: " << seed 
              << " | spawn: (" << result.spawnX << "," << result.spawnY << ")\n";

    return result;
}


// ─────────────────────────────────────────────
//  PHASE IMPLEMENTATIONS
// ─────────────────────────────────────────────

void MapGenerator::FillRandom(Grid& grid, int w, int h, float wallChance, RNG& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            grid[y][x] = (dist(rng) < wallChance)
                        ? GameConfig::TILE_WATER_SOLID
                        : GameConfig::TILE_DIRT;
}

void MapGenerator::Smooth(Grid& grid, int passes) {
    int h = (int)grid.size();
    int w = (int)grid[0].size();

    for (int pass = 0; pass < passes; pass++) {
        Grid copy = grid;
        for (int y = 1; y < h - 1; y++) {
            for (int x = 1; x < w - 1; x++) {
                int walls = CountWallNeighbours(grid, x, y);
                copy[y][x] = (walls >= 5)
                            ? GameConfig::TILE_WATER_SOLID
                            : GameConfig::TILE_DIRT;
            }
        }
        grid = copy;
    }
}

int MapGenerator::CountWallNeighbours(const Grid& grid, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (grid[y+dy][x+dx] == GameConfig::TILE_WATER_SOLID) count++;
        }
    return count;
}

void MapGenerator::FloodFillKeepLargest(Grid& grid, int w, int h) {
    // BFS from every unvisited floor tile, recording each connected region
    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<std::vector<std::pair<int,int>>> regions;

    const int dx[] = {0, 0, 1, -1};
    const int dy[] = {1, -1, 0, 0};

    for (int startY = 1; startY < h - 1; startY++) {
        for (int startX = 1; startX < w - 1; startX++) {
            if (visited[startY][startX]) continue;
            if (grid[startY][startX] != GameConfig::TILE_DIRT) continue;

            // BFS — collect all tiles in this connected region
            std::vector<std::pair<int,int>> region;
            std::queue<std::pair<int,int>> queue;
            queue.push({startX, startY});
            visited[startY][startX] = true;

            while (!queue.empty()) {
                auto [cx, cy] = queue.front(); queue.pop();
                region.push_back({cx, cy});

                for (int d = 0; d < 4; d++) {
                    int nx = cx + dx[d];
                    int ny = cy + dy[d];
                    if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
                    if (visited[ny][nx]) continue;
                    if (grid[ny][nx] != GameConfig::TILE_DIRT) continue;
                    visited[ny][nx] = true;
                    queue.push({nx, ny});
                }
            }
            regions.push_back(region);
        }
    }

    if (regions.empty()) return;

    // Find the largest region
    size_t largestIdx = 0;
    for (size_t i = 1; i < regions.size(); i++)
        if (regions[i].size() > regions[largestIdx].size())
            largestIdx = i;

    // Fill every region that isn't the largest with walls
    for (size_t i = 0; i < regions.size(); i++) {
        if (i == largestIdx) continue;
        for (auto [x, y] : regions[i])
            grid[y][x] = GameConfig::TILE_WATER_SOLID;
    }
}

void MapGenerator::PlaceProps(const Grid& ground, Grid& props,
                               int w, int h, RNG& rng) {
    std::uniform_int_distribution<int> roll(0, 99);
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            if (ground[y][x] != GameConfig::TILE_DIRT) continue;
            if (props[y][x]  != 0) continue;

            int r = roll(rng);
            if      (r < 15) props[y][x] = GameConfig::PROP_TALLGRASS;
            else if (r < 18) props[y][x] = GameConfig::ITEM_POKEBALL;
            else if (r < 21) {
                props[y][x] = GameConfig::PROP_TREE;
                props[y+1][x] = 999;
                props[y+1][x+1] = 999;
            }

        }
    }
}

void MapGenerator::EnsureSpawnClear(Grid& ground, Grid& props,
                                     int spawnX, int spawnY, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = spawnX + dx;
            int y = spawnY + dy;
            if (x < 1 || y < 1 ||
                y >= (int)ground.size()    - 1 ||
                x >= (int)ground[0].size() - 1) continue;
            ground[y][x] = GameConfig::TILE_DIRT;
            props[y][x]  = 0;
        }
    }
}

bool MapGenerator::PlaceExit(Grid& props, const Grid& ground, int w, int h) {
    bool exitPlaced = false;
    for (int y = h - 2; y > h / 2 && !exitPlaced; y--) {
        for (int x = w - 2; x > w / 2 && !exitPlaced; x--) {
            if (ground[y][x] == GameConfig::TILE_DIRT && props[y][x] == 0) {
                props[y][x] = GameConfig::PROP_INVISIBLE_DOOR;
                exitPlaced = true;
            }
        }
    }
    return exitPlaced;
}