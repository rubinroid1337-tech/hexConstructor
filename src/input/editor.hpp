#pragma once
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "raymath.h"
#include "../core/circuit.hpp"
#include "../render/circuit-render.hpp"

struct PinHit {
    bool valid = false;
    bool isOutput = false; // true = выход (out), false = вход (in)
    NodeType type = GATE;
    int index = -1;
    bool gatePin = false;  // false = верхний вход (A), true = нижний (B)
    Vector2 pos{};
};

struct Editor {
    NodeType dragType = GATE;
    int dragIndex = -1;
    Vector2 dragOffset{};
    Vector2 clickStart{};

    PinHit wireStart{}; // Активный пин, от которого сейчас тянется провод

    // Поиск пина под курсором (радиус захвата 10 пикселей для удобства)
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

    void deleteNode(Circuit& c, Layout& pos, NodeType type, int idx) {
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
        dragIndex = -1;
        wireStart.valid = false;
    }

    // Возвращает true, если нажали ESC для закрытия окна (когда провод не тянется)
    bool update(Circuit& c, Layout& pos) {
        Vector2 mouse = GetMousePosition();
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        // 1. Обработка ESC: отменяем тянущийся провод или разрешаем выход из программы
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (wireStart.valid) {
                wireStart.valid = false;
                return false;
            }
            return true;
        }

        // 2. Спавн новых элементов по цифрам 1, 2, 3 прямо под курсором
        if (IsKeyPressed(KEY_ONE)) {
            c.gates.push_back({});
            pos.gates.push_back({ mouse.x - 30, mouse.y - 20 });
        }
        if (IsKeyPressed(KEY_TWO)) {
            c.switches.push_back({});
            pos.switches.push_back({ mouse.x - 20, mouse.y - 20 });
        }
        if (IsKeyPressed(KEY_THREE)) {
            c.leds.push_back({});
            pos.leds.push_back({ mouse.x - 20, mouse.y - 20 });
        }

        // 3. Создание провода: Shift + клик (начало) или просто клик по второму пину (завершение)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PinHit hit = findPin(mouse, pos);
            if (!wireStart.valid && shift && hit.valid) {
                wireStart = hit;
                return false;
            }
            if (wireStart.valid) {
                // Соединяем, только если один пин — выход, а второй — вход (и это не один и тот же элемент)
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

        // 4. Обычный Drag-and-Drop элементов (если не тянем провод)
        if (!wireStart.valid && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            clickStart = mouse;
            auto tryGrab = [&](std::vector<Vector2>& list, NodeType type, float w, float h) {
                for (int i = (int)list.size() - 1; i >= 0; --i) {
                    if (CheckCollisionPointRec(mouse, { list[i].x, list[i].y, w, h })) {
                        dragType = type;
                        dragIndex = i;
                        dragOffset = { mouse.x - list[i].x, mouse.y - list[i].y };
                        return true;
                    }
                }
                return false;
            };
            if (!tryGrab(pos.gates, GATE, 60, 40))
                if (!tryGrab(pos.switches, SWITCH, 40, 40))
                    tryGrab(pos.leds, LED, 40, 40);
        }

        if (dragIndex != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 nextPos = { mouse.x - dragOffset.x, mouse.y - dragOffset.y };
            if (dragType == GATE)        pos.gates[dragIndex] = nextPos;
            else if (dragType == SWITCH) pos.switches[dragIndex] = nextPos;
            else if (dragType == LED)    pos.leds[dragIndex] = nextPos;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (dragIndex != -1 && dragType == SWITCH && Vector2Distance(clickStart, mouse) < 4.0f) {
                c.switches[dragIndex].on = !c.switches[dragIndex].on;
            }
            dragIndex = -1;
        }

        // 5. Удаление элемента или провода по клавише DEL
        if (IsKeyPressed(KEY_DELETE)) {
            for (int i = (int)pos.gates.size() - 1; i >= 0; --i)
                if (CheckCollisionPointRec(mouse, { pos.gates[i].x, pos.gates[i].y, 60, 40 }))
                    return deleteNode(c, pos, GATE, i), false;

            for (int i = (int)pos.switches.size() - 1; i >= 0; --i)
                if (CheckCollisionPointRec(mouse, { pos.switches[i].x, pos.switches[i].y, 40, 40 }))
                    return deleteNode(c, pos, SWITCH, i), false;

            for (int i = (int)pos.leds.size() - 1; i >= 0; --i)
                if (CheckCollisionPointRec(mouse, { pos.leds[i].x, pos.leds[i].y, 40, 40 }))
                    return deleteNode(c, pos, LED, i), false;

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
        Vector2 m = GetMousePosition();
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
