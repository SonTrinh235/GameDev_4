#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <SDL3/SDL.h>

// ─── Window / Grid constants ────────────────────────────────
constexpr int SCREEN_W        = 1440;      // Độ rộng của cửa sổ
constexpr int SCREEN_H        = 900;       // Độ cao của cửa sổ
constexpr int GRID_COLS       = 16;        // Số cột map 
constexpr int GRID_ROWS       = 16;        // Số hàng map
constexpr int TILE_SIZE       = 48;        // Phóng to ô cờ 
constexpr int MAP_OFFSET_X    = 336;       // Mở rộng Panel bên trái 
constexpr int MAP_OFFSET_Y    = 52;        // Mở rộng Top HUD 
constexpr int PANEL_RIGHT_W   = 336;       // Mở rộng Panel bên phải 
constexpr int LOG_HEIGHT      = 80;        // Mở rộng khung Log dưới cùng 

// ─── Colour palette ─────────────────────────────────────────
struct Colour {
    static SDL_Color bg()        { return {10,  12,  20,  255}; }
    static SDL_Color panel()     { return {15,  18,  32,  255}; }
    static SDL_Color card()      { return {21,  26,  46,  255}; }
    static SDL_Color border()    { return {30,  42,  74,  255}; }
    static SDL_Color gold()      { return {200, 168, 75,  255}; }
    static SDL_Color blue()      { return {58,  123, 213, 255}; }
    static SDL_Color red()       { return {213, 58,  58,  255}; }
    static SDL_Color green()     { return {58,  181, 120, 255}; }
    static SDL_Color purple()    { return {139, 58,  213, 255}; }
    static SDL_Color hp()        { return {224, 80,  80,  255}; }
    static SDL_Color ap()        { return {245, 200, 66,  255}; }
    static SDL_Color white()     { return {200, 207, 224, 255}; }
    static SDL_Color dim()       { return {106, 117, 144, 255}; }
};

// ─── Enumerations ────────────────────────────────────────────
enum class GameState {
    MAIN_MENU,
    TEAM_BUILDER,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum class ClassType {
    WARRIOR, MAGE, ASSASSIN, MEDIC, PALADIN, ARCHER, NONE
};

enum class SkillType {
    PHYSICAL, MAGIC, HEAL, BUFF, DEBUFF, TELEPORT, TRAP, MIXED
};

enum class AIMode {
    RANDOM, MINIMAX, MCTS
};

enum class TileHighlight {
    NONE, MOVE, ATTACK, SKILL
};

enum class EffectType {
    POISON, FREEZE, SLOW, ARMOR_UP, HASTE, SHIELD, DIV_SHIELD, TAUNTED
};

// ─── Simple structs ───────────────────────────────────────────
struct Vec2i { int r = 0, c = 0; };

struct Effect {
    EffectType  type;
    int         turns   = 0;
    int         value   = 0;    // damage per tick / armor bonus / etc.
};

struct TrapTile {
    int  r = 0, c = 0;
    int  damage = 20;
    bool isPlayer = false;      // who placed it
};

struct FloatText {
    float x, y;
    float vy     = -1.5f;
    float alpha  = 1.0f;
    int   frames = 0;
    std::string text;
    SDL_Color   color;
    int         fontSize = 16;
};

struct LogEntry {
    std::string text;
    SDL_Color   color;
};

// ─── Utility helpers ─────────────────────────────────────────
inline float dist(int r1, int c1, int r2, int c2) {
    return std::sqrt(static_cast<float>((r1-r2)*(r1-r2) + (c1-c2)*(c1-c2)));
}

inline SDL_Color withAlpha(SDL_Color c, Uint8 a) {
    return {c.r, c.g, c.b, a};
}

inline SDL_Rect tileRect(int r, int c) {
    return {
        MAP_OFFSET_X + c * TILE_SIZE,
        MAP_OFFSET_Y + r * TILE_SIZE,
        TILE_SIZE, TILE_SIZE
    };
}

// Convert class enum to display string
inline std::string className(ClassType ct) {
    switch (ct) {
        case ClassType::WARRIOR:  return "Warrior";
        case ClassType::MAGE:     return "Mage";
        case ClassType::ASSASSIN: return "Assassin";
        case ClassType::MEDIC:    return "Medic";
        case ClassType::PALADIN:  return "Paladin";
        case ClassType::ARCHER:   return "Archer";
        default: return "None";
    }
}

inline SDL_Color classColor(ClassType ct) {
    switch (ct) {
        case ClassType::WARRIOR:  return {224, 80,  80,  255};
        case ClassType::MAGE:     return {112, 80,  224, 255};
        case ClassType::ASSASSIN: return {80,  224, 144, 255};
        case ClassType::MEDIC:    return {80,  200, 224, 255};
        case ClassType::PALADIN:  return {224, 192, 80,  255};
        case ClassType::ARCHER:   return {128, 192, 80,  255};
        default: return {200, 200, 200, 255};
    }
}

inline std::string classEmoji(ClassType ct) {
    switch (ct) {
        case ClassType::WARRIOR:  return "[W]";
        case ClassType::MAGE:     return "[M]";
        case ClassType::ASSASSIN: return "[A]";
        case ClassType::MEDIC:    return "[H]";
        case ClassType::PALADIN:  return "[P]";
        case ClassType::ARCHER:   return "[R]";
        default: return "[?]";
    }
}