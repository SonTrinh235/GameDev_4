#pragma once
#include "Character.hpp"
#include <vector>
#include <memory>

// ─── Team ────────────────────────────────────────────────────
// Owns a group of Character objects (4 per team in normal play).
class Team {
public:
    explicit Team(bool isPlayer) : m_isPlayer(isPlayer) {}

    void addCharacter(Character ch);
    void clearAll();

    bool isPlayer() const { return m_isPlayer; }
    bool anyAlive()  const;
    int  aliveCount() const;

    // Access
    std::vector<Character>& characters()             { return m_chars; }
    const std::vector<Character>& characters() const { return m_chars; }

    Character* findById(const std::string& id);
    Character* findAt(int row, int col);

    // Build a team from a list of class types
    static Team build(const std::vector<ClassType>& classes,
                      bool isPlayer,
                      int startRow, int startCol);   // anchor position

private:
    bool                   m_isPlayer;
    std::vector<Character> m_chars;
};