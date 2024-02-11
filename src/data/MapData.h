#pragma once

#include "../constants.h"
#include "../FsPacker.h"
#include "../utils.h"

#include <vector>
#include <string>
#include <sstream>

class LevelData {
public:
    virtual std::string toString() = 0;
    virtual std::vector<uint8_t> compile() = 0;
    uint32_t magicNumber = 0;
};

class ScenData : public LevelData {
public:
    ScenData(std::vector<uint8_t> &mpdzBytes, uint32_t &mpdzIndex);
    std::string toString() {
        std::stringstream ss;
        ss << "ScenData { Subdata Count: ";
        ss << std::dec << subScenData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (int i = 0; i < subScenData.size(); i++) {
            auto subCompiled = subScenData.at(i).compile();
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::SCEN_MAGIC_NUM,result);
        return result;
    };
    uint32_t magicNumber = 0;
    std::vector<LevelData> subScenData;
};

class MapData : public LevelData {
public:
    MapData(std::vector<uint8_t> mpdzBytes, bool compressed = true);
    uint32_t magicNumber = Constants::MPDZ_MAGIC_NUM;
    std::string toString() {
        std::stringstream ss;
        ss << "MapData { Subdata Count: ";
        ss << std::dec << subData.size();
        ss << " }";
        return ss.str();
    };
    std::vector<uint8_t> compile() {
        std::vector<uint8_t> result;
        for (int i = 0; i < subData.size(); i++) {
            auto subCompiled = subData.at(i).compile();
            YUtils::appendVector(result,subCompiled);
        }
        result = FsPacker::packInstruction(Constants::MPDZ_MAGIC_NUM,result);
        return result;
    };
    // Others
    ScenData* getScenByBg(int bg);
private:    
    std::vector<LevelData> subData;
};