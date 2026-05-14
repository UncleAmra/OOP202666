#include "BattleAnimation.hpp"
// Uses nlohmann/json — add to your CMakeLists: find_package(nlohmann_json)
// or drop the single-header json.hpp into your include folder.
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include "Util/Logger.hpp"

// ============================================================================
// AnimationLibrary::LoadFromJson
//
// Reads the JSON produced by rxdata_to_json.py and populates m_Defs.
// Expected JSON structure (one entry):
// {
//   "id": 8,
//   "name": "Pokeball_effect",
//   "animation_name": "leer",
//   "frame_max": 6,
//   "position": 1,
//   "timings": [ { "frame":0, "sound":"recall", "volume":100, "pitch":100,
//                  "flash_scope":0, "flash_duration":5,
//                  "flash_color":[255,255,255,255] } ],
//   "frames": [
//     { "cell_max": 1, "cells": [
//         { "pattern":0, "x":-8, "y":8, "scale":100, "rotation":0,
//           "blend_type":0, "opacity":255, "mirror":0 }
//       ]
//     }, ...
//   ]
// }
// ============================================================================

#include "BattleAnimation.hpp"
#include "Util/Logger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>

void AnimationLibrary::LoadFromJson(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        LOG_ERROR("AnimationLibrary failed to open: {}", jsonPath);
        return;
    }

    try {
        nlohmann::json j = nlohmann::json::parse(f);

        m_Defs.clear();
        m_Index.clear();

        for (const auto& item : j) {
            BattleAnimDef def;

            // ── Top-level fields ─────────────────────────────────────────
            def.name       = item.value("name",     "");   // "Move:THUNDERSHOCK"
            // In AnimationLibrary::LoadFromJson, after reading def.graphic:
            def.graphic = item.value("graphic", "");

                      
            def.position   = static_cast<AnimPosition>(item.value("position", 1));
            def.id         = item.value("id", -1);

            // Skip entries with no name
            if (def.name.empty()) continue;

            // ── Frames ───────────────────────────────────────────────────
            // Each frame is a flat array of cel objects (no "cels" wrapper)
            if (item.contains("frames") && item["frames"].is_array()) {
                for (const auto& jFrame : item["frames"]) {
                    AnimFrame frame;
                    // jFrame is directly an array of cel objects
                    if (jFrame.is_array()) {
                        for (const auto& jCel : jFrame) {
                            AnimCel cel;
                            cel.pattern    = jCel.value("pattern",    0);
                            cel.x          = jCel.value("x",          0.0f);
                            cel.y          = jCel.value("y",          0.0f);
                            cel.zoom_x     = jCel.value("zoom_x",     100.0f);
                            cel.zoom_y     = jCel.value("zoom_y",     100.0f);
                            cel.rotation   = jCel.value("rotation",   0.0f);
                            cel.mirror     = jCel.value("mirror",     false);
                            cel.blend_type = jCel.value("blend_type", 0);
                            cel.opacity    = jCel.value("opacity",    255);
                            cel.focus      = static_cast<CelFocus>(jCel.value("focus", 1));
                            frame.push_back(cel);
                        }
                    }
                    def.frames.push_back(frame);
                }
            }

            // ── Timings ──────────────────────────────────────────────────
            if (item.contains("timings") && item["timings"].is_array()) {
                for (const auto& jT : item["timings"]) {
                    AnimTiming t;
                    t.type  = static_cast<TimingType>(jT.value("type",  -1));
                    t.frame = jT.value("frame", 0);

                    if (t.type == TimingType::SE) {
                        t.sound  = jT.value("sound",  "");
                        t.volume = jT.value("volume", 100);
                        t.pitch  = jT.value("pitch",  100);
                    } else if (t.type == TimingType::SCREEN_FLASH) {
                        t.flash_r        = jT.value("flash_r",   255);
                        t.flash_g        = jT.value("flash_g",   255);
                        t.flash_b        = jT.value("flash_b",   255);
                        t.flash_a        = jT.value("flash_a",   255);
                        t.flash_duration = jT.value("duration",    8);
                    }
                    def.timings.push_back(t);
                }
            }

            m_Index[def.name] = m_Defs.size();
            m_Defs.push_back(std::move(def));
        }

        LOG_INFO("Successfully loaded {} animations!", m_Defs.size());

    } catch (const std::exception& e) {
        LOG_ERROR("JSON PARSE ERROR: {}", e.what());
    }
}

const BattleAnimDef* AnimationLibrary::Find(const std::string& name) const {
    auto it = m_Index.find(name);
    if (it != m_Index.end()) {
        return &m_Defs[it->second];
    }
    return nullptr;
}