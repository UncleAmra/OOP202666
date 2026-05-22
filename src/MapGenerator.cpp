#include "MapGenerator.hpp"
#include <queue>
#include <random>
#include <iostream>
#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  GenerateCave — top-level entry point
// ─────────────────────────────────────────────────────────────────────────────

GeneratedMap MapGenerator::GenerateCave(int width, int height, int seed) {
    if (seed < 0) seed = (int)std::random_device{}();
    RNG rng(seed);

    StampCollection stamps = LoadStamps(
        std::string(RESOURCE_DIR) + "/stamps/city_stamps.json");
        // swap to cave_stamps.json for cave biome

    GeneratedMap result;
    result.seed      = seed;
    result.floorTile = stamps.floorTile;
    result.wallTile  = stamps.wallTile;

    result.ground.assign(height, std::vector<int>(width, stamps.floorTile));
    result.props .assign(height, std::vector<int>(width, 0));

    // =========================================================================
    // PHASE 1: TERRAIN GENERATION (Ground layer only)
    // =========================================================================
    
    if (stamps.biome == "cave") {
        // Fill as 100% walls first
        for (auto& row : result.ground)
            std::fill(row.begin(), row.end(), stamps.wallTile);

        std::uniform_int_distribution<int> spawnRangeX(2, width  / 4);
        std::uniform_int_distribution<int> spawnRangeY(2, height / 4);
        std::uniform_int_distribution<int> exitRangeX (width  * 3/4, width  - 3);
        std::uniform_int_distribution<int> exitRangeY (height * 3/4, height - 3);

        int startX = spawnRangeX(rng), startY = spawnRangeY(rng);
        int exitX  = exitRangeX(rng),  exitY  = exitRangeY(rng);

        CarvePath(result.ground, startX, startY, exitX, exitY, stamps.floorTile, rng);
        CarveBranches(result.ground, width, height, 6, 18, stamps.floorTile, stamps.wallTile, rng);
        Smooth(result.ground, 2, stamps.floorTile, stamps.wallTile);
        FloodFillKeepLargest(result.ground, width, height, stamps.floorTile, stamps.wallTile);
        
        // Save for Phase 4
        result.spawnX = startX;
        result.spawnY = startY;

    } else {
        // Lay down the road network
        GenerateRoadGrid(result.ground, width, height, 
                         stamps.roadTile, stamps.blockSpacingX, stamps.blockSpacingY);
        
        // Roads must be connected — skipping EraseRandomRoads to keep full grid
                         
        result.spawnX = width / 2;
        result.spawnY = height / 2;
    }

    // =========================================================================
    // PHASE 2: STAMP PLACEMENT (Overrides terrain, anchors major landmarks)
    // =========================================================================
    
    for (const Stamp& stamp : stamps.guaranteed) {
        ApplyStamp(result.ground, result.props, stamp,
                   stamp.anchorX, stamp.anchorY,
                   result.spawnX, result.spawnY);
    }

    // Random stamps are now handled within FillCityBlocks to ensure they are centered in blocks

    // =========================================================================
    // PHASE 3: POPULATION (Fills empty areas around stamps)
    // =========================================================================
    
    if (stamps.biome == "cave") {
        GrowGrassPatches(result.ground, result.props, width, height, 4, rng, stamps.floorTile);
        PlaceRewardsInNooks(result.ground, result.props, width, height, stamps.floorTile);
        PlaceGuardNPCs(result.ground, result.props, width, height, stamps.floorTile, stamps.wallTile);
    } else {
        FillCityBlocks(result.ground, result.props, width, height, 
                       stamps.roadTile, stamps.floorTile, stamps, rng);
        PlaceSidewalkProps(result.ground, result.props, width, height, stamps.roadTile);
        ApplyDistrictDensity(result.props, width, height, rng);
    }

    // =========================================================================
    // PHASE 4: CLEANUP
    // =========================================================================
    
    EnsureSpawnClear(result.ground, result.props, result.spawnX, result.spawnY, 2, stamps.floorTile);

    std::cout << "Map generated  | biome: " << stamps.biome
              << " | seed: "  << seed
              << " | floor: " << stamps.floorTile
              << " | spawn: (" << result.spawnX << "," << result.spawnY << ")\n";

    return result;
}


// ─────────────────────────────────────────────────────────────────────────────
//  CITY GENERATION METHODS
// ─────────────────────────────────────────────────────────────────────────────

