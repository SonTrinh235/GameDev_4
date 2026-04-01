#include "../../include/Logic/SaveSystem.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

SaveSystem::SaveSystem(const std::string& filePath) : m_path(filePath) {}

bool SaveSystem::hasSave() const {
    std::ifstream f(m_path);
    return f.good();
}

void SaveSystem::deleteSave() {
    std::remove(m_path.c_str());
}

// ── tiny helpers ──────────────────────────────────────────────
std::string SaveSystem::intToStr(int v) { return std::to_string(v); }

static std::string qi(const std::string& k, int v) {
    return "\"" + k + "\":" + std::to_string(v);
}
static std::string qs(const std::string& k, const std::string& v) {
    return "\"" + k + "\":\"" + v + "\"";
}

// Extract integer value for a key from flat JSON substring
int SaveSystem::extractInt(const std::string& json, const std::string& key) {
    std::string tok = "\"" + key + "\":";
    size_t pos = json.find(tok);
    if (pos == std::string::npos) return 0;
    pos += tok.size();
    // skip whitespace
    while (pos < json.size() && json[pos] == ' ') pos++;
    size_t end = pos;
    while (end < json.size() && (std::isdigit(json[end]) || json[end] == '-')) end++;
    return std::stoi(json.substr(pos, end - pos));
}

