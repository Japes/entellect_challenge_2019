#include "catch.hpp"
#include "../GameEngine/GameEngine.hpp"
#include "../GameEngine/Evaluators/AveHpScoreEvaluator.hpp"
#include "../GameEngine/Evaluators/MaxHpScoreEvaluator.hpp"
#include "GameEngineTestUtils.hpp"
#include "../Utilities/Utilities.hpp"

TEST_CASE( "AveHpScoreEvaluator", "[AveHpScoreEvaluator][.debug]" ) {
    GIVEN("an AveHpScoreEvaluator and some states")
    {
        AveHpScoreEvaluator eval;

        GameState state;
        /*
            0   1   2   
        0   .   .   .   
        1   .   11  D   
        2   .   .   .   
        */

        bool player1 = GENERATE(true, false);
        place_worm(player1, 1, {1,1}, state);
        place_worm(player1, 2, {20,1}, state);
        place_worm(player1, 3, {20,2}, state);
        
        place_worm(!player1, 1, {20,3}, state);
        place_worm(!player1, 2, {20,4}, state);
        place_worm(!player1, 3, {20,5}, state);

        state.SetCellTypeAt({2, 1}, CellType::DIRT);

        GameEngine eng(&state);
        eng.AdvanceState(DigCommand({2,1}), DoNothingCommand());
        auto evaluationAfter = eval.Evaluate(player1, &state);

        INFO(" evaluationAfter: " << evaluationAfter);

        WHEN("We work out the evaluation as done in the engine")
        {
            float frac = evaluationAfter / eval.BestPossible();
            // clamp scorediff to 0.25 - 0.75
            auto ret = Utilities::NormaliseTo(frac, 0.25, 0.75);

            THEN("It makes sense")
            {
                INFO(ret);
                REQUIRE(false);
            }
        }
    }
}

TEST_CASE( "BananaBonus", "[BananaBonus]" ) {
    GIVEN("an MaxHpScoreEvaluator and some states")
    {
        MaxHpScoreEvaluator eval;

        WHEN("We get the banana bonus")
        {

            THEN("It makes sense")
            {
                float bonusBefore1 = 1000;
                float bonusBefore2 = 1000;
                float bonusBefore3 = 1000;

                for(int round = 1; round <= 400; ++round) {

                    INFO("round: " << round);

                    float bon1 = eval.GetBananaBonus(1, round);
                    float bon2 = eval.GetBananaBonus(2, round);
                    float bon3 = eval.GetBananaBonus(3, round);

                    REQUIRE(bon3 >= bon2);
                    REQUIRE(bon2 >= bon1);

                    REQUIRE(bon1 <= bonusBefore1);
                    REQUIRE(bon2 <= bonusBefore2);
                    REQUIRE(bon3 <= bonusBefore3);

                    if(round == 100) {
                        REQUIRE(bon1 > 0);
                        REQUIRE(bon2 > 0);
                        REQUIRE(bon3 > 0);
                        REQUIRE(bon1 < 80);
                        REQUIRE(bon2 < 80);
                        REQUIRE(bon3 < 80);
                    }

                    if(round >= 350) {
                        REQUIRE(bon1 == 0);
                        REQUIRE(bon2 == 0);
                        REQUIRE(bon3 == 0);
                    } else {
                        REQUIRE(bon1 > 0);
                        REQUIRE(bon2 > 0);
                        REQUIRE(bon3 > 0);
                    }

                    bonusBefore1 = bon1;
                    bonusBefore2 = bon2;
                    bonusBefore3 = bon3;
                }
            }
        }
    }
}