void MapGenerator::GenerateRoadGrid(Grid& ground, int w, int h, 
                                     int roadTile, int blockSpacingX, int blockSpacingY) {
    for (int y = 0; y < h; y++) {
        if (y % blockSpacingY == 0) {
            for (int x = 0; x < w; x++) ground[y][x] = roadTile;
        }
    }
    for (int x = 0; x < w; x++) {
        if (x % blockSpacingX == 0) {
            for (int y = 0; y < h; y++) ground[y][x] = roadTile;
        }
    }
}

void MapGenerator::EraseRandomRoads(Grid& ground, int w, int h, int roadTile, int floorTile, 
                                     int spacingX, int spacingY, RNG& rng) {
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    float eraseChance = 0.25f; // 25% chance to erase a road segment between blocks

    // Erase horizontal segments
    for (int y = 0; y < h; y += spacingY) {
        for (int x = 0; x < w; x += spacingX) {
            if (x + spacingX < w && chance(rng) < eraseChance) {
                // Turn road to floor, leaving the intersections intact
                for (int ix = x + 1; ix < x + spacingX; ix++) {
                    ground[y][ix] = floorTile;
                }
            }
        }
    }
    
    // Erase vertical segments
    for (int x = 0; x < w; x += spacingX) {
        for (int y = 0; y < h; y += spacingY) {
            if (y + spacingY < h && chance(rng) < eraseChance) {
                for (int iy = y + 1; iy < y + spacingY; iy++) {
                    ground[iy][x] = floorTile;
                }
            }
        }
    }
}

void MapGenerator::FillCityBlocks(Grid& ground, Grid& props, int w, int h, 
                                   int roadTile, int floorTile, const StampCollection& stamps, RNG& rng) {
    enum class BlockType { COMMERCIAL, RESIDENTIAL, PARK, EMPTY_LOT };
    std::discrete_distribution<int> blockTypeDist({ 35, 30, 25, 10 });

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            if (ground[y][x] == roadTile) continue;

            bool topIsRoad  = (y > 0     && ground[y-1][x] == roadTile);
            bool leftIsRoad = (x > 0     && ground[y][x-1] == roadTile);
            if (!topIsRoad || !leftIsRoad) continue;

            BlockType type = static_cast<BlockType>(blockTypeDist(rng));

            // Measure block bounds
            int blockW = 0, blockH = 0;
            for (int bx = x; bx < w && ground[y][bx] != roadTile; bx++) blockW++;
            for (int by = y; by < h && ground[by][x] != roadTile; by++) blockH++;

            switch (type) {
                case BlockType::COMMERCIAL:
                case BlockType::RESIDENTIAL: {
                    if (!stamps.random.empty()) {
                        const Stamp& chosen = PickWeightedStamp(stamps.random, rng);
                        
                        // Center the building in the block
                        int anchorX = x + (blockW - chosen.width) / 2;
                        int anchorY = y + (blockH - chosen.height) / 2;
                        
                        // Ensure we don't bleed onto the road
                        int maxX = std::max(x, x + blockW - chosen.width);
                        int maxY = std::max(y, y + blockH - chosen.height);
                        anchorX = std::clamp(anchorX, x, maxX);
                        anchorY = std::clamp(anchorY, y, maxY);

                        int dummyX = 0, dummyY = 0;
                        ApplyStamp(ground, props, chosen, anchorX, anchorY, dummyX, dummyY);
                    }
                    break;
                }
                case BlockType::PARK: {
                    // Place one tree in the center
                    int centerX = x + blockW / 2;
                    int centerY = y + blockH / 2;
                    if (centerX < w && centerY < h) {
                        props[centerY][centerX] = GameConfig::PROP_TREE;
                    }

                    // Fill some grass randomly, but NO flowers
                    std::uniform_int_distribution<int> grassRoll(0, 99);
                    for (int by = y; by < y + blockH && by < h; by++) {
                        for (int bx = x; bx < x + blockW && bx < w; bx++) {
                            if (props[by][bx] == 0 && grassRoll(rng) < 15) {
                                props[by][bx] = GameConfig::PROP_TALLGRASS;
                            }
                        }
                    }
                    break;
                }
                case BlockType::EMPTY_LOT: {
                    // Just a few lamp posts or empty
                    std::uniform_int_distribution<int> lotRoll(0, 99);
                    if (lotRoll(rng) < 10) {
                        int lx = x + blockW / 2, ly = y + blockH / 2;
                        if (lx < w && ly < h) props[ly][lx] = GameConfig::PROP_LAMP_POST;
                    }
                    break;
                }
            }
        }
    }
}

