#include "NPC.hpp"
#include "Util/LoadTextFile.hpp"
#include "GameFlags.hpp"

// 1. Initialize m_UseDynamicZ to false by default
NPC::NPC(float x, float y, const std::string& spritePath,
     const std::string& dialoguePath, 
     const std::string& altDialoguePath, 
     const std::string& flagCondition) 

    : Character(x, y), m_SpritePath(spritePath), m_FlagCondition(flagCondition) {
    m_UseDynamicZ = false;                                                                      // Set the BASE class variable inside the body!
    if (!dialoguePath.empty())    {m_DialogueLines    = Util::LoadDialogueFile(dialoguePath);}  // Load the text file if the path isn't empty!
         else {m_DialogueLines.push_back("...");}
    if (!altDialoguePath.empty()) {m_AltDialogueLines = Util::LoadDialogueFile(altDialoguePath);}// Load Alternative Dialogue (if provided)
    LoadSprites();
    UpdateSprite();
}

void NPC::LoadSprites() {
    // For now, we just use the same image for all 4 directions
    std::vector<std::string> downFrames  = {m_SpritePath + "_Down.png"};
    std::vector<std::string> upFrames    = {m_SpritePath + "_Up.png"};
    std::vector<std::string> leftFrames  = {m_SpritePath + "_Left.png"};
    std::vector<std::string> rightFrames = {m_SpritePath + "_Right.png"};
    
    m_AnimDown  = std::make_shared<Util::Animation>(downFrames, false, 150, true, 0);
    m_AnimUp    = std::make_shared<Util::Animation>(upFrames, false, 150, true, 0);
    m_AnimLeft  = std::make_shared<Util::Animation>(leftFrames, false, 150, true, 0);
    m_AnimRight = std::make_shared<Util::Animation>(rightFrames, false, 150, true, 0);
}

glm::vec2 NPC::Update(std::shared_ptr<Map> map) {
    //if (!map) return {0.0f, 0.0f}; 
    
    // We deleted the math! The parent class handles the Y-sorting perfectly now.
    return Character::Update(map); 
}

std::vector<std::string> NPC::Interact() {
    std::vector<std::string> lines;
    
    // If this NPC has a flag condition AND that flag is currently true...
    if (!m_FlagCondition.empty() && GameFlags::Get(m_FlagCondition)) {
        return m_AltDialogueLines; // Return the post-event text!
    }
    
    // Otherwise, return normal text
    return m_DialogueLines;
}

// In NPC.cpp
void NPC::SetAction(NPCAction type, const std::string& data, ItemCategory itemCategory) {
    m_ActionType     = type;
    m_ActionData     = data;
    m_ActionCategory = itemCategory;
}