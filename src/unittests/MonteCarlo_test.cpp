#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "GameStateLoader.hpp"
#include "Utilities.hpp"
#include "../NextTurn.hpp"
#include "../MonteCarlo/MCMove.hpp"
#include "../MonteCarlo/PlayersMonteCarlo.hpp"
#include "../MonteCarlo/MonteCarloNode.hpp"
#include "../GameEngine/Evaluators.hpp"

TEST_CASE( "Best move", "[BestNode]" ) {
    GIVEN("A bunch of moves passed to a monte carlo")
    {
        std::vector<std::shared_ptr<Command>> moves;
        moves.push_back(std::make_shared<TeleportCommand>(Position(1,1)));
        moves.push_back(std::make_shared<TeleportCommand>(Position(2,2)));
        moves.push_back(std::make_shared<TeleportCommand>(Position(3,3)));

        PlayersMonteCarlo mc(moves, 1);
        auto nodeRef = mc.GetNodes();

        WHEN("We add some playthroughs to the nodes")
        {
            nodeRef[0]->AddPlaythroughResult(1);
            nodeRef[1]->AddPlaythroughResult(0.1);
            nodeRef[1]->AddPlaythroughResult(0.1);
            nodeRef[1]->AddPlaythroughResult(0.1);
            nodeRef[2]->AddPlaythroughResult(1);

            THEN("The one with the most playthroughs is the best (not the one with highest win rate")
            {
                REQUIRE(mc.GetBestMove()->GetCommandString() == "move 2 2");
            }
        }
    }
}

TEST_CASE( "Childnode generation works as I expect - max depth", "[BestNode][branches]" ) {
    GIVEN("A monte carlo node")
    {
        auto state = std::make_shared<GameState>();

        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,4}, state);
        place_worm(player1, 3, {1,7}, state);

        place_worm(!player1, 1, {7,1}, state);
        place_worm(!player1, 2, {7,4}, state);
        place_worm(!player1, 3, {7,7}, state);

        int nodeDepth = GENERATE(1, 2, 3, 54, 450, 161653);
        int playthroughDepth = GENERATE(1, 2, 5, 10, 100);
        MonteCarloNode MCNode(state, Evaluators::Health, nodeDepth, playthroughDepth, 2);

        REQUIRE(MCNode.NumImmediateChildren() == 0);

        WHEN("We do a playthrough")
        {
            int dummy;
            MCNode.AddPlaythrough(dummy);

            THEN("We have gained a child")
            {
                REQUIRE(MCNode.NumImmediateChildren() == 1);
            }
        }

        WHEN("We do a playthrough for MinNumBranches, we gain a kid each time")
        {
            int dummy;

            int numkids = MCNode.NumImmediateChildren();
            for(int i = 0; i < MCNode.MinNumBranches(); ++i ) {
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumImmediateChildren() == ++numkids);
            }
        }
    }

    GIVEN("A monte carlo node with depth 0")
    {
        auto state = std::make_shared<GameState>();

        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,4}, state);
        place_worm(player1, 3, {1,7}, state);

        place_worm(!player1, 1, {7,1}, state);
        place_worm(!player1, 2, {7,4}, state);
        place_worm(!player1, 3, {7,7}, state);

        int nodeDepth = 0;
        int playthroughDepth = GENERATE(1, 2, 5, 10, 100);
        MonteCarloNode MCNode(state, Evaluators::Health, nodeDepth, playthroughDepth, 2);

        REQUIRE(MCNode.NumImmediateChildren() == 0);

        WHEN("We do playthroughs, we never get kids")
        {
            for(int i = 0; i < 100; ++i) {
                int dummy;
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumImmediateChildren() == 0);
            }
        }
    }
}

TEST_CASE( "Childnode generation works as I expect - build tree", "[tree]" ) {
    GIVEN("A monte carlo node with depth < 0")
    {
        //bool player1 = GENERATE(true, false);
        //auto state = std::make_shared<GameState>();
        //place_worm(player1, 1, {1,1}, state);
        //place_worm(player1, 2, {1,4}, state);
        //place_worm(player1, 3, {1,7}, state);
        //place_worm(!player1, 1, {7,1}, state);
        //place_worm(!player1, 2, {7,4}, state);
        //place_worm(!player1, 3, {7,7}, state);

        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapFight.json");
        auto state = GameStateLoader::LoadGameStatePtr(roundJSON);

        int nodeDepth = GENERATE(-1);//, -2, -3, -54161653);
        int playthroughDepth = GENERATE(10);//, 2, 5, 10, 100);
        MonteCarloNode MCNode(state, Evaluators::Health, nodeDepth, playthroughDepth, 1.4f);

        REQUIRE(MCNode.NumImmediateChildren() == 0);

        WHEN("We do playthroughs")
        {
            int dummy;

            THEN("We create a new node each time")
            {
                int numkids = MCNode.TotalNumChildren();
                int depth = MCNode.MaxTreeDepth();

                for(int i = 0; i < 100; ++i ) {
                    MCNode.AddPlaythrough(dummy);
                    REQUIRE( ( (MCNode.TotalNumChildren() > numkids) && (MCNode.MaxTreeDepth() >= depth) ) );
                    numkids = MCNode.TotalNumChildren();
                    depth = MCNode.MaxTreeDepth();

                    /*
                    if(numkids % 1000 == 0) {
                        std::cerr << "(" << __FUNCTION__ << ") depth: " << depth << " numkids: " << numkids << std::endl;
                        MCNode.PrintState(true);
                    }
                    */
                }

                REQUIRE( MCNode.MaxTreeDepth() > 0 );
            }
        }
    }
}


