#pragma once
#include "../Core/Common.hpp"
#include "../Entities/Character.hpp"
#include "../Entities/Skill.hpp"
#include "../Map/Grid.hpp"
#include <functional>
#include <vector>
#include <string>

// ─── CombatResult ────────────────────────────────────────────
struct CombatResult {
    bool         targetDied = false;
    int          damageDone = 0;
    int          healDone   = 0;
    std::string  logMsg;
};

// Callback types so Combat can push log/float entries upward
using LogCb   = std::function<void(const std::string&, SDL_Color)>;
using FloatCb = std::function<void(int r, int c, const std::string&, SDL_Color, int)>;

// ─── Combat ──────────────────────────────────────────────────
// Stateless: all methods are static.
// Receives pointers to the characters that exist in TurnManager.
class Combat {
public:
    // Deal physical or magic damage to one target.
    static CombatResult dealDamage(Character& attacker,
                                   Character& target,
                                   float      dmgMult,
                                   SkillType  type,
                                   bool       ignoreShield,
                                   LogCb      logCb,
                                   FloatCb    floatCb);

    // Resolve a full skill use (handles AOE, heal, buff, trap, etc.)
    static void resolveSkill(Character&              user,
                             const Skill&            skill,
                             int                     targetRow,
                             int                     targetCol,
                             Character*              primaryTarget,   // may be nullptr
                             std::vector<Character*> allChars,
                             Grid&                   grid,
                             LogCb                   logCb,
                             FloatCb                 floatCb);

    // Move a character to (row, col), spending AP. Returns AP cost.
    static int  moveCharacter(Character& ch,
                              int        toRow, int toCol,
                              Grid&      grid,
                              std::vector<Character*> allChars,
                              LogCb  logCb,
                              FloatCb floatCb);

private:
    static void applyEffect(Character& target, EffectType type, int turns, int value,
                            LogCb logCb);
    static std::vector<Character*> getCharsInRadius(
        std::vector<Character*>& all, int r, int c, float radius, int teamFilter);
        // teamFilter: 0=enemy only, 1=ally only, -1=all
};