#include "../../include/Entities/Skill.hpp"

std::vector<Skill> Skill::forClass(ClassType ct) {
    std::vector<Skill> skills;
    switch (ct) {
    // ── WARRIOR ─────────────────────────────────────────────
    case ClassType::WARRIOR:
        skills.push_back({ "slash",   "Chem",       2, SkillType::PHYSICAL, 1.2f, 1.5f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Don co ban. Gay 1.2x physDmg." });
        skills.push_back({ "charge",  "Xung Kich",  3, SkillType::PHYSICAL, 1.4f, 3.0f, false, 1.5f,
            true,  false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Lao den ke dich, gay dame + giam Speed 1 luot." });
        skills.push_back({ "warcry",  "Tieng Gam",  4, SkillType::BUFF,     0.0f, 2.0f, true,  2.0f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 6, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Tang +6 Armor cho ban than + dong doi gan 2 luot." });
        skills.push_back({ "sweep",   "Quet Kiem",  5, SkillType::PHYSICAL, 1.0f, 1.5f, true,  1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Gay dame vat ly AOE cho moi ke dich xung quanh." });
        break;

    // ── MAGE ────────────────────────────────────────────────
    case ClassType::MAGE:
        skills.push_back({ "bolt",    "Tia Set",    2, SkillType::MAGIC,    1.0f, 4.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Don phep co ban. Tam 4 o." });
        skills.push_back({ "fireball","Cau Lua",    4, SkillType::MAGIC,    1.8f, 4.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Sat thuong phep lon len 1 muc tieu." });
        skills.push_back({ "freeze",  "Bang Gia",   4, SkillType::MAGIC,    0.7f, 3.0f, false, 1.5f,
            false, true,  false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Gay dame + dong bang: mat 2 AP luot sau." });
        skills.push_back({ "mshield", "Khien Phep", 3, SkillType::BUFF,     0.0f, 0.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, true,  false, false, false, 1.f, 0, false, false,
            "Hap thu 40% dame thanh AP mat trong 1 luot." });
        break;

    // ── ASSASSIN ────────────────────────────────────────────
    case ClassType::ASSASSIN:
        skills.push_back({ "stab",       "Dam Len",   2, SkillType::PHYSICAL, 1.3f, 1.5f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Don vat ly co ban." });
        skills.push_back({ "shadowstep", "Am Bo",     3, SkillType::TELEPORT, 0.0f, 4.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, true,  1.5f, 0, false, false,
            "Dich chuyen ben canh dich, don tiep theo +50% dame." });
        skills.push_back({ "poison",     "Doc Dao",   4, SkillType::PHYSICAL, 1.0f, 1.5f, false, 1.5f,
            false, false, true,  2, 3, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Gay dame + doc 2HP/luot trong 3 luot." });
        skills.push_back({ "execute",    "Ket Lieu",  6, SkillType::PHYSICAL, 2.5f, 1.5f, false, 1.5f,
            false, false, false, 0, 0, true,  0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Sat thuong cuc lon neu HP muc tieu < 30%." });
        break;

    // ── MEDIC ───────────────────────────────────────────────
    case ClassType::MEDIC:
        skills.push_back({ "cleanse",   "Thanh Tay",   2, SkillType::MAGIC,  0.8f, 3.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, true,  false,
            "Tan cong nhe hoac giai debuff cho dong doi." });
        skills.push_back({ "heal",      "Hoi Phuc",    3, SkillType::HEAL,   0.0f, 2.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.5f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Hoi HP = 1.5x magDmg cho dong doi." });
        skills.push_back({ "haste",     "Ban Phuoc",   4, SkillType::BUFF,   0.0f, 2.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.20f, false, false, false, false, 1.f, 0, false, false,
            "Tang 20% Speed cho 1 dong doi 2 luot." });
        skills.push_back({ "massHeal",  "Vong Hoi",    6, SkillType::HEAL,   0.0f, 99.f, true,  99.f,
            false, false, false, 0, 0, false, 0.0f, 1.0f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Hoi HP toan doi (1.0x magDmg moi nguoi)." });
        break;

    // ── PALADIN ─────────────────────────────────────────────
    case ClassType::PALADIN:
        skills.push_back({ "punish",   "Trung Phat",  2, SkillType::PHYSICAL, 1.2f, 1.5f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Don vat ly co ban." });
        skills.push_back({ "divshield","Khien Thanh", 3, SkillType::BUFF,     0.0f, 1.5f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, true,  false, false, 1.f, 0, false, false,
            "Che chan cho dong doi, hap thu 50% dame 1 luot." });
        skills.push_back({ "smite",    "Phan Xet",    4, SkillType::MIXED,    1.5f, 1.5f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Gay dame vat ly + phep hon hop." });
        skills.push_back({ "provoke",  "Khieu Khich", 3, SkillType::DEBUFF,   0.0f, 2.5f, true,  2.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, true,  false, 1.f, 0, false, false,
            "Ep dich xung quanh tap trung vao Paladin 1 luot." });
        break;

    // ── ARCHER ──────────────────────────────────────────────
    case ClassType::ARCHER:
        skills.push_back({ "shoot",     "Ban Ten",    2, SkillType::PHYSICAL, 1.2f, 5.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Ban tam xa, range 5 o." });
        skills.push_back({ "multishot", "Mua Ten",    4, SkillType::PHYSICAL, 0.8f, 4.0f, true,  4.0f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Ban tat ca ke dich trong tam range 4." });
        skills.push_back({ "pierce",    "Xuyen Giap", 3, SkillType::PHYSICAL, 1.5f, 5.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.60f, 1.f, 0, 0.f, false, false, false, false, 1.f, 0, false, false,
            "Bo qua 60% Armor cua muc tieu." });
        skills.push_back({ "trap",      "Bay",        4, SkillType::TRAP,     0.0f, 4.0f, false, 1.5f,
            false, false, false, 0, 0, false, 0.0f, 1.f, 0, 0.f, false, false, false, false, 1.f, 20, false, true,
            "Dat bay o o bat ky. Ke dich buoc vao chiu 20 dame." });
        break;

    default:
        break;
    }
    return skills;
}