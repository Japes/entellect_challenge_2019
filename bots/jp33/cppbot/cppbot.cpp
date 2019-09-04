#include <iostream>
#include <fstream>
#include <string>
#include "Utilities.hpp"
#include "Bot.hpp"
#include "Evaluators/MaxHpScoreEvaluator.hpp"

std::string executeRound(Bot& bot, std::string& roundNumber)
{
    const std::string filePath = "./rounds/" + roundNumber + "/state.json";
    try {
        
        rapidjson::Document roundJSON = Utilities::ReadJsonFile(filePath);
        return "C;" + roundNumber + ";" + bot.runStrategy(roundJSON) + "\n";

    } catch(...) {
        return "C;" + roundNumber + ";error executeRound \n";
    }
}

int main(int argc, char** argv)
{
    //note total play depth is nodeDepth + playThroughDepth
    int playThroughDepth{11};
    int nodeDepth{1};

    int dirtsForBanana{12}; //only if super awesome
    int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
    bool patternDetectEnable{false};

    uint64_t mcTime_ns{880000000};
    float mc_c{std::sqrt(2)};
    int mc_runsBeforeClockCheck{50};

    MaxHpScoreEvaluator eval;
    Bot bot(&eval, 
            playThroughDepth, nodeDepth, 
            dirtsForBanana, clearSpaceForHeuristic, patternDetectEnable,
            mcTime_ns, mc_c, mc_runsBeforeClockCheck);

    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(bot, roundNumber);
    }
}
