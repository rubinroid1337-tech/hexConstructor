#pragma once
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "raymath.h"
#include "../core/circuit.hpp"
#include "../render/circuit-render.hpp"
#include "../core/kind.hpp"
#include "pin-hit.hpp"

struct Clipboard {
    std::vector<Vector2> gates;
    std::vector<Vector2> switches;
    std::vector<Vector2> leds;
    std::vector<Wire> wires;

    bool empty() const { return gates.empty() && switches.empty() && leds.empty(); }
};

struct Editor {
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

    PinHit wireStart{}; // Активный пин, от которого сейчас тянется провод

    PinHit findPin(Vector2 mouse, const Layout& pos) {
        for (int i = 0; i < (int)pos.gates.size(); ++i) {
            Vector2 p = pos.gates[i];
            if (CheckCollisionPointCircle(mouse, { p.x, p.y + 10 }, 10))
                return { true, false, GATE, i, false, { p.x, p.y + 10 } };
            if (CheckCollisionPointCircle(mouse, { p.x, p.y + 30 }, 10))
                return { true, false, GATE, i, true,  { p.x, p.y + 30 } };
            if (CheckCollisionPointCircle(mouse, { p.x + 60, p.y + 20 }, 10))
                return { true, true,  GATE, i, false, { p.x + 60, p.y + 20 } };
        }
        for (int i = 0; i < (int)pos.switches.size(); ++i) {
            Vector2 p = { pos.switches[i].x + 40, pos.switches[i].y + 20 };
            if (CheckCollisionPointCircle(mouse, p, 10))
                return { true, true, SWITCH, i, false, p };
        }
        for (int i = 0; i < (int)pos.leds.size(); ++i) {
            Vector2 p = { pos.leds[i].x, pos.leds[i].y + 20 };
            if (CheckCollisionPointCircle(mouse, p, 10))
                return { true, false, LED, i, false, p };
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
        wireStart.valid = false;
    }

    bool update(Circuit& c, Layout& pos) {
        Vector2 screenMouse = GetMousePosition();
        Vector2 mouse = GetScreenToWorld2D(screenMouse, camera);
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        bool space = IsKeyDown(KEY_SPACE);

        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

        // Копирование выделенной группы (Ctrl + C)
        if (ctrl && IsKeyPressed(KEY_C) && !selection.empty()) {
            clipboard = {};
            Vector2 center{};
            int total = 0;

            // Считаем центр масс выделенной группы, чтобы при вставке она появлялась ровно под курсором
            for (int i : selection.gates)    { center = Vector2Add(center, pos.gates[i]);    total++; }
            for (int i : selection.switches) { center = Vector2Add(center, pos.switches[i]); total++; }
            for (int i : selection.leds)     { center = Vector2Add(center, pos.leds[i]);     total++; }
            center = Vector2Scale(center, 1.0f / total);

            // Сохраняем относительные координаты элементов
            for (int i : selection.gates)    clipboard.gates.push_back(Vector2Subtract(pos.gates[i], center));
            for (int i : selection.switches) clipboard.switches.push_back(Vector2Subtract(pos.switches[i], center));
            for (int i : selection.leds)     clipboard.leds.push_back(Vector2Subtract(pos.leds[i], center));

            // Вспомогательная лямбда для поиска нового локального индекса элемента в буфере
            auto findLocalIdx = [&](Kind type, int oldIdx) -> int {
                const auto& list = (type == GATE) ? selection.gates : (type == SWITCH ? selection.switches : selection.leds);
                auto it = std::find(list.begin(), list.end(), oldIdx);
                return (it != list.end()) ? (int)std::distance(list.begin(), it) : -1;
            };

            // Копируем только внутренние провода (где оба конца попали в выделение)
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

            for (const auto& rel : clipboard.gates) {
                selection.gates.push_back((int)c.gates.size());
                c.gates.push_back({});
                pos.gates.push_back(Vector2Add(mouse, rel));
            }
            for (const auto& rel : clipboard.switches) {
                selection.switches.push_back((int)c.switches.size());
                c.switches.push_back({});
                pos.switches.push_back(Vector2Add(mouse, rel));
            }
            for (const auto& rel : clipboard.leds) {
                selection.leds.push_back((int)c.leds.size());
                c.leds.push_back({});
                pos.leds.push_back(Vector2Add(mouse, rel));
            }
            for (auto w : clipboard.wires) {
                w.from += (w.fromType == SWITCH) ? baseSwitch : baseGate;
                w.to   += (w.toType == LED)      ? baseLed    : baseGate;
                c.wires.push_back(w);
            }
        }

        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (wireStart.valid) { wireStart.valid = false; return false; }
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
            PinHit hit = findPin(mouse, pos);
            if (!wireStart.valid && shift && hit.valid) {
                wireStart = hit;
                return false;
            }
            if (wireStart.valid) {
                if (hit.valid && hit.isOutput != wireStart.isOutput &&
                    !(hit.type == wireStart.type && hit.index == wireStart.index)) {
                    PinHit outPin = wireStart.isOutput ? wireStart : hit;
                    PinHit inPin  = wireStart.isOutput ? hit : wireStart;
                    c.wires.push_back({ outPin.index, inPin.index, outPin.type, inPin.type, inPin.gatePin, false });
                }
                wireStart.valid = false;
                return false;
            }
        }

        if (!wireStart.valid && !space && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
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
                Vector2 end = (w.toType == LED)
                    ? Vector2{ pos.leds[w.to].x,  pos.leds[w.to].y + 20 }
                    : Vector2{ pos.gates[w.to].x, pos.gates[w.to].y + (w.pin ? 30.0f : 10.0f) };

                if (isMouseOverWire(mouse, start, end)) {
                    c.wires.erase(c.wires.begin() + i);
                    return false;
                }
            }
        }
        return false;
    }

    void draw(const Layout& pos) {
        if (isSelecting) {
            DrawRectangleRec(selectBox, Fade(SKYBLUE, 0.2f));
            DrawRectangleLinesEx(selectBox, 1.5f, SKYBLUE);
        }
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), camera);
        PinHit hover = findPin(m, pos);

        // Подсвечиваем пин под курсором, если зажат Shift или уже тянется провод
        if (hover.valid && (wireStart.valid || IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
            DrawCircleLinesV(hover.pos, 7.0f, RAYWHITE);
        }

        if (!wireStart.valid) return;

        // Проверяем, можно ли прикрепить провод к пину под курсором
        bool canConnect = hover.valid && hover.isOutput != wireStart.isOutput &&
                          !(hover.type == wireStart.type && hover.index == wireStart.index);

        // Если можно подключить — конец провода прилипает точно к пину
        Vector2 target = canConnect ? hover.pos : m;

        if (wireStart.isOutput) DrawWire(wireStart.pos, target, true);
        else                    DrawWire(target, wireStart.pos, true);
    }
};