void MapGenerator::PlaceSidewalkProps(Grid& ground, Grid& props, int w, int h, int roadTile) {
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            if (ground[y][x] == roadTile) continue; 
            if (props[y][x] != 0) continue;         
            if (!IsAdjacentToRoad(ground, x, y, roadTile, w, h)) continue;

            bool roadUp    = (ground[y-1][x] == roadTile);
            bool roadDown  = (ground[y+1][x] == roadTile);
            bool roadLeft  = (ground[y][x-1] == roadTile);
            bool roadRight = (ground[y][x+1] == roadTile);

            // 1. Prevent lamps on the bottom edge of a block (where they overlap road visuals)
            if (roadDown) continue;

            // 2. Deterministic placement every 5 tiles (increased from 3 to reduce density)
            if (((roadUp) && (x % 5 == 0)) || 
                ((roadLeft || roadRight) && (y % 5 == 0))) {
                props[y][x] = GameConfig::PROP_LAMP_POST;
            }
        }
    }
}

void MapGenerator::ApplyDistrictDensity(Grid& props, int w, int h, RNG& rng) {
    float centerX = w / 2.0f;
    float centerY = h / 2.0f;
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (props[y][x] == 0) continue;

            // FIX: Exempt structural and landmark props from culling!
            int p = props[y][x];
            if (p == GameConfig::PROP_INVISIBLE_WALL || 
                p == GameConfig::PROP_INVISIBLE_DOOR || 
                p == GameConfig::PROP_LAMP_POST) {
                continue; 
            }

            float dist = std::sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
            float centrality = 1.0f - (dist / maxDist); // 1.0 = downtown, 0.0 = edge

            float keepChance = 0.3f + centrality * 0.7f; 
            if (chance(rng) > keepChance) {
                props[y][x] = 0; 
            }
        }
    }
}

bool MapGenerator::IsAdjacentToRoad(const Grid& ground, int x, int y, int roadTile, int w, int h) {
    if (y > 0     && ground[y-1][x] == roadTile) return true;
    if (y < h - 1 && ground[y+1][x] == roadTile) return true;
    if (x > 0     && ground[y][x-1] == roadTile) return true;
    if (x < w - 1 && ground[y][x+1] == roadTile) return true;
    return false;
}


// ─────────────────────────────────────────────────────────────────────────────
//  CAVE GENERATION METHODS
// ─────────────────────────────────────────────────────────────────────────────

void MapGenerator::CarvePath(Grid& grid, int startX, int startY, int endX, int endY, int floorTile, RNG& rng) {
    int cx = startX, cy = startY;

    CarveCircle(grid, cx, cy, 2, floorTile);
    CarveCircle(grid, endX, endY, 2, floorTile);

    int maxSteps = (int)grid.size() * (int)grid[0].size() * 2;
    std::uniform_int_distribution<int> bias(0, 3); 

    for (int step = 0; step < maxSteps; step++) {
        grid[cy][cx] = floorTile;

        int distX = std::abs(cx - endX);
        int distY = std::abs(cy - endY);
        if (distX <= 2 && distY <= 2) break;

        int dx = 0, dy = 0;
        int roll = bias(rng);

        if (roll < 2) {
            if (distX > distY) dx = (endX > cx) ? 1 : -1;
            else               dy = (endY > cy) ? 1 : -1;
        } else {
            std::uniform_int_distribution<int> dir(0, 3);
            int d = dir(rng);
            if      (d == 0) dy = -1;
            else if (d == 1) dy =  1;
            else if (d == 2) dx = -1;
            else             dx =  1;
        }

        int nx = cx + dx;
        int ny = cy + dy;

        cx = std::clamp(nx, 1, (int)grid[0].size() - 2);
        cy = std::clamp(ny, 1, (int)grid.size()    - 2);
    }
}

