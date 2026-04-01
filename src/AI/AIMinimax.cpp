#include "../../include/AI/AIMinimax.hpp"
#include "../../include/Logic/TurnManager.hpp"
#include "../../include/Logic/Combat.hpp"
#include <algorithm>
#include <climits>

int AIMinimax::evaluate(TurnManager& tm) const {
    int score = 0;
    for (auto& c : tm.enemyTeam().characters()) score += c.alive ? c.hp + c.ap * 3 : -500;
    for (auto& c : tm.playerTeam().characters()) score -= c.alive ? c.hp + c.ap * 3 : -500;
    return score;
}

int AIMinimax::scoreAction(const Character& actor, int skillIdx, const Character& target, TurnManager& tm) const {
    const Skill& sk = actor.skills[skillIdx];
    int score = 0;
    if (sk.type == SkillType::PHYSICAL || sk.type == SkillType::MIXED) {
        int d = std::max(0, static_cast<int>(actor.physDmg * sk.dmgMult) - target.armor);
        score += d; if (target.hp - d <= 0) score += 300; if (target.isLowHP() && sk.hasExecute) score += 200; if (sk.aoe) score += 50;
    }
    if (sk.type == SkillType::MAGIC || sk.type == SkillType::MIXED) {
        int d = std::max(0, static_cast<int>(actor.magDmg * sk.dmgMult) - target.magicRes);
        score += d; if (target.hp - d <= 0) score += 300; if (sk.hasFreeze) score += 40; if (sk.aoe) score += 50;
    }
    if (sk.type == SkillType::HEAL) score += 80; if (sk.type == SkillType::BUFF) score += 40;
    if (sk.type == SkillType::DEBUFF) score += 50; if (sk.hasDot) score += sk.dotDmg * sk.dotTurns; if (sk.hasSlow) score += 20;
    if (sk.apCost >= 5 && target.hp < 15) score -= 30;
    return score;
}

void AIMinimax::takeTurn(Character& actor, TurnManager& tm, Grid& grid) {
    auto logCb   = tm.getLogCb();
    auto floatCb = tm.getFloatCb();
    int loopCount = 0;

    while (actor.ap > 0 && loopCount++ < 10) {
        auto enemies = getEnemies(actor, tm);
        if (enemies.empty()) break;

        struct Candidate { int skillIdx; Character* target; int score; };
        std::vector<Candidate> candidates;

        for (int i = 0; i < (int)actor.skills.size(); i++) {
            const Skill& sk = actor.skills[i];
            if (!actor.canAfford(sk.apCost)) continue;
            
            if (sk.type == SkillType::HEAL || sk.type == SkillType::BUFF) {
                auto allies = getAllies(actor, tm); Character* t = &actor;
                if (!allies.empty()) for (auto* a : allies) if (a->hp < t->hp) t = a;
                candidates.push_back({i, t, scoreAction(actor, i, *t, tm)});
            } else {
                for (auto* e : enemies) if (inRange(actor, *e, sk)) candidates.push_back({i, e, scoreAction(actor, i, *e, tm)});
            }
        }

        if (!candidates.empty()) {
            auto best = *std::max_element(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b){ return a.score < b.score; });
            auto all = tm.allAlive();
            Combat::resolveSkill(actor, actor.skills[best.skillIdx], best.target->row, best.target->col, best.target, all, grid, logCb, floatCb);
            continue; 
        }

        Character* closest = nullptr; float minDist = 9999.0f;
        for (auto* e : enemies) {
            float d = distTo(actor, *e);
            if (d < minDist) { minDist = d; closest = e; }
        }
        if (closest && actor.canAfford(1)) { if (!stepToward(actor, *closest, tm, grid)) break; } 
        else break;
    }
}