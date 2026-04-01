#include "../../include/AI/AIMCTS.hpp"
#include "../../include/Logic/TurnManager.hpp"
#include "../../include/Logic/Combat.hpp"
#include <algorithm>
#include <cstdlib>
#include <climits>

bool AIMCTS::isTerminal(const TurnManager& tm) const { return !tm.playerTeam().anyAlive() || !tm.enemyTeam().anyAlive(); }
int AIMCTS::evaluateState(const TurnManager& tm) const {
    if (!tm.enemyTeam().anyAlive())  return 0;  
    if (!tm.playerTeam().anyAlive()) return 1;  
    int eHP = 0, pHP = 0;
    for (auto& c : tm.enemyTeam().characters())  if (c.alive) eHP += c.hp;
    for (auto& c : tm.playerTeam().characters()) if (c.alive) pHP += c.hp;
    return eHP > pHP ? 1 : 0;
}

int AIMCTS::simulate(const TurnManager& tm, int actionSkill, Character* actionTarget, int maxDepth) const {
    auto nolog2   = [](const std::string&, SDL_Color){};
    auto nofloat2 = [](int,int,const std::string&,SDL_Color,int){};
    TurnManager tmCopy(nolog2, nofloat2); TurnManager::SaveData sd = tm.exportSave(); tmCopy.importSave(sd);

    Character* actor = tmCopy.currentChar();
    if (actor && actionSkill >= 0 && actionTarget) {
        Character* target = nullptr;
        for (auto& c : tmCopy.playerTeam().characters()) if (c.id == actionTarget->id) { target = &c; break; }
        if (target) { auto all = tmCopy.allAlive(); Combat::resolveSkill(*actor, actor->skills[actionSkill], target->row, target->col, target, all, *new Grid(), nolog2, nofloat2); }
    }

    for (int d = 0; d < maxDepth; d++) {
        if (isTerminal(tmCopy)) break;
        tmCopy.advanceTurn(); Character* cur = tmCopy.currentChar(); if (!cur) break;
        auto enemies = cur->isPlayer ? std::vector<Character*>() : [&]{ std::vector<Character*> v; for (auto& c : tmCopy.playerTeam().characters()) if (c.alive) v.push_back(&c); return v; }();
        if (enemies.empty()) continue;

        for (int i = 0; i < (int)cur->skills.size(); i++) {
            const Skill& sk = cur->skills[i];
            if (!cur->canAfford(sk.apCost)) continue;
            Character* randTarget = enemies[std::rand() % (int)enemies.size()]; auto all = tmCopy.allAlive(); Grid dummyGrid;
            Combat::resolveSkill(*cur, sk, randTarget->row, randTarget->col, randTarget, all, dummyGrid, nolog2, nofloat2);
            break;
        }
    }
    return evaluateState(tmCopy);
}

void AIMCTS::takeTurn(Character& actor, TurnManager& tm, Grid& grid) {
    auto logCb   = tm.getLogCb();
    auto floatCb = tm.getFloatCb();
    int loopCount = 0;

    while (actor.ap > 0 && loopCount++ < 10) {
        auto enemies = getEnemies(actor, tm);
        if (enemies.empty()) break;

        struct ActionNode { int skillIdx; Character* target; int wins; int plays; };
        std::vector<ActionNode> nodes;

        for (int i = 0; i < (int)actor.skills.size(); i++) {
            const Skill& sk = actor.skills[i];
            if (!actor.canAfford(sk.apCost)) continue;
            if (sk.type == SkillType::HEAL || sk.type == SkillType::BUFF) nodes.push_back({i, &actor, 0, 0});
            else for (auto* e : enemies) if (inRange(actor, *e, sk)) { nodes.push_back({i, e, 0, 0}); break; }
        }

        if (!nodes.empty()) {
            for (int s = 0; s < m_sims; s++) {
                auto& node = nodes[s % (int)nodes.size()];
                int result = simulate(tm, node.skillIdx, node.target, 6);
                node.wins  += result; node.plays++;
            }
            auto& best = *std::max_element(nodes.begin(), nodes.end(), [](const ActionNode& a, const ActionNode& b){
                float wa = a.plays > 0 ? (float)a.wins/a.plays : 0.f; float wb = b.plays > 0 ? (float)b.wins/b.plays : 0.f; return wa < wb;
            });
            auto all = tm.allAlive();
            Combat::resolveSkill(actor, actor.skills[best.skillIdx], best.target->row, best.target->col, best.target, all, grid, logCb, floatCb);
            continue;
        }

        Character* closest = nullptr; float minDist = 9999.0f;
        for (auto* e : enemies) { float d = distTo(actor, *e); if (d < minDist) { minDist = d; closest = e; } }
        if (closest && actor.canAfford(1)) { if (!stepToward(actor, *closest, tm, grid)) break; }
        else break;
    }
}