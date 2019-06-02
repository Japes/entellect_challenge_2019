#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include <fstream>
#include <sstream>
#include <chrono>

rapidjson::Document ReadJsonFile(std::string filePath)
{
    std::ifstream dataIn;
    dataIn.open(filePath, std::ifstream::in);
    REQUIRE(dataIn.is_open());

    std::stringstream buffer;
    buffer << dataIn.rdbuf();
    std::string stateJson = buffer.str();
    rapidjson::Document roundJSON;
    const bool parsed = !roundJSON.Parse(stateJson.c_str()).HasParseError();
    REQUIRE(parsed);

    return roundJSON;
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

    auto roundJSON = ReadJsonFile("./Test_files/state2.json");

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
    CHECK(state.player1.GetScore() == 1847);

    CheckWorm(&state.player1.worms[0], 1, 0, Position(14, 27), Position(14, 27), movementRange, diggingRange, weaponDmg, weaponRange);
    CheckWorm(&state.player1.worms[1], 2, 54, Position(14, 27), Position(14, 27), movementRange, diggingRange, weaponDmg, weaponRange); //dunno htw pos is the same as worm 1 but ok
    CheckWorm(&state.player1.worms[2], 3, 128, Position(13, 27), Position(13, 27), movementRange, diggingRange, weaponDmg, weaponRange);

    CHECK(state.player2.id == 2);
    CHECK(state.player2.consecutiveDoNothingCount == 0);
    CHECK(state.player2.currentWormId == 2);
    CHECK(state.player2.health == 188);
    CHECK(state.player2.GetScore() == 1381);

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

bool Contains_one(std::vector<std::shared_ptr<Command>>& haystack, std::shared_ptr<Command> needle)
{
    int num_found = 0;
    for(unsigned i = 0; i < haystack.size(); i++) {

        {
            ShootCommand* hay_ptr = dynamic_cast<ShootCommand*>(haystack[i].get());
            ShootCommand* needle_ptr = dynamic_cast<ShootCommand*>(needle.get());
            if(hay_ptr != nullptr && needle_ptr != nullptr &&
                *hay_ptr == *needle_ptr) {
                num_found++;
            }
        }

        {
            TeleportCommand* hay_ptr = dynamic_cast<TeleportCommand*>(haystack[i].get());
            TeleportCommand* needle_ptr = dynamic_cast<TeleportCommand*>(needle.get());
            if(hay_ptr != nullptr && needle_ptr != nullptr &&
                *hay_ptr == *needle_ptr) {
                num_found++;
            }
        }

        {
            DigCommand* hay_ptr = dynamic_cast<DigCommand*>(haystack[i].get());
            DigCommand* needle_ptr = dynamic_cast<DigCommand*>(needle.get());
            if(hay_ptr != nullptr && needle_ptr != nullptr &&
                *hay_ptr == *needle_ptr) {
                num_found++;
            }
        }
    }

    return num_found == 1;
}

TEST_CASE( "Get valid moves for a worm", "[valid_moves_for_worm]" ) {
    GIVEN("A semi realistic game state and engine")
    {
        /*
        0   1   2   3   4   5   6   7
    0   .   .   .   .   .   .   .   .
    1   .   .   .   .   22  .   .   .
    2   .   .   .   .   .   .   .   .
    3   .   .   .   13  .   23  .   .
    4   .   .   .   .   .   D   .   .
    5   .   .   .   .   D   11  12  S
    6   .   .   .   .   D   21  .   .
    7   .   .   .   .   .   .   .   .            
        */

        auto state = std::make_shared<GameState>();
        GameEngine eng(state);
        place_worm(true, 1, {5,5}, state);
        place_worm(true, 2, {6,5}, state);
        place_worm(true, 3, {3,3}, state);
        place_worm(false, 1, {5,6}, state);
        place_worm(false, 2, {4,1}, state);
        place_worm(false, 3, {5,3}, state);
        state->Cell_at({4, 5})->type = CellType::DIRT;
        state->Cell_at({5, 4})->type = CellType::DIRT;
        state->Cell_at({4, 6})->type = CellType::DIRT;
        state->Cell_at({7, 5})->type = CellType::DEEP_SPACE;

        THEN("Valid moves for player 1 are as expected")
        {
            std::vector<std::shared_ptr<Command>> moves = eng.GetValidMovesForWorm(true, state);
            std::vector<std::shared_ptr<Command>> expected_moves;
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position(4,4)));
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position({6,4})));
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position({6,6})));
            expected_moves.push_back(std::make_shared<DigCommand>(Position(5,4)));
            expected_moves.push_back(std::make_shared<DigCommand>(Position(4,5)));
            expected_moves.push_back(std::make_shared<DigCommand>(Position(4,6)));

            REQUIRE(moves.size() == expected_moves.size());

            for(unsigned i = 0; i < expected_moves.size(); i++) {
                bool containsExactlyOne = Contains_one(moves, expected_moves[i]);
                CHECK(containsExactlyOne);
            }
        }

        THEN("Valid moves for player 2 are as expected")
        {
            std::vector<std::shared_ptr<Command>> moves = eng.GetValidMovesForWorm(false, state);
            std::vector<std::shared_ptr<Command>> expected_moves;
            expected_moves.push_back(std::make_shared<DigCommand>(Position(4,6)));
            expected_moves.push_back(std::make_shared<DigCommand>(Position(4,5)));
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position(6,6)));
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position({6,7})));
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position({5,7})));
            expected_moves.push_back(std::make_shared<TeleportCommand>(Position({4,7})));

            REQUIRE(moves.size() == expected_moves.size());

            for(unsigned i = 0; i < expected_moves.size(); i++) {
                bool containsExactlyOne = Contains_one(moves, expected_moves[i]);
                CHECK(containsExactlyOne);
            }
        }

        AND_THEN("Progressing the game forward 1 turn")
        {
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Valid moves for player 1 are as expected")
            {
                std::vector<std::shared_ptr<Command>> moves = eng.GetValidMovesForWorm(true, state);
                std::vector<std::shared_ptr<Command>> expected_moves;

                expected_moves.push_back(std::make_shared<DigCommand>(Position(5,4)));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({6,4})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({7,4})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({7,6})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({6,6})));

                REQUIRE(moves.size() == expected_moves.size());

                for(unsigned i = 0; i < expected_moves.size(); i++) {
                    bool containsExactlyOne = Contains_one(moves, expected_moves[i]);
                    CHECK(containsExactlyOne);
                }
            }
        }

        AND_THEN("Moving a dude, and cycling to his turn again")
        {
            eng.AdvanceState(TeleportCommand({4,4}), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());
            eng.AdvanceState(DoNothingCommand(), DoNothingCommand());

            THEN("Valid moves for player 1 are as expected")
            {
                std::vector<std::shared_ptr<Command>> moves = eng.GetValidMovesForWorm(true, state);
                std::vector<std::shared_ptr<Command>> expected_moves;
                expected_moves.push_back(std::make_shared<DigCommand>(Position(5,4)));
                expected_moves.push_back(std::make_shared<DigCommand>(Position(4,5)));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({3,5})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({3,4})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({4,3})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({5,5})));

                REQUIRE(moves.size() == expected_moves.size());

                for(unsigned i = 0; i < expected_moves.size(); i++) {
                    bool containsExactlyOne = Contains_one(moves, expected_moves[i]);
                    CHECK(containsExactlyOne);
                }
            }

            THEN("Valid moves for player 1 are as expected if we ask to trim")
            {
                std::vector<std::shared_ptr<Command>> moves = eng.GetValidMovesForWorm(true, state, true);
                std::vector<std::shared_ptr<Command>> expected_moves;
                expected_moves.push_back(std::make_shared<DigCommand>(Position(5,4)));
                expected_moves.push_back(std::make_shared<DigCommand>(Position(4,5)));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({3,5})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({3,4})));
                expected_moves.push_back(std::make_shared<TeleportCommand>(Position({4,3})));
                //expected_moves.push_back(std::make_shared<TeleportCommand>(Position({5,5}))); NOTE THIS IS EXCLUDED (place i just came from)

                CHECK(moves.size() == expected_moves.size());

                for(unsigned i = 0; i < expected_moves.size(); i++) {
                    bool containsExactlyOne = Contains_one(moves, expected_moves[i]);
                    INFO("Don't have expected move " << i << "(" << expected_moves[i]->GetCommandString() << ")" );
                    CHECK(containsExactlyOne);
                }
            }
        }
    }
}

