#include "../../include/Map/Grid.hpp"
#include <cmath>

Grid::Grid() {
    reset();
}

void Grid::reset() {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            m_tiles[r][c].row       = r;
            m_tiles[r][c].col       = c;
            m_tiles[r][c].walkable  = true;
            m_tiles[r][c].hasTrap   = false;
            m_tiles[r][c].trapDamage= 0;
            m_tiles[r][c].trapByPlayer = false;
            m_tiles[r][c].highlight = TileHighlight::NONE;
        }
}

void Grid::clearHighlights() {
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++)
            m_tiles[r][c].highlight = TileHighlight::NONE;
}

bool Grid::inBounds(int r, int c) const {
    return r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS;
}

// ── Move range ────────────────────────────────────────────────
void Grid::highlightMoveRange(const Character& ch, int maxDist) {
    clearHighlights();
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            if (r == ch.row && c == ch.col) continue;
            int d = std::max(std::abs(r - ch.row), std::abs(c - ch.col)); // Chebyshev
            if (d <= maxDist)
                m_tiles[r][c].highlight = TileHighlight::MOVE;
        }
}

// ── Skill range ───────────────────────────────────────────────
void Grid::highlightSkillRange(const Character& user,
                               const Skill& skill,
                               const std::vector<Character*>& allChars) {
    clearHighlights();
    float range = skill.range;

    // Self-target only
    if (range <= 0.0f) {
        m_tiles[user.row][user.col].highlight = TileHighlight::SKILL;
        return;
    }

    auto charAt = [&](int r, int c) -> const Character* {
        for (auto* ch : allChars)
            if (ch->alive && ch->row == r && ch->col == c) return ch;
        return nullptr;
    };

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (r == user.row && c == user.col) continue;
            float d = dist(user.row, user.col, r, c);
            if (d > range) continue;

            const Character* occupant = charAt(r, c);

            if (skill.isTrap) {
                if (!occupant) m_tiles[r][c].highlight = TileHighlight::SKILL;
                continue;
            }
            if (skill.type == SkillType::HEAL || skill.type == SkillType::BUFF) {
                // Buff/heal: target allies OR empty tiles for AOE buffs
                if (occupant && occupant->isPlayer == user.isPlayer && occupant->alive)
                    m_tiles[r][c].highlight = TileHighlight::SKILL;
                else if (!occupant && skill.aoe)
                    m_tiles[r][c].highlight = TileHighlight::SKILL;
            } else if (skill.type == SkillType::TELEPORT) {
                if (occupant && occupant->isPlayer != user.isPlayer)
                    m_tiles[r][c].highlight = TileHighlight::ATTACK;
            } else {
                // Damage: highlight enemy occupants; also empty for AOE
                if (occupant && occupant->isPlayer != user.isPlayer)
                    m_tiles[r][c].highlight = TileHighlight::ATTACK;
                else if (!occupant && skill.aoe)
                    m_tiles[r][c].highlight = TileHighlight::SKILL;
            }
        }
    }
}

// ── Traps ─────────────────────────────────────────────────────
void Grid::placeTrap(int r, int c, int dmg, bool byPlayer) {
    if (!inBounds(r, c)) return;
    m_tiles[r][c].hasTrap      = true;
    m_tiles[r][c].trapDamage   = dmg;
    m_tiles[r][c].trapByPlayer = byPlayer;
}

bool Grid::checkTrap(int r, int c, bool isPlayer, int& outDmg) {
    if (!inBounds(r, c)) return false;
    Tile& t = m_tiles[r][c];
    if (!t.hasTrap) return false;
    // Trap triggers when the OPPOSITE team steps on it
    if (t.trapByPlayer == isPlayer) return false;
    outDmg = t.trapDamage;
    t.hasTrap = false;
    return true;
}

void Grid::removeTrap(int r, int c) {
    if (inBounds(r, c)) m_tiles[r][c].hasTrap = false;
}

