#pragma once
#include "Common.hpp"
#include <SDL3/SDL_ttf.h>
#include <map>

// Manages fonts
class Resource {
public:
    static Resource& instance();

    bool init(SDL_Renderer* renderer);
    void shutdown();

    // Returns a cached TTF_Font* at requested pt size.
    // Falls back to a default size if not loaded yet.
    TTF_Font* font(int ptSize);

    // Render UTF-8 text as a temporary texture (caller frees).
    SDL_Texture* renderText(const std::string& text,
                            SDL_Color color,
                            int ptSize = 14);

    SDL_Renderer* renderer() const { return m_renderer; }

private:
    Resource() = default;
    SDL_Renderer*             m_renderer = nullptr;
    std::map<int, TTF_Font*>  m_fonts;
    std::string               m_fontPath = "assets/fonts/font.ttf";
};