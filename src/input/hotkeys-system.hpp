#pragma once

#include "editor.hpp"
#include "pin-hit.hpp"

struct Hotkeys {
    Editor editor{};

    Camera2D camera{ {0, 0}, {0, 0}, 0.0f, 1.0f };
    Selection selection{};
    Clipboard clipboard{};

    bool isPanning = false;
    Vector2 panLastMouse{};

    bool isSelecting = false;
    Vector2 selectStart{};
    Rectangle selectBox{};

    bool isDragging = false;
    Vector2 dragLastWorld{};
    Vector2 clickStartWorld{};

    PinHit wireStart{};

    bool hotkeysUpdate(Circuit& c, Layout& pos) {
        Vector2 screenMouse = GetMousePosition();
        Vector2 mouse = GetScreenToWorld2D(screenMouse, camera);
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        bool space = IsKeyDown(KEY_SPACE);

        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

        if (ctrl && IsKeyPressed(KEY_C) && !selection.empty()) {
            clipboard = {};
            Vector2 center{};
            int total = 0;

            for (int i : selection.gates)    { center = Vector2Add(center, pos.gates[i]);    total++; }
            for (int i : selection.switches) { center = Vector2Add(center, pos.switches[i]); total++; }
            for (int i : selection.leds)     { center = Vector2Add(center, pos.leds[i]);     total++; }
            center = Vector2Scale(center, 1.0f / total);

            for (int i : selection.gates)    clipboard.gates.push_back(Vector2Subtract(pos.gates[i], center));
            for (int i : selection.switches) clipboard.switches.push_back(Vector2Subtract(pos.switches[i], center));
            for (int i : selection.leds)     clipboard.leds.push_back(Vector2Subtract(pos.leds[i], center));

            auto findLocalIdx = [&](Kind type, int oldIdx) -> int {
                const auto& list = (type == GATE) ? selection.gates : (type == SWITCH ? selection.switches : selection.leds);
                auto it = std::find(list.begin(), list.end(), oldIdx);
                return (it != list.end()) ? (int)std::distance(list.begin(), it) : -1;
            };

            for (const auto& w : c.wires) {
                int newFrom = findLocalIdx(w.fromType, w.from);
                int newTo   = findLocalIdx(w.toType, w.to);
                if (newFrom != -1 && newTo != -1) {
                    clipboard.wires.push_back({ newFrom, newTo, w.fromType, w.toType, w.pin, false });
                }
            }
        }

        if (ctrl && IsKeyPressed(KEY_V) && !clipboard.empty()) {
            selection.clear();

            int baseGate   = (int)c.gates.size();
            int baseSwitch = (int)c.switches.size();
            int baseLed    = (int)c.leds.size();

        auto pasteAll = [&]() {
            auto paste = [&](const auto& clipList, auto& selList, auto& cList, auto& posList) {
                for (const auto& rel : clipList) {
                    selList.push_back((int)cList.size());
                    cList.push_back({});
                    posList.push_back(Vector2Add(mouse, rel));
                }
            };

            paste(clipboard.gates,    selection.gates,    c.gates,    pos.gates);
            paste(clipboard.switches, selection.switches, c.switches, pos.switches);
            paste(clipboard.leds,     selection.leds,     c.leds,     pos.leds);
        };

            for (auto w : clipboard.wires) {
                w.from += (w.fromType == SWITCH) ? baseSwitch : baseGate;
                w.to   += (w.toType == LED)      ? baseLed    : baseGate;
                c.wires.push_back(w);
            }
        }

        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (wireStart) { wireStart = {}; return false; }
            if (!selection.empty()) { selection.clear(); return false; }
            return true;
        }