void MapGenerator::CarveBranches(Grid& grid, int w, int h, int numBranches, int branchLength, int floorTile, int wallTile, RNG& rng) {
    std::vector<std::pair<int,int>> floorTiles;
    for (int y = 1; y < h - 1; y++)
        for (int x = 1; x < w - 1; x++)
            if (grid[y][x] == floorTile)
                floorTiles.push_back({x, y});

    if (floorTiles.empty()) return;

    std::uniform_int_distribution<int> pickTile(0, (int)floorTiles.size() - 1);
    std::uniform_int_distribution<int> pickDir(0, 3);
    std::uniform_int_distribution<int> pickLen(branchLength / 2, branchLength);

    const int dx[] = { 0,  0, 1, -1 };
    const int dy[] = { 1, -1, 0,  0 };

    for (int b = 0; b < numBranches; b++) {
        auto [bx, by] = floorTiles[pickTile(rng)];
        int dir = pickDir(rng);
        int len = pickLen(rng);

        for (int step = 0; step < len; step++) {
            if (rng() % 5 == 0) dir = pickDir(rng);

            int nx = bx + dx[dir];
            int ny = by + dy[dir];

            if (nx < 1 || ny < 1 || nx >= w - 1 || ny >= h - 1) break;

            grid[ny][nx] = floorTile;
            bx = nx; by = ny;
        }
    }
}

void MapGenerator::CarveCircle(Grid& grid, int cx, int cy, int radius, int floorTile) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy > radius * radius) continue;
            int x = cx + dx;
            int y = cy + dy;
            if (x < 1 || y < 1 || y >= (int)grid.size() - 1 || x >= (int)grid[0].size() - 1) continue;
            grid[y][x] = floorTile;
        }
    }
}

void MapGenerator::GrowGrassPatches(const Grid& ground, Grid& props, int w, int h, int numSeeds, RNG& rng, int floorTile) {
    std::vector<std::pair<int,int>> openTiles;
    for (int y = 1; y < h - 1; y++)
        for (int x = 1; x < w - 1; x++)
            if (ground[y][x] == floorTile && props[y][x] == 0)
                openTiles.push_back({x, y});

    if (openTiles.empty()) return;

    std::uniform_int_distribution<int> pickTile(0, (int)openTiles.size() - 1);
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    std::vector<std::vector<bool>> isGrass(h, std::vector<bool>(w, false));

    for (int s = 0; s < numSeeds; s++) {
        auto [sx, sy] = openTiles[pickTile(rng)];
        props[sy][sx]    = GameConfig::PROP_TALLGRASS;
        isGrass[sy][sx]  = true;
    }

    const int dx[] = { 0,  0, 1, -1 };
    const int dy[] = { 1, -1, 0,  0 };

    for (int pass = 0; pass < 3; pass++) {
        auto snapshot = isGrass;
        for (int y = 1; y < h - 1; y++) {
            for (int x = 1; x < w - 1; x++) {
                if (!snapshot[y][x]) continue;

                for (int d = 0; d < 4; d++) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx < 1 || ny < 1 || nx >= w - 1 || ny >= h - 1) continue;
                    if (ground[ny][nx] != floorTile || props[ny][nx] != 0) continue;
                    
                    if (chance(rng) < 0.4f) {
                        props[ny][nx]   = GameConfig::PROP_TALLGRASS;
                        isGrass[ny][nx] = true;
                    }
                }
            }
        }
    }
}

void MapGenerator::PlaceRewardsInNooks(const Grid& ground, Grid& props, int w, int h, int floorTile) {
    const int dx[] = { 0,  0, 1, -1 };
    const int dy[] = { 1, -1, 0,  0 };

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            if (ground[y][x] != floorTile || props[y][x] != 0) continue;

            int floorNeighbours = 0;
            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d];
                int ny = y + dy[d];
                if (nx >= 0 && ny >= 0 && nx < w && ny < h && ground[ny][nx] == floorTile)
                    floorNeighbours++;
            }

            if (floorNeighbours == 1) {
                props[y][x] = GameConfig::ITEM_POKEBALL;
            }
        }
    }
}

