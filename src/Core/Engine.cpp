#include "../../include/Core/Engine.hpp"
#include "../../include/Core/Resource.hpp"
#include "../../include/Map/Grid.hpp"
#include "../../include/Logic/TurnManager.hpp"
#include "../../include/Logic/SaveSystem.hpp"
#include "../../include/Logic/Combat.hpp"
#include "../../include/AI/AIRandom.hpp"
#include "../../include/AI/AIMinimax.hpp"
#include "../../include/AI/AIMCTS.hpp"
#include <SDL3/SDL_ttf.h>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iostream>

static bool   g_aiPending   = false;
static Uint64 g_aiFireAt    = 0;
static constexpr Uint64 AI_DELAY_MS = 600;

struct Btn { int x,y,w,h; std::string label; };
static std::vector<Btn> g_menuBtns;
static std::vector<Btn> g_tbBtns;
static std::vector<Btn> g_hudBtns;
static std::vector<Btn> g_skillBtns;
static std::vector<Btn> g_actionBtns;
static std::vector<Btn> g_pauseBtns;

static const ClassType ALL_CLASSES[] = {
    ClassType::WARRIOR, ClassType::MAGE, ClassType::ASSASSIN,
    ClassType::MEDIC,   ClassType::PALADIN, ClassType::ARCHER
};

Engine::Engine() {
    m_playerClasses = {ClassType::WARRIOR, ClassType::MAGE, ClassType::ASSASSIN, ClassType::MEDIC};
    m_enemyClasses  = {ClassType::WARRIOR, ClassType::MAGE, ClassType::ASSASSIN, ClassType::PALADIN};
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}
Engine::~Engine() {
    Resource::instance().shutdown();
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Engine::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) return false;
    m_window = SDL_CreateWindow("BTL4 - Tactics Arena", SCREEN_W, SCREEN_H, 0);
    if (!m_window) return false;
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) return false;
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    if (!Resource::instance().init(m_renderer)) return false;

    m_grid        = std::make_unique<Grid>();
    m_saveSystem  = std::make_unique<SaveSystem>();
    m_turnManager = std::make_unique<TurnManager>(
        [this](const std::string& text, SDL_Color color) { addLog(text, color); },
        [this](int r, int c, const std::string& text, SDL_Color color, int size) { spawnFloat(r, c, text, color, size); }
    );
    m_running = true;
    return true;
}

void Engine::run() {
    Uint64 prev = SDL_GetTicks();
    SDL_Event e;
    while (m_running) {
        Uint64 now = SDL_GetTicks();
        float dt = (now - prev) / 1000.f;
        prev = now;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) { m_running = false; break; }
            handleEvents(e);
        }
        update(dt);
        render();
        SDL_Delay(16);
    }
}
void Engine::quit() { m_running = false; }

void Engine::handleEvents(SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
        if (m_state == GameState::PLAYING) m_state = GameState::PAUSED;
        else if (m_state == GameState::PAUSED) m_state = GameState::PLAYING;
    }
    if (e.type != SDL_EVENT_MOUSE_BUTTON_DOWN) return;
    int px = (int)e.button.x, py = (int)e.button.y;

    switch (m_state) {
    case GameState::MAIN_MENU:    onMenuClick(px, py);       break;
    case GameState::TEAM_BUILDER: onTeamBuilderClick(px, py);break;
    case GameState::PLAYING:
        if (px >= MAP_OFFSET_X && px < SCREEN_W - PANEL_RIGHT_W && py >= MAP_OFFSET_Y && py < SCREEN_H - LOG_HEIGHT)
            onMapClick(px, py);
        else
            onPanelClick(px, py); // CHÚ Ý: Kích hoạt onPanelClick
        break;
    case GameState::PAUSED:       onPauseClick(px, py);      break;
    default: break;
    }
}

