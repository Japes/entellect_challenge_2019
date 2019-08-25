#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "../MCMove.hpp"
#include "../MonteCarlo.hpp"

TEST_CASE( "Best node", "[BestNode]" ) {
    GIVEN("A bunch of nodes passed to a monte carlo")
    {
        std::vector<std::shared_ptr<MCMove>> nodes;
        auto node1 = std::make_shared<MCMove>(std::make_shared<TeleportCommand>(Position(1,1)));
        auto node2 = std::make_shared<MCMove>(std::make_shared<TeleportCommand>(Position(2,2)));
        auto node3 = std::make_shared<MCMove>(std::make_shared<TeleportCommand>(Position(3,3)));
        nodes.push_back(node1);
        nodes.push_back(node2);
        nodes.push_back(node3);

        MonteCarlo mc(nodes, 1);

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