void MapGenerator::PlaceGuardNPCs(const Grid& ground, Grid& props, int w, int h, int floorTile, int wallTile) {
    const int maxGuards = 3;
    int guardsPlaced = 0;

    for (int y = 2; y < h - 2 && guardsPlaced < maxGuards; y++) {
        for (int x = 2; x < w - 2 && guardsPlaced < maxGuards; x++) {
            if (ground[y][x] != floorTile || props[y][x] != 0) continue;

            bool hChoke = ground[y-1][x] == wallTile && ground[y+1][x] == wallTile &&
                          ground[y][x-1] == floorTile && ground[y][x+1] == floorTile;

            bool vChoke = ground[y][x-1] == wallTile && ground[y][x+1] == wallTile &&
                          ground[y-1][x] == floorTile && ground[y+1][x] == floorTile;

            if (hChoke || vChoke) {
                bool tooClose = false;
                for (int gy = y - 5; gy <= y + 5 && !tooClose; gy++)
                    for (int gx = x - 5; gx <= x + 5 && !tooClose; gx++)
                        if (gy >= 0 && gx >= 0 && gy < h && gx < w && props[gy][gx] == GameConfig::NPC_TA1)
                            tooClose = true;

                if (!tooClose) {
                    props[y][x] = GameConfig::NPC_TA1;
                    guardsPlaced++;
                }
            }
        }
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  UTILITY & STAMP SYSTEM
// ─────────────────────────────────────────────────────────────────────────────

StampCollection MapGenerator::LoadStamps(const std::string& filepath) {
    StampCollection collection;
    std::ifstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "MapGenerator: could not open stamps file: " << filepath << "\n";
        return collection;
    }

    json j;
    try { file >> j; } 
    catch (const json::parse_error& e) {
        std::cerr << "MapGenerator: JSON parse error in " << filepath << " — " << e.what() << "\n";
        return collection;
    }

    collection.biome         = j.value("biome",         std::string("cave"));
    collection.floorTile     = j.value("floorTile",     GameConfig::TILE_DIRT);
    collection.wallTile      = j.value("wallTile",      GameConfig::TILE_WATER_SOLID);
    collection.roadTile      = j.value("roadTile",      GameConfig::TILE_ROAD);
    collection.blockSpacingX = j.value("blockSpacingX", 8);
    collection.blockSpacingY = j.value("blockSpacingY", 6);

    auto parseStamp = [](const json& s) -> Stamp {
        Stamp stamp;
        stamp.name    = s.value("name",    "unnamed");
        stamp.width   = s.value("width",   0);
        stamp.height  = s.value("height",  0);
        stamp.spawnX  = s.value("spawnX",  -1);
        stamp.spawnY  = s.value("spawnY",  -1);
        stamp.anchorX = s.value("anchorX", -1);
        stamp.anchorY = s.value("anchorY", -1);
        stamp.weight  = s.value("weight",  1);

        if (s.contains("ground"))
            for (const auto& row : s["ground"])
                stamp.ground.push_back(row.get<std::vector<int>>());

        if (s.contains("props"))
            for (const auto& row : s["props"])
                stamp.props.push_back(row.get<std::vector<int>>());

        return stamp;
    };

    if (j.contains("guaranteed"))
        for (const auto& s : j["guaranteed"])
            collection.guaranteed.push_back(parseStamp(s));

    if (j.contains("random"))
        for (const auto& s : j["random"])
            collection.random.push_back(parseStamp(s));

    return collection;
}

const Stamp& MapGenerator::PickWeightedStamp(const std::vector<Stamp>& stamps, RNG& rng) {
    int totalWeight = 0;
    for (const Stamp& s : stamps) totalWeight += s.weight;

    std::uniform_int_distribution<int> dist(0, totalWeight - 1);
    int roll = dist(rng);

    int cumulative = 0;
    for (const Stamp& s : stamps) {
        cumulative += s.weight;
        if (roll < cumulative) return s;
    }
    return stamps.back();
}

void MapGenerator::ApplyStamp(Grid& ground, Grid& props, const Stamp& stamp,
                               int anchorX, int anchorY, int& outSpawnX, int& outSpawnY) {
    for (int sy = 0; sy < stamp.height; sy++) {
        for (int sx = 0; sx < stamp.width; sx++) {
            int wx = anchorX + sx;
            int wy = anchorY + sy;

            if (wy < 0 || wy >= (int)ground.size() || wx < 0 || wx >= (int)ground[0].size()) continue;

            if (!stamp.ground.empty() && sy < (int)stamp.ground.size() && sx < (int)stamp.ground[sy].size())
                ground[wy][wx] = stamp.ground[sy][sx];

            if (!stamp.props.empty() && sy < (int)stamp.props.size() && sx < (int)stamp.props[sy].size()) {
                int propVal = stamp.props[sy][sx];
                if (propVal != -1) props[wy][wx] = propVal;
            }
        }
    }

    if (stamp.spawnX >= 0 && stamp.spawnY >= 0) {
        outSpawnX = anchorX + stamp.spawnX;
        outSpawnY = anchorY + stamp.spawnY;
    }
}

void MapGenerator::FillRandom(Grid& grid, int w, int h, float wallChance, RNG& rng, int floorTile, int wallTile) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            grid[y][x] = (dist(rng) < wallChance) ? wallTile : floorTile;
}

