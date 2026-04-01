#include "../../include/Logic/TurnManager.hpp"
#include "../../include/Logic/Combat.hpp"
#include <algorithm>

TurnManager::TurnManager(LogCb2 logCb, FloatCb2 floatCb)
    : m_log(std::move(logCb)), m_float(std::move(floatCb)) {}

// ─── newGame ──────────────────────────────────────────────────
void TurnManager::newGame(const std::vector<ClassType>& playerClasses,
                          const std::vector<ClassType>& enemyClasses) {
    // Player team: bottom area
    m_playerTeam = std::make_unique<Team>(
        Team::build(playerClasses, true, GRID_ROWS - 3, 2));
    // Enemy team: top area
    m_enemyTeam  = std::make_unique<Team>(
        Team::build(enemyClasses, false, 1, GRID_COLS - 5));

    m_round      = 1;
    m_turnIndex  = 0;
    m_selected   = nullptr;
    m_activeSkill= nullptr;
    m_moveMode   = false;

    buildTurnQueue();
    onTurnStart(*m_turnQueue[0]);
    m_selected = m_turnQueue[0];
}

// ─── buildTurnQueue ───────────────────────────────────────────
void TurnManager::buildTurnQueue() {
    m_turnQueue.clear();
    for (auto& c : m_playerTeam->characters()) m_turnQueue.push_back(&c);
    for (auto& c : m_enemyTeam->characters())  m_turnQueue.push_back(&c);
    // Sort descending by speed
    std::stable_sort(m_turnQueue.begin(), m_turnQueue.end(),
        [](const Character* a, const Character* b){ return a->speed > b->speed; });
}

// ─── Accessors ────────────────────────────────────────────────
Character* TurnManager::currentChar() const {
    if (m_turnQueue.empty()) return nullptr;
    
    // Dùng biến tạm (idx) để duyệt qua hàng đợi, bỏ qua các nhân vật đã chết
    // mà không làm thay đổi biến m_turnIndex gốc của class.
    int idx = m_turnIndex;
    int loops = 0;
    while (!m_turnQueue[idx]->alive) {
        idx = (idx + 1) % static_cast<int>(m_turnQueue.size());
        if (++loops > static_cast<int>(m_turnQueue.size())) return nullptr;
    }
    
    return m_turnQueue[idx];
}

std::vector<Character*> TurnManager::allAlive() {
    std::vector<Character*> all;
    for (auto& c : m_playerTeam->characters()) if (c.alive) all.push_back(&c);
    for (auto& c : m_enemyTeam->characters())  if (c.alive) all.push_back(&c);
    return all;
}

void TurnManager::setSelected(Character* ch) {
    m_selected    = ch;
    m_activeSkill = nullptr;
    m_moveMode    = false;
}

void TurnManager::setActiveSkill(const Skill* sk) {
    m_activeSkill = sk;
    m_moveMode    = false;
}

void TurnManager::setMoveMode(bool on) {
    m_moveMode    = on;
    m_activeSkill = nullptr;
}

// ─── refreshHighlights ────────────────────────────────────────
void TurnManager::refreshHighlights(Grid& grid) {
    grid.clearHighlights();
    Character* cur = currentChar();
    if (!cur || !cur->isPlayer) return;

    auto all = allAlive();

    if (m_moveMode) {
        int maxDist = 4; // movement range in tiles
        grid.highlightMoveRange(*cur, maxDist);
    } else if (m_activeSkill) {
        grid.highlightSkillRange(*cur, *m_activeSkill, all);
    }
}

// ─── onTurnStart ─────────────────────────────────────────────
void TurnManager::onTurnStart(Character& ch) {
    // Tick freeze drain before AP restore
    if (ch.hasEffect(EffectType::FREEZE)) {
        ch.ap = std::max(0, ch.ap - 2);
        m_log("  >> " + ch.label() + " bi dong bang, mat 2 AP",
              {80, 200, 224, 255});
    }
    ch.restoreAP(2);
    ch.tickEffects();
    m_log("--- Luot cua " + ch.label() + " | AP hoi +2 (" +
          std::to_string(ch.ap) + "/" + std::to_string(ch.maxAp) + ") ---",
          {200, 168, 75, 255});
}

