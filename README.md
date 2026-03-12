# Pokemon

## Abstract
**Members:** 柳艾德，阿木樂

## Game Introduction
Pokemon is a 2D open-world RPG with turn-based gameplay, where the player can explore different areas, collect different Pokemons, and battle against different bosses.

---

## Development Timeline

### Week 01: Proposal
- 項目一 Proposal
- 項目二

### Week 02: Character Assets
- 項目一 Collect spritesheets for our our character's movements as well as NPCs and working on our trainer class
- 項目二 Implement Input Handler for 4-directional grid-based movement.

### Week 03: Map Design
- 項目一 Collect assests for our map and towns. The map is made of many towns with routes connecting them and a single town have multiple buildings like a gym (boss fight), hospital (pokemon centre) and NPC homes. The routes in between the towns can comprise of different environments and scenery like forests, caves or regular roads.
- 項目二 Implement a Tilemap System or Image-based background loading.

### Week 04: Map Levels
- 項目一 Implement Collision Detection (Restricting player movement on water/walls).
- 項目二

### Week 05: Puzzles & Obstacles
- 項目一 Implement NPC interaction logic (Dialogue boxes using PTSD text rendering).
- 項目二 Create interactable objects (e.g., items to pickup).

### Week 06: Combat Assets
- 項目一 Collect UI assets (Battle menus, HP bars, move selection boxes).
- 項目二 Design the Battle Scene State (Transitioning from Map to Battle).

### Week 07: Pokemon Assets
- 項目一 Gather front/back sprites for the initial Pokémon roster.
- 項目二 Create a Base Pokémon Class (Attributes: HP, Attack, Defense, Type).

### Week 08: 期中 Demo
- 項目一 Core Loop: Walking on map. Encountering NPC/Wild Pokémon.
- 項目二 Basic UI display of character stats.

### Week 09: 期中 Demo
- 項目一
- 項目二

### Week 10: Adding Consumables & Small Assets
- 項目一 Implement Inventory System (Potions, Pokéballs, Key Items).
- 項目二

### Week 11: Adding Consumables & Small Assets
- 項目一 Design Item Pickup logic on the map.
- 項目二 Add "Bag" UI to the main menu.

### Week 12: Combat Mechanics
- 項目一 Implement Turn-based Logic (Player Turn Enemy AI Turn).
- 項目二 Code the Move/Skill system (Damage calculation formula).

### Week 13: Combat Mechanics
- 項目一 Implement Status Effects (Paralysis, Poison) and Type Advantages.
- 項目二 Add EXP gain and Level-up logic.
  

### Week 14: Boss Fights
- 項目一 Implement Trainer Battle logic (Handling multiple Pokémon in a party).
- 項目二 Design the Gym Leader AI (Specific move priorities).

### Week 15: Boss Fight
- 項目一 Implement "Victory" rewards (Badges or specific Items).
- 項目二

### Week 16: Sound Design & Refining Assets
- 項目一 Integrate BGM (Town, Route, Battle) and SFX (Bumps, Attacks).
- 項目二

### Week 17: 提交
- 拍攝影片
- 製作遊戲簡報
- 驗收並提交






# PTSD Template

This is a [PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design) framework template for students taking OOPL2024s.

## Quick Start

1. Use this template to create a new repository
   ![github screenshot](https://github.com/ntut-rick/ptsd-template/assets/126899559/ef62242f-03ed-481d-b858-12b730c09beb)

2. Clone your repository

   ```bash
   git clone YOUR_GIT_URL --recursive
   ```

3. Build your project

  > [!WARNING]
  > Please build your project in `Debug` because our `Release` path is broken D:
   
   ```sh
   cmake -DCMAKE_BUILD_TYPE=Debug -B build # -G Ninja
   ```
   better read [PTSD README](https://github.com/ntut-open-source-club/practical-tools-for-simple-design)