uint64_t Get_ns_since_epoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}

TEST_CASE( "Performance tests", "[.performance]" ) {

    unsigned gameCount = 0;
    unsigned turnCount = 0;
    unsigned num_seconds = 3;
    auto start_time = Get_ns_since_epoch();

    auto roundJSON = ReadJsonFile("./Test_files/state2.json");
    auto original_state = std::make_shared<GameState>(roundJSON);

    while(Get_ns_since_epoch() < start_time + (num_seconds * 1000000000)) {
    //while(true) {

        auto state = std::make_shared<GameState>(*original_state); //no idea why it needs to be done this way
        GameEngine eng(state);

        while(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS) {
            eng.AdvanceState(*eng.GetRandomValidMoveForWorm(true, state).get(), *eng.GetRandomValidMoveForWorm(false, state).get());
            ++turnCount;
        }
        ++gameCount;
    }

    INFO("Moves per second: " << turnCount/num_seconds << ", Moves per game: " << turnCount/gameCount << " (" << turnCount << " moves in " << gameCount << " games in " << num_seconds << " seconds)");
    CHECK(false);
}

TEST_CASE( "Performance tests - trim moves", "[.performance][trim]" ) {

    unsigned gameCount = 0;
    unsigned turnCount = 0;
    unsigned num_seconds = 3;
    auto start_time = Get_ns_since_epoch();

    auto roundJSON = ReadJsonFile("./Test_files/state2.json");
    auto original_state = std::make_shared<GameState>(roundJSON);

    while(Get_ns_since_epoch() < start_time + (num_seconds * 1000000000)) {
    //while(true) {

        auto state = std::make_shared<GameState>(*original_state); //no idea why it needs to be done this way
        GameEngine eng(state);

        while(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS) {
            eng.AdvanceState(*eng.GetRandomValidMoveForWorm(true, state, true).get(), *eng.GetRandomValidMoveForWorm(false, state, true).get());
            ++turnCount;
        }
        ++gameCount;
    }

    INFO("Moves per second: " << turnCount/num_seconds << ", Moves per game: " << turnCount/gameCount << " (" << turnCount << " moves in " << gameCount << " games in " << num_seconds << " seconds)");
    CHECK(false);
}

