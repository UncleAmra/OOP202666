#include "PokemonMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

const std::string POKEMON_RES = std::string(RESOURCE_DIR) + "/Pokemon/";

PokemonMenu::PokemonMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // --- Box background ---
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto boxImg = ResourceManager::GetImageStore().Get(
        RESOURCE_DIR "/UI/InventoryBoxUI.png");
    m_BoxUI->SetDrawable(boxImg);
    m_BoxUI->SetZIndex(90.0f);
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f};
    m_BoxUI->SetVisible(false);
    m_Renderer->AddChild(m_BoxUI);

    // --- Cursor ---
    m_CursorUI = std::make_shared<Util::GameObject>();
    auto cursorImg = ResourceManager::GetImageStore().Get(
        RESOURCE_DIR "/UI/Cursor.png");
    m_CursorUI->SetDrawable(cursorImg);
    m_CursorUI->SetZIndex(92.0f);
    m_CursorUI->SetVisible(false);
    m_Renderer->AddChild(m_CursorUI);
}

void PokemonMenu::ClearSlots() {
    for (auto& slot : m_Slots) {
        if (slot.sprite) m_Renderer->RemoveChild(slot.sprite);
        if (slot.text)   m_Renderer->RemoveChild(slot.text);
    }
    m_Slots.clear();
}

void PokemonMenu::BuildSlots(const std::vector<std::shared_ptr<Pokemon>>& party) {
    ClearSlots();

    if (party.empty()) {
        // Create a single slot with "No Pokemon in party!" text
        Slot emptySlot;
        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/micross.ttf", 24,
            "No Pokemon in party!", Util::Color(50, 50, 50));
        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(txt);
        txtObj->SetZIndex(91.0f);
        // Centre the text (roughly)
        float textW = txt->GetSize().x;
        txtObj->m_Transform.translation = { -textW/2.0f, SLOT_START_Y };
        m_Renderer->AddChild(txtObj);
        emptySlot.text = txtObj;
        m_Slots.push_back(emptySlot);
        return;
    }

    for (size_t i = 0; i < party.size(); ++i) {
        const auto& p = party[i];
        Slot slot;

        // 1. SPRITE
        std::string spritePath = POKEMON_RES + p->GetName() + "_front_1.png";
        auto spriteObj = std::make_shared<Util::GameObject>();
        auto spriteImg = ResourceManager::GetImageStore().Get(spritePath);
        if (!spriteImg) {
            // Fallback to a placeholder if image missing
            spriteImg = ResourceManager::GetImageStore().Get(
                RESOURCE_DIR "/Pokemon/placeholder.png");
        }
        spriteObj->SetDrawable(spriteImg);
        spriteObj->m_Transform.scale = {SPRITE_SCALE, SPRITE_SCALE};
        spriteObj->SetZIndex(91.0f);

        // 2. INFO TEXT
        std::string info = p->GetName()
                         + "  Lv." + std::to_string(p->GetLevel())
                         + "  HP: " + std::to_string(p->GetCurrentHP())
                         + "/" + std::to_string(p->GetMaxHP());
        auto textDraw = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/micross.ttf", 24,
            info, Util::Color(50, 50, 50));
        auto textObj = std::make_shared<Util::GameObject>();
        textObj->SetDrawable(textDraw);
        textObj->SetZIndex(91.0f);

        // 3. POSITIONING
        float y = SLOT_START_Y - static_cast<float>(i) * SLOT_SPACING;
        // Sprite: centred at SPRITE_LEFT_X
        spriteObj->m_Transform.translation = { SPRITE_LEFT_X, y };
        // Text: left‑aligned at TEXT_LEFT_MARGIN (manually offset by half width)
        float textW = textDraw->GetSize().x;
        textObj->m_Transform.translation = { TEXT_LEFT_MARGIN + textW / 2.0f, y };

        m_Renderer->AddChild(spriteObj);
        m_Renderer->AddChild(textObj);

        slot.sprite = spriteObj;
        slot.text   = textObj;
        m_Slots.push_back(slot);
    }
}

void PokemonMenu::Show(const std::vector<std::shared_ptr<Pokemon>>& party) {
    m_PartySize = static_cast<int>(party.size());
    m_CursorIndex = 0;
    m_InputCooldown = 0;

    BuildSlots(party);

    m_BoxUI->SetVisible(true);
    m_CursorUI->SetVisible(!party.empty());
    UpdateCursorPosition();
}

void PokemonMenu::Hide() {
    m_BoxUI->SetVisible(false);
    m_CursorUI->SetVisible(false);
    ClearSlots();   // remove all dynamic objects from renderer
}

bool PokemonMenu::Update() {
    // Input cooldown
    if (m_InputCooldown > 0) {
        --m_InputCooldown;
        // Still allow immediate cancel
        if (Util::Input::IsKeyDown(Util::Keycode::X) ||
            Util::Input::IsKeyDown(Util::Keycode::ESCAPE))
            return true;
        return false;
    }

    if (m_PartySize > 0) {
        if (Util::Input::IsKeyDown(Util::Keycode::UP) ||
            Util::Input::IsKeyDown(Util::Keycode::W)) {
            m_CursorIndex = (m_CursorIndex - 1 + m_PartySize) % m_PartySize;
            UpdateCursorPosition();
            m_InputCooldown = INPUT_DELAY;
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) ||
                 Util::Input::IsKeyDown(Util::Keycode::S)) {
            m_CursorIndex = (m_CursorIndex + 1) % m_PartySize;
            UpdateCursorPosition();
            m_InputCooldown = INPUT_DELAY;
        }
    }

    if (Util::Input::IsKeyDown(Util::Keycode::X) ||
        Util::Input::IsKeyDown(Util::Keycode::ESCAPE))
        return true;

    return false;
}

void PokemonMenu::UpdateCursorPosition() {
    if (m_PartySize == 0 || m_CursorIndex >= m_PartySize)
        return;

    // Align cursor next to the selected slot’s sprite
    float y = SLOT_START_Y - m_CursorIndex * SLOT_SPACING + CURSOR_Y_ADJUST;
    m_CursorUI->m_Transform.translation = {
        SPRITE_LEFT_X + CURSOR_OFFSET_X,
        y
    };
}