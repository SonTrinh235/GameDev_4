#include "../../include/Core/Resource.hpp"
#include <SDL3/SDL_ttf.h>
#include <iostream>

Resource& Resource::instance() {
    static Resource inst;
    return inst;
}

bool Resource::init(SDL_Renderer* renderer) {
    m_renderer = renderer;
    if (!TTF_Init()) {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << "\n";
        return false;
    }
    // Pre-load common sizes
    font(12); font(13); font(14); font(16); font(20); font(24);
    return true;
}

void Resource::shutdown() {
    for (auto& [size, f] : m_fonts)
        if (f) TTF_CloseFont(f);
    m_fonts.clear();
    TTF_Quit();
}

TTF_Font* Resource::font(int ptSize) {
    auto it = m_fonts.find(ptSize);
    if (it != m_fonts.end()) return it->second;

    TTF_Font* f = TTF_OpenFont(m_fontPath.c_str(), ptSize);
    if (!f) {
        // Try fallback system fonts (Windows + Linux)
        const char* fallbacks[] = {
            "C:\\Windows\\Fonts\\arial.ttf",         
            "C:\\Windows\\Fonts\\segoeui.ttf",       
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
            nullptr
        };
        for (int i = 0; fallbacks[i]; i++) {
            f = TTF_OpenFont(fallbacks[i], ptSize);
            if (f) {
                std::cout << "Loaded font from: " << fallbacks[i] << "\n";
                break;
            }
        }
        if (!f) {
            std::cerr << "Could not load any font at size " << ptSize << "\n";
            return nullptr;
        }
    }
    m_fonts[ptSize] = f;
    return f;
}

SDL_Texture* Resource::renderText(const std::string& text,
                                  SDL_Color color,
                                  int ptSize) {
    TTF_Font* f = font(ptSize);
    if (!f || text.empty()) return nullptr;
    SDL_Surface* surf = TTF_RenderText_Blended(f, text.c_str(), text.length(), color);
    if (!surf) return nullptr;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surf);
    SDL_DestroySurface(surf);
    return tex;
}