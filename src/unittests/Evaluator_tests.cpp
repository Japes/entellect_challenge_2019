#include "catch.hpp"
#include "../GameEngine/GameEngine.hpp"
#include "../GameEngine/Evaluators/AveHpScoreEvaluator.hpp"
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
        Worm* worm11 = place_worm(player1, 1, {1,1}, state);
        Worm* worm12 = place_worm(player1, 2, {20,1}, state);
        Worm* worm13 = place_worm(player1, 3, {20,2}, state);
        
        Worm* worm21 = place_worm(!player1, 1, {20,3}, state);
        Worm* worm22 = place_worm(!player1, 2, {20,4}, state);
        Worm* worm23 = place_worm(!player1, 3, {20,5}, state);

        state.SetCellTypeAt({2, 1}, CellType::DIRT);

        auto evaluationBefore = eval.Evaluate(player1, &state);
        GameEngine eng(&state);
        eng.AdvanceState(DigCommand({2,1}), DoNothingCommand());
        auto evaluationAfter = eval.Evaluate(player1, &state);

        INFO("evaluationBefore: " << evaluationBefore << " evaluationAfter: " << evaluationAfter);

        WHEN("We work out the evaluation as done in the engine")
        {
            int numPlies = 1;
            float bestPossible = eval.BestPossiblePerPly()*numPlies;
            float frac = (evaluationAfter - evaluationBefore) / bestPossible;
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
