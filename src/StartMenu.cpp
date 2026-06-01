#include "StartMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

StartMenu::StartMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // 1. Box (unchanged)
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto boxImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/MenuBoxUI2.png");
    m_BoxUI->SetDrawable(boxImg);
    m_BoxUI->SetZIndex(90.0f);
    m_BoxUI->m_Transform.scale = {BOX_SCALE_X, BOX_SCALE_X};
    m_BoxUI->m_Transform.translation = {BOX_POS_X, BOX_POS_Y};
    m_Renderer->AddChild(m_BoxUI);

    // 2. Text lines – built from m_Items
    BuildMenuGraphics();

    // 3. Cursor
    m_CursorUI = std::make_shared<Util::GameObject>();
    auto cursorImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/Cursor.png");
    m_CursorUI->SetDrawable(cursorImg);
    m_CursorUI->SetZIndex(92.0f);
    m_CursorUI->m_Transform.scale = {2.5f, 2.5f};
    m_Renderer->AddChild(m_CursorUI);

    SetVisible(false);
}

void StartMenu::BuildMenuGraphics() {
    // remove old text objects
    for (auto& go : m_ItemTexts)
        m_Renderer->RemoveChild(go);
    m_ItemTexts.clear();

    for (size_t i = 0; i < m_Items.size(); ++i) {
        auto go = std::make_shared<Util::GameObject>();
        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/micross.ttf", 32,
            m_Items[i].label,
            Util::Color(50, 50, 50)
        );
        go->SetDrawable(txt);
        go->SetZIndex(91.0f);

        // Get the width of the rendered text
        float textWidth = txt->GetSize().x;   // assumes GetSize() returns width and height
        // Left‑align: centre of the text should be at leftMargin + half its width
        float posX = TEXT_LEFT_MARGIN + textWidth / 2.0f;
        // Y: keep your original convention (topmost line at TEXT_START_Y, then move
        //   upward/negatively for each subsequent line; verify this matches your screen axes)
        float posY = TEXT_START_Y - static_cast<float>(i) * LINE_SPACING;

        go->m_Transform.translation = {posX, posY};
        m_Renderer->AddChild(go);
        m_ItemTexts.push_back(go);
    }
}

void StartMenu::SetVisible(bool visible) {
    m_BoxUI->SetVisible(visible);
    for (auto& go : m_ItemTexts) go->SetVisible(visible);
    m_CursorUI->SetVisible(visible);

    if (visible) {
        m_CursorIndex = 0;
        UpdateCursorPosition();
    }
}

StartMenu::Option StartMenu::Update() {
    return ProcessInput();
}

StartMenu::Option StartMenu::ProcessInput() {
    // 1. Input cooldown (prevents cursor from flying)
    if (m_InputTimer > 0) {
        --m_InputTimer;
        // Still allow immediate action keys (cancel, select) even during cooldown? 
        // For safety, only allow them when timer is 0, but you can adjust.
        // Here we check after cooldown only.
        return Option::NONE;
    }

    // 2. Navigation
    if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::W)) {
        m_CursorIndex = (m_CursorIndex - 1 + m_Items.size()) % m_Items.size();
        UpdateCursorPosition();
        m_InputTimer = INPUT_COOLDOWN;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) || Util::Input::IsKeyDown(Util::Keycode::S)) {
        m_CursorIndex = (m_CursorIndex + 1) % m_Items.size();
        UpdateCursorPosition();
        m_InputTimer = INPUT_COOLDOWN;
    }

    // 3. Selection
    if (Util::Input::IsKeyDown(Util::Keycode::RETURN) || Util::Input::IsKeyDown(Util::Keycode::Z)) {
        return m_Items[m_CursorIndex].value;
    }

    // 4. Cancel / close menu
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) || Util::Input::IsKeyDown(Util::Keycode::X)) {
        return Option::CANCEL;
    }

    return Option::NONE;
}

void StartMenu::UpdateCursorPosition() {
    if (m_CursorIndex < 0 || m_CursorIndex >= static_cast<int>(m_ItemTexts.size()))
        return;

    float textY = m_ItemTexts[m_CursorIndex]->m_Transform.translation.y;
    // Place the cursor to the left of the text; adjust vertical fine‑tuning as needed
    m_CursorUI->m_Transform.translation = {TEXT_LEFT_MARGIN - 20.0f, textY + 5.0f};
}