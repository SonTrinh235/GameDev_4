#pragma once
#include "AIInterface.hpp"
#include <map>

// ─── AIMCTS (+3đ) ────────────────────────────────────────────
// Monte-Carlo Tree Search.
// For each candidate action we run N random playouts and pick
// the action with the highest average win-rate estimate.
class AIMCTS : public AIInterface {
public:
    explicit AIMCTS(int simulations = 20) : m_sims(simulations) {}

    void takeTurn(Character& actor,
                  TurnManager& tm,
                  Grid& grid) override;

private:
    int m_sims;

    struct ActionNode {
        int   skillIdx    = -1;
        Character* target = nullptr;
        int   wins        = 0;
        int   plays       = 0;
        float winRate() const { return plays > 0 ? static_cast<float>(wins)/plays : 0.f; }
    };

    // Simulate a random playout from the given game state.
    // Returns 1 if enemy (AI) wins, 0 if player wins.
    int simulate(const TurnManager& tm, int actionSkill,
                 Character* actionTarget, int maxDepth = 6) const;

    int  evaluateState(const TurnManager& tm) const;
    bool isTerminal(const TurnManager& tm) const;
};