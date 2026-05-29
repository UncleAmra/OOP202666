#include "InventoryMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include <algorithm>

InventoryMenu::InventoryMenu(std::shared_ptr<Util::Renderer> renderer) {
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto invBoxImage = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/InventoryBoxUI.png");    
    m_BoxUI->SetDrawable(invBoxImage);
    m_BoxUI->SetZIndex(15.0f); 
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f}; 

    m_TextUI = std::make_shared<Util::GameObject>();
    m_Text   = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/micross.ttf", 
        24, "Inventory", Util::Color(50, 50, 50) 
    );
    m_TextUI->SetDrawable(m_Text);
    m_TextUI->SetZIndex(16.0f);  

    renderer->AddChild(m_BoxUI);
    renderer->AddChild(m_TextUI);
    Hide();
}

void InventoryMenu::Show(const std::map<ItemCategory, std::vector<std::pair<std::string, int>>>& categorizedItems) {
    m_CategorizedItems = categorizedItems;
    
    m_CurrentTab = ItemCategory::GENERAL;
    m_SelectedIndex = 0;
    m_ScrollOffset = 0;

    UpdateText(); 

    m_BoxUI->SetVisible(true);
    m_TextUI->SetVisible(true);
}

void InventoryMenu::Hide() {
    m_BoxUI->SetVisible(false);
    m_TextUI->SetVisible(false);
}

bool InventoryMenu::Update() {
    if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        return true; 
    }

    bool needsRedraw = false;
    const auto& currentList = m_CategorizedItems[m_CurrentTab];

    // --- LEFT/RIGHT TAB SWITCHING ---
    if (Util::Input::IsKeyDown(Util::Keycode::RIGHT)) {
        int nextTab = (static_cast<int>(m_CurrentTab) + 1) % static_cast<int>(ItemCategory::COUNT);
        m_CurrentTab = static_cast<ItemCategory>(nextTab);
        m_SelectedIndex = 0; 
        m_ScrollOffset = 0;
        needsRedraw = true;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::LEFT)) {
        int prevTab = (static_cast<int>(m_CurrentTab) - 1);
        if (prevTab < 0) prevTab = static_cast<int>(ItemCategory::COUNT) - 1;
        m_CurrentTab = static_cast<ItemCategory>(prevTab);
        m_SelectedIndex = 0; 
        m_ScrollOffset = 0;
        needsRedraw = true;
    }

    // --- UP/DOWN SCROLLING ---
    if (!currentList.empty()) {
        if (Util::Input::IsKeyDown(Util::Keycode::UP)) {
            if (m_SelectedIndex > 0) {
                m_SelectedIndex--;
                if (m_SelectedIndex < m_ScrollOffset) m_ScrollOffset--;
                needsRedraw = true;
            }
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::DOWN)) {
            if (m_SelectedIndex < static_cast<int>(currentList.size()) - 1) {
                m_SelectedIndex++;
                if (m_SelectedIndex >= m_ScrollOffset + MAX_VISIBLE_ITEMS) m_ScrollOffset++;
                needsRedraw = true;
            }
        }
    }

    if (needsRedraw) UpdateText();
    return false;
}

void InventoryMenu::UpdateText() {
    std::string displayStr = "";

    // 1. Draw Tab Header based on Enum
    if (m_CurrentTab == ItemCategory::GENERAL)        displayStr += "  <     ITEMS     >\n\n";
    else if (m_CurrentTab == ItemCategory::POKEBALLS) displayStr += "  <   POKEBALLS   >\n\n";
    else if (m_CurrentTab == ItemCategory::KEY_ITEMS) displayStr += "  <   KEY ITEMS   >\n\n";

    // 2. Draw Current List
    const auto& currentList = m_CategorizedItems[m_CurrentTab];

    if (currentList.empty()) {
        displayStr += "\n    (Empty)";
    } else {
        int endIndex = std::min(static_cast<int>(currentList.size()), m_ScrollOffset + MAX_VISIBLE_ITEMS);
        
        if (m_ScrollOffset > 0) displayStr += "   /\\ ...\n";
        else displayStr += "\n"; 

        for (int i = m_ScrollOffset; i < endIndex; ++i) {
            if (i == m_SelectedIndex) {
                displayStr += " > " + currentList[i].first + " x" + std::to_string(currentList[i].second) + "\n";
            } else {
                displayStr += "   " + currentList[i].first + " x" + std::to_string(currentList[i].second) + "\n";
            }
        }

        if (endIndex < static_cast<int>(currentList.size())) displayStr += "   \\/ ...\n";
    }

    m_Text->SetText(displayStr);

    float invHalfWidth = m_Text->GetSize().x / 2.0f;
    m_TextUI->m_Transform.translation.x = -400.0f + invHalfWidth;
}