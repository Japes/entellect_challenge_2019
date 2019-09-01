#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "../MonteCarlo/MCMove.hpp"
#include "../MonteCarlo/PlayersMonteCarlo.hpp"
#include "../MonteCarlo/MonteCarloNode.hpp"
#include "../GameEngine/Evaluators/HealthEvaluator.hpp"


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

        WHEN("We do a playthrough for every branch, we gain a kid each time")
        {
            int dummy;

            int numkids = MCNode.NumChildren();
            for(int i = 0; i < MCNode.NumBranches(); ++i ) {
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumChildren() == ++numkids); //what the, this isn't always true!
            }

            AND_THEN("We do more, we don't gain any more") {
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumChildren() == MCNode.NumBranches());
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumChildren() == MCNode.NumBranches());
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumChildren() == MCNode.NumBranches());
                MCNode.AddPlaythrough(dummy);
                REQUIRE(MCNode.NumChildren() == MCNode.NumBranches());
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

//get that keys are different for different orders (player1, player2)
//check that child nodes only get created when necessary
//test nodedepth
//check for nodedepth 0, must never make children
