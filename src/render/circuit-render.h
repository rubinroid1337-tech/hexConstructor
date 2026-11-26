#pragma once
#include <raylib.h>
#include "../core/circuit.h"
#include "../core/kind.h"
#include "layout.h"
#include "selection.h"
#include "draw-wire.h"

void DrawCircuit(const Circuit *c, const Layout *pos, const Selection *sel);

