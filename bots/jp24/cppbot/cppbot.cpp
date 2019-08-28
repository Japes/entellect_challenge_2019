#include <iostream>
#include <fstream>
#include <string>
#include "Utilities.hpp"
#include "Bot.hpp"
#include "Evaluators/HealthEvaluator.hpp"
#include "Evaluators/ScoreEvaluator.hpp"
#include "Evaluators/AverageHealthEvaluator.hpp"

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
    int playThroughDepth{24};
    int dirtsForBanana{10};
    int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
    uint64_t mcTime_ns{880000000};
    float mc_c{std::sqrt(2)};
    int mc_runsBeforeClockCheck{50};

    AverageHealthEvaluator eval;
    Bot bot(&eval, playThroughDepth, dirtsForBanana, clearSpaceForHeuristic, mcTime_ns, mc_c, mc_runsBeforeClockCheck);

    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(bot, roundNumber);
    }
}
