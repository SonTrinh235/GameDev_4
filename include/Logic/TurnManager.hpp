#pragma once
#include "../Core/Common.hpp"
#include "../Entities/Team.hpp"
#include "../Entities/Skill.hpp"
#include "../Entities/Character.hpp"
#include "../Map/Grid.hpp"
#include <vector>
#include <memory>
#include <functional>

using LogCb2 = std::function<void(const std::string&, SDL_Color)>;
using FloatCb2 = std::function<void(int, int, const std::string&, SDL_Color, int)>;

class TurnManager {
public:
    TurnManager(LogCb2 logCb, FloatCb2 floatCb);

    void newGame(const std::vector<ClassType>& playerClasses, const std::vector<ClassType>& enemyClasses);
    void buildTurnQueue();
    
    Character* currentChar() const;
    std::vector<Character*> allAlive();
    
    void setSelected(Character* ch);
    Character* selected() const { return m_selected; }
    
    void setActiveSkill(const Skill* sk);
    const Skill* activeSkill() const { return m_activeSkill; }
    
    void setMoveMode(bool on);
    bool isMoveMode() const { return m_moveMode; }

    void refreshHighlights(Grid& grid);
    void onTurnStart(Character& ch);
    void advanceTurn();
    
    bool playerMove(int toRow, int toCol, Grid& grid);
    bool playerUseSkill(int targetRow, int targetCol, Character* targetChar, Grid& grid);
    void playerEndTurn();
    bool isGameOver(bool& outPlayerWon) const;

    struct SaveData {
        std::vector<Character> playerChars;
        std::vector<Character> enemyChars;
        int turnIndex;
        int round;
        bool playerTurn;
    };
    SaveData exportSave() const;
    void importSave(const SaveData& sd);

    Team& playerTeam() { return *m_playerTeam; }
    const Team& playerTeam() const { return *m_playerTeam; }
    Team& enemyTeam() { return *m_enemyTeam; }
    const Team& enemyTeam() const { return *m_enemyTeam; }

    // HÀM CHO AI MƯỢN ĐỂ IN LOG
    LogCb2 getLogCb() const { return m_log; }
    FloatCb2 getFloatCb() const { return m_float; }

private:
    LogCb2 m_log;
    FloatCb2 m_float;
    std::unique_ptr<Team> m_playerTeam;
    std::unique_ptr<Team> m_enemyTeam;
    std::vector<Character*> m_turnQueue;
    int m_turnIndex = 0;
    int m_round = 1;
    Character* m_selected = nullptr;
    const Skill* m_activeSkill = nullptr;
    bool m_moveMode = false;
};