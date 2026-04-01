#include "../../include/Entities/Character.hpp"
#include <sstream>

BaseStats BaseStats::forClass(ClassType ct) {
    switch (ct) {
    case ClassType::WARRIOR:  return {120, 8, 7,  14, 4,  22, 0 };
    case ClassType::MAGE:     return {70,  8, 7,  3,  14, 0,  32};
    case ClassType::ASSASSIN: return {80,  8, 14, 5,  4,  28, 0 };
    case ClassType::MEDIC:    return {85,  8, 9,  5,  10, 0,  20};
    case ClassType::PALADIN:  return {150, 8, 5,  20, 14, 16, 14};
    case ClassType::ARCHER:   return {85,  8, 10, 5,  4,  26, 0 };
    default:                  return {};
    }
}

Character Character::create(ClassType ct, bool isPlayer, int teamIndex, int row, int col) {
    Character c;
    BaseStats s = BaseStats::forClass(ct);
    c.classType  = ct;
    c.isPlayer   = isPlayer;
    c.teamIndex  = teamIndex;
    c.row        = row;
    c.col        = col;
    c.maxHp      = s.maxHp;   c.hp      = s.maxHp;
    c.maxAp      = s.maxAp;   c.ap      = s.maxAp;
    c.speed      = s.speed;
    c.armor      = s.armor;
    c.magicRes   = s.magicRes;
    c.physDmg    = s.physDmg;
    c.magDmg     = s.magDmg;
    c.alive      = true;
    c.shield     = false;
    c.divShield  = false;
    c.protector  = nullptr;
    c.taunted    = nullptr;
    c.nextBonus  = 1.0f;

    std::ostringstream oss;
    oss << (isPlayer ? "p" : "e") << teamIndex;
    c.id   = oss.str();
    
    // ĐỔI TÊN Ở ĐÂY - Thêm chữ "Enemy"
    c.name = (isPlayer ? "" : "Enemy ") + className(ct) + " " + std::to_string(teamIndex + 1);

    c.skills = Skill::forClass(ct);
    return c;
}

void Character::addEffect(EffectType type, int turns, int value) {
    removeEffect(type);
    Effect e; e.type = type; e.turns = turns; e.value = value;
    effects.push_back(e);
    if (type == EffectType::ARMOR_UP) armor += value;
    if (type == EffectType::HASTE)    speed += value;
    if (type == EffectType::SLOW)     speed  = std::max(1, static_cast<int>(speed * 0.7f));
    if (type == EffectType::SHIELD)   shield = true;
    if (type == EffectType::DIV_SHIELD) divShield = true;
}

void Character::removeEffect(EffectType type) {
    for (auto it = effects.begin(); it != effects.end(); ) {
        if (it->type == type) {
            if (type == EffectType::ARMOR_UP) armor -= it->value;
            if (type == EffectType::HASTE)    speed -= it->value;
            if (type == EffectType::SLOW)     speed  = std::max(1, static_cast<int>(speed / 0.7f));
            if (type == EffectType::SHIELD)   shield = false;
            if (type == EffectType::DIV_SHIELD) divShield = false;
            it = effects.erase(it);
        } else ++it;
    }
}

bool Character::hasEffect(EffectType type) const {
    for (auto& e : effects) if (e.type == type) return true;
    return false;
}

void Character::tickEffects() {
    std::vector<EffectType> expired;
    for (auto& e : effects) {
        if (e.type == EffectType::POISON) {
            hp = std::max(0, hp - e.value);
            if (hp == 0) alive = false;
        }
        e.turns--;
        if (e.turns <= 0) expired.push_back(e.type);
    }
    for (auto t : expired) removeEffect(t);
}

std::string Character::label() const {
    return classEmoji(classType) + " " + name;
}