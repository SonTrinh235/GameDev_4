#include "../../include/AI/AIInterface.hpp"
#include "../../include/Logic/TurnManager.hpp"
#include "../../include/Logic/Combat.hpp"
#include <cmath>
#include <algorithm>

std::vector<Character*> AIInterface::getEnemies(Character& actor, TurnManager& tm) {
    std::vector<Character*> res;
    if (actor.isPlayer) {
        for (auto& c : tm.enemyTeam().characters()) if (c.alive) res.push_back(&c);
    } else {
        for (auto& c : tm.playerTeam().characters()) if (c.alive) res.push_back(&c);
    }
    return res;
}

std::vector<Character*> AIInterface::getAllies(Character& actor, TurnManager& tm) {
    std::vector<Character*> res;
    if (!actor.isPlayer) {
        for (auto& c : tm.enemyTeam().characters()) if (c.alive && c.id != actor.id) res.push_back(&c);
    } else {
        for (auto& c : tm.playerTeam().characters()) if (c.alive && c.id != actor.id) res.push_back(&c);
    }
    return res;
}

float AIInterface::distTo(const Character& a, const Character& b) {
    return dist(a.row, a.col, b.row, b.col);
}

int AIInterface::calcPhysDmg(const Character& attacker, const Character& target, const Skill& skill) {
    int d = static_cast<int>(attacker.physDmg * skill.dmgMult);
    if (skill.armorPierce > 0.f) d -= static_cast<int>(target.armor * (1.f - skill.armorPierce));
    else d -= target.armor;
    return std::max(0, d);
}

int AIInterface::calcMagDmg(const Character& attacker, const Character& target, const Skill& skill) {
    return std::max(0, static_cast<int>(attacker.magDmg * skill.dmgMult) - target.magicRes);
}

bool AIInterface::inRange(const Character& user, const Character& target, const Skill& skill) {
    return distTo(user, target) <= skill.range;
}

bool AIInterface::stepToward(Character& actor, const Character& target, TurnManager& tm, Grid& grid) {
    if (!actor.canAfford(1)) return false;

    int dr = 0, dc = 0;
    if (target.row < actor.row) dr = -1; else if (target.row > actor.row) dr = 1;
    if (target.col < actor.col) dc = -1; else if (target.col > actor.col) dc = 1;

    int nr = actor.row + dr, nc = actor.col + dc;
    if (nr < 0 || nr >= GRID_ROWS || nc < 0 || nc >= GRID_COLS) return false;

    auto all = tm.allAlive();
    auto isOccupied = [&](int r, int c) {
        for (auto* ch : all) if (ch != &actor && ch->row == r && ch->col == c) return true;
        return false;
    };

    if (isOccupied(nr, nc)) {
        if (dr != 0 && dc != 0) { 
            if (!isOccupied(actor.row + dr, actor.col)) { nr = actor.row + dr; nc = actor.col; }
            else if (!isOccupied(actor.row, actor.col + dc)) { nr = actor.row; nc = actor.col + dc; }
            else return false; 
        } else {
            return false;
        }
    }

    Combat::moveCharacter(actor, nr, nc, grid, all, tm.getLogCb(), tm.getFloatCb());
    return true;
}