void Engine::update(float dt) {
    for (auto& ft : m_floats) { ft.y += ft.vy; ft.alpha -= 0.03f; ft.frames++; }
    m_floats.erase(std::remove_if(m_floats.begin(), m_floats.end(),
        [](const FloatText& f){ return f.alpha <= 0.f || f.frames > 40; }), m_floats.end());

    if (m_state != GameState::PLAYING) return;

    // SỬA LỖI AI KẸT: Cập nhật biến g_aiPending liên tục nếu tới lượt địch
    Character* currentTurn = m_turnManager->currentChar();
    if (currentTurn && !currentTurn->isPlayer && currentTurn->alive && !g_aiPending) {
        g_aiPending = true;
        g_aiFireAt = SDL_GetTicks() + AI_DELAY_MS;
    }

    if (g_aiPending && SDL_GetTicks() >= g_aiFireAt) {
        g_aiPending = false;
        Character* cur = m_turnManager->currentChar();
        if (cur && !cur->isPlayer && cur->alive) {
            addLog("[AI] " + cur->label() + " dang hanh dong...", Colour::purple());
            
            if (m_aiMode == AIMode::RANDOM) {
                AIRandom ai; ai.takeTurn(*cur, *m_turnManager, *m_grid);
            } else if (m_aiMode == AIMode::MINIMAX) {
                AIMinimax ai(2); ai.takeTurn(*cur, *m_turnManager, *m_grid);
            } else {
                AIMCTS ai(16); ai.takeTurn(*cur, *m_turnManager, *m_grid);
            }
            checkVictory();
            if (m_state == GameState::PLAYING) {
                m_turnManager->advanceTurn();
            }
        }
    }
}

void Engine::render() {
    SDL_SetRenderDrawColor(m_renderer, 10, 12, 20, 255);
    SDL_RenderClear(m_renderer);
    switch (m_state) {
    case GameState::MAIN_MENU:    renderMainMenu();    break;
    case GameState::TEAM_BUILDER: renderTeamBuilder(); break;
    case GameState::PLAYING:      renderGame();        break;
    case GameState::PAUSED:       renderGame(); renderPause(); break;
    case GameState::GAME_OVER:    renderGame(); renderGameOver(); break;
    }
    SDL_RenderPresent(m_renderer);
}

void Engine::drawRect(int x, int y, int w, int h, SDL_Color c, bool filled) {
    SDL_SetRenderDrawColor(m_renderer, c.r, c.g, c.b, c.a);
    SDL_FRect r = {(float)x,(float)y,(float)w,(float)h};
    if (filled) SDL_RenderFillRect(m_renderer, &r);
    else        SDL_RenderRect(m_renderer, &r);
}

void Engine::drawBorder(int x, int y, int w, int h, SDL_Color c, int t) {
    for (int i = 0; i < t; i++) drawRect(x+i, y+i, w-2*i, h-2*i, c, false);
}

