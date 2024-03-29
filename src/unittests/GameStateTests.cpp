#include "catch.hpp"
#include "GameState/GameState.hpp"
#include "GameState/GameStateLoader.hpp"
#include "GameEngineTestUtils.hpp"
#include "Utilities.hpp"
#include "Command.hpp"
#include "GameEngine.hpp"

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
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
        GameState state = GameStateLoader::LoadGameState(roundJSON);

        THEN("That worm isn't on the map")
        {
            REQUIRE(state.Worm_at({29,15}) == nullptr);
        }

        AND_THEN("When we make a copy")
        {
            GameState state2 = state;
            THEN("That worm doesn't appear on the map.")
            {
                REQUIRE(state2.Worm_at({29,15}) == nullptr);
            }
        }
    }
}

TEST_CASE( "Copy constructor", "[copy_constructor]" ) {
    WHEN("We make a copy of a state")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
        auto original_state = GameStateLoader::LoadGameStatePtr(roundJSON);
        auto copied_state = std::make_shared<GameState>(*original_state);

        THEN("A true deep copy happens")
        {
            //reference consistency
            REQUIRE(copied_state.get() != original_state.get());
            REQUIRE(copied_state->player1.state == copied_state.get());
            REQUIRE(copied_state->player1.worms[0].state == copied_state.get());
            REQUIRE(copied_state->player1.GetCurrentWorm() != original_state->player1.GetCurrentWorm());
            for(int i = 0; i < GameConfig::mapSize; ++i) {
                for(int j = 0; j < GameConfig::mapSize; ++j) {
                    INFO("position " << i << ", " << j);
                    Position pos(i,j);
                    REQUIRE(copied_state->CellType_at(pos) == original_state->CellType_at(pos));

                    if(copied_state->PowerUp_at(pos) == nullptr) {
                        REQUIRE(copied_state->PowerUp_at(pos) == original_state->PowerUp_at(pos));
                    } else {
                        REQUIRE(original_state->PowerUp_at(pos) != nullptr);
                    }

                    if(copied_state->Worm_at(pos) == nullptr) {
                        REQUIRE(copied_state->Worm_at(pos) == original_state->Worm_at(pos));
                    } else {
                        REQUIRE(copied_state->Worm_at(pos) != original_state->Worm_at(pos));
                    }

                    REQUIRE(copied_state->LavaAt(pos) == original_state->LavaAt(pos));
                }
            }

            //spot check on values
            REQUIRE(copied_state->roundNumber == original_state->roundNumber);
            REQUIRE(copied_state->player1.GetCurrentWorm()->id == original_state->player1.GetCurrentWorm()->id);
            REQUIRE(copied_state->player1.GetAverageWormHealth() == original_state->player1.GetAverageWormHealth());
            REQUIRE(copied_state->player1.remainingWormSelections == original_state->player1.remainingWormSelections);
            
            REQUIRE(copied_state->player1.worms[0].IsDead() == original_state->player1.worms[0].IsDead());
            REQUIRE(copied_state->player1.worms[1].IsDead() == original_state->player1.worms[1].IsDead());
            REQUIRE(copied_state->player1.worms[2].IsDead() == original_state->player1.worms[2].IsDead());

            REQUIRE(copied_state->player1.worms[0].health == original_state->player1.worms[0].health);
            REQUIRE(copied_state->player1.worms[1].health == original_state->player1.worms[1].health);
            REQUIRE(copied_state->player1.worms[2].health == original_state->player1.worms[2].health);

            REQUIRE(copied_state->player1.previousCommand == original_state->player1.previousCommand);
            REQUIRE(copied_state->player2.previousCommand == original_state->player2.previousCommand);

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
            state.AddLavaAt({5,7});

            THEN("The cell is of that type")
            {
                REQUIRE(state.CellType_at({1,2}) == CellType::DIRT);
                REQUIRE(state.CellType_at({2,3}) == CellType::AIR);
                REQUIRE(state.CellType_at({4,5}) == CellType::AIR);
                REQUIRE(state.CellType_at({3,4}) == CellType::DEEP_SPACE);
                REQUIRE(state.LavaAt({5,7}));
            }
        }

        WHEN("A cell has lava under dirt")
        {
            Position testPos(9,9);
            state.AddLavaAt(testPos);
            state.SetCellTypeAt(testPos, CellType::DIRT);

            AND_THEN("It gets dug")
            {
                auto ptr = std::make_shared<GameState>(state);
                GameEngine eng(ptr);
                place_worm(true, 1, testPos + Position(1,0), ptr);
                eng.AdvanceState(DigCommand(testPos), DoNothingCommand());
                THEN("It is air for one round")
                {
                    REQUIRE(ptr->CellType_at(testPos) == CellType::AIR);
                    REQUIRE(!ptr->LavaAt(testPos));
                }
            }

            AND_THEN("It gets banana'd")
            {
                auto ptr = std::make_shared<GameState>(state);
                GameEngine eng(ptr);
                place_worm(true, 1, testPos + Position(1,0), ptr);
                ptr->player1.worms[0].SetProffession(Worm::Proffession::AGENT);
                eng.AdvanceState(BananaCommand(testPos), DoNothingCommand());
                THEN("It is air for one round")
                {
                    REQUIRE(ptr->CellType_at(testPos) == CellType::AIR);
                    REQUIRE(!ptr->LavaAt(testPos));
                }
            }
        }

        WHEN("We place a powerup on a cell")
        {
            state.PlacePowerupAt({1,2});

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

TEST_CASE( "DirtsBananaWillHit", "[DirtsBananaWillHit]" ) {
    GIVEN("A contrived situation")
    {
        /*
            0   1   2   3   4   5   6   7
        0   S   D   D   .   .   .   .   .
        1   .   .   .   .   .   .   .   .
        2   .   .   .   .   .   .   .   .
        3   .   .   .   .   .   .   .   .
        4   D   D   D   D   D   .   .   .
        5   D   D   D   D   D   .   .   .
        6   D   D   D   D   D   .   .   .
        7   D   D   D   D   D   .   .   .
        8   D   D   D   D   D   .   .   .            
        9   .   .   .   .   .   .   .   .            
        10  .   .   .   .   .   .   .   .            
        */

        auto state = std::make_shared<GameState>();

        state->SetCellTypeAt({0, 0}, CellType::DEEP_SPACE);
        state->SetCellTypeAt({1, 0}, CellType::DIRT);
        state->SetCellTypeAt({2, 0}, CellType::DIRT);

        for(int x = 0; x < 5; ++x) {
            for(int y = 4; y < 9; ++y) {
                state->SetCellTypeAt({x, y}, CellType::DIRT);
            }
        }

        THEN("DirtsBananaWillHit behaves correctly...")
        {
            REQUIRE(state->DirtsBananaWillHit({-1,0}) == 0);
            REQUIRE(state->DirtsBananaWillHit({1,-1}) == 0);

            //check top row
            REQUIRE(state->DirtsBananaWillHit({0,0}) == 0); //deep space
            REQUIRE(state->DirtsBananaWillHit({1,0}) == 2);
            REQUIRE(state->DirtsBananaWillHit({2,0}) == 2);
            REQUIRE(state->DirtsBananaWillHit({4,0}) == 1);
            REQUIRE(state->DirtsBananaWillHit({5,0}) == 0);

            //sweep down
            REQUIRE(state->DirtsBananaWillHit({2,5}) == 12);
            REQUIRE(state->DirtsBananaWillHit({2,6}) == 13);
            REQUIRE(state->DirtsBananaWillHit({2,7}) == 12);
            REQUIRE(state->DirtsBananaWillHit({2,8}) == 9);
            REQUIRE(state->DirtsBananaWillHit({2,9}) == 4);
            REQUIRE(state->DirtsBananaWillHit({2,10}) == 1);
            REQUIRE(state->DirtsBananaWillHit({2,11}) == 0);

            //sweep right
            REQUIRE(state->DirtsBananaWillHit({3,6}) == 12);
            REQUIRE(state->DirtsBananaWillHit({4,6}) == 9);
            REQUIRE(state->DirtsBananaWillHit({5,6}) == 4);
            REQUIRE(state->DirtsBananaWillHit({6,6}) == 1);
            REQUIRE(state->DirtsBananaWillHit({7,6}) == 0);

            //sweep NE
            REQUIRE(state->DirtsBananaWillHit({3,5}) == 11);
            REQUIRE(state->DirtsBananaWillHit({4,4}) == 6);
            REQUIRE(state->DirtsBananaWillHit({5,3}) == 1);
            REQUIRE(state->DirtsBananaWillHit({6,2}) == 0);

        }
    }
}

TEST_CASE( "Closest_dirt", "[Closest_dirt]" ) {
    GIVEN("A contrived situation with NO DIRTS at first")
    {
        /*
            0   1   2   3   4   5   6   7   8
        0   .   .   .   D   .   .   .   .   .
        1   .   .   .   .   .   .   .   D   .
        2   .   .   .   .   .   .   .   .   .   
        3   .   .   .   .   .   .   .   .   .
        4   .   .   .   .   .   .   .   .   .
        5   .   .   .   .   .   .   .   .   .
        6   11  .   .   .   .   .   .   .   .
        7   .   .   .   .   .   .   .   .   .
        8   .   .   .   .   .   .   .   .   .            
        9   .   .   .   .   .   .   .   .   .            
        10  .   .   .   .   .   .   .   .   .            
        */

        auto state = std::make_shared<GameState>();
        Position fromPos(0,6);

        THEN("Closest_dirt behaves correctly...")
        {
            REQUIRE(state->Closest_dirt(fromPos) == Position(-1,-1));
        }

        WHEN("We add the furthest dirt")
        {
            state->SetCellTypeAt({32, 5}, CellType::DIRT);
            THEN("Closest_dirt returns it")
            {
                REQUIRE(state->Closest_dirt(fromPos) == Position(32, 5));
            }
        }

        WHEN("We add some dirts")
        {
            state->SetCellTypeAt({3, 0}, CellType::DIRT);
            state->SetCellTypeAt({7, 1}, CellType::DIRT);

            THEN("Closest_dirt returns the closest")
            {
                REQUIRE(state->Closest_dirt(fromPos) == Position(3,0));
            }

            AND_THEN("We add some closer dirts")
            {
                state->SetCellTypeAt({2, 8}, CellType::DIRT);
                THEN("Closest_dirt returns the closest")
                {
                    REQUIRE(state->Closest_dirt(fromPos) == Position(2,8));
                }
            }
        }

        WHEN("We check things from the opposite side of the map")
        {
            fromPos = Position(32,32);
            state->SetCellTypeAt({0, 0}, CellType::DIRT);
            THEN("Closest_dirt returns it")
            {
                REQUIRE(state->Closest_dirt(fromPos) == Position(0, 0));
            }
        }
    }
}
