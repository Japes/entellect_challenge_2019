#include "MonteCarloNode.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"

MonteCarloNode::MonteCarloNode(std::shared_ptr<GameState> state, 
                                const std::vector<std::shared_ptr<Command>>& p1_cmds, const std::vector<std::shared_ptr<Command>>& p2_cmds,
                                EvaluationFn_t eval, int nodeDepth, int playthroughDepth, float c) :
    _state(state),
    _player1_mc(p1_cmds, c),
    _player2_mc(p2_cmds, c),
    _evaluator(eval),
    _nodeDepth(nodeDepth),
    _playthroughDepth(playthroughDepth),
    _c{c},
    _terminalNodeEvaluation{-1,-1}
{
    //check if we're a terminal node
    auto res = GameEngine::GetResult(_state.get());

    if(res.result != GameEngine::ResultType::IN_PROGRESS) {
        if(res.winningPlayer == &_state->player1) {
            _terminalNodeEvaluation = {1, 0};
        } else {
            _terminalNodeEvaluation = {0, 1};
        }
    }
}

MonteCarloNode::MonteCarloNode(std::shared_ptr<GameState> state, EvaluationFn_t eval, int nodeDepth, int playthroughDepth, float c) :
                                MonteCarloNode(state, 
                                                NextTurn::AllValidMovesForPlayer(true, state.get(), true),
                                                NextTurn::AllValidMovesForPlayer(false, state.get(), true),
                                                eval, nodeDepth, playthroughDepth, c)
{
}

void MonteCarloNode::Promote()
{
    ++_nodeDepth;
    for (auto& child: _childNodes) {
        child.second->Promote();
    }
}

std::shared_ptr<MonteCarloNode> MonteCarloNode::TryGetComputedState(std::shared_ptr<GameState> state)
{
    auto p1PreviousCmd = state->player1.previousCommand;
    auto p2PreviousCmd = state->player2.previousCommand;

    //can we grab a child and carry on from there?
    //was there an error in the game engine last round
    auto child = GetChild({p1PreviousCmd, p2PreviousCmd});
    if(child == nullptr || !child->StateEquals(state)) {
        return nullptr;
    }
    
    return child;
}

bool MonteCarloNode::StateEquals(std::shared_ptr<GameState> state)
{
    return (*_state.get()) == (*state.get());
}

//note this needs to be threadsafe
std::pair<float, float> MonteCarloNode::AddPlaythrough(int& numplies, bool canMakeChild)
{
    _mtx.lock();
    //choose next node
    MCMove* player1_next_move = _player1_mc.NextMove();
    MCMove* player2_next_move = _player2_mc.NextMove();
    _mtx.unlock();

    /*
    if(_nodeDepth == 0) {
        std::cerr << "\t";
    }
    std::cerr << "(" << __FUNCTION__ << ") depth " << _nodeDepth << 
            " doing playthrough for " << player1_next_move->GetCommand()->GetCommandString() << " " << player2_next_move->GetCommand()->GetCommandString() << std::endl;

    if(_nodeDepth == 0) {
        std::cerr << std::endl;
    }
    */

    auto thisScore = GetScore(player1_next_move->GetCommand(), player2_next_move->GetCommand(), numplies, canMakeChild);

    _mtx.lock();
    //remember Playthrough always returns score in terms of player 1
    player1_next_move->AddPlaythroughResult(thisScore.first);
    _player1_mc.UpdateNumSamples();

    player2_next_move->AddPlaythroughResult(thisScore.second);
    _player2_mc.UpdateNumSamples();

    _mtx.unlock();

    return thisScore;
}