std::string SaveSystem::extractStr(const std::string& json, const std::string& key) {
    std::string tok = "\"" + key + "\":\"";
    size_t pos = json.find(tok);
    if (pos == std::string::npos) return "";
    pos += tok.size();
    size_t end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

// ── Serialise one Character ───────────────────────────────────
std::string SaveSystem::charToJson(const Character& c) {
    std::ostringstream o;
    o << "{";
    o << qs("id", c.id) << ",";
    o << qs("name", c.name) << ",";
    o << qi("classType", static_cast<int>(c.classType)) << ",";
    o << qi("isPlayer", c.isPlayer ? 1 : 0) << ",";
    o << qi("teamIndex", c.teamIndex) << ",";
    o << qi("row", c.row) << ",";
    o << qi("col", c.col) << ",";
    o << qi("hp", c.hp) << ",";
    o << qi("maxHp", c.maxHp) << ",";
    o << qi("ap", c.ap) << ",";
    o << qi("maxAp", c.maxAp) << ",";
    o << qi("speed", c.speed) << ",";
    o << qi("armor", c.armor) << ",";
    o << qi("magicRes", c.magicRes) << ",";
    o << qi("physDmg", c.physDmg) << ",";
    o << qi("magDmg", c.magDmg) << ",";
    o << qi("alive", c.alive ? 1 : 0);
    o << "}";
    return o.str();
}

// ── Deserialise one Character block ───────────────────────────
bool SaveSystem::charFromJson(const std::string& block, Character& out) {
    out.id        = extractStr(block, "id");
    out.name      = extractStr(block, "name");
    out.classType = static_cast<ClassType>(extractInt(block, "classType"));
    out.isPlayer  = extractInt(block, "isPlayer") != 0;
    out.teamIndex = extractInt(block, "teamIndex");
    out.row       = extractInt(block, "row");
    out.col       = extractInt(block, "col");
    out.hp        = extractInt(block, "hp");
    out.maxHp     = extractInt(block, "maxHp");
    out.ap        = extractInt(block, "ap");
    out.maxAp     = extractInt(block, "maxAp");
    out.speed     = extractInt(block, "speed");
    out.armor     = extractInt(block, "armor");
    out.magicRes  = extractInt(block, "magicRes");
    out.physDmg   = extractInt(block, "physDmg");
    out.magDmg    = extractInt(block, "magDmg");
    out.alive     = extractInt(block, "alive") != 0;
    out.skills    = Skill::forClass(out.classType);
    out.effects.clear();
    out.shield     = false;
    out.divShield  = false;
    out.protector  = nullptr;
    out.taunted    = nullptr;
    out.nextBonus  = 1.0f;
    return !out.id.empty();
}

// ── saveGame ─────────────────────────────────────────────────
bool SaveSystem::saveGame(const TurnManager::SaveData& data, AIMode aiMode,
                          const std::vector<ClassType>& playerClasses,
                          const std::vector<ClassType>& enemyClasses,
                          int round) {
    std::ofstream f(m_path);
    if (!f.is_open()) {
        std::cerr << "SaveSystem: cannot open " << m_path << "\n";
        return false;
    }
    f << "{\n";
    f << "  " << qi("aiMode", static_cast<int>(aiMode)) << ",\n";
    f << "  " << qi("round", round) << ",\n";
    f << "  " << qi("turnIndex", data.turnIndex) << ",\n";

    // Class lists
    f << "  \"playerClasses\":[";
    for (size_t i = 0; i < playerClasses.size(); i++)
        f << static_cast<int>(playerClasses[i]) << (i+1<playerClasses.size()?",":"");
    f << "],\n";

    f << "  \"enemyClasses\":[";
    for (size_t i = 0; i < enemyClasses.size(); i++)
        f << static_cast<int>(enemyClasses[i]) << (i+1<enemyClasses.size()?",":"");
    f << "],\n";

    // Characters
    f << "  \"playerChars\":[\n";
    for (size_t i = 0; i < data.playerChars.size(); i++)
        f << "    " << charToJson(data.playerChars[i])
          << (i+1<data.playerChars.size()?",":"") << "\n";
    f << "  ],\n";

    f << "  \"enemyChars\":[\n";
    for (size_t i = 0; i < data.enemyChars.size(); i++)
        f << "    " << charToJson(data.enemyChars[i])
          << (i+1<data.enemyChars.size()?",":"") << "\n";
    f << "  ]\n";

    f << "}\n";
    return true;
}

// ── loadGame ─────────────────────────────────────────────────
bool SaveSystem::loadGame(TurnManager::SaveData& outData, AIMode& outAIMode,
                          std::vector<ClassType>& outPlayerClasses,
                          std::vector<ClassType>& outEnemyClasses,
                          int& outRound) {
    std::ifstream f(m_path);
    if (!f.is_open()) return false;
    std::ostringstream ss; ss << f.rdbuf();
    std::string json = ss.str();

    outAIMode    = static_cast<AIMode>(extractInt(json, "aiMode"));
    outRound     = extractInt(json, "round");
    outData.turnIndex = extractInt(json, "turnIndex");

    // Parse class arrays
    auto parseClasses = [&](const std::string& key) -> std::vector<ClassType> {
        std::vector<ClassType> out;
        std::string tok = "\"" + key + "\":[";
        size_t pos = json.find(tok);
        if (pos == std::string::npos) return out;
        pos += tok.size();
        size_t end = json.find(']', pos);
        std::string arr = json.substr(pos, end - pos);
        std::istringstream iss(arr);
        std::string num;
        while (std::getline(iss, num, ',')) {
            if (!num.empty()) out.push_back(static_cast<ClassType>(std::stoi(num)));
        }
        return out;
    };
    outPlayerClasses = parseClasses("playerClasses");
    outEnemyClasses  = parseClasses("enemyClasses");

    // Parse character arrays
    auto parseChars = [&](const std::string& key) -> std::vector<Character> {
        std::vector<Character> out;
        std::string tok = "\"" + key + "\":[";
        size_t pos = json.find(tok);
        if (pos == std::string::npos) return out;
        pos += tok.size();
        // Walk through objects
        while (pos < json.size()) {
            size_t start = json.find('{', pos);
            if (start == std::string::npos) break;
            size_t end   = json.find('}', start);
            if (end == std::string::npos) break;
            std::string block = json.substr(start, end - start + 1);
            Character c;
            if (charFromJson(block, c)) out.push_back(c);
            pos = end + 1;
            if (json[pos] == ']') break;
        }
        return out;
    };
    outData.playerChars = parseChars("playerChars");
    outData.enemyChars  = parseChars("enemyChars");

    return true;
}