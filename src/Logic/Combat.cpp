#include "../../include/Logic/Combat.hpp"
#include <algorithm>
#include <sstream>
#include <cmath>

// ─── Internal helpers ─────────────────────────────────────────
static void killCharacter(Character& ch, FloatCb floatCb, LogCb logCb) {
    ch.alive = false;
    ch.hp    = 0;
    floatCb(ch.row, ch.col, "DEAD", {213, 58, 58, 255}, 20);
    logCb("[X] " + ch.label() + " bi tieu diet!", {213, 58, 58, 255});
}

static std::vector<Character*> charsInRadius(
    std::vector<Character*>& all, int r, int c, float radius,
    int teamFilter,   // -1=all, 0=player, 1=enemy
    bool isUserPlayer)
{
    std::vector<Character*> result;
    for (auto* ch : all) {
        if (!ch->alive) continue;
        if (dist(ch->row, ch->col, r, c) > radius) continue;
        if (teamFilter == 0 && ch->isPlayer != isUserPlayer) continue;
        if (teamFilter == 1 && ch->isPlayer == isUserPlayer) continue;
        result.push_back(ch);
    }
    return result;
}

// ─── dealDamage ───────────────────────────────────────────────
CombatResult Combat::dealDamage(Character& attacker,
                                Character& target,
                                float      dmgMult,
                                SkillType  type,
                                bool       ignoreShield,
                                LogCb      logCb,
                                FloatCb    floatCb) {
    CombatResult res;
    if (!target.alive) return res;

    int raw = 0;
    if (type == SkillType::PHYSICAL || type == SkillType::MIXED) {
        int phys = static_cast<int>(attacker.physDmg * dmgMult);
        raw += std::max(0, phys - target.armor);
    }
    if (type == SkillType::MAGIC || type == SkillType::MIXED) {
        int mag = static_cast<int>(attacker.magDmg * dmgMult);
        raw += std::max(0, mag - target.magicRes);
    }

    int finalDmg = raw;

    // Magic shield: absorb 40% into AP
    if (target.shield && !ignoreShield) {
        int absorbed = static_cast<int>(finalDmg * 0.4f);
        target.ap    = std::max(0, target.ap - absorbed / 5);
        finalDmg    -= absorbed;
        logCb("  >> " + target.label() + ": Khien hap thu " + std::to_string(absorbed) + " dame",
              {58, 181, 120, 255});
    }

    // Protector (divine shield): split 50% to protector
    if (target.protector && target.protector->alive && !ignoreShield) {
        int half = finalDmg / 2;
        target.protector->hp = std::max(0, target.protector->hp - half);
        floatCb(target.protector->row, target.protector->col,
                "-" + std::to_string(half), {224, 192, 80, 255}, 16);
        if (target.protector->hp == 0)
            killCharacter(*target.protector, floatCb, logCb);
        finalDmg -= half;
    }

    target.hp = std::max(0, target.hp - finalDmg);
    res.damageDone = finalDmg;

    SDL_Color floatColor = finalDmg > 25
        ? SDL_Color{255, 60, 60, 255}
        : SDL_Color{224, 80, 80, 255};
    floatCb(target.row, target.col, "-" + std::to_string(finalDmg), floatColor, finalDmg > 25 ? 20 : 16);

    std::ostringstream oss;
    oss << attacker.label() << " -> " << target.label()
        << ": -" << finalDmg << " HP (con " << target.hp << ")";
    logCb(oss.str(), {213, 100, 100, 255});

    if (target.hp <= 0) {
        killCharacter(target, floatCb, logCb);
        res.targetDied = true;
    }
    return res;
}

