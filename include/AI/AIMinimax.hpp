#pragma once
#include "AIInterface.hpp"

// ─── AIMinimax (+3đ) ─────────────────────────────────────────
// Depth-limited minimax with alpha-beta pruning.
// For each possible (skill, target) pair the AI evaluates a
// heuristic score and picks the best action greedily.
// Depth = 2 (one AI action + expected best player counter-action).
class AIMinimax : public AIInterface {
public:
    explicit AIMinimax(int depth = 2) : m_depth(depth) {}

    void takeTurn(Character& actor,
                  TurnManager& tm,
                  Grid& grid) override;

private:
    int m_depth;

    struct Action {
        int   skillIdx   = -1;
        Character* target = nullptr;
        bool  moveFirst   = false;
        int   moveR = 0, moveC = 0;
        int   score       = INT_MIN;
    };

    int evaluate(TurnManager& tm) const;
    int minimax(TurnManager& tmCopy, int depth, int alpha, int beta, bool maximising) const;
    int scoreAction(const Character& actor, int skillIdx,
                    const Character& target, TurnManager& tm) const;
};