#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../GameEngine.hpp"
#include "AllCommands.hpp"

TEST_CASE( "I can make a game engine instance", "[sanity]" ) {
    GameEngine eng;
    REQUIRE( true );
}

TEST_CASE( "Commands are resolved in the right order", "[command_order]" ) {
    //according to the rules:
    //move
    //dig
    //shoot

    //seems to be this order in the engine:
    //dig
    //move
    //shoot
}

TEST_CASE( "Invalid commands are replaced with Do_Nothing", "[invalid_commands]" ) {
    GameState state;

    state.player1.worms[0].position = {10,10};
    GameEngine eng(state);

    //check all command types
    REQUIRE(state.player1.consecutiveDoNothingCount == 0);
    REQUIRE(state.player2.consecutiveDoNothingCount == 0);

    DigCommand dig({1,1});
    eng.AdvanceState(dig,dig);
    
    REQUIRE(state.player1.consecutiveDoNothingCount == 1);
    REQUIRE(state.player2.consecutiveDoNothingCount == 1);
}

TEST_CASE( "Active worms are chosen correctly", "[active_worm]" ) {
    //test when worms have died
}

TEST_CASE( "12 do nothings means disqualified", "[disqualified]" ) {
    //check for player 1 and player 2
}

TEST_CASE( "Points are allocated correctly", "[command_order]" ) {
    //check for each thing mentioned in the rules and confirm game engine concurs
}

TEST_CASE( "Comparison with jave engine", "[comparison]" ) {
    //read a bunch of known state files, moves, and expected outputs generated by their engine.
    //read in those files, apply the moves, and compare result with theirs.
}