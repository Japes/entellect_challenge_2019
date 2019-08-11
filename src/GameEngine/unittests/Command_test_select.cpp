#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Select command", "[Select_command]" ) {

    GIVEN("A game state and a select command")
    {
        auto state = std::make_shared<GameState>();

        place_worm(true, 3, {10,10}, state);
        GameEngine eng(state);

        SelectCommand sel(3, std::make_shared<TeleportCommand>(Position(11,11)));

        int numSelectsBefore = state->player1.remainingWormSelections;
        int doNothingsBefore = state->player1.consecutiveDoNothingCount;

        WHEN("We apply a select command")
        {
            eng.AdvanceState(sel, DoNothingCommand());
            REQUIRE(state->player1.consecutiveDoNothingCount == doNothingsBefore);

            THEN("A select is used up for that player") {
                REQUIRE(state->player1.remainingWormSelections == numSelectsBefore -1);
            }
        }
    }
}

TEST_CASE( "Get select move order", "[Select_string]" ) {
    //couldn't get GENERATE working here for some reason
    GIVEN("A select command") {
        std::shared_ptr<Command> cmd = std::make_shared<TeleportCommand>(Position(11,11));
        SelectCommand sel(2, cmd);

        THEN("It's order is the order of it's selected move") {
            REQUIRE(sel.Order() == cmd->Order());
        }
    }
    GIVEN("A select command") {
        std::shared_ptr<Command> cmd = std::make_shared<DigCommand>(Position(11,11));
        SelectCommand sel(2, cmd);

        THEN("It's order is the order of it's selected move") {
            REQUIRE(sel.Order() == cmd->Order());
        }
    }
    GIVEN("A select command") {
        std::shared_ptr<Command> cmd = std::make_shared<BananaCommand>(Position(11,11));
        SelectCommand sel(2, cmd);

        THEN("It's order is the order of it's selected move") {
            REQUIRE(sel.Order() == cmd->Order());
        }
    }
}

TEST_CASE( "Get select string", "[Select_string]" ) {
    auto state = std::make_shared<GameState>();

    SelectCommand sel(2, std::make_shared<DigCommand>(Position(11,11)));
    REQUIRE(sel.GetCommandString() == "select 2;dig 11 11");

    SelectCommand sel1(3, std::make_shared<TeleportCommand>(Position(11,11)));
    REQUIRE(sel1.GetCommandString() == "select 3;move 11 11");
}
