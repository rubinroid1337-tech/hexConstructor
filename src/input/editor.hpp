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
        if (undoStack.size() >= 100) undoStack.erase(undoStack.begin());
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
        PinHit res{};
        auto hit = [&](Vector2 pt, PinHit cand) {
            if (!CheckCollisionPointCircle(mouse, pt, 10)) return false;
            res = cand;
            res.pos = pt;
            return true;
        };

        for (int i = 0; i < (int)pos.gates.size(); ++i) {
            auto [x, y] = pos.gates[i];
            if (hit({x + 60, y + 20}, {.idx = i, .isOut = true})) return res;
            if (c.gates[i].isSingleInput()) {
                if (hit({x, y + 20}, {.idx = i})) return res;
            } else {
                if (hit({x, y + 10}, {.idx = i})) return res;
                if (hit({x, y + 30}, {.idx = i, .gatePin = true})) return res;
            }
        }

        for (int i = 0; i < (int)pos.switches.size(); ++i)
            if (hit({pos.switches[i].x + 40, pos.switches[i].y + 20}, {.idx = i, .type = SWITCH, .isOut = true})) return res;

        for (int i = 0; i < (int)pos.leds.size(); ++i)
            if (hit({pos.leds[i].x, pos.leds[i].y + 20}, {.idx = i, .type = LED})) return res;

        return {};
    }

    bool isMouseOverWire(Vector2 mouse, Vector2 start, Vector2 end) {
        float off = fmaxf(fabsf(end.x - start.x) * 0.5f, 40.0f);
        Vector2 c1 = { start.x + off, start.y }, c2 = { end.x - off, end.y }, prev = start;
        for (int s = 1; s <= 16; ++s) {
            Vector2 curr = GetSplinePointBezierCubic(start, end, c1, c2, s / 16.0f);
            if (CheckCollisionPointLine(mouse, prev, curr, 8)) return true;
            prev = curr;
        }
        return false;
    }

    void deleteNode(Circuit& c, Layout& pos, Kind type, int idx) {
        auto upd = [&](Kind k, int& id) {
            if (k == type && id > idx) id--;
            return k == type && id == idx;
        };
        for (int i = (int)c.wires.size() - 1; i >= 0; --i)
            if (upd(c.wires[i].fromType, c.wires[i].from) || upd(c.wires[i].toType, c.wires[i].to))
                c.wires.erase(c.wires.begin() + i);

        auto rem = [&](auto& a, auto& b) { a.erase(a.begin() + idx); b.erase(b.begin() + idx); };
        if (type == GATE)        rem(c.gates, pos.gates);
        else if (type == SWITCH) rem(c.switches, pos.switches);
        else if (type == LED)    rem(c.leds, pos.leds);
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
            auto sum = [&](const auto& sel, const auto& pts) {
                for (int i : sel) center = Vector2Add(center, pts[i]);
            };
            sum(selection.gates, pos.gates);
            sum(selection.switches, pos.switches);
            sum(selection.leds, pos.leds);
            center = Vector2Scale(center, 1.0f / (selection.gates.size() + selection.switches.size() + selection.leds.size()));

            for (int i : selection.gates) {
                clipboard.gates.push_back(Vector2Subtract(pos.gates[i], center));
                clipboard.gateTypes.push_back(c.gates[i].type);
            }
            for (int i : selection.switches) clipboard.switches.push_back(Vector2Subtract(pos.switches[i], center));
            for (int i : selection.leds)     clipboard.leds.push_back(Vector2Subtract(pos.leds[i], center));

            auto localIdx = [&](Kind k, int id) {
                const auto& v = (k == GATE) ? selection.gates : (k == SWITCH ? selection.switches : selection.leds);
                for (int i = 0; i < (int)v.size(); ++i) if (v[i] == id) return i;
                return -1;
            };
            for (const auto& w : c.wires)
                if (int a = localIdx(w.fromType, w.from), b = localIdx(w.toType, w.to); a != -1 && b != -1)
                    clipboard.wires.push_back({ a, b, w.fromType, w.toType, w.pin, false });
        }

        if (ctrl && IsKeyPressed(KEY_V) && !clipboard.empty()) {
            pushHistory(c, pos);
            selection.clear();

            int base[3] = { (int)c.gates.size(), (int)c.switches.size(), (int)c.leds.size() };
            auto paste = [&](const auto& src, auto& sel, auto& dst, auto& pts) {
                for (Vector2 rel : src) {
                    sel.push_back((int)dst.size());
                    dst.push_back({});
                    pts.push_back(Vector2Add(mouse, rel));
                }
            };
            paste(clipboard.gates,    selection.gates,    c.gates,    pos.gates);
            paste(clipboard.switches, selection.switches, c.switches, pos.switches);
            paste(clipboard.leds,     selection.leds,     c.leds,     pos.leds);
            for (size_t i = 0; i < clipboard.gateTypes.size(); ++i)
                c.gates[base[GATE] + i].type = clipboard.gateTypes[i];

            for (auto w : clipboard.wires)
                c.wires.push_back({ w.from + base[w.fromType], w.to + base[w.toType], w.fromType, w.toType, w.pin, false });
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

        if (IsKeyPressed(KEY_Q))   spawnGate(GateType::AND);
        if (IsKeyPressed(KEY_W))   spawnGate(GateType::OR);
        if (IsKeyPressed(KEY_E))   spawnGate(GateType::XOR);
        if (IsKeyPressed(KEY_R))   spawnGate(GateType::NOT);
        if (IsKeyPressed(KEY_ONE)) { pushHistory(c, pos); c.switches.push_back({}); pos.switches.push_back({ mouse.x - 20, mouse.y - 20 }); }
        if (IsKeyPressed(KEY_TWO)) { pushHistory(c, pos); c.leds.push_back({});     pos.leds.push_back({ mouse.x - 20, mouse.y - 20 }); }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (PinHit hit = findPin(mouse, c, pos); !wireStart && shift && hit) {
                wireStart = hit;
                return false;
            } else if (wireStart) {
                if (hit && hit.isOut != wireStart.isOut && !(hit.type == wireStart.type && hit.idx == wireStart.idx)) {
                    pushHistory(c, pos);
                    PinHit outPin = wireStart.isOut ? wireStart : hit, inPin = wireStart.isOut ? hit : wireStart;
                    c.wires.push_back({ outPin.idx, inPin.idx, outPin.type, inPin.type, inPin.gatePin, false });
                }
                wireStart = {};
                return false;
            }

            if (!space) {
                clickStartWorld = dragLastWorld = mouse;
                Kind hitType = GATE;
                int hitIdx = -1;
                auto checkHit = [&](const auto& list, Kind type, float w, float h) {
                    for (int i = (int)list.size() - 1; i >= 0; --i)
                        if (CheckCollisionPointRec(mouse, { list[i].x, list[i].y, w, h }))
                            return hitType = type, hitIdx = i, true;
                    return false;
                };

                if (checkHit(pos.gates, GATE, 60, 40) || checkHit(pos.switches, SWITCH, 40, 40) || checkHit(pos.leds, LED, 40, 40)) {
                    pushHistory(c, pos);
                    isDragging = true;
                    if (isSingleDrag = !selection.has(hitType, hitIdx); isSingleDrag) {
                        selection.clear();
                        (hitType == GATE ? selection.gates : (hitType == SWITCH ? selection.switches : selection.leds)).push_back(hitIdx);
                    }
                } else {
                    selection.clear();
                    isSelecting = true;
                    isSingleDrag = false;
                    selectStart = mouse;
                    selectBox = { mouse.x, mouse.y, 0, 0 };
                }
            }
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (isDragging) {
                Vector2 delta = Vector2Subtract(mouse, dragLastWorld);
                auto move = [&](const auto& sel, auto& pts) { for (int i : sel) pts[i] = Vector2Add(pts[i], delta); };
                move(selection.gates, pos.gates);
                move(selection.switches, pos.switches);
                move(selection.leds, pos.leds);
                dragLastWorld = mouse;
            } else if (isSelecting) {
                selectBox = { fminf(selectStart.x, mouse.x), fminf(selectStart.y, mouse.y), fabsf(mouse.x - selectStart.x), fabsf(mouse.y - selectStart.y) };
                selection.clear();
                auto boxSel = [&](const auto& pts, auto& sel, float w, float h) {
                    for (int i = 0; i < (int)pts.size(); ++i)
                        if (CheckCollisionRecs(selectBox, { pts[i].x, pts[i].y, w, h })) sel.push_back(i);
                };
                boxSel(pos.gates, selection.gates, 60, 40);
                boxSel(pos.switches, selection.switches, 40, 40);
                boxSel(pos.leds, selection.leds, 40, 40);
            }
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