void MapGenerator::Smooth(Grid& grid, int passes, int floorTile, int wallTile) {
    int h = (int)grid.size(), w = (int)grid[0].size();
    for (int pass = 0; pass < passes; pass++) {
        Grid copy = grid;
        for (int y = 1; y < h - 1; y++) {
            for (int x = 1; x < w - 1; x++) {
                int walls = CountWallNeighbours(grid, x, y, wallTile);
                copy[y][x] = (walls >= 5) ? wallTile : floorTile;
            }
        }
        grid = copy;
    }
}

int MapGenerator::CountWallNeighbours(const Grid& grid, int x, int y, int wallTile) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            if (grid[y + dy][x + dx] == wallTile) count++;
        }
    return count;
}

void MapGenerator::FloodFillKeepLargest(Grid& grid, int w, int h, int floorTile, int wallTile) {
    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<std::vector<std::pair<int,int>>> regions;
    const int dx[] = { 0,  0, 1, -1 }, dy[] = { 1, -1, 0,  0 };

    for (int startY = 1; startY < h - 1; startY++) {
        for (int startX = 1; startX < w - 1; startX++) {
            if (visited[startY][startX] || grid[startY][startX] != floorTile) continue;

            std::vector<std::pair<int,int>> region;
            std::queue<std::pair<int,int>> queue;
            queue.push({startX, startY});
            visited[startY][startX] = true;

            while (!queue.empty()) {
                auto [cx, cy] = queue.front(); queue.pop();
                region.push_back({cx, cy});

                for (int d = 0; d < 4; d++) {
                    int nx = cx + dx[d], ny = cy + dy[d];
                    if (nx < 0 || ny < 0 || nx >= w || ny >= h || visited[ny][nx] || grid[ny][nx] != floorTile) continue;
                    visited[ny][nx] = true;
                    queue.push({nx, ny});
                }
            }
            regions.push_back(region);
        }
    }

    if (regions.empty()) return;

    size_t largestIdx = 0;
    for (size_t i = 1; i < regions.size(); i++)
        if (regions[i].size() > regions[largestIdx].size()) largestIdx = i;

    for (size_t i = 0; i < regions.size(); i++) {
        if (i == largestIdx) continue;
        for (auto [x, y] : regions[i]) grid[y][x] = wallTile;
    }
}

void MapGenerator::PlaceProps(const Grid& ground, Grid& props, int w, int h, RNG& rng, int floorTile) {
    std::uniform_int_distribution<int> roll(0, 99);
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            if (ground[y][x] != floorTile || props[y][x] != 0) continue;
            int r = roll(rng);
            if (r < 18) props[y][x] = GameConfig::ITEM_POKEBALL;
            else if (r < 21 && y + 1 < h - 1 && x + 1 < w - 1) {
                props[y][x]       = GameConfig::PROP_TREE;
                props[y + 1][x]   = 999;
                props[y + 1][x+1] = 999;
            }
        }
    }
}

void MapGenerator::EnsureSpawnClear(Grid& ground, Grid& props, int spawnX, int spawnY, int radius, int floorTile) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = spawnX + dx, y = spawnY + dy;
            if (x < 1 || y < 1 || y >= (int)ground.size() - 1 || x >= (int)ground[0].size() - 1) continue;
            ground[y][x] = floorTile;
            props[y][x]  = 0;
        }
    }
}

bool MapGenerator::PlaceExit(Grid& props, const Grid& ground, int w, int h, int floorTile) {
    for (int y = h - 2; y > h / 2; y--) {
        for (int x = w - 2; x > w / 2; x--) {
            if (ground[y][x] == floorTile && props[y][x] == 0) {
                props[y][x] = GameConfig::PROP_INVISIBLE_DOOR;
                return true;
            }
        }
    }
    return false;
}