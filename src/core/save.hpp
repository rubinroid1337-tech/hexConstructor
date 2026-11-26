#pragma once

#include <nlohmann/json.hpp>
#include <raylib.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "circuit.hpp"
#include "../render/layout.hpp"

namespace fs = std::filesystem;

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

inline bool saveFile(const Circuit& circuit, const Layout& layout, const fs::path& path) {
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    std::ofstream out(path);
    if (!out) {
        return false;
    }

    const nlohmann::json data = SaveData{circuit, layout};
    out << data.dump(4);
    return out.good();
}

inline bool loadFile(Circuit& circuit, Layout& layout, const fs::path& path) {
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

class SaveManager {
public:
    explicit SaveManager(fs::path directory = "saves", float autoSaveSec = 10.0f)
        : dir(std::move(directory)), autoSaveInterval(autoSaveSec) {
        refresh();
    }

    void refresh() {
        saves.clear();
        if (!fs::exists(dir)) {
            fs::create_directories(dir);
            return;
        }
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                saves.push_back(entry.path().stem().string());
            }
        }
        std::sort(saves.begin(), saves.end());
    }

    bool createProject(const std::string& name, Circuit& circuit, Layout& layout) {
        if (name.empty()) return false;
        circuit = Circuit{};
        layout = Layout{};
        currentProject = name;
        timer = 0.0f;
        bool ok = saveCurrent(circuit, layout);
        refresh();
        return ok;
    }

    bool openProject(const std::string& name, Circuit& circuit, Layout& layout) {
        if (!loadFile(circuit, layout, pathFor(name))) return false;
        currentProject = name;
        timer = 0.0f;
        return true;
    }

    bool saveCurrent(const Circuit& circuit, const Layout& layout) {
        if (currentProject.empty()) return false;
        timer = 0.0f;
        return saveFile(circuit, layout, pathFor(currentProject));
    }

    void deleteProject(const std::string& name) {
        fs::remove(pathFor(name));
        if (currentProject == name) {
            currentProject.clear();
        }
        refresh();
    }

    void updateAutoSave(float dt, const Circuit& circuit, const Layout& layout) {
        if (currentProject.empty()) return;
        timer += dt;
        if (timer >= autoSaveInterval) {
            saveCurrent(circuit, layout);
        }
    }

    void closeProject(const Circuit& circuit, const Layout& layout) {
        saveCurrent(circuit, layout);
        currentProject.clear();
        refresh();
    }

    const std::vector<std::string>& list() const { return saves; }
    const std::string& activeName() const { return currentProject; }

private:
    fs::path pathFor(const std::string& name) const {
        return dir / (name + ".json");
    }

    fs::path dir;
    std::vector<std::string> saves;
    std::string currentProject;
    float autoSaveInterval;
    float timer = 0.0f;
};

