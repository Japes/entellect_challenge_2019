#include <iostream>
#include <fstream>
#include <string>
#include "Utilities.hpp"
#include "Bot.hpp"

std::string executeRound(Bot& bot, std::string& roundNumber)
{
    const std::string filePath = "./rounds/" + roundNumber + "/state.json";
    try {
        
        rapidjson::Document roundJSON = Utilities::ReadJsonFile(filePath);
        return "C;" + roundNumber + ";" + bot.runStrategy(roundJSON) + "\n";

    } catch(...)        {
        return "C;" + roundNumber + ";error executeRound \n";
    }
}

int main(int argc, char** argv)
{
    unsigned playThroughDepth{24};
    unsigned dirtsForBanana{10};
    uint64_t mcTime_ns{880000000};
    float mc_c{std::sqrt(2)};
    unsigned mc_runsBeforeClockCheck{50};

    Bot bot(playThroughDepth, dirtsForBanana, mcTime_ns, mc_c, mc_runsBeforeClockCheck);

    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(bot, roundNumber);
    }
}