void Engine::drawText(const std::string& text, int x, int y, SDL_Color c, int pt, bool centered) {
    if (text.empty()) return;
    SDL_Texture* tex = Resource::instance().renderText(text, c, pt);
    if (!tex) return;
    float tw = 0, th = 0; SDL_GetTextureSize(tex, &tw, &th);
    if (centered) x -= (int)(tw / 2);
    SDL_FRect dst = {(float)x, (float)y, tw, th};
    SDL_RenderTexture(m_renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void Engine::drawBar(int x, int y, int w, int h, float frac, SDL_Color fill, SDL_Color bg) {
    drawRect(x, y, w, h, bg);
    int filled = static_cast<int>(w * std::max(0.f, std::min(1.f, frac)));
    if (filled > 0) drawRect(x, y, filled, h, fill);
}

void Engine::renderMainMenu() {
    g_menuBtns.clear();
    SDL_SetRenderDrawColor(m_renderer, 20, 30, 60, 30);
    for (int x = 0; x < SCREEN_W; x += 48) SDL_RenderLine(m_renderer, (float)x, 0, (float)x, (float)SCREEN_H);
    for (int y = 0; y < SCREEN_H; y += 48) SDL_RenderLine(m_renderer, 0, (float)y, (float)SCREEN_W, (float)y);

    drawText("TACTICS ARENA", SCREEN_W/2, 140, Colour::gold(), 40, true);
    drawText("Turn-Based Tactical Battle", SCREEN_W/2, 195, Colour::dim(), 14, true);

    std::vector<std::string> labels = {"Tran Dau Moi"};
    if (m_saveSystem->hasSave()) labels.push_back("Tiep Tuc");
    labels.push_back("Thoat");

    for (int i = 0; i < (int)labels.size(); i++) {
        int bw = 260, bh = 44, bx = SCREEN_W/2 - bw/2, by = 270 + i * 56;
        g_menuBtns.push_back({bx, by, bw, bh, labels[i]});
        drawRect(bx, by, bw, bh, {15, 18, 32, 220});
        drawBorder(bx, by, bw, bh, {30, 42, 74, 255});
        drawText(labels[i], bx + bw/2, by + 12, Colour::white(), 16, true);
    }
}

void Engine::onMenuClick(int px, int py) {
    if (m_state == GameState::PLAYING || m_state == GameState::PAUSED) return;
    for (auto& b : g_menuBtns) {
        if (px >= b.x && px < b.x+b.w && py >= b.y && py < b.y+b.h) {
            if (b.label == "Tran Dau Moi")  { m_state = GameState::TEAM_BUILDER; }
            else if (b.label == "Tiep Tuc") { loadGame(); }
            else if (b.label == "Thoat")    { m_running = false; }
        }
    }
}

void Engine::renderTeamBuilder() {
    g_tbBtns.clear();
    drawText("CHON DOI HINH", SCREEN_W/2, 20, Colour::gold(), 22, true);

    drawText("Doi Ban (4 nhan vat)", 80, 56, Colour::blue(), 13);
    for (int i = 0; i < 6; i++) {
        ClassType ct = ALL_CLASSES[i];
        int bx = 60 + (i%3)*130, by = 76 + (i/3)*70;
        bool selected = std::find(m_playerClasses.begin(), m_playerClasses.end(), ct) != m_playerClasses.end();
        SDL_Color border = selected ? classColor(ct) : Colour::border();
        drawRect(bx, by, 120, 60, Colour::card()); drawBorder(bx, by, 120, 60, border);
        drawText(classEmoji(ct) + " " + className(ct), bx + 60, by + 20, selected ? classColor(ct) : Colour::dim(), 13, true);
        if (selected) drawText("*", bx + 110, by + 4, Colour::gold(), 14);
        g_tbBtns.push_back({bx, by, 120, 60, "pc_" + className(ct)});
    }

    drawText("Doi Dich (AI):", 480, 56, Colour::red(), 13);
    for (int i = 0; i < (int)m_enemyClasses.size(); i++) {
        int bx = 480 + (i%2)*130, by = 76 + (i/2)*70;
        ClassType ct = m_enemyClasses[i];
        drawRect(bx, by, 120, 60, Colour::card()); drawBorder(bx, by, 120, 60, Colour::border());
        drawText(classEmoji(ct) + " " + className(ct), bx + 60, by + 20, classColor(ct), 13, true);
    }

    drawText("Che do AI:", 750, 56, Colour::dim(), 13);
    const char* aiLabels[] = {"AI Random", "AI Minimax", "AI MCTS"};
    for (int i = 0; i < 3; i++) {
        int bx = 750, by = 76 + i * 52;
        bool sel = (int)m_aiMode == i;
        drawRect(bx, by, 150, 40, sel ? SDL_Color{20,30,60,255} : Colour::card());
        drawBorder(bx, by, 150, 40, sel ? Colour::gold() : Colour::border());
        drawText(aiLabels[i], bx + 75, by + 12, sel ? Colour::gold() : Colour::dim(), 13, true);
        g_tbBtns.push_back({bx, by, 150, 40, "ai_" + std::to_string(i)});
    }

    int bw = 160, by = SCREEN_H - 80;
    g_tbBtns.push_back({80, by, bw, 44, "back"});
    g_tbBtns.push_back({SCREEN_W/2-bw/2, by, bw, 44, "start"});

    drawRect(80, by, bw, 44, Colour::card()); drawBorder(80, by, bw, 44, Colour::border());
    drawText("Quay Lai", 80+bw/2, by+13, Colour::dim(), 14, true);

    bool canStart = (int)m_playerClasses.size() == 4;
    drawRect(SCREEN_W/2-bw/2, by, bw, 44, canStart ? SDL_Color{20,40,80,255} : Colour::card());
    drawBorder(SCREEN_W/2-bw/2, by, bw, 44, canStart ? Colour::gold() : Colour::border());
    drawText("Bat Dau Chien!", SCREEN_W/2, by+13, canStart ? Colour::gold() : Colour::dim(), 14, true);
}

void Engine::onTeamBuilderClick(int px, int py) {
    for (auto& b : g_tbBtns) {
        if (px < b.x || px >= b.x+b.w || py < b.y || py >= b.y+b.h) continue;
        if (b.label.substr(0,3) == "pc_") {
            std::string cname = b.label.substr(3);
            ClassType ct = ClassType::NONE;
            for (auto& c : ALL_CLASSES) if (className(c) == cname) { ct = c; break; }
            if (ct == ClassType::NONE) continue;
            auto it = std::find(m_playerClasses.begin(), m_playerClasses.end(), ct);
            if (it != m_playerClasses.end()) m_playerClasses.erase(it);
            else {
                if ((int)m_playerClasses.size() >= 4) m_playerClasses.erase(m_playerClasses.begin());
                m_playerClasses.push_back(ct);
            }
        } else if (b.label.substr(0,3) == "ai_") {
            m_aiMode = static_cast<AIMode>(std::stoi(b.label.substr(3)));
        } else if (b.label == "back") {
            m_state = GameState::MAIN_MENU;
        } else if (b.label == "start" && (int)m_playerClasses.size() == 4) {
            startNewGame();
        }
    }
}

void Engine::drawTopHUD() {
    g_hudBtns.clear();
    int left = MAP_OFFSET_X, right = SCREEN_W - PANEL_RIGHT_W;
    drawRect(left, 0, right - left, MAP_OFFSET_Y, Colour::panel());
    drawBorder(left, 0, right - left, MAP_OFFSET_Y, Colour::border());

    Character* cur = m_turnManager->currentChar();
    if (cur) {
        std::string turnStr = "Luot: " + cur->label() + "  |  AP: " + std::to_string(cur->ap) + "/" + std::to_string(cur->maxAp);
        drawText(turnStr, left + 12, 13, cur->isPlayer ? Colour::blue() : Colour::red(), 13);
    }
    drawText("Vong " + std::to_string(m_round), left + 320, 13, Colour::dim(), 12);

    struct HudBtn { std::string label; SDL_Color col; };
    std::vector<HudBtn> hbts = {{"Dung|Tiep", Colour::dim()}, {"Luu", Colour::dim()}, {"Menu", Colour::dim()}};
    int bx = right - 10;
    for (int i = (int)hbts.size()-1; i >= 0; i--) {
        int bw = 68, bh = 28, by2 = 7;
        bx -= bw + 6;
        drawRect(bx, by2, bw, bh, Colour::card()); drawBorder(bx, by2, bw, bh, Colour::border());
        drawText(hbts[i].label, bx+bw/2, by2+7, hbts[i].col, 11, true);
        g_hudBtns.push_back({bx, by2, bw, bh, hbts[i].label});
    }
}

void Engine::drawLeftPanel() {
    drawRect(0, 0, MAP_OFFSET_X, SCREEN_H, Colour::panel());
    drawBorder(0, 0, MAP_OFFSET_X, SCREEN_H, Colour::border());

    auto drawCharMini = [&](const Character& c, int y) {
        bool isCur = m_turnManager->currentChar() == &c;
        bool isSel = m_turnManager->selected() == &c;
        drawRect(6, y, MAP_OFFSET_X-12, 62, isCur ? SDL_Color{20,30,55,255} : Colour::card());
        drawBorder(6, y, MAP_OFFSET_X-12, 62, isCur ? Colour::gold() : isSel ? Colour::blue() : Colour::border());

        if (!c.alive) { drawText("[DEAD] " + c.name, 10, y+22, Colour::dim(), 11); return; }
        drawText(classEmoji(c.classType) + " " + c.name, 10, y+4, classColor(c.classType), 11);
        drawText("HP", 10, y+18, Colour::dim(), 10);
        drawBar(30, y+20, MAP_OFFSET_X-40, 6, c.hpFrac(), Colour::hp(), {40,20,20,255});
        drawText(std::to_string(c.hp) + "/" + std::to_string(c.maxHp), 10, y+30, Colour::dim(), 9);
        drawText("AP", 10, y+42, Colour::dim(), 10);
        drawBar(30, y+44, MAP_OFFSET_X-40, 5, c.apFrac(), Colour::ap(), {40,40,10,255});
        drawText(std::to_string(c.ap), 10, y+48, Colour::dim(), 9);
    };

    int y = 10;
    drawText("DOI BAN", 8, y, Colour::blue(), 11); y += 18;
    for (auto& c : m_turnManager->playerTeam().characters()) { drawCharMini(c, y); y += 68; }
    y += 8;
    drawText("DOI DICH", 8, y, Colour::red(), 11); y += 18;
    for (auto& c : m_turnManager->enemyTeam().characters()) { drawCharMini(c, y); y += 68; }
}

void Engine::drawRightPanel() {
    g_skillBtns.clear(); g_actionBtns.clear();
    int rx = SCREEN_W - PANEL_RIGHT_W;
    drawRect(rx, 0, PANEL_RIGHT_W, SCREEN_H, Colour::panel());
    drawBorder(rx, 0, PANEL_RIGHT_W, SCREEN_H, Colour::border());

    Character* sel = m_turnManager->selected(); Character* cur = m_turnManager->currentChar();
    int y = MAP_OFFSET_Y + 8;
    drawText("NHAN VAT", rx+8, y, Colour::dim(), 11); y += 16;

    if (sel) {
        drawText(classEmoji(sel->classType) + " " + sel->name, rx+8, y, classColor(sel->classType), 13); y += 18;
        auto stat = [&](const std::string& k, int v, SDL_Color vc) { drawText(k+":", rx+8, y, Colour::dim(), 11); drawText(std::to_string(v), rx+90, y, vc, 11); y += 15; };
        stat("HP", sel->hp, Colour::hp()); drawBar(rx+8, y, PANEL_RIGHT_W-16, 5, sel->hpFrac(), Colour::hp(), {40,20,20,255}); y += 8;
        stat("AP", sel->ap, Colour::ap()); stat("Speed", sel->speed, Colour::dim());
        stat("Armor", sel->armor, Colour::dim()); stat("MRes", sel->magicRes, Colour::dim());
        if (sel->physDmg > 0) stat("PhysDmg", sel->physDmg, Colour::red());
        if (sel->magDmg > 0) stat("MagDmg", sel->magDmg, {139,58,213,255});
        y += 4;
        if (!sel->effects.empty()) {
            drawText("Status:", rx+8, y, Colour::dim(), 10); y += 14;
            for (auto& eff : sel->effects) {
                std::string ename = (eff.type == EffectType::POISON) ? "Poison" : (eff.type == EffectType::FREEZE) ? "Freeze" : "Effect";
                drawText(ename + "(" + std::to_string(eff.turns) + ")", rx+8, y, {200,168,75,255}, 10); y += 13;
            }
        }
        y += 6;
    }

    drawText("KY NANG", rx+8, y, Colour::dim(), 11); y += 16;
    if (sel && cur && sel == cur && cur->isPlayer) {
        for (int i = 0; i < (int)sel->skills.size(); i++) {
            const Skill& sk = sel->skills[i];
            bool canUse = sel->canAfford(sk.apCost); bool isActive = m_turnManager->activeSkill() == &sk;
            int bx2 = rx+6, by2 = y, bw2 = PANEL_RIGHT_W-12, bh2 = 48;
            drawRect(bx2, by2, bw2, bh2, isActive ? SDL_Color{25,40,70,255} : Colour::card());
            drawBorder(bx2, by2, bw2, bh2, isActive ? Colour::gold() : canUse ? Colour::border() : SDL_Color{20,20,30,255});
            drawText(sk.name, bx2+6, by2+4, canUse ? (isActive ? Colour::gold() : Colour::white()) : Colour::dim(), 12);
            drawText(std::to_string(sk.apCost) + " AP", bx2+6, by2+20, Colour::ap(), 10);
            drawText(sk.desc, bx2+6, by2+33, Colour::dim(), 9);
            g_skillBtns.push_back({bx2, by2, bw2, bh2, "skill_" + std::to_string(i)}); y += 52;
        }
    }
    y += 6;

    drawText("HANH DONG", rx+8, y, Colour::dim(), 11); y += 16;
    bool isPlayerTurn = cur && cur->isPlayer;
    bool movActive = m_turnManager->isMoveMode();
    drawRect(rx+6, y, PANEL_RIGHT_W-12, 34, Colour::card());
    drawBorder(rx+6, y, PANEL_RIGHT_W-12, 34, movActive ? Colour::blue() : Colour::border());
    drawText(movActive ? "* Di Chuyen *" : "Di Chuyen", rx+6 + (PANEL_RIGHT_W-12)/2, y+10, isPlayerTurn ? (movActive ? Colour::blue() : Colour::white()) : Colour::dim(), 13, true);
    g_actionBtns.push_back({rx+6, y, PANEL_RIGHT_W-12, 34, "move"}); y += 40;

    drawRect(rx+6, y, PANEL_RIGHT_W-12, 34, Colour::card()); drawBorder(rx+6, y, PANEL_RIGHT_W-12, 34, Colour::red());
    drawText("Ket Thuc Luot", rx+6 + (PANEL_RIGHT_W-12)/2, y+10, isPlayerTurn ? Colour::red() : Colour::dim(), 13, true);
    g_actionBtns.push_back({rx+6, y, PANEL_RIGHT_W-12, 34, "endturn"});
}

void Engine::drawBottomLog() {
    int lx = MAP_OFFSET_X, lw = SCREEN_W - MAP_OFFSET_X - PANEL_RIGHT_W, ly = SCREEN_H - LOG_HEIGHT;
    drawRect(lx, ly, lw, LOG_HEIGHT, Colour::panel()); drawBorder(lx, ly, lw, LOG_HEIGHT, Colour::border());
    int start = std::max(0, (int)m_log.size() - 4);
    for (int i = start; i < (int)m_log.size(); i++) drawText(m_log[i].text, lx + 6, ly + 6 + (i - start) * 15, m_log[i].color, 11);
}

void Engine::drawFloatTexts() {
    for (auto& ft : m_floats) drawText(ft.text, (int)ft.x, (int)ft.y, withAlpha(ft.color, (Uint8)(ft.alpha * 255)), ft.fontSize, true);
}

static void renderChar(SDL_Renderer* r, const Character& c, bool isCurrent, bool isSelected) {
    if (!c.alive) return;
    int x = MAP_OFFSET_X + c.col * TILE_SIZE, y = MAP_OFFSET_Y + c.row * TILE_SIZE;
    SDL_FRect cell = {(float)x,(float)y,(float)TILE_SIZE,(float)TILE_SIZE};
    SDL_Color col = classColor(c.classType);
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, isCurrent ? 90 : 55); SDL_RenderFillRect(r, &cell);

    if (isCurrent) {
        SDL_SetRenderDrawColor(r, 200,168,75,255);
        for (int t = 0; t < 2; t++) { SDL_FRect ring = {(float)(x+t),(float)(y+t),(float)(TILE_SIZE-2*t),(float)(TILE_SIZE-2*t)}; SDL_RenderRect(r, &ring); }
    } else if (isSelected) {
        SDL_SetRenderDrawColor(r, 58,123,213,200);
        SDL_FRect ring = {(float)(x+1),(float)(y+1),(float)(TILE_SIZE-2),(float)(TILE_SIZE-2)}; SDL_RenderRect(r, &ring);
    }
    SDL_Color strip = c.isPlayer ? SDL_Color{58,123,213,180} : SDL_Color{213,58,58,180};
    SDL_SetRenderDrawColor(r, strip.r, strip.g, strip.b, strip.a);
    SDL_FRect stripR = {(float)(x+2),(float)(y+TILE_SIZE-5),(float)(TILE_SIZE-4),4}; SDL_RenderFillRect(r, &stripR);

    SDL_SetRenderDrawColor(r, 30,15,15,200);
    SDL_FRect hpBg = {(float)(x+2),(float)(y+TILE_SIZE-11),(float)(TILE_SIZE-4),4}; SDL_RenderFillRect(r, &hpBg);
    float hpF = c.hpFrac();
    SDL_Color hpC = hpF > 0.5f ? SDL_Color{58,181,120,255} : hpF > 0.25f ? SDL_Color{224,192,80,255} : SDL_Color{224,80,80,255};
    SDL_SetRenderDrawColor(r, hpC.r, hpC.g, hpC.b, 255);
    int hpW = static_cast<int>((TILE_SIZE-4) * hpF);
    if (hpW > 0) { SDL_FRect hpFill = {(float)(x+2),(float)(y+TILE_SIZE-11),(float)hpW,4}; SDL_RenderFillRect(r, &hpFill); }

    int apDots = std::min(4, c.ap / 2 + (c.ap % 2));
    for (int d = 0; d < 4; d++) {
        SDL_Color dc = d < apDots ? SDL_Color{245,200,66,220} : SDL_Color{40,40,40,150};
        SDL_SetRenderDrawColor(r, dc.r, dc.g, dc.b, dc.a);
        SDL_FRect dot = {(float)(x+3+d*8),(float)(y+2),6,4}; SDL_RenderFillRect(r, &dot);
    }
}

void Engine::renderGame() {
    drawLeftPanel(); drawTopHUD(); m_grid->render(m_renderer);
    Character* cur = m_turnManager->currentChar(); Character* sel = m_turnManager->selected();
    for (auto& c : m_turnManager->playerTeam().characters()) renderChar(m_renderer, c, &c == cur, &c == sel);
    for (auto& c : m_turnManager->enemyTeam().characters()) renderChar(m_renderer, c, &c == cur, &c == sel);
    drawRightPanel(); drawBottomLog(); drawFloatTexts();
}

void Engine::renderPause() {
    g_pauseBtns.clear();
    drawRect(SCREEN_W/2-160, SCREEN_H/2-140, 320, 280, {10,12,25,240});
    drawBorder(SCREEN_W/2-160, SCREEN_H/2-140, 320, 280, Colour::gold());
    drawText("TAM DUNG", SCREEN_W/2, SCREEN_H/2 - 120, Colour::gold(), 22, true);
    std::vector<std::string> labels = {"Tiep Tuc","Luu Game","Tai Game","Về Menu"};
    for (int i = 0; i < (int)labels.size(); i++) {
        int bx = SCREEN_W/2-110, by = SCREEN_H/2-80 + i*55, bw = 220, bh = 40;
        drawRect(bx, by, bw, bh, Colour::card()); drawBorder(bx, by, bw, bh, Colour::border());
        drawText(labels[i], bx+bw/2, by+11, Colour::white(), 14, true);
        g_pauseBtns.push_back({bx, by, bw, bh, labels[i]});
    }
}

void Engine::onPauseClick(int px, int py) {
    for (auto& b : g_pauseBtns) {
        if (px<b.x||px>=b.x+b.w||py<b.y||py>=b.y+b.h) continue;
        if (b.label == "Tiep Tuc") m_state = GameState::PLAYING;
        else if (b.label == "Luu Game") saveGame();
        else if (b.label == "Tai Game") loadGame();
        else if (b.label == "Về Menu")  m_state = GameState::MAIN_MENU;
    }
}

void Engine::renderGameOver() {
    drawRect(SCREEN_W/2-200, SCREEN_H/2-100, 400, 200, {10,12,25,240});
    drawBorder(SCREEN_W/2-200, SCREEN_H/2-100, 400, 200, Colour::gold());
    if (m_playerWon) {
        drawText("CHIEN THANG!", SCREEN_W/2, SCREEN_H/2-70, Colour::gold(), 30, true);
        drawText("Doi ban da chien thang!", SCREEN_W/2, SCREEN_H/2-30, Colour::white(), 16, true);
    } else {
        drawText("THAT BAI", SCREEN_W/2, SCREEN_H/2-70, Colour::red(), 30, true);
        drawText("Doi ban da bi tieu diet...", SCREEN_W/2, SCREEN_H/2-30, Colour::dim(), 16, true);
    }
}

void Engine::onPanelClick(int px, int py) {
    if (!m_turnManager) return;
    Character* cur = m_turnManager->currentChar(); Character* sel = m_turnManager->selected();
    bool isPlayerTurn = (cur && cur->isPlayer && cur == sel); 

    for (auto& b : g_actionBtns) {
        if (px >= b.x && px < b.x + b.w && py >= b.y && py < b.y + b.h) {
            if (b.label == "move" && isPlayerTurn) {
                m_turnManager->setMoveMode(true); m_turnManager->refreshHighlights(*m_grid);
                addLog("Chon o tren ban do de di chuyen", Colour::blue());
            } else if (b.label == "endturn" && cur && cur->isPlayer) {
                m_turnManager->playerEndTurn();
            }
            return;
        }
    }
    if (isPlayerTurn) {
        for (auto& b : g_skillBtns) {
            if (px >= b.x && px < b.x + b.w && py >= b.y && py < b.y + b.h) {
                int idx = std::stoi(b.label.substr(6));
                if (idx >= 0 && idx < (int)cur->skills.size()) {
                    m_turnManager->setActiveSkill(&cur->skills[idx]); m_turnManager->refreshHighlights(*m_grid);
                    addLog("Chon muc tieu cho: " + cur->skills[idx].name, Colour::gold());
                }
                return;
            }
        }
    }
    for (auto& b : g_hudBtns) {
        if (px >= b.x && px < b.x + b.w && py >= b.y && py < b.y + b.h) {
            if (b.label == "Luu") saveGame(); else if (b.label == "Menu") m_state = GameState::MAIN_MENU;
            return;
        }
    }
}

void Engine::onMapClick(int px, int py) {
    if (m_state != GameState::PLAYING || !m_turnManager) return;
    Character* cur = m_turnManager->currentChar();
    if (!cur || cur->isPlayer == false) return;

    int gridX = (px - MAP_OFFSET_X) / TILE_SIZE, gridY = (py - MAP_OFFSET_Y) / TILE_SIZE;
    if (gridX < 0 || gridX >= GRID_COLS || gridY < 0 || gridY >= GRID_ROWS) { m_turnManager->setSelected(nullptr); return; }

    Character* target = nullptr;
    for (auto& c : m_turnManager->playerTeam().characters()) if (c.alive && c.row == gridY && c.col == gridX) { target = &c; break; }
    if (!target) for (auto& c : m_turnManager->enemyTeam().characters()) if (c.alive && c.row == gridY && c.col == gridX) { target = &c; break; }

    if (m_turnManager->activeSkill()) { m_turnManager->playerUseSkill(gridY, gridX, target, *m_grid); }
    else if (m_turnManager->isMoveMode()) {
        if (!target) { if (!m_turnManager->playerMove(gridY, gridX, *m_grid)) addLog("Khong the di chuyen toi do!", {213, 58, 58, 255}); }
        else addLog("O da co nguoi dung!", {213, 58, 58, 255});
    } else {
        if (target) { m_turnManager->setSelected(target); addLog(target->label() + " tro nen noi bat", Colour::gold()); }
        else m_turnManager->setSelected(nullptr);
    }
}

void Engine::startNewGame() {
    m_turnManager->newGame(m_playerClasses, m_enemyClasses);
    for (auto& c : m_turnManager->playerTeam().characters()) { c.row = rand() % (GRID_ROWS / 2); c.col = rand() % (GRID_COLS / 3); }
    for (auto& c : m_turnManager->enemyTeam().characters()) { c.row = GRID_ROWS / 2 + rand() % (GRID_ROWS / 2); c.col = GRID_COLS - 1 - (rand() % (GRID_COLS / 3)); }
    m_state = GameState::PLAYING; m_round = 1; m_playerWon = false; m_log.clear(); m_floats.clear();
    addLog("Tran dau da bat dau!", Colour::gold());
    m_turnManager->advanceTurn();
}

void Engine::saveGame() {
    if (m_saveSystem && m_turnManager) {
        TurnManager::SaveData data = m_turnManager->exportSave();
        m_saveSystem->saveGame(data, m_aiMode, m_playerClasses, m_enemyClasses, m_round);
        addLog("Tro choi da duoc luu", Colour::gold());
    }
}

void Engine::loadGame() {
    if (m_saveSystem && m_turnManager) {
        TurnManager::SaveData data;
        if (m_saveSystem->loadGame(data, m_aiMode, m_playerClasses, m_enemyClasses, m_round)) {
            m_turnManager->importSave(data); m_state = GameState::PLAYING; addLog("Tro choi da duoc tai", Colour::gold());
        } else { addLog("Khong the tai tro choi", Colour::red()); m_state = GameState::MAIN_MENU; }
    }
}

void Engine::addLog(const std::string& text, SDL_Color color) {
    m_log.push_back({text, color}); if (m_log.size() > 50) m_log.erase(m_log.begin());
}

void Engine::spawnFloat(int row, int col, const std::string& text, SDL_Color color, int size) {
    float x = MAP_OFFSET_X + col * TILE_SIZE + TILE_SIZE / 2.f;
    float y = MAP_OFFSET_Y + row * TILE_SIZE;
    
    // THÊM CHỮ "FloatText" VÀO TRƯỚC DẤU NGOẶC NHỌN
    m_floats.push_back(FloatText{x, y, -1.5f, 1.0f, 0, text, color, size});
}

void Engine::checkVictory() {
    bool playerAlive = false, enemyAlive = false;
    for (auto& c : m_turnManager->playerTeam().characters()) if (c.alive) playerAlive = true;
    for (auto& c : m_turnManager->enemyTeam().characters()) if (c.alive) enemyAlive = true;
    if (!playerAlive) { m_playerWon = false; m_state = GameState::GAME_OVER; addLog("Doi ban da thua cuoc", Colour::red()); }
    else if (!enemyAlive) { m_playerWon = true; m_state = GameState::GAME_OVER; addLog("Doi ban da chien thang!", Colour::gold()); }
}