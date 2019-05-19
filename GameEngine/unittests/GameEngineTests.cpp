#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"

TEST_CASE( "I can make a game engine instance", "[sanity]" ) {
    GameEngine eng;
    REQUIRE( true );
}

void CheckWorm(Worm* worm, int id, int health, Position pos, Position last_pos, int moveRange, int digRange, int weaponDmg, int weaponRange)
{
    CHECK(worm->id == id);
    if(health > 0) {
        CHECK(worm->health == health);
    } else {
        CHECK(worm->health <= 0);
    }
    CHECK(worm->position == pos);
    CHECK(worm->previous_position == last_pos); //unknown from this file - should be set equal to currentPosition
    CHECK(worm->movementRange == moveRange);
    CHECK(worm->diggingRange == digRange);

    CHECK(worm->weapon.damage == weaponDmg);
    CHECK(worm->weapon.range == weaponRange);
    int readRange = worm->weapon.range;
    int expectedDiagRange = std::ceil(std::sqrt((readRange*readRange)/2));
    CHECK(worm->weapon.diagRange == expectedDiagRange);
}

void CheckCellEmpty(Cell* cell, CellType type)
{
    CHECK(cell->type == type );
    CHECK(cell->worm == nullptr );
    CHECK(cell->powerup == nullptr );
}

TEST_CASE( "Load GameState from rapidJSON object", "[IO]" ) {
    
    const std::string filePath = "./Test_files/state2.json";
    std::ifstream dataIn;
    dataIn.open(filePath, std::ifstream::in);
    REQUIRE(dataIn.is_open());

    std::stringstream buffer;
    buffer << dataIn.rdbuf();
    std::string stateJson = buffer.str();
    rapidjson::Document roundJSON;
    const bool parsed = !roundJSON.Parse(stateJson.c_str()).HasParseError();
    REQUIRE(parsed);

    GameState state(roundJSON);

    CHECK(state.roundNumber == 250);

    //should I load GameConfig stuff?  Maybe later...

    //this stuff might never change...
    int movementRange = 1;
    int diggingRange = 1;
    int weaponDmg = 8;
    int weaponRange = 4;

    CHECK(state.player1.id == 1);
    CHECK(state.player1.consecutiveDoNothingCount == 0); //unknown from this file - should be default starting
    CHECK(state.player1.currentWormId == 1); //unknown from this file - should be default starting
    CHECK(state.player1.health == 182); //implied from worm health
    CHECK(state.player1.score == 1847);

    CheckWorm(&state.player1.worms[0], 1, 0, Position(14, 27), Position(14, 27), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player1.worms[1], 2, 54, Position(14, 27), Position(14, 27), movementRange, diggingRange, weaponDmg, weaponRange); //dunno htw pos is the same as worm 1 but ok
    CheckWorm(&state.player1.worms[2], 3, 128, Position(13, 27), Position(13, 27), movementRange, diggingRange, weaponDmg, weaponRange);

    CHECK(state.player2.id == 2);
    CHECK(state.player2.consecutiveDoNothingCount == 0);
    CHECK(state.player2.currentWormId == 2);
    CHECK(state.player2.health == 188);
    CHECK(state.player2.score == 1381);

    CheckWorm(&state.player2.worms[0], 1, -2, Position(30, 17), Position(30, 17), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player2.worms[1], 2, 38, Position(12, 27), Position(12, 27), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player2.worms[2], 3, 150, Position(7, 3), Position(7,3), movementRange, diggingRange, weaponDmg, weaponRange);

    //ill just do some spot checks on the map
    CheckCellEmpty(state.Cell_at({9, 0}), CellType::DEEP_SPACE);
    CheckCellEmpty(state.Cell_at({10, 0}), CellType::DEEP_SPACE);
    CheckCellEmpty(state.Cell_at({11, 0}), CellType::AIR);
    CheckCellEmpty(state.Cell_at({12, 0}), CellType::AIR);

    //player2 worm
    CheckCellEmpty(state.Cell_at({7, 2}), CellType::AIR);
    CHECK(state.Cell_at({7, 3})->type == CellType::AIR);
    //CHECK(state.Cell_at({7, 3})->powerup == nullptr);
    //CHECK(state.Cell_at({7, 3})->worm != nullptr);
    //CHECK(state.Cell_at({7, 3})->worm->id == 3);
    //CHECK(state.Cell_at({7, 3})->worm->health == 150);
    //CHECK(state.Cell_at({7, 3})->worm->position == Position(7,3));
    CheckCellEmpty(state.Cell_at({7, 4}), CellType::AIR);

    //player1 worm
    CHECK(state.Cell_at({13, 27})->type == CellType::AIR);
    CHECK(state.Cell_at({13, 27})->powerup == nullptr);
    CHECK(state.Cell_at({13, 27})->worm != nullptr);
    CHECK(state.Cell_at({13, 27})->worm->id == 3);
    CHECK(state.Cell_at({13, 27})->worm->health == 128);
    CHECK(state.Cell_at({13, 27})->worm->position == Position(13,27));

    //powerup and dirt
    CheckCellEmpty(state.Cell_at({10, 24}), CellType::AIR);
    CHECK(state.Cell_at({11, 24})->type == CellType::AIR);
    CHECK(state.Cell_at({11, 24})->worm == nullptr);
    CHECK(state.Cell_at({11, 24})->powerup != nullptr);
    CHECK(state.Cell_at({11, 24})->powerup->value == 10);
    CheckCellEmpty(state.Cell_at({12, 24}), CellType::DIRT);
}

TEST_CASE( "Commands are resolved in the right order", "[command_order]" ) {
    //according to the rules:
    //move
    //dig
    //shoot
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

//tests from WormsRoundProcessorTest