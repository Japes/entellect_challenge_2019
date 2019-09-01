#ifndef BOT_H
#define BOT_H

#include "rapidjson/document.h"
#include "GameState.hpp"
#include "PlayersMonteCarlo.hpp"
#include "Evaluators/EvaluatorBase.hpp"
#include "MonteCarloNode.hpp"

#include <mutex>

class Bot
{
	public:
    Bot(EvaluatorBase* evaluator, 
        int playthroughDepth, int nodeDepth, 
        int dirtsForBanana, int distanceForLost, 
        uint64_t mcTime_ns, float mc_c, int mc_runsBeforeClockCheck);

    std::string runStrategy(rapidjson::Document& roundJSON);
    uint64_t GetNumPlies();
    void AdjustOpponentSpellCount(bool player1, GameStatePtr current_state, GameStatePtr prev_state);

    private:

    std::shared_ptr<GameState> _last_round_state{nullptr};
    std::mutex _mtx;

    int _playthroughDepth;
    int _nodeDepth;
    int _dirtsForBanana;
    int _distanceForLost;
    uint64_t _mc_Time_ns;
    float _mc_c;
    int _mc_runsBeforeClockCheck;
    uint64_t _numplies;

    EvaluatorBase* _evaluator;


    int Dist_to_closest_enemy(GameStatePtr state1, bool player1);
    void runMC(uint64_t stopTime, std::shared_ptr<MonteCarloNode> mc);
};

#endif