//check that terminal states behave properly


TEST_CASE( "Childnode keys work as I expect", "[BestNode]" ) {
    GIVEN("A map of type childNodeKey_t") {

        std::unordered_map<childNodeKey_t, int> nodes;
        REQUIRE(nodes.size() == 0);

        WHEN("We add a pair of moves")
        {
            auto a = std::make_shared<DoNothingCommand>();
            auto b = std::make_shared<TeleportCommand>(Position(1,1));

            childNodeKey_t key = MonteCarloNode::GetChildKey({a, b});
            nodes[key] = 1;

            THEN("Size increases") {
                REQUIRE(nodes.size() == 1);
            }

            AND_THEN("We add that same thing again") {
                nodes[key] = 3;
                THEN("Size stays the same ") {
                    REQUIRE(nodes.size() == 1);
                }
            }

            AND_THEN("We add another one but with the moves reversed") {
                childNodeKey_t rev_key = MonteCarloNode::GetChildKey({b, a});
                nodes[rev_key] = 1;
                THEN("Size increases again ") {
                    REQUIRE(nodes.size() == 2);
                }
            }
        }
    }
}

TEST_CASE( "Child nodes don't break calculations", "[ChildNodeCalc]" ) {

    GIVEN("Monte carlo nodes with various nodeDepths")
    {
        auto state = std::make_shared<GameState>();

        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,10}, state);
        place_worm(player1, 3, {1,20}, state);

        place_worm(!player1, 1, {20,1}, state);
        place_worm(!player1, 2, {20,10}, state);
        place_worm(!player1, 3, {20,20}, state);

        auto state0 = std::make_shared<GameState>(*state.get());
        auto state1 = std::make_shared<GameState>(*state.get());
        auto state2 = std::make_shared<GameState>(*state.get());

        MonteCarloNode MCNode0(state0, Evaluators::Health, 0, 3, 2); //nodeDepth, playthroughDepth, c
        MonteCarloNode MCNode1(state1, Evaluators::Health, 1, 2, 2);
        MonteCarloNode MCNode2(state2, Evaluators::Health, 2, 1, 2);

        WHEN("We do a playthroughs")
        {
            int dummy;
            auto ret0 = MCNode0.AddPlaythrough(dummy);
            auto ret1 = MCNode1.AddPlaythrough(dummy);
            auto ret2 = MCNode2.AddPlaythrough(dummy);

            THEN("Results are the same")
            {
                REQUIRE(ret0 == ret1);
                REQUIRE(ret1 == ret2);
            }
        }
    }
}

TEST_CASE( "Debug monte carlo", "[.DebugMonteCarlo]" ) {
    GIVEN("A monte carlo node and a contrived game state")
    {

        //    0   1   2   3   4   5   6   7   8
        //0   S   S   S   S   S   S   S   S   5
        //1   .   11  .   D   S   D   .   21  .
        //2   S   S   S   S   s   S   S   S   S
        //3   S   S   S   S   S   S   S   S   S

        
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);

        bool player1 = true;// GENERATE(true, false);
        
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,4}, state);
        place_worm(player1, 3, {1,7}, state);

        place_worm(!player1, 1, {7,1}, state);
        place_worm(!player1, 2, {7,4}, state);
        place_worm(!player1, 3, {7,7}, state);

        state->player1.worms[0].SetProffession(Worm::Proffession::COMMANDO);
        state->player1.worms[1].SetProffession(Worm::Proffession::COMMANDO);
        state->player1.worms[2].SetProffession(Worm::Proffession::COMMANDO);
        state->player2.worms[0].SetProffession(Worm::Proffession::COMMANDO);
        state->player2.worms[1].SetProffession(Worm::Proffession::COMMANDO);
        state->player2.worms[2].SetProffession(Worm::Proffession::COMMANDO);

        state->player1.worms[1].health = -10;
        state->player1.worms[2].health = -10;
        state->player2.worms[1].health = -10;
        state->player2.worms[2].health = -10;

        for(int x = 0; x <= 8; ++x) {
            for(int y = 0; y <= 3; ++y) {
                if(y == 1) {
                    if(x == 3 || x == 5) {
                        state->SetCellTypeAt({x, y}, CellType::DIRT);
                        continue;
                    }
                    if(x != 4) {
                        continue;
                    }
                }

                state->SetCellTypeAt({x, y}, CellType::DEEP_SPACE);
            }
        }

        int nodeDepth = 0;
        int playthroughDepth = 2;
        float c = std::sqrt(2);
        MonteCarloNode MCNode(state, Evaluators::Score, nodeDepth, playthroughDepth, c);

        WHEN("We do playthroughs")
        {
            int dummy;
            for(unsigned i = 0; i < 400; ++i) {
                std::cerr << "(" << __FUNCTION__ << ") DOING A PLAYTHROUGH------------------------" << std::endl;
                MCNode.AddPlaythrough(dummy);
                MCNode.PrintState(player1);
                std::cerr << "(" << __FUNCTION__ << ") FINISHED PLAYTHROUGH------------------------" << std::endl;
                std::cerr << std::endl;
            }
        }

    }
}

