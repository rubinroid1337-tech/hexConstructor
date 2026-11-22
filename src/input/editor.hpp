#pragma once
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "raymath.h"
#include "../core/circuit.hpp"
#include "../render/circuit-render.hpp"
#include "../core/kind.hpp"
#include "pin-hit.hpp"
#include "clipboard.hpp"

struct Snapshot {
    Circuit circuit;
    Layout layout;
};

struct Editor {
    Camera2D camera{ {0, 0}, {0, 0}, 0.0f, 1.0f };
    Selection selection{};
    Clipboard clipboard{};

    bool isSingleDrag = false;
    bool isPanning = false;
    Vector2 panLastMouse{};

    bool isSelecting = false;
    Vector2 selectStart{};
    Rectangle selectBox{};

    bool isDragging = false;
    Vector2 dragLastWorld{};
    Vector2 clickStartWorld{};

    PinHit wireStart{};

    std::vector<Snapshot> undoStack;
    std::vector<Snapshot> redoStack;

    void pushHistory(const Circuit& c, const Layout& pos) {
        if (undoStack.size() >= 100) undoStack.erase(undoStack.begin()); // лимит 100 шагов
        undoStack.push_back({ c, pos });
        redoStack.clear();
    }

    void undo(Circuit& c, Layout& pos) {
        if (undoStack.empty()) return;
        redoStack.push_back({ c, pos });
        c   = std::move(undoStack.back().circuit);
        pos = std::move(undoStack.back().layout);
        undoStack.pop_back();
        selection.clear();
        wireStart = {};
    }

    void redo(Circuit& c, Layout& pos) {
        if (redoStack.empty()) return;
        undoStack.push_back({ c, pos });
        c   = std::move(redoStack.back().circuit);
        pos = std::move(redoStack.back().layout);
        redoStack.pop_back();
        selection.clear();
        wireStart = {};
    }

    PinHit findPin(Vector2 mouse, const Circuit& c, const Layout& pos) {
        auto hit = [&](Vector2 pt, PinHit res) -> bool {
            if (!CheckCollisionPointCircle(mouse, pt, 10)) return false;
            res.pos = pt;
            return true;
        };

        for (int i = 0; i < (int)pos.gates.size(); ++i) {
            Vector2 p = pos.gates[i];
            if (c.gates[i].isSingleInput()) {
                if (CheckCollisionPointCircle(mouse, { p.x, p.y + 20 }, 10))
                    return { .pos = { p.x, p.y + 20 }, .idx = i };
            } else {
                if (CheckCollisionPointCircle(mouse, { p.x, p.y + 10 }, 10))
                    return { .pos = { p.x, p.y + 10 }, .idx = i };
                if (CheckCollisionPointCircle(mouse, { p.x, p.y + 30 }, 10))
                    return { .pos = { p.x, p.y + 30 }, .idx = i, .gatePin = true };
            }
            if (CheckCollisionPointCircle(mouse, { p.x + 60, p.y + 20 }, 10))
                return { .pos = { p.x + 60, p.y + 20 }, .idx = i, .isOut = true };
            }
        for (int i = 0; i < (int)pos.switches.size(); ++i) {
            Vector2 p = { pos.switches[i].x + 40, pos.switches[i].y + 20 };
            if (CheckCollisionPointCircle(mouse, p, 10))
                return { .pos = p, .idx = i, .type = SWITCH, .isOut = true };
        }
        for (int i = 0; i < (int)pos.leds.size(); ++i) {
            Vector2 p = { pos.leds[i].x, pos.leds[i].y + 20 };
            if (CheckCollisionPointCircle(mouse, p, 10))
                return { .pos = p, .idx = i, .type = LED };
        }
        return {};
    }

    bool isMouseOverWire(Vector2 mouse, Vector2 start, Vector2 end) {
        float offset = std::max(std::abs(end.x - start.x) * 0.5f, 40.0f);
        Vector2 c1 = { start.x + offset, start.y };
        Vector2 c2 = { end.x - offset, end.y };
        Vector2 prev = start;
        for (int s = 1; s <= 16; ++s) {
            float t = s / 16.0f, u = 1.0f - t;
            Vector2 curr = {
                u*u*u*start.x + 3*u*u*t*c1.x + 3*u*t*t*c2.x + t*t*t*end.x,
                u*u*u*start.y + 3*u*u*t*c1.y + 3*u*t*t*c2.y + t*t*t*end.y
            };
            if (CheckCollisionPointLine(mouse, prev, curr, 8)) return true;
            prev = curr;
        }
        return false;
    }