        if (space && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isPanning = true;
            panLastMouse = screenMouse;
            return false;
        }
        if (isPanning) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                camera.target = Vector2Subtract(camera.target, Vector2Subtract(screenMouse, panLastMouse));
                panLastMouse = screenMouse;
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                return false;
            }
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            isPanning = false;
        }

        if (IsKeyPressed(KEY_ONE))   { c.gates.push_back({});    pos.gates.push_back({ mouse.x - 30, mouse.y - 20 }); }
        if (IsKeyPressed(KEY_TWO))   { c.switches.push_back({}); pos.switches.push_back({ mouse.x - 20, mouse.y - 20 }); }
        if (IsKeyPressed(KEY_THREE)) { c.leds.push_back({});     pos.leds.push_back({ mouse.x - 20, mouse.y - 20 }); }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PinHit hit = editor.findPin(mouse, pos);
            if (!wireStart && shift && hit) {
                wireStart = hit;
                return false;
            }
            if (wireStart) {
                if (hit && hit.isOut != wireStart.isOut &&
                    !(hit.type == wireStart.type && hit.idx == wireStart.idx)) {
                    PinHit outPin = wireStart.isOut ? wireStart : hit;
                    PinHit inPin  = wireStart.isOut ? hit : wireStart;
                    c.wires.push_back({ outPin.idx, inPin.idx, outPin.type, inPin.type, inPin.gatePin, false });
                }
                wireStart = {};
                return false;
            }
        }

        if (!wireStart && !space && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            clickStartWorld = mouse;
            dragLastWorld = mouse;

            Kind hitType = GATE;
            int hitIdx = -1;

            auto checkHit = [&](const std::vector<Vector2>& list, Kind type, float w, float h) {
                for (int i = (int)list.size() - 1; i >= 0; --i) {
                    if (CheckCollisionPointRec(mouse, { list[i].x, list[i].y, w, h })) {
                        hitType = type; hitIdx = i; return true;
                    }
                }
                return false;
            };

            if (checkHit(pos.gates, GATE, 60, 40) || checkHit(pos.switches, SWITCH, 40, 40) || checkHit(pos.leds, LED, 40, 40)) {
                isDragging = true;
                if (!selection.has(hitType, hitIdx)) {
                    selection.clear();
                    if (hitType == GATE) selection.gates.push_back(hitIdx);
                    else if (hitType == SWITCH) selection.switches.push_back(hitIdx);
                    else if (hitType == LED) selection.leds.push_back(hitIdx);
                }
            } else {
                selection.clear();
                isSelecting = true;
                selectStart = mouse;
                selectBox = { mouse.x, mouse.y, 0, 0 };
            }
        }

        if (isDragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = Vector2Subtract(mouse, dragLastWorld);
            for (int idx : selection.gates)    pos.gates[idx]    = Vector2Add(pos.gates[idx], delta);
            for (int idx : selection.switches) pos.switches[idx] = Vector2Add(pos.switches[idx], delta);
            for (int idx : selection.leds)     pos.leds[idx]     = Vector2Add(pos.leds[idx], delta);
            dragLastWorld = mouse;
        }

        if (isSelecting && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            selectBox = {
                std::min(selectStart.x, mouse.x),
                std::min(selectStart.y, mouse.y),
                std::abs(mouse.x - selectStart.x),
                std::abs(mouse.y - selectStart.y)
            };
            selection.clear();
            for (int i = 0; i < (int)pos.gates.size(); ++i)
                if (CheckCollisionRecs(selectBox, { pos.gates[i].x, pos.gates[i].y, 60, 40 })) selection.gates.push_back(i);
            for (int i = 0; i < (int)pos.switches.size(); ++i)
                if (CheckCollisionRecs(selectBox, { pos.switches[i].x, pos.switches[i].y, 40, 40 })) selection.switches.push_back(i);
            for (int i = 0; i < (int)pos.leds.size(); ++i)
                if (CheckCollisionRecs(selectBox, { pos.leds[i].x, pos.leds[i].y, 40, 40 })) selection.leds.push_back(i);
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (isDragging && Vector2Distance(clickStartWorld, mouse) < 4.0f) {
                if (selection.switches.size() == 1 && selection.gates.empty() && selection.leds.empty()) {
                    c.switches[selection.switches[0]].on = !c.switches[selection.switches[0]].on;
                }
            }
            isDragging = false;
            isSelecting = false;
        }

        if (IsKeyPressed(KEY_X)) {
            if (!selection.empty()) {
                std::sort(selection.gates.rbegin(), selection.gates.rend());
                std::sort(selection.switches.rbegin(), selection.switches.rend());
                std::sort(selection.leds.rbegin(), selection.leds.rend());

                for (int idx : selection.gates)    editor.deleteNode(c, pos, GATE, idx);
                for (int idx : selection.switches) editor.deleteNode(c, pos, SWITCH, idx);
                for (int idx : selection.leds)     editor.deleteNode(c, pos, LED, idx);
                selection.clear();
                return false;
            }
        }
        return false;
    }
};
