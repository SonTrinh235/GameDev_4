#include "../../include/AI/AIRandom.hpp"
#include "../../include/Logic/TurnManager.hpp"
#include "../../include/Logic/Combat.hpp"
#include <cstdlib>

void AIRandom::takeTurn(Character& actor, TurnManager& tm, Grid& grid) {
    auto logCb   = tm.getLogCb();
    auto floatCb = tm.getFloatCb();
    int loopCount = 0;
    
    while (actor.ap > 0 && loopCount++ < 10) {
        auto enemies = getEnemies(actor, tm);
        if (enemies.empty()) break;

        struct Option { int idx; Character* target; };
        std::vector<Option> options;
        for (int i = 0; i < (int)actor.skills.size(); i++) {
            const Skill& sk = actor.skills[i];
            if (!actor.canAfford(sk.apCost)) continue;
            if (sk.type == SkillType::HEAL || sk.type == SkillType::BUFF) options.push_back({i, &actor});
            else for (auto* e : enemies) if (inRange(actor, *e, sk)) options.push_back({i, e});
        }

        if (!options.empty()) {
            auto& opt = options[std::rand() % (int)options.size()];
            auto all = tm.allAlive();
            Combat::resolveSkill(actor, actor.skills[opt.idx], opt.target->row, opt.target->col, opt.target, all, grid, logCb, floatCb);
            continue;
        }

        Character* closest = nullptr; float minDist = 9999.f;
        for (auto* e : enemies) { float d = distTo(actor, *e); if (d < minDist) { minDist = d; closest = e; } }
        if (closest && actor.canAfford(1)) { if (!stepToward(actor, *closest, tm, grid)) break; } 
        else break;
    }
}