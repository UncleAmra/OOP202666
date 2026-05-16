#include "Prop.hpp" 
#include "ResourceManager.hpp" 
#include "GameConfig.hpp"

// NEW CONSTRUCTOR: Uses the vector list of image paths
Prop::Prop(const std::vector<std::string>& imagePaths, const glm::vec2& startPosition, float scale, float zIndex, int gridX, int gridY) {
    
    
    // 1. Setup the basic transform stuff
    m_Transform.translation = startPosition;
    m_Transform.scale = {scale, scale};
    SetZIndex(zIndex);

    // 2. Save the grid coordinates!
    m_GridX = gridX;
    m_GridY = gridY;
    m_BaseZIndex = zIndex;

    // 3. Load ALL images provided in the list
    for (const std::string& path : imagePaths) {
        if (!path.empty()) {
            m_Images.push_back(std::make_shared<Util::Image>(path));
        }
    }

    // 4. Set the default graphic to the first image (if one exists)
    if (!m_Images.empty()) {
        SetDrawable(m_Images[0]);
    }
}

// NEW FUNCTION: Switches the current sprite based on the state index
void Prop::SetState(int stateIndex) {
    if (stateIndex >= 0 && stateIndex < static_cast<int>(m_Images.size())) {
        m_CurrentState = stateIndex;
        SetDrawable(m_Images[stateIndex]);
    }
}

// UPDATED LOGIC: Uses SetState to flatten/unflatten the grass
void Prop::SetSteppedOn(bool stepped) {
    // If we are already doing this, ignore the command
    if (m_IsSteppedOn == stepped) return; 
    
    m_IsSteppedOn = stepped;
    
    if (stepped) {
        // Aim for the fully flattened state (the very last image in the list)
        m_TargetState = m_Images.size() - 1; 
    } else {
        // Aim for the fully upright state (the very first image)
        m_TargetState = 0; 
    }
}
void Prop::SetAnimMode(PropAnimMode mode, int frameDelay) {
    m_AnimMode = mode;
    m_AnimFrameDelay = frameDelay;
}

// RESTORED LOGIC: Your original dynamic Z-sorting math!
void Prop::Update() {
    // --- 1. DYNAMIC Z-SORTING ---
    // In Prop::Update(), account for sprite height when computing footY:
    if (m_UseDynamicZ) {
        float footY = m_Transform.translation.y;

        // Y-based sort within the layer — scaled small enough to never 
        // cross into an adjacent layer (layers spaced ~0.1f apart)
        float yOffset = footY / 10000.0f;

        // Cantor pairing — unique key per grid cell, scaled tiny enough
        // to only resolve same-row same-Y conflicts, never override layer or Y order
        // Height contribution — taller sprites get higher Z since they extend
    // further down from their centre anchor, scaled tiny so it only affects
    // same-position same-baseZ conflicts
        float spriteHeight = m_Drawable ? (m_Drawable->GetSize().y * GameConfig::SCALE) : 0.0f;
        float heightWeight = spriteHeight * 0.00002f;

        int cantorKey = (m_GridX >= 0 && m_GridY >= 0)
                    ? ((m_GridX + m_GridY) * (m_GridX + m_GridY + 1) / 2 + m_GridY)
                    : (int)(m_Transform.translation.x + m_Transform.translation.y * 1000);
        float tiebreak = cantorKey * 0.000001f + heightWeight;

        // Priority: m_BaseZIndex (layer) > yOffset (row) > tiebreak (cell)
        SetZIndex(m_BaseZIndex - yOffset + tiebreak);
    }

    // --- 2. PLAYER-TRIGGERED ANIMATION (existing tall grass logic) ---
    if (m_AnimMode == PropAnimMode::STATIC && m_CurrentState != m_TargetState) {
        m_FrameDelayCounter++;
        if (m_FrameDelayCounter >= 8) {
            m_FrameDelayCounter = 0;
            if (m_CurrentState < m_TargetState) SetState(m_CurrentState + 1);
            else SetState(m_CurrentState - 1);
        }
        return;
    }

    // --- 3. AUTO ANIMATION ---
    if (m_AnimMode == PropAnimMode::LOOP && m_Images.size() > 1) {
        m_AnimCounter++;
        if (m_AnimCounter >= m_AnimFrameDelay) {
            m_AnimCounter = 0;
            SetState((m_CurrentState + 1) % (int)m_Images.size());
        }
    }
    else if (m_AnimMode == PropAnimMode::PING_PONG && m_Images.size() > 1) {
        m_AnimCounter++;
        if (m_AnimCounter >= m_AnimFrameDelay) {
            m_AnimCounter = 0;
            int next = m_CurrentState + m_AnimDirection;
            if (next >= (int)m_Images.size() || next < 0) {
                m_AnimDirection *= -1;
                next = m_CurrentState + m_AnimDirection;
            }
            SetState(next);
        }
    }
}