TEST_CASE( "Comparison with java engine", "[comparison]" ) {
    //read a bunch of known state files, moves, and expected outputs generated by their engine.
    //read in those files, apply the moves, and compare result with theirs.
}


TEST_CASE( "Copy constructor", "[copy_constructor]" ) {
    WHEN("We make a copy of a state")
    {
        auto roundJSON = ReadJsonFile("./Test_files/state2.json");
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
                    auto copied_cell = copied_state->Cell_at({i,j});
                    auto original_cell = original_state->Cell_at({i,j});
                    REQUIRE(copied_cell != original_cell);
                    REQUIRE(copied_cell->type == original_cell->type);

                    if(copied_cell->powerup == nullptr) {
                        REQUIRE(copied_cell->powerup == original_cell->powerup);
                    } else {
                        REQUIRE(original_cell->powerup != nullptr);
                    }

                    if(copied_cell->worm == nullptr) {
                        REQUIRE(copied_cell->worm == original_cell->worm);
                    } else {
                        REQUIRE(copied_cell->worm != original_cell->worm);
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

TEST_CASE( "Playthroughs from map", "[playthrough_map]" )
{
    GIVEN("A realistic game state and engine")
    {
        auto roundJSON = ReadJsonFile("./Test_files/state2.json");
        auto state = std::make_shared<GameState>(roundJSON);
        GameEngine eng(state);
        
        WHEN("We do a playthrough to a depth -1")
        {
            auto nextMoveFn = std::bind(GameEngine::GetRandomValidMoveForWorm, std::placeholders::_1, std::placeholders::_2, false);
            int depth = -1;
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, depth);
        }
    }
}
