#include "catch.hpp"
#include "../GameState/GameState.hpp"
#include "GameEngineTestUtils.hpp"
#include "Utilities.hpp"

TEST_CASE( "I can make a state instance", "[sanity_state]" ) {
    GameState state;
    for(int x = 0; x < GameConfig::mapSize; ++x) {
        for(int y = 0; y < GameConfig::mapSize; ++y) {
            REQUIRE( state.Worm_at({x,y}) == nullptr );
            REQUIRE( state.PowerUp_at({x,y}) == nullptr );
        }
    }
}

TEST_CASE( "GameState deep copy", "[state_deep_copy]" ) {
    GIVEN("A game state file with a dead worm in it")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/state_dead_worm.json");
        GameState state(roundJSON);

        THEN("That worm isn't on the map")
        {
            REQUIRE(state.Worm_at({20,25}) == nullptr);
        }

        AND_THEN("When we make a copy")
        {
            GameState state2 = state;
            THEN("That worm doesn't appear on the map.")
            {
                REQUIRE(state2.Worm_at({20,25}) == nullptr);
            }
        }
    }
    GIVEN("A game state file with a dead worm in it (for player 2)")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/invalids.json");
        GameState state(roundJSON);

        THEN("That worm isn't on the map")
        {
            REQUIRE(state.Worm_at({18,6}) == nullptr);
        }

        AND_THEN("When we make a copy")
        {
            GameState state2 = state;
            THEN("That worm doesn't appear on the map.")
            {
                REQUIRE(state2.Worm_at({18,6}) == nullptr);
            }
        }
    }
}

TEST_CASE( "Copy constructor", "[copy_constructor]" ) {
    WHEN("We make a copy of a state")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/state22.json");
        auto original_state = std::make_shared<GameState>(roundJSON);
        auto copied_state = std::make_shared<GameState>(*original_state);

        THEN("A true deep-copy happens")
        {
            //reference consistency
            REQUIRE(copied_state.get() != original_state.get());
            REQUIRE(copied_state->player1.state == copied_state.get());
            REQUIRE(copied_state->player1.worms[0].state == copied_state.get());
            REQUIRE(copied_state->player1.GetCurrentWorm() != original_state->player1.GetCurrentWorm());
            for(int i = 0; i < GameConfig::mapSize; ++i) {
                for(int j = 0; j < GameConfig::mapSize; ++j) {
                    INFO("position " << i << ", " << j);
                    auto copied_cell = copied_state->Cell_at({i,j});
                    auto original_cell = original_state->Cell_at({i,j});
                    REQUIRE(copied_cell.type == original_cell.type);

                    if(copied_cell.powerup == nullptr) {
                        REQUIRE(copied_cell.powerup == original_cell.powerup);
                    } else {
                        REQUIRE(original_cell.powerup != nullptr);
                    }

                    if(copied_cell.worm == nullptr) {
                        REQUIRE(copied_cell.worm == original_cell.worm);
                    } else {
                        REQUIRE(copied_cell.worm != original_cell.worm);
                    }
                }
            }

            //spot check on values
            REQUIRE(copied_state->roundNumber == original_state->roundNumber);
            REQUIRE(copied_state->player1.GetCurrentWorm()->id == original_state->player1.GetCurrentWorm()->id);
            REQUIRE(copied_state->player1.GetAverageWormHealth() == original_state->player1.GetAverageWormHealth());
            
            REQUIRE(copied_state->player1.worms[0].IsDead() == original_state->player1.worms[0].IsDead());
            REQUIRE(copied_state->player1.worms[1].IsDead() == original_state->player1.worms[1].IsDead());
            REQUIRE(copied_state->player1.worms[2].IsDead() == original_state->player1.worms[2].IsDead());

            REQUIRE(copied_state->player1.worms[0].health == original_state->player1.worms[0].health);
            REQUIRE(copied_state->player1.worms[1].health == original_state->player1.worms[1].health);
            REQUIRE(copied_state->player1.worms[2].health == original_state->player1.worms[2].health);
        }
    }
}

TEST_CASE( "Get/sets", "[Gamestate_get_set]" ) {
    GIVEN("A game state ")
    {
        GameState state;

        WHEN("We set the type of a cell")
        {
            state.SetCellTypeAt({1,2}, CellType::DIRT);
            state.SetCellTypeAt({2,3}, CellType::AIR);
            state.SetCellTypeAt({3,4}, CellType::DEEP_SPACE);

            THEN("The cell is of that type")
            {
                REQUIRE(state.CellType_at({1,2}) == CellType::DIRT);
                REQUIRE(state.CellType_at({2,3}) == CellType::AIR);
                REQUIRE(state.CellType_at({4,5}) == CellType::AIR);
                REQUIRE(state.CellType_at({3,4}) == CellType::DEEP_SPACE);
            }
        }

        WHEN("We place a powerup on a cell")
        {
            state.PlacePowerupAt({1,2}, 0);

            THEN("That cell has a powerup...")
            {
                REQUIRE(state.PowerUp_at({1,2}) != nullptr);
                REQUIRE(state.PowerUp_at({2,3}) == nullptr);
                REQUIRE(state.PowerUp_at({3,4}) == nullptr);
            }

            AND_THEN("We clear it")
            {
                state.ClearPowerupAt({1,2});
                THEN("its gone")
                {
                    REQUIRE(state.PowerUp_at({1,2}) == nullptr);
                }
            }
        }

        WHEN("We place a worm on a cell")
        {
            state.PlaceWormAt({1,2}, &state.player1.worms[0]);

            THEN("That cell has a worm...")
            {
                REQUIRE(state.Worm_at({1,2}) == &state.player1.worms[0]);
                REQUIRE(state.Worm_at({2,3}) == nullptr);
                REQUIRE(state.Worm_at({3,4}) == nullptr);
            }
        }

    }
}