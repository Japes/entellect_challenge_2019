#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

int NumLavas(std::shared_ptr<GameState> state)
{
    int ret = 0;
    for(int x = 0; x < GameConfig::mapSize; ++x) {
        for(int y = 0; y < GameConfig::mapSize; ++y) {
            if(state->LavaAt({x,y})) {
                ++ret;
            }
        }
    }
    return ret;
}

TEST_CASE( "Lava flow sanity", "[lava]" ) {
    GIVEN("A game state")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);

        WHEN("It is earlier than battleRoyaleStart, there is no lava")
        {
            while( (state->roundNumber + 1) < GameConfig::maxRounds * GameConfig::battleRoyaleStart) {
                eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
                state->player1.consecutiveDoNothingCount = 0;
                state->player2.consecutiveDoNothingCount = 0;
                INFO("Round number: " << state->roundNumber);
                REQUIRE(NumLavas(state) == 0);
            }

            WHEN("It is between battleRoyaleStart and battleRoyaleEnd, lava grows")
            {
                while(state->roundNumber < GameConfig::maxRounds * GameConfig::battleRoyaleEnd) {
                    int numLavasBefore = NumLavas(state);

                    eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
                    state->player1.consecutiveDoNothingCount = 0;
                    state->player2.consecutiveDoNothingCount = 0;

                    int numLavasAfter = NumLavas(state);

                    INFO("Round number: " << state->roundNumber);
                    REQUIRE(numLavasAfter >= numLavasBefore);
                }

                WHEN("It is after battleRoyaleEnd, lava does not grow")
                {
                    int numLavasBefore = NumLavas(state);
                    while(state->roundNumber < GameConfig::maxRounds) {

                        eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
                        state->player1.consecutiveDoNothingCount = 0;
                        state->player2.consecutiveDoNothingCount = 0;

                        INFO("Round number: " << state->roundNumber);
                        REQUIRE(NumLavas(state) == numLavasBefore);
                    }
                }
            }
        }
    }
}

TEST_CASE( "Lava affects", "[lava]" ) {
    //test effect on health packs
    //test effects on dirt/deep space
    //test damage dealt
    GIVEN("A game state with various things in it")
    {
        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        auto liveWorm = place_worm(true, 1, {1,1}, state);
        auto deadWorm = place_worm(true, 2, {2,1}, state);
        deadWorm->health = -2;
        state->SetCellTypeAt({2, 2}, CellType::DIRT);
        state->SetCellTypeAt({3, 3}, CellType::DEEP_SPACE);
        place_powerup({4,4}, state);

        auto liveWormHealthBefore = liveWorm->health;
        auto deadWormHealthBefore = deadWorm->health;

        WHEN("We make everything lava and progress game state")
        {
            for(int x = 0; x < GameConfig::mapSize; ++x) {
                for(int y = 0; y < GameConfig::mapSize; ++y) {
                    state->AddLavaAt({x,y});
                }
            }
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Lava affects things as expected")
            {
                CHECK(liveWorm->health == liveWormHealthBefore - 3);
                CHECK(deadWorm->health == deadWormHealthBefore);
                CHECK(state->Worm_at({1, 1}) != nullptr);
                CHECK(state->CellType_at({2, 2}) ==  CellType::DIRT);
                CHECK(state->CellType_at({3, 3}) ==  CellType::DEEP_SPACE);
                CHECK(state->PowerUp_at({4, 4}) != nullptr);
                CHECK(state->LavaAt({5, 5}));
            }
        }
    }
}


//test filling up of dug dirt
