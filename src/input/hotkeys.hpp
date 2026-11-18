#pragma once

#include "keybind.hpp"

namespace hotkeys {
    constexpr KeyBind move         = { KEY_SPACE };
    constexpr KeyBind spawn_wire   = { KEY_LEFT_SHIFT };
    constexpr KeyBind spawn_gate   = { KEY_ONE };
    constexpr KeyBind spawn_switch = { KEY_TWO };
    constexpr KeyBind spawn_led    = { KEY_THREE };
    constexpr KeyBind fullscreen   = { KEY_F11 };
} // namespace hotkeys
