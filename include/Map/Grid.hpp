#pragma once
#include "Tile.hpp"
#include "../Entities/Character.hpp"
#include <vector>
#include <functional>

// ─── Grid ────────────────────────────────────────────────────
// Manages the 2-D tile array and all spatial queries.
class Grid {
public:
    Grid();

    void reset();   // clear highlights + traps

    // ── Highlight API ────────────────────────────────────────
    void clearHighlights();
    void highlightMoveRange(const Character& ch, int maxDist);
    void highlightSkillRange(const Character& user, const Skill& skill,
                             const std::vector<Character*>& allChars);

    // ── Tile access ──────────────────────────────────────────
    Tile& at(int r, int c)             { return m_tiles[r][c]; }
    const Tile& at(int r, int c) const { return m_tiles[r][c]; }
    bool inBounds(int r, int c) const;

    // ── Trap management ──────────────────────────────────────
    void placeTrap(int r, int c, int dmg, bool byPlayer);
    bool checkTrap(int r, int c, bool isPlayer, int& outDmg);  // returns true if triggered
    void removeTrap(int r, int c);

    // ── Spatial queries ──────────────────────────────────────
    // Returns all tile positions inside Euclidean radius
    std::vector<Vec2i> tilesInRadius(int r, int c, float radius) const;

    // Is a tile highlighted for a given type?
    bool isHighlighted(int r, int c, TileHighlight type) const;

    // Returns screen pixel (x,y) top-left of tile
    static SDL_Point tileToScreen(int r, int c);

    // Converts screen pixel to tile (returns false if outside map area)
    static bool screenToTile(int px, int py, int& outR, int& outC);

    // ── Rendering ────────────────────────────────────────────
    void render(SDL_Renderer* renderer) const;

private:
    Tile m_tiles[GRID_ROWS][GRID_COLS];
};