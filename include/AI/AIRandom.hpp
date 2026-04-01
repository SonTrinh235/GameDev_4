#pragma once
#include "AIInterface.hpp"

// ─── AIRandom (+2đ) ──────────────────────────────────────────
// Picks a random valid skill against a random living enemy.
// Moves toward a random enemy before attacking if AP allows.
class AIRandom : public AIInterface {
public:
    void takeTurn(Character& actor,
                  TurnManager& tm,
                  Grid& grid) override;
};