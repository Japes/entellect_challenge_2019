#include "MonteCarloNode.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"

MonteCarloNode::MonteCarloNode(std::shared_ptr<GameState> state, const EvaluatorBase* eval, int playthroughDepth, float c) :
    _state(state),
    _player1_mc(NextTurn::AllValidMovesForPlayer(true, state.get(), true), c),
    _player2_mc(NextTurn::AllValidMovesForPlayer(false, state.get(), true), c),
    _evaluator(eval),
    _playthroughDepth(playthroughDepth)
{
}

//note this needs to be threadsafe
int64_t MonteCarloNode::AddPlaythrough()
{
    _mtx.lock();
    //choose next node
    auto player1_next_node = _player1_mc.NextNode();
    auto player2_next_node = _player2_mc.NextNode();
    _mtx.unlock();

    //load the state
    GameState state = *(_state.get()); //make a copy :/
    GameEngine eng(&state);

    auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
    int numplies{0};

    auto thisScore = eng.Playthrough(player1_next_node->GetCommand(), player2_next_node->GetCommand(),
                                    nextMoveFn, _evaluator, _playthroughDepth, numplies);

    _mtx.lock();
    //remember Playthrough always returns score in terms of player 1
    player1_next_node->AddPlaythroughResult(thisScore);
    _player1_mc.UpdateNumSamples();

    player2_next_node->AddPlaythroughResult(1 - thisScore);
    _player2_mc.UpdateNumSamples();
    _mtx.unlock();

    return numplies;
}

std::shared_ptr<Command> MonteCarloNode::GetBestMove(bool player1)
{
    PlayersMonteCarlo& pmc = player1 ? _player1_mc : _player2_mc;
    return pmc.GetBestMove();
}

void MonteCarloNode::PrintState(bool player1)
{
    auto my_mc =     player1 ? _player1_mc : _player2_mc;
    auto enemy_mc = !player1 ? _player1_mc : _player2_mc;

    std::cerr << Command::latestBot << ":" << std::endl;
    my_mc.PrintState();

    std::cerr << Command::latestBot << " Opponent:" << std::endl;
    enemy_mc.PrintState();
}
