#include "../../include/Entities/Team.hpp"

void Team::addCharacter(Character ch) {
    m_chars.push_back(std::move(ch));
}

void Team::clearAll() {
    m_chars.clear();
}

bool Team::anyAlive() const {
    for (auto& c : m_chars)
        if (c.alive) return true;
    return false;
}

int Team::aliveCount() const {
    int n = 0;
    for (auto& c : m_chars)
        if (c.alive) n++;
    return n;
}

Character* Team::findById(const std::string& id) {
    for (auto& c : m_chars)
        if (c.id == id) return &c;
    return nullptr;
}

Character* Team::findAt(int row, int col) {
    for (auto& c : m_chars)
        if (c.alive && c.row == row && c.col == col) return &c;
    return nullptr;
}

// Place 4 characters in a 2×2 block anchored at (startRow, startCol).
// Player team: bottom-right area; Enemy team: top-left area.
Team Team::build(const std::vector<ClassType>& classes,
                 bool isPlayer,
                 int startRow, int startCol) {
    Team t(isPlayer);
    // Offsets: 4 characters arranged in 2 columns × 2 rows
    const int offsets[4][2] = {{0,0},{0,2},{1,0},{1,2}};
    for (int i = 0; i < static_cast<int>(classes.size()) && i < 4; i++) {
        int r = startRow + offsets[i][0];
        int c = startCol + offsets[i][1];
        t.addCharacter(Character::create(classes[i], isPlayer, i, r, c));
    }
    return t;
}