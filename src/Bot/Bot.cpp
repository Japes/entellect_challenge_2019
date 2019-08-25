#include "Bot.hpp"
#include "Utilities.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameStateLoader.hpp"
#include <thread>

Bot::Bot(int playthroughDepth, int dirtsForBanana, int distanceForLost, uint64_t mcTime_ns, float mc_c, int mc_runsBeforeClockCheck) :
    _playthroughDepth{playthroughDepth},
    _dirtsForBanana{dirtsForBanana},
    _distanceForLost{distanceForLost},
    _mc_Time_ns{mcTime_ns},
    _mc_c{mc_c},
    _mc_runsBeforeClockCheck{mc_runsBeforeClockCheck},
    _numplies{0}
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

    AdjustOpponentSpellCount(ImPlayer1, state1.get(), _last_round_state.get());
    _last_round_state = std::make_shared<GameState>(*state1.get()); //no idea why it needs to be done this way

    //do some heuristics---------------------------------------------------------------
    //select
    std::string selectPrefix = NextTurn::TryApplySelect(ImPlayer1, state1.get());

    //begin monte carlo----------------------------------------------------------------
    auto mc = std::make_shared<MonteCarloNode>(state1, &_evaluator, _playthroughDepth, _mc_c);

    _numplies = 0;
    std::thread t1(&Bot::runMC, this, start_time + _mc_Time_ns, mc);
    std::thread t2(&Bot::runMC, this, start_time + _mc_Time_ns, mc);
    t1.join();
    t2.join();

    //output result--------------------------------------------------------------------
    //choose the best move and do it
    auto best_move = mc->GetBestMove(ImPlayer1);
    mc->PrintState(ImPlayer1);

    return selectPrefix + best_move->GetCommandString();
}

uint64_t Bot::GetNumPlies()
{
    return _numplies;
}

void Bot::runMC(uint64_t stopTime, std::shared_ptr<MonteCarloNode> mc)
{
    while(Utilities::Get_ns_since_epoch() < stopTime) {

        for(int i = 0; i < _mc_runsBeforeClockCheck; ++i) {
            _numplies += mc->AddPlaythrough();
        }
    }
}

void Bot::AdjustOpponentSpellCount(bool player1, GameStatePtr current_state, GameStatePtr prev_state)
{
    //try to figure out if the opponent threw a banana (rough heuristic here)
    if(prev_state != nullptr) {
        
        Player* opposingPlayerNow = player1? &current_state->player2 : &current_state->player1;
        Player* opposingPlayerPreviously = player1? &prev_state->player2 : &prev_state->player1;

        Command::CommandType prevCommand = static_cast<Command::CommandType>(opposingPlayerNow->previousCommand->Order());

        opposingPlayerNow->worms[1].banana_bomb_count = opposingPlayerPreviously->worms[1].banana_bomb_count;
        opposingPlayerNow->worms[2].snowball_count = opposingPlayerPreviously->worms[2].snowball_count;

        if(prevCommand == Command::CommandType::BANANA && opposingPlayerNow->worms[1].banana_bomb_count > 0) {
            --opposingPlayerNow->worms[1].banana_bomb_count;
        } else if(prevCommand == Command::CommandType::SNOWBALL && opposingPlayerNow->worms[2].snowball_count > 0) {
            --opposingPlayerNow->worms[2].snowball_count;            
        }
    }
}
