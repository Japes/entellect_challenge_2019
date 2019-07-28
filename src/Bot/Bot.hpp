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
    Bot(unsigned playthroughDepth, unsigned dirtsForBanana, unsigned distanceForLost, uint64_t mcTime_ns, float mc_c, unsigned mc_runsBeforeClockCheck);

    std::string runStrategy(rapidjson::Document& roundJSON);

    private:

    std::shared_ptr<GameState> _last_round_state{nullptr};
    std::mutex _mtx;

    unsigned _playthroughDepth;
    unsigned _dirtsForBanana;
    unsigned _distanceForLost;
    uint64_t _mc_Time_ns;
    float _mc_c;
    unsigned _mc_runsBeforeClockCheck;

    int Dist_to_closest_enemy(std::shared_ptr<GameState> state1, bool player1);
    void runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> mc, std::shared_ptr<GameState> state1, bool ImPlayer1, unsigned playthroughDepth);
    void AdjustOpponentBananaCount(bool player1, std::shared_ptr<GameState> state1);

};

#endif