// ── Spatial ───────────────────────────────────────────────────
std::vector<Vec2i> Grid::tilesInRadius(int r, int c, float radius) const {
    std::vector<Vec2i> result;
    int ir = static_cast<int>(std::ceil(radius));
    for (int dr = -ir; dr <= ir; dr++)
        for (int dc = -ir; dc <= ir; dc++) {
            int nr = r + dr, nc = c + dc;
            if (!inBounds(nr, nc)) continue;
            if (dist(r, c, nr, nc) <= radius)
                result.push_back({nr, nc});
        }
    return result;
}

bool Grid::isHighlighted(int r, int c, TileHighlight type) const {
    if (!inBounds(r, c)) return false;
    return m_tiles[r][c].highlight == type;
}

SDL_Point Grid::tileToScreen(int r, int c) {
    return { MAP_OFFSET_X + c * TILE_SIZE, MAP_OFFSET_Y + r * TILE_SIZE };
}

bool Grid::screenToTile(int px, int py, int& outR, int& outC) {
    int relX = px - MAP_OFFSET_X;
    int relY = py - MAP_OFFSET_Y;
    if (relX < 0 || relY < 0) return false;
    outC = relX / TILE_SIZE;
    outR = relY / TILE_SIZE;
    return outC < GRID_COLS && outR < GRID_ROWS;
}

// ── Rendering ─────────────────────────────────────────────────
void Grid::render(SDL_Renderer* renderer) const {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            const Tile& tile = m_tiles[r][c];
            int x = MAP_OFFSET_X + c * TILE_SIZE;
            int y = MAP_OFFSET_Y + r * TILE_SIZE;

            // Checkerboard base
            SDL_Color base = ((r + c) % 2 == 0)
                ? SDL_Color{14, 18, 32, 255}
                : SDL_Color{12, 16, 24, 255};
            SDL_SetRenderDrawColor(renderer, base.r, base.g, base.b, base.a);
            SDL_FRect cell = {(float)x,(float)y,(float)TILE_SIZE,(float)TILE_SIZE};
            SDL_RenderFillRect(renderer, &cell);

            // Highlight overlay
            if (tile.highlight == TileHighlight::MOVE) {
                SDL_SetRenderDrawColor(renderer, 58, 123, 213, 70);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, 58, 123, 213, 180);
                SDL_FRect border = {(float)x+1,(float)y+1,(float)TILE_SIZE-2,(float)TILE_SIZE-2};
                SDL_RenderRect(renderer, &border);
            } else if (tile.highlight == TileHighlight::ATTACK) {
                SDL_SetRenderDrawColor(renderer, 213, 58, 58, 70);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, 213, 58, 58, 180);
                SDL_FRect border = {(float)x+1,(float)y+1,(float)TILE_SIZE-2,(float)TILE_SIZE-2};
                SDL_RenderRect(renderer, &border);
            } else if (tile.highlight == TileHighlight::SKILL) {
                SDL_SetRenderDrawColor(renderer, 200, 168, 75, 55);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, 200, 168, 75, 150);
                SDL_FRect border = {(float)x+1,(float)y+1,(float)TILE_SIZE-2,(float)TILE_SIZE-2};
                SDL_RenderRect(renderer, &border);
            }

            // Trap indicator
            if (tile.hasTrap) {
                SDL_Color tc = tile.trapByPlayer ? SDL_Color{128,192,80,200} : SDL_Color{213,58,58,200};
                SDL_SetRenderDrawColor(renderer, tc.r, tc.g, tc.b, tc.a);
                SDL_FRect trapMark = {(float)x+5,(float)y+5,(float)TILE_SIZE-10,(float)TILE_SIZE-10};
                SDL_RenderRect(renderer, &trapMark);
            }

            // Grid lines
            SDL_SetRenderDrawColor(renderer, 30, 42, 74, 80);
            SDL_RenderRect(renderer, &cell);
        }
    }
}