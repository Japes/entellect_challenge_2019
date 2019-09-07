#include <iostream>
#include <fstream>
#include <string>
#include "Utilities.hpp"
#include "Bot.hpp"
#include "Evaluators.hpp"
#include "NextTurn.hpp"

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

EvaluationFn_t GetEvaluator(bool player1, GameStatePtr state)
{
    Player* me = state->GetPlayer(player1);
    Player* opponent = state->GetPlayer(!player1);

    //int numLiveWormsMe = 0;
    //state->ForAllLiveWorms(player1, [&](Worm& worm) { ++numLiveWormsMe; });

    if(state->GetHealthPackPos().size() > 0 && me->GetCurrentWorm()->id == 1) {
        std::cerr << "(" << __FUNCTION__ << ") WORM RUSHING-------------" << std::endl;
        return Evaluators::RushHealth;
    }

    if( state->roundNumber > 260 && (me->GetScore() > opponent->GetScore()) ) {
        std::cerr << "(" << __FUNCTION__ << ") DANCE MODE-------------" << std::endl;
        return Evaluators::Dance;
    }

    std::cerr << "(" << __FUNCTION__ << ") MaxHpScore MODE-------------" << std::endl;
    return Evaluators::MaxHpScore;
}

int main(int argc, char** argv)
{
    //note total play depth is nodeDepth + playThroughDepth
    int playThroughDepth{11};
    int nodeDepth{1};

    int dirtsForBanana{13}; //only if super awesome
    int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
    bool patternDetectEnable{false};

    uint64_t mcTime_ns{880000000};

    float mc_c{std::sqrt(2)};
  
    int mc_runsBeforeClockCheck{50};

    Bot bot(GetEvaluator,
            playThroughDepth, nodeDepth, 
            dirtsForBanana, clearSpaceForHeuristic, patternDetectEnable, NextTurn::WormIsntFrozen,
            mcTime_ns, mc_c, mc_runsBeforeClockCheck);

    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(bot, roundNumber);
    }
}
