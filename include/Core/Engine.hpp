#pragma once
#include "Common.hpp"
#include <SDL3/SDL_ttf.h>
#include <memory>

class Grid;
class TurnManager;
class SaveSystem;

class Engine {
public:
    Engine();
    ~Engine();
    bool init();
    void run();
    void quit();
private:
    SDL_Window* m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool          m_running  = false;
    GameState     m_state    = GameState::MAIN_MENU;

    std::unique_ptr<Grid>        m_grid;
    std::unique_ptr<TurnManager> m_turnManager;
    std::unique_ptr<SaveSystem>  m_saveSystem;

    std::vector<ClassType> m_playerClasses;
    std::vector<ClassType> m_enemyClasses;
    AIMode                 m_aiMode = AIMode::MINIMAX;

    std::vector<LogEntry>    m_log;
    std::vector<FloatText>   m_floats;
    int                      m_round = 1;
    bool                     m_playerWon = false;

    void handleEvents(SDL_Event& e);
    void update(float dt);
    void render();

    void renderMainMenu();
    void renderTeamBuilder();
    void renderGame();
    void renderPause();
    void renderGameOver();

    void drawRect(int x, int y, int w, int h, SDL_Color c, bool filled = true);
    void drawBorder(int x, int y, int w, int h, SDL_Color c, int thickness = 1);
    void drawText(const std::string& text, int x, int y, SDL_Color c, int pt = 13, bool centered = false);
    void drawBar(int x, int y, int w, int h, float frac, SDL_Color fill, SDL_Color bg);
    void drawLeftPanel();
    void drawRightPanel();
    void drawTopHUD();
    void drawBottomLog();
    void drawFloatTexts();

    void onMapClick(int px, int py);
    void onMenuClick(int px, int py);
    void onPanelClick(int px, int py); // Đã thêm
    void onTeamBuilderClick(int px, int py);
    void onPauseClick(int px, int py);

    void startNewGame();
    void loadGame();
    void saveGame();
    void addLog(const std::string& text, SDL_Color color);
    void spawnFloat(int row, int col, const std::string& text, SDL_Color color, int size = 16);
    void checkVictory();
};