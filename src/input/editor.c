#include "editor.h"
#include "../render/draw-wire.h"
#include "utils.h"

void editor_init(Editor *ed) {
    *ed = (Editor){
        .camera = { .zoom = 1.0f },
        .wireStart = pinhit_empty(),
    };
}

void editor_free(Editor *ed) {
    clipboard_clear(&ed->clipboard);
    history_free(&ed->undoStack);
    history_free(&ed->redoStack);
}

void editor_push_history(Editor *ed, const Circuit *c, const Layout *pos) {
    history_push(&ed->undoStack, c, pos);
    history_clear(&ed->redoStack);
}

void editor_undo(Editor *ed, Circuit *c, Layout *pos) {
    Snapshot prev;
    if (!history_pop(&ed->undoStack, &prev)) return;
    history_push(&ed->redoStack, c, pos);
    *c = prev.circuit;
    *pos = prev.layout;
    selection_clear(&ed->selection);
    ed->wireStart = pinhit_empty();
}

void editor_redo(Editor *ed, Circuit *c, Layout *pos) {
    Snapshot next;
    if (!history_pop(&ed->redoStack, &next)) return;
    history_push(&ed->undoStack, c, pos);
    *c = next.circuit;
    *pos = next.layout;
    selection_clear(&ed->selection);
    ed->wireStart = pinhit_empty();
}

void editor_draw(const Editor *ed, const Circuit *c, const Layout *pos) {
    if (ed->isSelecting) {
        DrawRectangleRec(ed->selectBox, Fade(SKYBLUE, 0.2f));
        DrawRectangleLinesEx(ed->selectBox, 1.5f, SKYBLUE);
    }

    Vector2 m = GetScreenToWorld2D(GetMousePosition(), ed->camera);
    PinHit hover = findPin(m, c, pos);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    if (pinhit_valid(hover) && (pinhit_valid(ed->wireStart) || shift)) {
        DrawCircleLinesV(hover.pos, 7.0f, RAYWHITE);
    }

    if (!pinhit_valid(ed->wireStart)) return;

    bool canConnect = pinhit_valid(hover) &&
                      hover.isOut != ed->wireStart.isOut &&
                      !(hover.type == ed->wireStart.type && hover.idx == ed->wireStart.idx);

    Vector2 target = canConnect ? hover.pos : m;
    if (ed->wireStart.isOut) DrawWire(ed->wireStart.pos, target, true);
    else DrawWire(target, ed->wireStart.pos, true);
}