// ─── resolveSkill ─────────────────────────────────────────────
void Combat::resolveSkill(Character&              user,
                          const Skill&            skill,
                          int                     targetRow,
                          int                     targetCol,
                          Character*              primaryTarget,
                          std::vector<Character*> allChars,
                          Grid&                   grid,
                          LogCb                   logCb,
                          FloatCb                 floatCb) {
    if (!user.canAfford(skill.apCost)) {
        logCb("Khong du AP!", {213, 58, 58, 255});
        return;
    }
    user.spendAP(skill.apCost);

    float bonus = user.nextBonus;
    user.nextBonus = 1.0f;

    logCb(user.label() + " dung [" + skill.name + "] (" + std::to_string(skill.apCost) + " AP)",
          {200, 168, 75, 255});

    // ── TRAP ────────────────────────────────────────────────
    if (skill.isTrap) {
        grid.placeTrap(targetRow, targetCol, skill.trapDmg, user.isPlayer);
        logCb("  >> Bay dat tai (" + std::to_string(targetRow) + "," +
              std::to_string(targetCol) + ")", {128, 192, 80, 255});
        return;
    }

    // ── TELEPORT (Assassin Am Bo) ────────────────────────────
    if (skill.type == SkillType::TELEPORT && primaryTarget) {
        // Move user adjacent to target
        const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
        for (auto& d : dirs) {
            int nr = primaryTarget->row + d[0];
            int nc = primaryTarget->col + d[1];
            if (nr >= 0 && nr < GRID_ROWS && nc >= 0 && nc < GRID_COLS) {
                bool occupied = false;
                for (auto* ch : allChars)
                    if (ch->alive && ch->row == nr && ch->col == nc) { occupied = true; break; }
                if (!occupied) {
                    user.row = nr;
                    user.col = nc;
                    break;
                }
            }
        }
        user.nextBonus = skill.nextBonusMult;
        logCb("  >> " + user.label() + " dich chuyen! Don tiep x" +
              std::to_string(skill.nextBonusMult), {80, 224, 144, 255});
        return;
    }

    // ── HEAL ────────────────────────────────────────────────
    if (skill.type == SkillType::HEAL) {
        std::vector<Character*> targets;
        if (skill.aoe) {
            for (auto* ch : allChars)
                if (ch->alive && ch->isPlayer == user.isPlayer) targets.push_back(ch);
        } else if (primaryTarget && primaryTarget->alive) {
            targets.push_back(primaryTarget);
        }
        for (auto* t : targets) {
            int healAmt = static_cast<int>(user.magDmg * skill.healMult);
            t->hp = std::min(t->maxHp, t->hp + healAmt);
            floatCb(t->row, t->col, "+" + std::to_string(healAmt), {58, 181, 120, 255}, 18);
            logCb("  >> " + t->label() + " hoi +" + std::to_string(healAmt) + " HP",
                  {58, 181, 120, 255});
        }
        return;
    }

    // ── BUFF ────────────────────────────────────────────────
    if (skill.type == SkillType::BUFF) {
        // Self magic shield
        if (skill.selfShield) {
            user.addEffect(EffectType::SHIELD, 1, 0);
            logCb("  >> " + user.label() + " kich hoat Khien Phep", {80, 200, 224, 255});
            return;
        }
        // Armor buff (AOE)
        if (skill.buffArmor > 0) {
            auto targets = charsInRadius(allChars, user.row, user.col,
                                         skill.range, 0, user.isPlayer);
            targets.push_back(&user);
            for (auto* t : targets) {
                t->addEffect(EffectType::ARMOR_UP, 2, skill.buffArmor);
                logCb("  >> " + t->label() + " tang +" + std::to_string(skill.buffArmor) + " Armor",
                      {200, 168, 75, 255});
            }
            return;
        }
        // Speed buff
        if (skill.buffSpeedPct > 0.0f && primaryTarget) {
            int bonus2 = static_cast<int>(primaryTarget->speed * skill.buffSpeedPct);
            primaryTarget->addEffect(EffectType::HASTE, 2, bonus2);
            logCb("  >> " + primaryTarget->label() + " tang Speed +" + std::to_string(bonus2),
                  {80, 200, 224, 255});
            return;
        }
        // Divine shield (Paladin)
        if (skill.divShield && primaryTarget && primaryTarget->isPlayer == user.isPlayer) {
            primaryTarget->protector = &user;
            user.addEffect(EffectType::DIV_SHIELD, 1, 0);
            logCb("  >> " + user.label() + " che chan cho " + primaryTarget->label(),
                  {224, 192, 80, 255});
            return;
        }
        return;
    }

    // ── DEBUFF (Taunt) ───────────────────────────────────────
    if (skill.type == SkillType::DEBUFF && skill.hasTaunt) {
        auto enemies = charsInRadius(allChars, user.row, user.col,
                                     skill.range, 1, user.isPlayer);
        for (auto* e : enemies) {
            e->taunted = &user;
            e->addEffect(EffectType::TAUNTED, 1, 0);
        }
        logCb("  >> " + user.label() + " khieu khich " + std::to_string(enemies.size()) + " ke dich",
              {224, 192, 80, 255});
        return;
    }

    // ── PHYSICAL / MAGIC / MIXED (single or AOE) ─────────────
    std::vector<Character*> targets;
    if (skill.aoe) {
        targets = charsInRadius(allChars, targetRow, targetCol,
                                skill.aoeRadius, 1, user.isPlayer);
    } else if (primaryTarget && primaryTarget->alive) {
        targets.push_back(primaryTarget);
    }

    for (auto* t : targets) {
        float totalMult = skill.dmgMult * bonus;

        // Execute bonus
        if (skill.hasExecute && t->isLowHP()) totalMult *= 2.0f;

        // Armor pierce: reduce effective armor
        int savedArmor = t->armor;
        if (skill.armorPierce > 0.0f)
            t->armor = static_cast<int>(t->armor * (1.0f - skill.armorPierce));

        dealDamage(user, *t, totalMult, skill.type, false, logCb, floatCb);

        // Restore armor after calc
        t->armor = savedArmor;

        // Apply secondary effects
        if (skill.hasSlow)   t->addEffect(EffectType::SLOW,   1, 0);
        if (skill.hasFreeze) t->addEffect(EffectType::FREEZE, 1, 2);
        if (skill.hasDot)    t->addEffect(EffectType::POISON, skill.dotTurns, skill.dotDmg);
    }
}

// ─── moveCharacter ────────────────────────────────────────────
int Combat::moveCharacter(Character&              ch,
                          int                     toRow, int toCol,
                          Grid&                   grid,
                          std::vector<Character*> allChars,
                          LogCb                   logCb,
                          FloatCb                 floatCb) {
    int dx = std::max(std::abs(toRow - ch.row), std::abs(toCol - ch.col));
    int apCost = std::max(1, (dx + 1) / 2);

    ch.row = toRow;
    ch.col = toCol;
    ch.spendAP(apCost);

    logCb(ch.label() + " di chuyen -> (" + std::to_string(toRow) + "," +
          std::to_string(toCol) + ") [-" + std::to_string(apCost) + " AP]",
          {58, 123, 213, 255});

    // Check trap
    int trapDmg = 0;
    if (grid.checkTrap(toRow, toCol, ch.isPlayer, trapDmg)) {
        ch.hp = std::max(0, ch.hp - trapDmg);
        floatCb(toRow, toCol, "BAY! -" + std::to_string(trapDmg), {213, 58, 58, 255}, 18);
        logCb("  >> " + ch.label() + " dinh bay! -" + std::to_string(trapDmg) + " HP",
              {213, 58, 58, 255});
        if (ch.hp == 0) killCharacter(ch, floatCb, logCb);
    }
    return apCost;
}