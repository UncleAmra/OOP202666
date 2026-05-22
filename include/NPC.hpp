#ifndef NPC_HPP
#define NPC_HPP

#include "Character.hpp"
#include <string>
#include <vector> // Make sure vector is included


enum class NPCAction {
    NONE,           // Just standard dialogue
    HEAL,           // Heals the player's party
    SHOP,           // Opens the Pokemart UI
    GIVE_ITEM,      // Gives the player an item
    BATTLE          // Starts a Pokemon battle
};
class NPC : public Character {
public:
    // 1. Updated constructor with default empty strings for the new features
    NPC(float x, float y, const std::string& spritePath, 
        const std::string& dialoguePath, 
        const std::string& altDialoguePath = "", 
        const std::string& flagCondition = "");

    glm::vec2 Update(std::shared_ptr<Map>) override;
    void SetDynamicZ(bool dynamic) { m_UseDynamicZ = dynamic; }
    
    std::vector<std::string> Interact();
    // Add these new methods!
    void SetAction(NPCAction type, const std::string& data = "",
                   ItemCategory itemCategory = ItemCategory::GENERAL);
    NPCAction GetActionType() const { return m_ActionType; }
    std::string GetActionData() const { return m_ActionData; }
    ItemCategory GetActionCategory() const { return m_ActionCategory; } 

protected:
    void LoadSprites() override;

private:
    std::string m_SpritePath;
    //bool m_UseDynamicZ;
    
    // 2. Changed m_Dialogue to a vector called m_DialogueLines
    std::vector<std::string> m_DialogueLines;       
    std::vector<std::string> m_AltDialogueLines;    
    std::string m_FlagCondition;     
    NPCAction m_ActionType = NPCAction::NONE;
    ItemCategory m_ActionCategory = ItemCategory::GENERAL;
    std::string m_ActionData = ""; // Stores item names, shop inventory IDs, or Trainer IDs               
};

#endif