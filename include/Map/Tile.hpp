#pragma once
#include "../Core/Common.hpp"

// ─── Tile ────────────────────────────────────────────────────
// One cell of the grid.
struct Tile {
    int row = 0, col = 0;

    bool walkable   = true;   // impassable terrain (future use)
    bool hasTrap    = false;
    int  trapDamage = 0;
    bool trapByPlayer = false;

    TileHighlight highlight = TileHighlight::NONE;

    // Computed draw rect (screen coordinates)
    SDL_Rect rect() const;
};