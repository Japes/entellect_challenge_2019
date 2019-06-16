#include "catch.hpp"
#include "../GameState/GameState.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "I can make a state instance", "[sanity_state]" ) {
    GameState state;
    for(unsigned x = 0; x < GameConfig::mapSize; ++x) {
        for(unsigned y = 0; y < GameConfig::mapSize; ++y) {
            REQUIRE( state.map[x][x].worm == nullptr );
            REQUIRE( state.map[x][x].powerup == nullptr );
        }
    }
}

TEST_CASE( "GameState deep copy", "[state_deep_copy]" ) {
    GIVEN("A game state file with a dead worm in it")
    {
        auto roundJSON = ReadJsonFile("./Test_files/state_dead_worm.json");
        GameState state(roundJSON);

        THEN("That worm isn't on the map")
        {
            REQUIRE(state.Cell_at({20,25})->worm == nullptr);
        }

        AND_THEN("When we make a copy")
        {
            GameState state2 = state;
            THEN("That worm doesn't appear on the map.")
            {
                REQUIRE(state2.Cell_at({20,25})->worm == nullptr);
            }
        }
    }
}

