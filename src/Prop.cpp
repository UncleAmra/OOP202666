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
    if (m_UseDynamicZ) {
        float footY = m_Transform.translation.y - (GameConfig::SCALED_TILE_SIZE * 0.5f);
        float dynamicZ = 0.5f - (footY / 10000.0f);
        SetZIndex(dynamicZ);
    }

    // --- 2. PLAYER-TRIGGERED ANIMATION (existing tall grass logic) ---
    if (m_AnimMode == PropAnimMode::STATIC && m_CurrentState != m_TargetState) {
        m_FrameDelayCounter++;
        if (m_FrameDelayCounter >= 8) {
            m_FrameDelayCounter = 0;
            if (m_CurrentState < m_TargetState) SetState(m_CurrentState + 1);
            else SetState(m_CurrentState - 1);
        }
        return; // Don't run auto-anim while transitioning
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
            // Reverse at ends
            if (next >= (int)m_Images.size() || next < 0) {
                m_AnimDirection *= -1;
                next = m_CurrentState + m_AnimDirection;
            }
            SetState(next);
        }
    }
}