    void deleteNode(Circuit& c, Layout& pos, Kind type, int idx) {
        for (int i = (int)c.wires.size() - 1; i >= 0; --i) {
            auto& w = c.wires[i];
            if ((w.fromType == type && w.from == idx) || (w.toType == type && w.to == idx)) {
                c.wires.erase(c.wires.begin() + i);
            } else {
                if (w.fromType == type && w.from > idx) w.from--;
                if (w.toType == type && w.to > idx) w.to--;
            }
        }
        if (type == GATE)        { c.gates.erase(c.gates.begin() + idx);       pos.gates.erase(pos.gates.begin() + idx); }
        else if (type == SWITCH) { c.switches.erase(c.switches.begin() + idx); pos.switches.erase(pos.switches.begin() + idx); }
        else if (type == LED)    { c.leds.erase(c.leds.begin() + idx);         pos.leds.erase(pos.leds.begin() + idx); }
        wireStart = {};
    }

    bool hotkeysUpdate(Circuit& c, Layout& pos) {
        Vector2 screenMouse = GetMousePosition();
        Vector2 mouse = GetScreenToWorld2D(screenMouse, camera);
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        bool space = IsKeyDown(KEY_SPACE);

        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        
        
        if (ctrl && IsKeyPressed(KEY_Z)) {
            if (shift) redo(c, pos);
            else       undo(c, pos);
            return false;
        }

        if (ctrl && IsKeyPressed(KEY_Y)) {
            redo(c, pos);
            return false;
        }


        if (ctrl && IsKeyPressed(KEY_C) && !selection.empty()) {
            clipboard = {};
            Vector2 center{};
            int total = 0;

            for (int i : selection.gates)    { center = Vector2Add(center, pos.gates[i]);    total++; }
            for (int i : selection.switches) { center = Vector2Add(center, pos.switches[i]); total++; }
            for (int i : selection.leds)     { center = Vector2Add(center, pos.leds[i]);     total++; }
            center = Vector2Scale(center, 1.0f / total);


            for (int i : selection.gates) {
                clipboard.gates.push_back(Vector2Subtract(pos.gates[i], center));
                clipboard.gateTypes.push_back(c.gates[i].type);
            }
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
            pushHistory(c, pos);
            selection.clear();

            int baseGate   = (int)c.gates.size();
            int baseSwitch = (int)c.switches.size();
            int baseLed    = (int)c.leds.size();

            auto paste = [&](const auto& clipList, auto& selList, auto& cList, auto& posList) {
                for (const auto& rel : clipList) {
                    selList.push_back((int)cList.size());
                    cList.push_back({});
                    posList.push_back(Vector2Add(mouse, rel));
                }
            };
            
            paste(clipboard.gates, selection.gates, c.gates, pos.gates);
            for (size_t i = 0; i < clipboard.gateTypes.size(); ++i) {
                c.gates[baseGate + i].type = clipboard.gateTypes[i];
            }
            paste(clipboard.switches, selection.switches, c.switches, pos.switches);
            paste(clipboard.leds,     selection.leds,     c.leds,     pos.leds);

            auto getBase = [&](Kind type) {
                if (type == GATE)   return baseGate;
                if (type == SWITCH) return baseSwitch;
                return baseLed;
            };

            for (auto w : clipboard.wires) {
                w.from += getBase(w.fromType);
                w.to   += getBase(w.toType);
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
                Vector2 delta = Vector2Subtract(screenMouse, panLastMouse);
                camera.target = Vector2Subtract(camera.target, Vector2Scale(delta, 1.0f / camera.zoom));
                panLastMouse = screenMouse;
                SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                return false;
            }
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            isPanning = false;
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            Vector2 mouseWorldBefore = GetScreenToWorld2D(screenMouse, camera);

            camera.offset = screenMouse;
            camera.target = mouseWorldBefore;

            float scaleFactor = 1.0f + (0.15f * std::abs(wheel));
            if (wheel < 0.0f) scaleFactor = 1.0f / scaleFactor;

            camera.zoom = std::clamp(camera.zoom * scaleFactor, 0.2f, 4.0f);
        }


        auto spawnGate = [&](GateType t) {
            pushHistory(c, pos);
            c.gates.push_back({ .type = t });
            pos.gates.push_back({ mouse.x - 30, mouse.y - 20 });
        };

        if (!ctrl) {
            if (IsKeyPressed(KEY_Q))   spawnGate(GateType::AND);
            if (IsKeyPressed(KEY_W))   spawnGate(GateType::OR);
            if (IsKeyPressed(KEY_E))   spawnGate(GateType::XOR);
            if (IsKeyPressed(KEY_R))   spawnGate(GateType::NOT);
            if (IsKeyPressed(KEY_ONE)) { pushHistory(c, pos); c.switches.push_back({}); pos.switches.push_back({ mouse.x - 20, mouse.y - 20 }); }
            if (IsKeyPressed(KEY_TWO)) { pushHistory(c, pos); c.leds.push_back({});     pos.leds.push_back({ mouse.x - 20, mouse.y - 20 }); }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PinHit hit = findPin(mouse, c, pos);
            if (!wireStart && shift && hit) {
                wireStart = hit;
                return false;
            }
            if (wireStart) {
                if (hit && hit.isOut != wireStart.isOut &&
                    !(hit.type == wireStart.type && hit.idx == wireStart.idx)) {
                    pushHistory(c, pos);
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
                pushHistory(c, pos);
                isDragging = true;
                if (!selection.has(hitType, hitIdx)) {
                    selection.clear();
                    if (hitType == GATE)        selection.gates.push_back(hitIdx);
                    else if (hitType == SWITCH) selection.switches.push_back(hitIdx);
                    else if (hitType == LED)    selection.leds.push_back(hitIdx);
                    isSingleDrag = true;
                } else {
                    isSingleDrag = false;
                }
            } else {
                selection.clear();
                isSelecting = true;
                isSingleDrag = false;
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
            if (isDragging) {
                if (Vector2Distance(clickStartWorld, mouse) < 4.0f) {
                    if (!undoStack.empty()) undoStack.pop_back(); // убираем пустой шаг из истории, раз не двигали
                    if (selection.switches.size() == 1 && selection.gates.empty() && selection.leds.empty()) {
                        c.switches[selection.switches[0]].on = !c.switches[selection.switches[0]].on;
                    }
                }
                if (isSingleDrag) {
                    selection.clear();
                }
            }
            isDragging = false;
            isSelecting = false;
            isSingleDrag = false;
        }

        if (IsKeyPressed(KEY_X)) {
            if (!selection.empty()) {
            pushHistory(c, pos);

            std::sort(selection.gates.rbegin(), selection.gates.rend());
            std::sort(selection.switches.rbegin(), selection.switches.rend());
            std::sort(selection.leds.rbegin(), selection.leds.rend());

            for (int idx : selection.gates)    deleteNode(c, pos, GATE, idx);
            for (int idx : selection.switches) deleteNode(c, pos, SWITCH, idx);
            for (int idx : selection.leds)     deleteNode(c, pos, LED, idx);
            selection.clear();
            return false;
        }

        for (int i = (int)c.wires.size() - 1; i >= 0; --i) {
            const auto& w = c.wires[i];
            Vector2 start = (w.fromType == SWITCH)
                ? Vector2{ pos.switches[w.from].x + 40, pos.switches[w.from].y + 20 }
                : Vector2{ pos.gates[w.from].x + 60,    pos.gates[w.from].y + 20 };

            Vector2 end{};
            if (w.toType == LED) {
                end = { pos.leds[w.to].x, pos.leds[w.to].y + 20 };
            } else {
                float pinY = c.gates[w.to].isSingleInput() ? 20.0f : (w.pin ? 30.0f : 10.0f);
                end = { pos.gates[w.to].x, pos.gates[w.to].y + pinY };
            }

            if (isMouseOverWire(mouse, start, end)) {
                pushHistory(c, pos);
                c.wires.erase(c.wires.begin() + i);
                return false;
            }
        }
    }
    return false;
    }

    void draw(const Circuit& c, const Layout& pos) {
        if (isSelecting) {
            DrawRectangleRec(selectBox, Fade(SKYBLUE, 0.2f));
            DrawRectangleLinesEx(selectBox, 1.5f, SKYBLUE);
        }
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), camera);
        PinHit hover = findPin(m, c, pos);

        if (hover && (wireStart || IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
            DrawCircleLinesV(hover.pos, 7.0f, RAYWHITE);
        }

        if (!wireStart) return;

        bool canConnect = hover && hover.isOut != wireStart.isOut &&
                          !(hover.type == wireStart.type && hover.idx == wireStart.idx);

        Vector2 target = canConnect ? hover.pos : m;

        if (wireStart.isOut) DrawWire(wireStart.pos, target, true);
        else                    DrawWire(target, wireStart.pos, true);
    }
};

