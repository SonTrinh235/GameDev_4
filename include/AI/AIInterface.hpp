#pragma once
#include "../Core/Common.hpp"
#include "../Entities/Character.hpp"
#include "../Map/Grid.hpp"
#include <functional>
#include <vector>

class TurnManager;

// ─── AIAction ────────────────────────────────────────────────
struct AIAction {
    enum class Kind { MOVE, SKILL, END_TURN } kind = Kind::END_TURN;
    int         targetRow  = 0;
    int         targetCol  = 0;
    Character*  targetChar = nullptr;
    int         skillIndex = -1;  // index into character's skills[]
};

// ─── AIInterface ─────────────────────────────────────────────
// All AI variants implement this interface.
class AIInterface {
public:
    virtual ~AIInterface() = default;

    // Decide and execute one full turn for `actor`.
    // Returns when the turn is over (all AP spent or END_TURN chosen).
    virtual void takeTurn(Character&              actor,
                          TurnManager&            tm,
                          Grid&                   grid) = 0;

protected:
    // Shared helpers available to all AI implementations.
    std::vector<Character*> getEnemies(Character& actor, TurnManager& tm);
    std::vector<Character*> getAllies(Character&  actor, TurnManager& tm);
    float                   distTo(const Character& a, const Character& b);
    int                     calcPhysDmg(const Character& attacker,
                                        const Character& target,
                                        const Skill& skill);
    int                     calcMagDmg(const Character& attacker,
                                       const Character& target,
                                       const Skill& skill);
    bool                    inRange(const Character& user,
                                    const Character& target,
                                    const Skill& skill);
    // Move actor one step toward `target` if possible
    bool                    stepToward(Character& actor,
                                       const Character& target,
                                       TurnManager& tm,
                                       Grid& grid);
};