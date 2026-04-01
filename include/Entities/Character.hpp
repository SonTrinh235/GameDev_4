#pragma once
#include "../Core/Common.hpp"
#include "Skill.hpp"
#include <vector>
#include <string>

// ─── BaseStats ───────────────────────────────────────────────
struct BaseStats {
    int maxHp    = 100;
    int maxAp    = 8;
    int speed    = 7;
    int armor    = 5;
    int magicRes = 5;
    int physDmg  = 15;
    int magDmg   = 0;

    static BaseStats forClass(ClassType ct);
};

// ─── Character ───────────────────────────────────────────────
class Character {
public:
    // Identity
    std::string  id;
    std::string  name;
    ClassType    classType  = ClassType::WARRIOR;
    bool         isPlayer   = true;
    int          teamIndex  = 0;   // 0-3 within team

    // Position on grid
    int row = 0, col = 0;

    // Stats (runtime, may differ from base due to buffs)
    int hp       = 100;
    int maxHp    = 100;
    int ap       = 8;
    int maxAp    = 8;
    int speed    = 7;
    int armor    = 5;
    int magicRes = 5;
    int physDmg  = 15;
    int magDmg   = 0;

    bool alive   = true;

    // Skills
    std::vector<Skill> skills;

    // Runtime modifiers
    std::vector<Effect> effects;
    bool     shield      = false;       // magic shield active
    bool     divShield   = false;       // paladin divine shield (protector)
    Character* protector = nullptr;     // who is blocking for this char
    Character* taunted   = nullptr;     // forced to attack this target
    float    nextBonus   = 1.0f;        // multiplier for next attack (assassin)

    // ── Factory ──────────────────────────────────────────────
    static Character create(ClassType ct, bool isPlayer, int teamIndex, int row, int col);

    // ── AP management ────────────────────────────────────────
    bool canAfford(int cost) const { return ap >= cost; }
    void spendAP(int cost)         { ap = std::max(0, ap - cost); }
    void restoreAP(int amount)     { ap = std::min(maxAp, ap + amount); }

    // ── Effect management ────────────────────────────────────
    void addEffect(EffectType type, int turns, int value = 0);
    void removeEffect(EffectType type);
    bool hasEffect(EffectType type) const;
    void tickEffects();   // called at start of character's turn

    // ── Status helpers ───────────────────────────────────────
    float hpFrac()  const { return maxHp > 0 ? static_cast<float>(hp) / maxHp : 0.f; }
    float apFrac()  const { return maxAp > 0 ? static_cast<float>(ap) / maxAp : 0.f; }
    bool  isLowHP() const { return hpFrac() < 0.3f; }

    std::string label() const;  // e.g. "[W] Warrior 1"
    SDL_Color   color() const   { return classColor(classType); }
};