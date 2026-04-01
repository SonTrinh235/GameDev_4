#include "../../include/Map/Tile.hpp"

SDL_Rect Tile::rect() const {
    return {
        MAP_OFFSET_X + col * TILE_SIZE,
        MAP_OFFSET_Y + row * TILE_SIZE,
        TILE_SIZE, TILE_SIZE
    };
}