TEST_CASE( "TryGetComputedState", "[TryGetComputedState]" ) {
    GIVEN("A monte carlo node with known children")
    {
        auto state = std::make_shared<GameState>();

        bool player1 = true;
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,10}, state);
        place_worm(player1, 3, {1,20}, state);

        place_worm(!player1, 1, {20,1}, state);
        place_worm(!player1, 2, {20,10}, state);
        place_worm(!player1, 3, {20,20}, state);

        MonteCarloNode MCNode(state, Evaluators::Health, 1, 3, 2);

        //after this, should have 8 children with each players 8 moves
        int dummy;
        for(int i = 0; i < 8; ++i) {
            MCNode.AddPlaythrough(dummy);
        }

        WHEN("We try to request with a state it couldn't have")
        {
            auto fakeState = std::make_shared<GameState>();
            ++fakeState->roundNumber;
            fakeState->player1.previousCommand = std::make_shared<DoNothingCommand>();
            fakeState->player2.previousCommand = std::make_shared<DoNothingCommand>();
            auto ret = MCNode.TryGetComputedState(fakeState);

            THEN("It should return null") {
                REQUIRE(ret == nullptr);
            }
        }

        WHEN("We try to request with a state it should have")
        {
            auto fakeState = std::make_shared<GameState>(*state.get());
            auto fakeP1Cmd = std::make_shared<TeleportCommand>(Position(0,0));
            auto fakeP2Cmd = std::make_shared<TeleportCommand>(Position(19,0));
            GameEngine eng(fakeState);
            eng.AdvanceState(*fakeP1Cmd.get(), *fakeP2Cmd.get());

            fakeState->player1.previousCommand = fakeP1Cmd;
            fakeState->player2.previousCommand = fakeP2Cmd;

            THEN("It shouldn't return null") {
                auto ret = MCNode.TryGetComputedState(fakeState);
                REQUIRE(ret != nullptr);
            }

            AND_THEN("We add an error") {

                place_worm(player1, 1, {5,0}, fakeState);

                THEN("It should return null") {
                    auto ret = MCNode.TryGetComputedState(fakeState);
                    REQUIRE(ret == nullptr);
                }
            }
        }
    }
}

TEST_CASE( "Promotion", "[promotion]" ) {
    GIVEN("A monte carlo node with depth 0")
    {
        auto state = std::make_shared<GameState>();
        bool player1 = true;
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,10}, state);
        place_worm(player1, 3, {1,20}, state);

        place_worm(!player1, 1, {20,1}, state);
        place_worm(!player1, 2, {20,10}, state);
        place_worm(!player1, 3, {20,20}, state);

        MonteCarloNode MCNode(state, Evaluators::Health, 0, 3, 2);

        REQUIRE(MCNode.MaxTreeDepth() == 1);
        REQUIRE(MCNode.NumImmediateChildren() == 0);

        WHEN("We promote it")
        {
            MCNode.Promote();
            THEN("It starts generating children")
            {
                int dummy = 0;
                for(int i = 0; i < 100; ++i) {
                    MCNode.AddPlaythrough(dummy);
                }

                REQUIRE(MCNode.MaxTreeDepth() == 2);
                REQUIRE(MCNode.NumImmediateChildren() > 0);
            }

            WHEN("We promote it again")
            {
                MCNode.Promote();
                THEN("Its children start generating children")
                {
                    int dummy = 0;
                    for(int i = 0; i < 100; ++i) {
                        MCNode.AddPlaythrough(dummy);
                    }

                    REQUIRE(MCNode.MaxTreeDepth() == 3);
                    REQUIRE(MCNode.NumImmediateChildren() > 0);
                }
            }
        }
    }
}

/*
   promotion logic
      depth goes up and thats all(?)
      */

//test nodedepth
//  and should behave the same as it used to....
