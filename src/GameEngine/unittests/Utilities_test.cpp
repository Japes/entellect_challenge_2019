#include "catch.hpp"
#include "../../Utilities/Utilities.hpp"
#include <iostream>

TEST_CASE( "NormaliseTo", "[NormaliseTo]" ) {
    GIVEN("A bunch of test data that looks like what we used to have")
    {
        int64_t scoreBefore = GENERATE(0, 1234, 2360, 3120);
        int64_t scoreDiff = GENERATE(56, 0, 80, 32, 16, 240);
        int64_t sign = GENERATE(1, -1);
        int64_t scoreAfter = scoreBefore + (scoreDiff*sign);
        WHEN("We call normalize with our old values")
        {
            float bestPossibleScore = 24 * 16; //"should be safe"
            float inputFrac = ( (scoreAfter - scoreBefore) / bestPossibleScore);
            float ret = Utilities::NormaliseTo(inputFrac, 0.25, 0.75f);

            THEN("it behaves like it used to")
            {
                INFO("inputFrac: " << inputFrac);
                REQUIRE(ret == ( (((scoreAfter - scoreBefore) / bestPossibleScore ) / 4) + 0.5f) );
            }
        }
    }
}