// ─── advanceTurn ─────────────────────────────────────────────
void TurnManager::advanceTurn() {
    m_activeSkill = nullptr;
    m_moveMode    = false;

    int sz = static_cast<int>(m_turnQueue.size());
    if (sz == 0) return;

    int next = (m_turnIndex + 1) % sz;
    int loops = 0;
    while (!m_turnQueue[next]->alive) {
        next = (next + 1) % sz;
        if (++loops > sz) return;
    }
    if (next <= m_turnIndex) m_round++;
    m_turnIndex = next;

    Character* ch = m_turnQueue[m_turnIndex];
    onTurnStart(*ch);
    m_selected = ch;
}

// ─── playerMove ───────────────────────────────────────────────
bool TurnManager::playerMove(int toRow, int toCol, Grid& grid) {
    Character* cur = currentChar();
    if (!cur || !cur->isPlayer || !cur->alive) return false;
    if (!cur->canAfford(1)) {
        m_log("Khong du AP de di chuyen!", {213, 58, 58, 255});
        return false;
    }
    auto all = allAlive();
    // Check destination is empty
    for (auto* ch : all)
        if (ch != cur && ch->row == toRow && ch->col == toCol) return false;

    Combat::moveCharacter(*cur, toRow, toCol, grid, all, m_log, m_float);
    m_moveMode = false;
    grid.clearHighlights();
    return true;
}

// ─── playerUseSkill ───────────────────────────────────────────
bool TurnManager::playerUseSkill(int targetRow, int targetCol,
                                  Character* targetChar, Grid& grid) {
    Character* cur = currentChar();
    if (!cur || !cur->isPlayer || !m_activeSkill) return false;
    if (!cur->canAfford(m_activeSkill->apCost)) {
        m_log("Khong du AP!", {213, 58, 58, 255});
        return false;
    }
    auto all = allAlive();
    Combat::resolveSkill(*cur, *m_activeSkill, targetRow, targetCol,
                         targetChar, all, grid, m_log, m_float);
    m_activeSkill = nullptr;
    grid.clearHighlights();
    return true;
}

// ─── playerEndTurn ───────────────────────────────────────────
void TurnManager::playerEndTurn() {
    m_log(currentChar() ? currentChar()->label() + " ket thuc luot." : "Ket thuc luot.",
          {106, 117, 144, 255});
    advanceTurn();
}

// ─── isGameOver ───────────────────────────────────────────────
bool TurnManager::isGameOver(bool& outPlayerWon) const {
    bool pAlive = m_playerTeam->anyAlive();
    bool eAlive = m_enemyTeam->anyAlive();
    if (!pAlive || !eAlive) {
        outPlayerWon = pAlive;
        return true;
    }
    return false;
}

// ─── Save/Load helpers ───────────────────────────────────────
TurnManager::SaveData TurnManager::exportSave() const {
    SaveData sd;
    for (auto& c : m_playerTeam->characters()) sd.playerChars.push_back(c);
    for (auto& c : m_enemyTeam->characters())  sd.enemyChars.push_back(c);
    sd.turnIndex  = m_turnIndex;
    sd.round      = m_round;
    sd.playerTurn = currentChar() ? currentChar()->isPlayer : true;
    return sd;
}

void TurnManager::importSave(const SaveData& sd) {
    m_playerTeam = std::make_unique<Team>(true);
    for (auto& c : sd.playerChars) m_playerTeam->addCharacter(c);
    m_enemyTeam  = std::make_unique<Team>(false);
    for (auto& c : sd.enemyChars)  m_enemyTeam->addCharacter(c);
    m_round = sd.round;
    buildTurnQueue();
    m_turnIndex = sd.turnIndex;
    if (m_turnIndex >= static_cast<int>(m_turnQueue.size()))
        m_turnIndex = 0;
    m_selected = currentChar();
}