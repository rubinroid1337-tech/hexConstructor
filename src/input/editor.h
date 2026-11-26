#pragma once
#include <raylib.h>
#include "../core/circuit.h"
#include "../render/layout.h"
#include "../render/selection.h"
#include "pin-hit.h"
#include "clipboard.h"
#include "snapshot.h"

typedef struct {
    Camera2D camera;
    Selection selection;
    Clipboard clipboard;

    bool isSingleDrag;
    bool isPanning;
    Vector2 panLastMouse;

    bool isSelecting;
    Vector2 selectStart;
    Rectangle selectBox;

    bool isDragging;
    Vector2 dragLastWorld;
    Vector2 clickStartWorld;

    PinHit wireStart;

    HistoryStack undoStack;
    HistoryStack redoStack;
} Editor;

void editor_init(Editor *ed);
void editor_free(Editor *ed);

void editor_push_history(Editor *ed, const Circuit *c, const Layout *pos);
void editor_undo(Editor *ed, Circuit *c, Layout *pos);
void editor_redo(Editor *ed, Circuit *c, Layout *pos);

void editor_draw(const Editor *ed, const Circuit *c, const Layout *pos);

