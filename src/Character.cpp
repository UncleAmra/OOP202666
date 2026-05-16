#include "Character.hpp"
#include "Util/Time.hpp"
#include "Map.hpp"

Character::Character(float x, float y) {
    m_Transform.translation = {x, y + 24.0f};
    m_ZIndex = 1.0f;
    m_Visible = true;
    m_State = State::IDLE;
    m_Direction = Direction::DOWN;
    m_Transform.scale = {GameConfig::SCALE, GameConfig::SCALE}; 
    
    // Note: We don't call LoadSprites() here anymore. 
    // The child classes must call it in their own constructors!
}

void Character::UpdateSprite() {
    switch (m_Direction) {
        case Direction::DOWN:  m_CurrentAnimation = m_AnimDown;  break;
        case Direction::UP:    m_CurrentAnimation = m_AnimUp;    break;
        case Direction::LEFT:  m_CurrentAnimation = m_AnimLeft;  break;
        case Direction::RIGHT: m_CurrentAnimation = m_AnimRight; break;
    }
    m_Drawable = m_CurrentAnimation;

    if (m_State == State::MOVING) {
        m_CurrentAnimation->Play(); 
    } else {
        m_CurrentAnimation->Pause();
        m_CurrentAnimation->SetCurrentFrame(0); 
    }
}

bool Character::TryMove(int dx, int dy, std::shared_ptr<Map> map) {
    int targetX = m_GridX + dx;
    int targetY = m_GridY + dy;

    // Visual direction mapping (Up is visually +y, Down is visually -y in OpenGL)
    m_CurrentDirection = {(float)dx, (float)-dy};

    if (map->IsWalkable(targetX, targetY)) {
        m_GridX = targetX;
        m_GridY = targetY;
        m_State = State::MOVING;
        m_IsMoving = true;
        return true;
    } else {
        m_State = State::IDLE;
        m_CurrentDirection = {0.0f, 0.0f};
        return false;
    }
}

glm::vec2 Character::Update(std::shared_ptr<Map> map) { 
    (void)map;
    glm::vec2 movement = {0.0f, 0.0f};

    // --- NEW: Get Delta Time in seconds ---
    // (Assuming GetDeltaTimeMs() returns milliseconds. If your engine just has 
    // GetDeltaTime() that already returns seconds, you don't need the / 1000.0f)
    float dt = Util::Time::GetDeltaTimeMs() / 1000.0f; 

    // 1. Z-SORTING FIX
    if (m_UseDynamicZ) {
        float footY = m_Transform.translation.y - (GameConfig::SCALED_TILE_SIZE * 0.5f);

        float yOffset = footY / 10000.0f;

        // Cantor pairing — same formula as Prop so characters sort
        // consistently within the same Z space as props
        int cantorKey = (m_GridX >= 0 && m_GridY >= 0)
                    ? ((m_GridX + m_GridY) * (m_GridX + m_GridY + 1) / 2 + m_GridY)
                    : (int)(m_Transform.translation.x + m_Transform.translation.y * 1000);
        float tiebreak = cantorKey * 0.000001f;

        // Priority: m_BaseZIndex (layer) > yOffset (row) > tiebreak (cell)
        SetZIndex(m_BaseZIndex - yOffset + tiebreak);
}
    
    //map->UpdatePropOverlap(m_GridX, m_GridY, m_Transform.translation.y - (GameConfig::SCALED_TILE_SIZE * 0.5f));

    if (m_IsMoving) {
        // --- NEW: Multiply speed by Delta Time ---
        //float step = m_Speed * dt; 
        float step = m_Speed * m_SpeedMultiplier * dt;
        
        if (m_PixelsMoved + step > GameConfig::EFFECTIVE_TILE_SIZE) { 
            step = GameConfig::EFFECTIVE_TILE_SIZE - m_PixelsMoved; 
        }
        m_PixelsMoved += step;
        movement = {m_CurrentDirection.x * step, m_CurrentDirection.y * step};

        if (m_PixelsMoved >= GameConfig::EFFECTIVE_TILE_SIZE) {
            m_IsMoving = false;
            m_PixelsMoved = 0.0f;
            m_CurrentDirection = {0.0f, 0.0f};
        }
    }

    UpdateSprite(); 
    return movement; 
}

void Character::StopMoving() {
    m_IsMoving = false;
    m_PixelsMoved = 0.0f;
    m_CurrentDirection = {0.0f, 0.0f};
    m_State = State::IDLE;
}

// Allows us to force a character to look a certain way
void Character::SetDirection(Direction dir) {
    m_Direction = dir;
    UpdateSprite();
}

void Character::AddItem(const std::string& itemName, ItemCategory category, int amount) {
    // If the item already exists in the bag, just add to the quantity
    if (m_Inventory.find(itemName) != m_Inventory.end()) {
        m_Inventory[itemName].quantity += amount;
    } else {
        // If it's a new item, store the amount AND the category
        m_Inventory[itemName] = {amount, category};
    }
}

bool Character::RemoveItem(const std::string& itemName, int amount) {
    // 1. First, check if the item actually exists in the backpack
    auto it = m_Inventory.find(itemName);
    
    if (it != m_Inventory.end()) {
        // 2. Check if they have enough of it
        if (it->second.quantity >= amount) {
            it->second.quantity -= amount;
            
            // 3. Clean up: If they used the last one, remove it from the map entirely
            if (it->second.quantity <= 0) {
                m_Inventory.erase(it);
            }
            
            return true; // Successfully used/removed
        }
    }
    
    LOG_TRACE("Not enough {}!", itemName);
    return false; // They don't have enough!
}

int Character::GetItemCount(const std::string& itemName) const {
    auto it = m_Inventory.find(itemName);
    if (it != m_Inventory.end()) {
        // Return just the quantity from our struct!
        return it->second.quantity; 
    }
    return 0; // They own 0 of this item
}

void Character::PrintInventory() const {
    LOG_INFO("--- PLAYER INVENTORY ---");
    for (const auto& pair : m_Inventory) {
        LOG_INFO("{}: {}", pair.first, pair.second.quantity);
    }
    LOG_INFO("------------------------");
}