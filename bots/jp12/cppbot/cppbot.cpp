#include <iostream>
#include "./rapidjson/document.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <cmath>
#include <chrono>
#include <random>
#include <limits>
#include "GameEngine.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "EvaluationFunctions.hpp"
#include "Utilities.hpp"
#include "MonteCarlo.hpp"
#include <thread>
#include <mutex>

uint64_t Get_ns_since_epoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}

int Dist_to_closest_enemy(std::shared_ptr<GameState> state1, bool player1) {
    //get distance to closest enemy
    //pos of my worm:
    Player * me = state1->GetPlayer(player1);
    Worm * worm = me->GetCurrentWorm();
    Position myWormPos = worm->position;

    //now calc
    Player * enemy = state1->GetPlayer(!player1);
    int closestDist = 9999;
    for(auto const & worm : enemy->worms) {
        if(worm.IsDead()) {
            continue;
        }
        auto dist = myWormPos.EuclideanDistanceTo(worm.position);
        if(dist < closestDist){
            closestDist = dist;
        }
    }

    return closestDist;
}

std::mutex mtx;
void runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> mc, std::shared_ptr<GameState> state1, bool ImPlayer1, unsigned playthroughDepth)
{
    //int closestDist = Dist_to_closest_enemy();
    //int distToConsider = std::max(closestDist, 7);
    int distToConsider = -1;

    while(Get_ns_since_epoch() < stopTime) {

        for(unsigned i = 0; i < 50; ++i) {
            mtx.lock();
            //choose next node
            auto next_node = mc->NextNode();
            mtx.unlock();

            //load the state
            auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int numplies{0};
            auto thisScore = eng.Playthrough(ImPlayer1, next_node->command, nextMoveFn, EvaluationFunctions::ScoreComparison, distToConsider, playthroughDepth, numplies);

            mtx.lock();

            next_node->score += thisScore;
            next_node->w += thisScore;
            ++next_node->n;

            mc->UpdateNumSamples();
            mtx.unlock();
        }
    }
}

std::shared_ptr<GameState> _last_round_state{nullptr};

void AdjustOpponentBananaCount(bool player1, std::shared_ptr<GameState> state1)
{
    //try to figure out if the opponent threw a banana (rough heuristic here)
    if(_last_round_state != nullptr) {
        Player* opposingPlayerNow = player1? &state1->player2 : &state1->player1;
        Player* opposingPlayerPreviously = player1? &_last_round_state->player2 : &_last_round_state->player1;

        opposingPlayerNow->worms[2].banana_bomb_count = opposingPlayerPreviously->worms[2].banana_bomb_count;

        if(opposingPlayerNow->worms[2].banana_bomb_count <= 0) {
            opposingPlayerNow->worms[2].banana_bomb_count = 0; //in case it somehow went negative
            return;
        }

        std::cerr << "(" << __FUNCTION__ << ") --------BANANA COUNT: " << opposingPlayerNow->worms[2].banana_bomb_count << std::endl;

        auto opponentScoreDiff = opposingPlayerNow->command_score - opposingPlayerPreviously->command_score;

        int pointsForDig = GameConfig::scores.dig;
        int pointsForShot = GameConfig::commandoWorms.weapon.damage*2;
        int pointsForKillShot = GameConfig::commandoWorms.weapon.damage*2 + GameConfig::scores.killShot;
        if(opponentScoreDiff > pointsForDig && 
            opponentScoreDiff != pointsForShot && 
            opponentScoreDiff != pointsForKillShot && 
            opposingPlayerNow->worms[2].banana_bomb_count > 0) {
            --opposingPlayerNow->worms[2].banana_bomb_count;
            std::cerr << "(" << __FUNCTION__ << ") I RECKON OPPONENT THREW A BANANA----------------COUNT NOW " << opposingPlayerNow->worms[2].banana_bomb_count << std::endl;
        }
    }
}

//expects command string to be returned e.g. "dig 5 6"
std::string runStrategy(rapidjson::Document& roundJSON)
{
    //Setup---------------------------------------------------------------------------
    uint64_t start_time = Get_ns_since_epoch();

    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;
    auto state1 = std::make_shared<GameState>(roundJSON);

    AdjustOpponentBananaCount(ImPlayer1, state1);
    _last_round_state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way

    NextTurn::Initialise();

    //do some heuristics---------------------------------------------------------------
    //banana mine
    auto bananaMove = NextTurn::GetBananaProspect(ImPlayer1, state1, 10);
    if(bananaMove != nullptr) {
        return bananaMove->GetCommandString();
    }
    //heuristic to avoid getting lost
    //int closestEnemy = Dist_to_closest_enemy();


    //select
    std::string selectPrefix = NextTurn::TryApplySelect(ImPlayer1, state1);

    //begin monte carlo----------------------------------------------------------------
    float c = std::sqrt(2);
    auto mc = std::make_shared<MonteCarlo>(NextTurn::AllValidMovesForPlayer(ImPlayer1, state1, true), c);

    //we'll adjust playthrough depth based on how many enemy worms are around us.
    unsigned playthroughDepth = 24;

    std::thread t1(runMC, start_time + 880000000, mc, state1, ImPlayer1, playthroughDepth);
    std::thread t2(runMC, start_time + 880000000, mc, state1, ImPlayer1, playthroughDepth);
    t1.join();
    t2.join();

    //output result--------------------------------------------------------------------
    //choose the best move and do it
    auto best_move = mc->GetBestMove();
    std::cerr << "JP12:" << std::endl;
    mc->PrintState();

    return selectPrefix + best_move->GetCommandString();
}

std::string executeRound(std::string& roundNumber)
{
    const std::string filePath = "./rounds/" + roundNumber + "/state.json";
    try {
        
        rapidjson::Document roundJSON = Utilities::ReadJsonFile(filePath);
        return "C;" + roundNumber + ";" + runStrategy(roundJSON) + "\n";

    } catch(...)        {
        return "C;" + roundNumber + ";error executeRound \n";
    }
}

int main(int argc, char** argv)
{
    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(roundNumber);
    }
}
