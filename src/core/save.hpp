#pragma once

#include <nlohmann/json.hpp>
#include <raylib.h>
#include <fstream>
#include <string>

#include "circuit.hpp"
#include "../render/layout.hpp"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2, x, y)
NLOHMANN_JSON_SERIALIZE_ENUM(GateType, {
    {GateType::NAND, "NAND"},
    {GateType::AND,  "AND"},
    {GateType::OR,   "OR"},
    {GateType::XOR,  "XOR"},
    {GateType::NOT,  "NOT"},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Gate, type, in, out)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Led, on)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Switch, on)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Wire, from, to, fromType, toType, pin, on)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Circuit, gates, leds, switches, wires)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Layout, gates, switches, leds)

struct SaveData {
    Circuit circuit;
    Layout layout;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, circuit, layout)

inline bool saveFile(const Circuit& circuit, const Layout& layout, const std::filesystem::path& path) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }

    const nlohmann::json data = SaveData{circuit, layout};
    out << data.dump(4);
    return out.good();
}

inline bool loadFile(Circuit& circuit, Layout& layout, const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    try {
        nlohmann::json data;
        in >> data;

        auto loaded = data.get<SaveData>();
        circuit = std::move(loaded.circuit);
        layout = std::move(loaded.layout);
        return true;
    } catch (const nlohmann::json::exception&) {
        return false;
    }
}
