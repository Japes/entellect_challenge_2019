#include "MonteCarloNode.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"

MonteCarloNode::MonteCarloNode(std::shared_ptr<GameState> state, const EvaluatorBase* eval, int nodeDepth, int playthroughDepth, float c) :
    _state(state),
    _player1_mc(NextTurn::AllValidMovesForPlayer(true, state.get(), true), c),
    _player2_mc(NextTurn::AllValidMovesForPlayer(false, state.get(), true), c),
    _evaluator(eval),
    _nodeDepth(nodeDepth),
    _playthroughDepth(playthroughDepth),
    _c{c}
{
}

//note this needs to be threadsafe
float MonteCarloNode::AddPlaythrough(int& numplies)
{
    _mtx.lock();
    //choose next node
    std::shared_ptr<MCMove> player1_next_move = _player1_mc.NextMove();
    std::shared_ptr<MCMove> player2_next_move = _player2_mc.NextMove();
    _mtx.unlock();

    numplies = 0;
    float thisScore;
    if (_nodeDepth > 0) {
        std::shared_ptr<MonteCarloNode> childNode = GetOrCreateChild({player1_next_move->GetCommand(), player2_next_move->GetCommand()});
        thisScore = childNode->AddPlaythrough(numplies);
    } else {

        //load the state
        GameState state = *(_state.get()); //make a copy :/
        GameEngine eng(&state);

        auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);

        thisScore = eng.Playthrough(player1_next_move->GetCommand(), player2_next_move->GetCommand(),
                                        nextMoveFn, _evaluator, _playthroughDepth, numplies);
    }
    

    _mtx.lock();
    //remember Playthrough always returns score in terms of player 1
    player1_next_move->AddPlaythroughResult(thisScore);
    _player1_mc.UpdateNumSamples();

    player2_next_move->AddPlaythroughResult(1 - thisScore);
    _player2_mc.UpdateNumSamples();
    _mtx.unlock();

    return thisScore;
}

std::shared_ptr<MonteCarloNode> MonteCarloNode::GetOrCreateChild(childNodeID_t id)
{
    childNodeKey_t key = GetChildKey(id);
    auto p1Cmd = id.first;
    auto p2Cmd = id.second;

    _mtx.lock();
    if (_childNodes.find(key) == _childNodes.end()) {
        auto child_state = std::make_shared<GameState>(*_state.get()); //make a copy :/
        GameEngine eng(child_state.get());
        eng.AdvanceState(*p1Cmd.get(), *p2Cmd.get());
        _childNodes[key] = std::make_shared<MonteCarloNode>(child_state, _evaluator, _nodeDepth - 1, _playthroughDepth - 1, _c);
    }
    auto ret = _childNodes[key];
    _mtx.unlock();

    return ret;
}

childNodeKey_t MonteCarloNode::GetChildKey(childNodeID_t id)
{
    return  id.first->GetCommandString() + id.second->GetCommandString();
}

std::shared_ptr<Command> MonteCarloNode::GetBestMove(bool player1)
{
    PlayersMonteCarlo& pmc = player1 ? _player1_mc : _player2_mc;
    return pmc.GetBestMove();
}

int MonteCarloNode::NumBranches()
{
    return NextTurn::AllValidMovesForPlayer(true, _state.get(), true).size() + NextTurn::AllValidMovesForPlayer(false, _state.get(), true).size();
}

int MonteCarloNode::NumChildren()
{
    return _childNodes.size();
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
