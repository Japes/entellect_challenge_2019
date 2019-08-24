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
    auto state1 = GameStateLoader::LoadGameState(roundJSON);

    AdjustOpponentSpellCount(ImPlayer1, &state1, _last_round_state.get());
    _last_round_state = std::make_shared<GameState>(state1); //no idea why it needs to be done this way

    //do some heuristics---------------------------------------------------------------
    //select
    std::string selectPrefix = NextTurn::TryApplySelect(ImPlayer1, &state1);

    //begin monte carlo----------------------------------------------------------------
    auto player1_mc = std::make_shared<MonteCarlo>(NextTurn::AllValidMovesForPlayer(true, &state1, true), _mc_c);
    auto player2_mc = std::make_shared<MonteCarlo>(NextTurn::AllValidMovesForPlayer(false, &state1, true), _mc_c);

    _numplies = 0;
    std::thread t1(&Bot::runMC, this, start_time + _mc_Time_ns, player1_mc, player2_mc, &state1, _playthroughDepth);
    std::thread t2(&Bot::runMC, this, start_time + _mc_Time_ns, player1_mc, player2_mc, &state1, _playthroughDepth);
    t1.join();
    t2.join();

    //output result--------------------------------------------------------------------
    //choose the best move and do it

    auto my_mc =     ImPlayer1 ? player1_mc : player2_mc;
    auto enemy_mc = !ImPlayer1 ? player1_mc : player2_mc;

    auto best_move = my_mc->GetBestMove();
    std::cerr << "JP20:" << std::endl;
    my_mc->PrintState();

    std::cerr << "JP20 Opponent:" << std::endl;
    enemy_mc->PrintState();

    return selectPrefix + best_move->GetCommandString();
}

uint64_t Bot::GetNumPlies()
{
    return _numplies;
}

void Bot::runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> player1_mc, std::shared_ptr<MonteCarlo> player2_mc, GameStatePtr state1, int playthroughDepth)
{
    while(Utilities::Get_ns_since_epoch() < stopTime) {

        for(int i = 0; i < _mc_runsBeforeClockCheck; ++i) {
            _mtx.lock();
            //choose next node
            auto player1_next_node = player1_mc->NextNode();
            auto player2_next_node = player2_mc->NextNode();
            _mtx.unlock();

            //load the state
            GameState state = *state1; //make a copy :/
            GameEngine eng(&state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int numplies{0};

            auto thisScore = eng.Playthrough(player1_next_node->GetCommand(), player2_next_node->GetCommand(),
                                            nextMoveFn, EvaluationFunctions::HealthComparison, playthroughDepth, numplies);
            _numplies += numplies;

            _mtx.lock();
            //remember Playthrough always returns score in terms of player 1
            player1_next_node->AddPlaythroughResult(thisScore);
            player1_mc->UpdateNumSamples();

            player2_next_node->AddPlaythroughResult(1 - thisScore);
            player2_mc->UpdateNumSamples();
            _mtx.unlock();
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
