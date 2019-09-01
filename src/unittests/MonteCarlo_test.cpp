#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "../MonteCarlo/MCMove.hpp"
#include "../MonteCarlo/PlayersMonteCarlo.hpp"
#include "../MonteCarlo/MonteCarloNode.hpp"
#include "../GameEngine/Evaluators/HealthEvaluator.hpp"
#include "../GameEngine/Evaluators/ScoreEvaluator.hpp"


TEST_CASE( "Best move", "[BestNode]" ) {
    GIVEN("A bunch of moves passed to a monte carlo")
    {
        std::vector<std::shared_ptr<MCMove>> moves;
        auto node1 = std::make_shared<MCMove>(std::make_shared<TeleportCommand>(Position(1,1)));
        auto node2 = std::make_shared<MCMove>(std::make_shared<TeleportCommand>(Position(2,2)));
        auto node3 = std::make_shared<MCMove>(std::make_shared<TeleportCommand>(Position(3,3)));
        moves.push_back(node1);
        moves.push_back(node2);
        moves.push_back(node3);

        PlayersMonteCarlo mc(moves, 1);

        WHEN("We add some playthroughs to the nodes")
        {
            node1->AddPlaythroughResult(1);
            node2->AddPlaythroughResult(0.1);
            node2->AddPlaythroughResult(0.1);
            node2->AddPlaythroughResult(0.1);
            node3->AddPlaythroughResult(1);

            THEN("The one with the most playthroughs is the best (not the one with highest win rate")
            {
                REQUIRE(mc.GetBestMove()->GetCommandString() == "move 2 2");
            }
        }
    }
}

TEST_CASE( "Childnode generation works as I expect", "[BestNode][branches]" ) {
    GIVEN("A monte carlo node")
    {
        auto state = std::make_shared<GameState>();

        bool player1 = true;
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {1,4}, state);
        place_worm(player1, 3, {1,7}, state);

        place_worm(!player1, 1, {7,1}, state);
        place_worm(!player1, 2, {7,4}, state);
        place_worm(!player1, 3, {7,7}, state);

        HealthEvaluator eval;
        MonteCarloNode MCNode(state, &eval, 1, 6, 2);

        REQUIRE(MCNode.NumChildren() == 0);


        WHEN("We do a playthrough")
        {
            int dummy;
            MCNode.AddPlaythrough(dummy);

            THEN("We have gained a child")
            {
                REQUIRE(MCNode.NumChildren() == 1);
            }
        }

        WHEN("We do a playthrough for MinNumBranches, we gain a kid each time")
        {
            int dummy;

            int numkids = MCNode.NumChildren();
            for(int i = 0; i < MCNode.MinNumBranches(); ++i ) {
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumChildren() == ++numkids);
            }
        }
    }
}

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

TEST_CASE( "Debug monte carlo", "[DebugMonteCarlo]" ) {
    GIVEN("A monte carlo node and a contrived game state")
    {

        //    0   1   2   3   4   5   6   7   8
        //0   S   .   S   S   S   S   S   .   5
        //1   S   11  D   S   S   S   D   21  S
        //2   S   S   S   S   s   S   S   S   S
        //3   S   .   S   S   S   S   S   .   S
        //4   S   12  D   S   S   S   D   22  S
        //5   S   S   S   S   S   S   S   S   S
        //6   S   .   S   S   S   S   S   .   S
        //7   S   13  D   S   S   S   D   23  S
        //8   S   S   S   S   S   S   S   S   S
        
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

        for(int x = 0; x <= 8; ++x) {
            for(int y = 0; y <= 8; ++y) {

                if(x == 2 || x == 6) {
                    if(y == 1 || y == 4 || y == 7) {
                        state->SetCellTypeAt({x, y}, CellType::DIRT);
                        continue;
                    }
                }

                if(x == 1 || x == 7) {
                    if(y == 0 || y == 3 || y == 6 || y == 1 || y == 4 || y == 7) {
                        continue;
                    }
                }

                state->SetCellTypeAt({x, y}, CellType::DEEP_SPACE);
            }
        }

        ScoreEvaluator eval;
        int nodeDepth = 1;
        int playthroughDepth = 6;
        float c = 2;
        MonteCarloNode MCNode(state, &eval, nodeDepth, playthroughDepth, c);

        WHEN("We do 8 playthroughs")
        {
            int dummy;
            for(unsigned i = 0; i < 5; ++i) {
                MCNode.AddPlaythrough(dummy);
            }

        }

    }
}


//test nodedepth
//check for nodedepth 0, must never make children
//  and should behave the same as it used to....
