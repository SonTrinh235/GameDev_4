#pragma once
#include "../Logic/TurnManager.hpp"
#include <string>

// ─── SaveSystem ──────────────────────────────────────────────
// Serialises / deserialises TurnManager::SaveData to/from a
// JSON-like text file (data/savegame.json).
// We write the JSON manually to avoid pulling in a third-party
// library — the format is intentionally simple.
class SaveSystem {
public:
    explicit SaveSystem(const std::string& filePath = "data/savegame.json");

    bool hasSave() const;
    bool saveGame(const TurnManager::SaveData& data, AIMode aiMode,
                  const std::vector<ClassType>& playerClasses,
                  const std::vector<ClassType>& enemyClasses,
                  int round);
    bool loadGame(TurnManager::SaveData& outData, AIMode& outAIMode,
                  std::vector<ClassType>& outPlayerClasses,
                  std::vector<ClassType>& outEnemyClasses,
                  int& outRound);
    void deleteSave();

private:
    std::string m_path;

    // Tiny write helpers
    static std::string charToJson(const Character& c);
    static bool        charFromJson(const std::string& block, Character& out);
    static std::string intToStr(int v);
    static int         extractInt(const std::string& json, const std::string& key);
    static std::string extractStr(const std::string& json, const std::string& key);
};