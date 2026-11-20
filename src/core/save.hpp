#pragma once

#include <nlohmann/json.hpp>
#include <raylib.h>
#include <fstream>
#include <string>

// Подключите ваши файлы со структурами (укажите ваши точные пути, например):
#include "circuit.hpp"
#include "../render/layout.hpp"

using json = nlohmann::json;

// Только ПОСЛЕ всех #include пишем макросы:
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Gate, in, out)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Led, on)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Switch, on)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Wire, from, to, fromType, toType, pin, on)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Circuit, gates, leds, switches, wires)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Layout, gates, switches, leds)

// 3. Сохранение
inline bool saveFile(const Circuit& circuit, const Layout& layout, const std::string& path) {
    json data;
    data["circuit"] = circuit;
    data["layout"] = layout;

    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }

    out << data.dump(4);
    return true;
}

// 4. Загрузка
inline bool loadFile(Circuit& circuit, Layout& layout, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    json data;
    in >> data;

    circuit = data.at("circuit").get<Circuit>();
    layout = data.at("layout").get<Layout>();
    return true;
}

