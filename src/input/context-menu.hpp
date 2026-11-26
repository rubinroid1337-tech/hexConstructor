#pragma once

#include <raylib.h>
#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "button.hpp"

class ContextMenu {
public:
    void addButton(std::string name, std::function<void()> onClick) {
        buttons.push_back(Button{
            .name = std::move(name),
            .bounds = { 0.0f, 0.0f, ITEM_WIDTH, ITEM_HEIGHT },
            .onClick = std::move(onClick)
        });
    }

    // Запоминает, после какой по счёту кнопки нужно нарисовать разделитель
    void addSeparator() {
        if (!buttons.empty()) {
            separatorIndices.push_back(buttons.size());
        }
    }

    void clear() {
        buttons.clear();
        separatorIndices.clear();
    }

    void open(Vector2 pos) {
        if (buttons.empty()) return;

        float totalHeight = static_cast<float>(buttons.size()) * ITEM_HEIGHT
                          + static_cast<float>(separatorIndices.size()) * SEPARATOR_HEIGHT;

        float screenW = static_cast<float>(GetScreenWidth());
        float screenH = static_cast<float>(GetScreenHeight());

        pos.x = std::clamp(pos.x, 0.0f, std::max(0.0f, screenW - ITEM_WIDTH));
        pos.y = std::clamp(pos.y, 0.0f, std::max(0.0f, screenH - totalHeight));

        position = pos;
        visible = true;
        separatorYCoords.clear();

        float currentY = pos.y;
        for (size_t i = 0; i < buttons.size(); ++i) {
            // Если перед этой кнопкой должен быть разделитель — добавляем отступ
            if (std::find(separatorIndices.begin(), separatorIndices.end(), i) != separatorIndices.end()) {
                separatorYCoords.push_back(currentY + SEPARATOR_HEIGHT * 0.5f);
                currentY += SEPARATOR_HEIGHT;
            }

            buttons[i].bounds = {
                pos.x,
                currentY,
                ITEM_WIDTH,
                ITEM_HEIGHT
            };
            currentY += ITEM_HEIGHT;
        }
    }

    void close() {
        visible = false;
    }

    void update() {
        if (!visible) return;

        if (IsKeyPressed(KEY_ESCAPE)) {
            close();
            return;
        }

        for (auto& btn : buttons) {
            btn.update();
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            close();
        }
    }

    void draw() const {
        if (!visible) return;

        float totalHeight = static_cast<float>(buttons.size()) * ITEM_HEIGHT
                          + static_cast<float>(separatorIndices.size()) * SEPARATOR_HEIGHT;

        // Общий фон меню (чтобы зазор разделителя не просвечивал)
        Rectangle bg = { position.x, position.y, ITEM_WIDTH, totalHeight };
        DrawRectangleRec(bg, Color{ 45, 48, 56, 255 });

        for (const auto& btn : buttons) {
            btn.draw();
        }

        // Рисуем тонкие горизонтальные линии разделителей
        for (float sepY : separatorYCoords) {
            DrawLineEx(
                { position.x + 10.0f, sepY },
                { position.x + ITEM_WIDTH - 10.0f, sepY },
                1.0f,
                Color{ 85, 92, 108, 255 }
            );
        }

        DrawRectangleLinesEx(bg, 1.0f, Color{ 90, 98, 115, 255 });
    }

    bool isVisible() const { return visible; }
    Vector2 getOpenPosition() const { return position; }

private:
    static constexpr float ITEM_WIDTH       = 180.0f;
    static constexpr float ITEM_HEIGHT      = 32.0f;
    static constexpr float SEPARATOR_HEIGHT = 8.0f;

    std::vector<Button> buttons;
    std::vector<size_t> separatorIndices;
    std::vector<float> separatorYCoords;
    Vector2 position{};
    bool visible = false;
};

