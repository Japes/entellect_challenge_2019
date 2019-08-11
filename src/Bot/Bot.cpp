#include "Bot.hpp"
#include "Utilities.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameStateLoader.hpp"
#include "EvaluationFunctions.hpp"
#include <thread>

Bot::Bot(int playthroughDepth, int dirtsForBanana, int distanceForLost, uint64_t mcTime_ns, float mc_c, int mc_runsBeforeClockCheck) :
    _playthroughDepth{playthroughDepth},
    _dirtsForBanana{dirtsForBanana},
    _distanceForLost{distanceForLost},
    _mc_Time_ns{mcTime_ns},
    _mc_c{mc_c},
    _mc_runsBeforeClockCheck{mc_runsBeforeClockCheck}
{
    NextTurn::Initialise();
}

//expects command string to be returned e.g. "dig 5 6"
std::string Bot::runStrategy(rapidjson::Document& roundJSON)
{
    //Setup---------------------------------------------------------------------------
    uint64_t start_time = Utilities::Get_ns_since_epoch();

    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;
    auto state1 = GameStateLoader::LoadGameStatePtr(roundJSON);

    AdjustOpponentBananaCount(ImPlayer1, state1);
    _last_round_state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way


    //do some heuristics---------------------------------------------------------------
    //select
    std::string selectPrefix = NextTurn::TryApplySelect(ImPlayer1, state1);

    //banana mine
    //auto bananaMove = NextTurn::GetBananaProspect(ImPlayer1, state1, _dirtsForBanana);
    //if(bananaMove != nullptr) {
    //    return selectPrefix + bananaMove->GetCommandString();
    //}

    ////heuristic to avoid getting lost
    //auto nearestDirtMove = NextTurn::GetNearestDirtHeuristic(ImPlayer1, state1, _distanceForLost);
    //if(nearestDirtMove != nullptr) {
    //    std::cerr << "(" << __FUNCTION__ << ") USING HEURISTIC TO PREVENT GETTING LOST" << std::endl;
    //    return selectPrefix + nearestDirtMove->GetCommandString();
    //}

    //begin monte carlo----------------------------------------------------------------
    auto mc = std::make_shared<MonteCarlo>(NextTurn::AllValidMovesForPlayer(ImPlayer1, state1, true), _mc_c);
    std::thread t1(&Bot::runMC, this, start_time + _mc_Time_ns, mc, state1, ImPlayer1, _playthroughDepth);
    std::thread t2(&Bot::runMC, this, start_time + _mc_Time_ns, mc, state1, ImPlayer1, _playthroughDepth);
    t1.join();
    t2.join();

    //output result--------------------------------------------------------------------
    //choose the best move and do it
    auto best_move = mc->GetBestMove();
    std::cerr << "JP14:" << std::endl;
    mc->PrintState();

    return selectPrefix + best_move->GetCommandString();
}

void Bot::runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> mc, std::shared_ptr<GameState> state1, bool ImPlayer1, int playthroughDepth)
{
    //int closestDist = Dist_to_closest_enemy();
    //int distToConsider = std::max(closestDist, 7);
    int distToConsider = -1;

    while(Utilities::Get_ns_since_epoch() < stopTime) {

        for(int i = 0; i < _mc_runsBeforeClockCheck; ++i) {
            _mtx.lock();
            //choose next node
            auto next_node = mc->NextNode();
            _mtx.unlock();

            //load the state
            auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int numplies{0};
            auto thisScore = eng.Playthrough(ImPlayer1, next_node->command, nextMoveFn, EvaluationFunctions::HealthComparison, distToConsider, playthroughDepth, numplies);

            _mtx.lock();

            next_node->score += thisScore;
            next_node->w += thisScore;
            ++next_node->n;

            mc->UpdateNumSamples();
            _mtx.unlock();
        }
    }
}

void Bot::AdjustOpponentBananaCount(bool player1, std::shared_ptr<GameState> state1)
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