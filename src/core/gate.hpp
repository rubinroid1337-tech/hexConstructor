#pragma once

enum class GateType {
    NAND,
    AND,
    OR,
    XOR,
    NOT
};

struct Gate {
    GateType type = GateType::NAND;
    bool in[2]{ false, false };
    bool out = false;

    bool isSingleInput() const {
        return type == GateType::NOT;
    }

    void evaluate() {
        switch (type) {
            case GateType::NAND: out = !(in[0] && in[1]); break;
            case GateType::AND:  out =  (in[0] && in[1]); break;
            case GateType::OR:   out =  (in[0] || in[1]); break;
            case GateType::XOR:  out =  (in[0] != in[1]); break;
            case GateType::NOT:  out = !in[0];            break;
        }
    }

    const char* label() const {
        switch (type) {
            case GateType::NAND: return "NAND";
            case GateType::AND:  return "AND";
            case GateType::OR:   return "OR";
            case GateType::XOR:  return "XOR";
            case GateType::NOT:  return "NOT";
        }
        return "?";
    }
};

