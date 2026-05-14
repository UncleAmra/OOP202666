#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// ============================================================================
// BattleAnimation.hpp
//
// Data model for RPG Maker XP / Pokemon Essentials / Infinite Fusion
// battle animations, parsed from Animations.rxdata (via animations_final.json)
//
// Pipeline:
//   Animations.rxdata
//     └─► rxdata_to_json.py  →  animations_final.json  (one-time offline step)
//           └─► AnimationLibrary::LoadFromJson()        (at game startup)
//                 └─► AnimationPlayer::Play()           (per battle move)
//                       └─► BattleAnimator              (renders each frame)
// ============================================================================

// ----------------------------------------------------------------------------
// One cel within one animation frame.
// Directly mirrors the RPG Maker XP cell_data Table columns.
// ----------------------------------------------------------------------------
// ── Cel anchor (per-cel focus override) ─────────────────────────────────────
enum class CelFocus {
    TARGET = 0,
    USER   = 1,
    SCREEN = 2,
    BOTH   = 3,
};

// ── Animation-level anchor ───────────────────────────────────────────────────
enum class AnimPosition {
    SCREEN = 0,
    TARGET = 1,
    USER   = 2,
    BOTH   = 3,
};

// ── One rendered cel within a frame ─────────────────────────────────────────
struct AnimCel {
    int      pattern    = 0;
    float    x          = 0.f;
    float    y          = 0.f;
    float    zoom_x     = 100.f;   // ← replaces old "scale"
    float    zoom_y     = 100.f;   // ← new separate Y scale
    float    rotation   = 0.f;
    bool     mirror     = false;
    int      blend_type = 0;       // 0=normal, 1=additive, 2=subtractive
    int      opacity    = 255;
    CelFocus focus      = CelFocus::TARGET;  // ← new per-cel anchor
};

// ── AnimFrame is now just a flat list of cels (no wrapper struct) ────────────
using AnimFrame = std::vector<AnimCel>;

// ── Timing types ─────────────────────────────────────────────────────────────
enum class TimingType {
    SE           = 0,
    BG_CHANGE    = 1,
    SCREEN_SHAKE = 2,
    SCREEN_FLASH = 3,
    HIDE_USER    = 4,
    SHOW_USER    = 5,
    UNKNOWN      = -1,
};

// ── One timing event ─────────────────────────────────────────────────────────
struct AnimTiming {
    TimingType  type  = TimingType::UNKNOWN;
    int         frame = 0;

    // SE (type == SE)
    std::string sound;
    int         volume = 100;
    int         pitch  = 100;

    // Flash (type == SCREEN_FLASH)
    int flash_r = 255, flash_g = 255, flash_b = 255, flash_a = 255;
    int flash_duration = 8;
};

// ── Complete animation definition ────────────────────────────────────────────
struct BattleAnimDef {
    std::string  name;      // "Move:THUNDERSHOCK"
    std::string  graphic;   // "PRAS- Electric.png"  ← replaces old sheet_name
    AnimPosition position   = AnimPosition::TARGET;
    int          id         = -1;

    std::vector<AnimFrame>  frames;
    std::vector<AnimTiming> timings;

    static constexpr int CEL_W      = 192;
    static constexpr int CEL_H      = 192;
    static constexpr int SHEET_COLS = 5;
};

// AnimFrame is now just a flat vector of cels — no wrapper struct needed
using AnimFrame = std::vector<AnimCel>;
// ----------------------------------------------------------------------------
// Library — holds all loaded animation defs, looked up by name
// ----------------------------------------------------------------------------
class AnimationLibrary {
public:
    // Load from the JSON exported by rxdata_to_json.py
    // Call once at startup: AnimationLibrary::Get().LoadFromJson("Resources/animations_final.json");
    void LoadFromJson(const std::string& jsonPath);

    // Look up by animation name (case-sensitive, matches @name field)
    const BattleAnimDef* Find(const std::string& name) const;

    // All loaded defs (for iteration)
    const std::vector<BattleAnimDef>& All() const { return m_Defs; }

    // Singleton
    static AnimationLibrary& Get() {
        static AnimationLibrary instance;
        return instance;
    }

private:
    std::vector<BattleAnimDef>                    m_Defs;
    std::unordered_map<std::string, std::size_t>  m_Index; // name → m_Defs index
};