std::pair<float, float> MonteCarloNode::GetScore(std::shared_ptr<Command> p1Cmd, std::shared_ptr<Command> p2Cmd, int& numplies, bool canMakeChild)
{
    if(_terminalNodeEvaluation.first >= 0 || _terminalNodeEvaluation.second >= 0) {
        return _terminalNodeEvaluation; //the game is over
    }

    if (_nodeDepth < 0) { //implies we're building the whole tree
        
        if(!canMakeChild) {
            return DoAPlayout(p1Cmd, p2Cmd, numplies);
        } else {
            bool createdOne{false};
            std::shared_ptr<MonteCarloNode> childNode = GetOrCreateChild({p1Cmd, p2Cmd}, createdOne);
            if(createdOne) {
                return childNode->AddPlaythrough(numplies, false);
            } else {
                ++numplies; //count the one that was precomputed in the child
                return childNode->AddPlaythrough(numplies);
            }
        }

    } else { //we're building up to a max depth
         if (_nodeDepth > 0) {

            bool createdOne{false};
            std::shared_ptr<MonteCarloNode> childNode = GetOrCreateChild({p1Cmd, p2Cmd}, createdOne);
            ++numplies; //count the one that was precomputed in the child
            return childNode->AddPlaythrough(numplies);

        } else {

            return DoAPlayout(p1Cmd, p2Cmd, numplies);
        }
    }
}

std::pair<float, float> MonteCarloNode::DoAPlayout(std::shared_ptr<Command> p1Cmd, std::shared_ptr<Command> p2Cmd, int& numplies)
{
    //load the state
    GameState state = *(_state.get()); //make a copy :/
    GameEngine eng(&state);

    auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);

    return eng.Playthrough(p1Cmd, p2Cmd, nextMoveFn, _evaluator, _playthroughDepth, numplies);
}

std::shared_ptr<MonteCarloNode> MonteCarloNode::GetChild(childNodeID_t id)
{
    childNodeKey_t key = GetChildKey(id);
    if (_childNodes.find(key) == _childNodes.end()) {
        return nullptr;
    }

    return _childNodes[key];
}

std::shared_ptr<MonteCarloNode> MonteCarloNode::GetOrCreateChild(childNodeID_t id, bool& createdNewOne)
{
    childNodeKey_t key = GetChildKey(id);
    auto p1Cmd = id.first;
    auto p2Cmd = id.second;
    createdNewOne = false;

    _mtx.lock();
    if (_childNodes.find(key) == _childNodes.end()) {
        auto child_state = std::make_shared<GameState>(*_state.get()); //make a copy :/
        GameEngine eng(child_state.get());
        eng.AdvanceState(*p1Cmd.get(), *p2Cmd.get());
        _childNodes[key] = std::make_shared<MonteCarloNode>(child_state, _evaluator, _nodeDepth - 1, _playthroughDepth, _c);
        createdNewOne = true;
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

//debug stuff----------------------------------------------------------------------------------

int MonteCarloNode::MinNumBranches()
{
    //max would be these 2 multiplied by each other
    return  std::max(NextTurn::AllValidMovesForPlayer(true, _state.get(), true).size(), 
                    NextTurn::AllValidMovesForPlayer(false, _state.get(), true).size());
}

int MonteCarloNode::NumImmediateChildren()
{
    return _childNodes.size();
}

int MonteCarloNode::TotalNumChildren()
{
    int ret = 0;
    for(auto const child : _childNodes) {
        ++ret;
        ret += child.second->TotalNumChildren();
    }
    return ret;
}

int MonteCarloNode::MaxTreeDepth()
{
    int maxChildDepth = 0;
    for (auto& child: _childNodes) {
        int childDepth = child.second->MaxTreeDepth();
        if(childDepth > maxChildDepth) {
            maxChildDepth = childDepth;
        }
    }

    return maxChildDepth + 1;
}

void MonteCarloNode::PrintState(bool player1)
{
    auto my_mc =     player1 ? _player1_mc : _player2_mc;
    auto enemy_mc = !player1 ? _player1_mc : _player2_mc;

    std::cerr << Command::latestBot << ": mc node with " << NumImmediateChildren() << " children and max depth " << MaxTreeDepth() << std::endl;
    my_mc.PrintState();

/*
    std::cerr << std::endl;
    if(_nodeDepth > 0) {
        std::cerr << Command::latestBot << " KIDS------------:" << std::endl;
        for (auto& child: _childNodes) {
            std::cerr << " KID ----- " << child.first << std::endl;
            child.second->PrintState(player1);
            std::cerr << std::endl;
            //break;
        }
        std::cerr << Command::latestBot << " FINISHED KIDS------------:" << std::endl;
    }
    std::cerr << Command::latestBot << " Opponent:" << std::endl;
    enemy_mc.PrintState();
*/
}
