#include <raylib.h>
#include <raymath.h>

#include "gate.hpp"
#include "gate-system.hpp"

#include <iostream>
#include <cassert>
using namespace std;

int main() {
    GateSystem sys;

    int g1 = sys.addGate();
    int g2 = sys.addGate();

    sys.gates[g1].inputA = 1;
    sys.gates[g1].inputB = 1;
    sys.updateAllGates();
    assert(sys.gates[g1].output == 0);

    sys.addWire(g1, g2, 0);

    sys.gates[g2].inputB = 0;
    sys.updateAllGates();
    assert(sys.gates[g2].output == 1);
}
