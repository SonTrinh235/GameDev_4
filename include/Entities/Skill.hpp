#pragma once
#include "../Core/Common.hpp"

// ─── Skill ───────────────────────────────────────────────────
struct Skill {
    std::string id;
    std::string name;
    int         apCost      = 2;
    SkillType   type        = SkillType::PHYSICAL;
    float       dmgMult     = 1.0f;     // multiplier on physDmg or magDmg
    float       range       = 1.5f;     // Chebyshev / Euclidean radius in tiles
    bool        aoe         = false;
    float       aoeRadius   = 1.5f;

    // Optional flags
    bool        hasSlow         = false;
    bool        hasFreeze       = false;
    bool        hasDot          = false;
    int         dotDmg          = 0;
    int         dotTurns        = 0;
    bool        hasExecute      = false;    // double dmg if target hp < 30 %
    float       armorPierce     = 0.0f;    // fraction of armor ignored [0,1]
    float       healMult        = 1.0f;    // for heal skills
    int         buffArmor       = 0;
    float       buffSpeedPct    = 0.0f;
    bool        selfShield      = false;
    bool        divShield       = false;
    bool        hasTaunt        = false;
    bool        hasNextBonus    = false;
    float       nextBonusMult   = 1.5f;
    int         trapDmg         = 20;
    bool        isTrap          = false;
    bool        cleanse         = false;

    std::string desc;   // shown in UI

    // ── Factory ──────────────────────────────────────────────
    // Returns the 4-skill list for a given class.
    static std::vector<Skill> forClass(ClassType ct);
};