#pragma once
#include <raylib.h>
#include "../core/circuit.h"
#include "../render/layout.h"
#include "pin-hit.h"

PinHit findPin(Vector2 mouse, const Circuit *c, const Layout *pos);
bool isMouseOverWire(Vector2 mouse, Vector2 start, Vector2 end);
void deleteNode(Circuit *c, Layout *pos, Kind type, int idx);

