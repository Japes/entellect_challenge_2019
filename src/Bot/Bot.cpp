#include "Bot.hpp"
#include "Utilities.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameStateLoader.hpp"
#include <thread>

Bot::Bot(EvaluatorBase* evaluator,
        int playthroughDepth, int nodeDepth,
        int dirtsForBanana, int distanceForLost, 
        uint64_t mcTime_ns, float mc_c, int mc_runsBeforeClockCheck) :
    _pattern_p1(8),
    _pattern_p2(8),
    _playthroughDepth{playthroughDepth},
    _nodeDepth{nodeDepth},
    _dirtsForBanana{dirtsForBanana},
    _distanceForLost{distanceForLost},
    _mc_Time_ns{mcTime_ns},
    _mc_c{mc_c},
    _mc_runsBeforeClockCheck{mc_runsBeforeClockCheck},
    _numplies{0},
    _evaluator{evaluator},
    _mc{nullptr}
{
    NextTurn::Initialise();
}

//expects command string to be returned e.g. "dig 5 6"
std::string Bot::runStrategy(rapidjson::Document& roundJSON)
{
    uint64_t start_time = Utilities::Get_ns_since_epoch();

    //Get state
    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;
    auto state_now = GameStateLoader::LoadGameStatePtr(roundJSON);
    AdjustOpponentSpellCount(ImPlayer1, state_now.get(), _last_round_state.get());
    _last_round_state = std::make_shared<GameState>(*state_now.get()); //no idea why it needs to be done this way

    //do some heuristics---------------------------------------------------------------
    //select
    std::string selectPrefix = NextTurn::TryApplySelect(ImPlayer1, state_now.get());    //note this modifies state

    //banana mine
    auto bananaMove = NextTurn::GetBananaProspect(ImPlayer1, state_now.get(), _dirtsForBanana);
    if(bananaMove != nullptr) {
        return selectPrefix + bananaMove->GetCommandString();
    }

    //detect patterns
    _pattern_p1.AddCommand(state_now->player1.previousCommand);
    auto p1pat = _pattern_p1.Prediction();
    if(p1pat != nullptr) {
        std::cerr << "(" << __FUNCTION__ << ") FOUND A PATTERN FOR PLAYER 1 ------------" << std::endl;
    }

    _pattern_p2.AddCommand(state_now->player2.previousCommand);
    auto p2pat = _pattern_p1.Prediction();
    if(p2pat != nullptr) {
        std::cerr << "(" << __FUNCTION__ << ") FOUND A PATTERN FOR PLAYER 2 ------------" << std::endl;
    }

    //begin monte carlo----------------------------------------------------------------
    
    GetNextMC(state_now); //note this must happen AFTER any changes to state...

    _numplies = 0;
    std::thread t1(&Bot::runMC, this, start_time + _mc_Time_ns, _mc);
    std::thread t2(&Bot::runMC, this, start_time + _mc_Time_ns, _mc);
    t1.join();
    t2.join();

    //output result--------------------------------------------------------------------
    //choose the best move and do it
    auto best_move = _mc->GetBestMove(ImPlayer1);
    _mc->PrintState(ImPlayer1);

    return selectPrefix + best_move->GetCommandString();
}

void Bot::GetNextMC(std::shared_ptr<GameState> state_now)
{
    if(_mc != nullptr) {
        _mc = _mc->TryGetComputedState(state_now);
        if(_mc != nullptr) {
            _mc->Promote();
            std::cerr << "(" << __FUNCTION__ << ") GOT A CHILD NODE TO REUSE!-----------------------------------------------------" << std::endl;
            _mc->PrintState(true);
            return;
        }
    }

    _mc = std::make_shared<MonteCarloNode>(state_now, _evaluator, _nodeDepth, _playthroughDepth, _mc_c);
}

uint64_t Bot::GetNumPlies()
{
    return _numplies;
}

void Bot::runMC(uint64_t stopTime, std::shared_ptr<MonteCarloNode> mc)
{
    while(Utilities::Get_ns_since_epoch() < stopTime) {

        for(int i = 0; i < _mc_runsBeforeClockCheck; ++i) {
            int numplies = 0;
            mc->AddPlaythrough(numplies);
            _numplies += numplies;
        }
    }
}

void Bot::AdjustOpponentSpellCount(bool player1, GameStatePtr current_state, GameStatePtr prev_state)
{
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
