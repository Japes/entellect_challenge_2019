#ifndef BOT_H
#define BOT_H

#include "rapidjson/document.h"
#include "GameState.hpp"
#include "MonteCarlo.hpp"

#include <mutex>

#include "Bot.hpp"
class Bot
{
	public:
    Bot(int playthroughDepth, int dirtsForBanana, int distanceForLost, uint64_t mcTime_ns, float mc_c, int mc_runsBeforeClockCheck);

    std::string runStrategy(rapidjson::Document& roundJSON);
    uint64_t GetNumPlies();

    private:

    std::shared_ptr<GameState> _last_round_state{nullptr};
    std::mutex _mtx;

    int _playthroughDepth;
    int _dirtsForBanana;
    int _distanceForLost;
    uint64_t _mc_Time_ns;
    float _mc_c;
    int _mc_runsBeforeClockCheck;
    uint64_t _numplies;

    int Dist_to_closest_enemy(GameStatePtr state1, bool player1);
    void runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> mc, GameStatePtr state1, bool ImPlayer1, int playthroughDepth);
    void AdjustOpponentBananaCount(bool player1, GameStatePtr state1);
};

#endif
