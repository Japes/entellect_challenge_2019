#include "catch.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "GameEngine.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Get sensible shoots", "[get_sensible_shoots]" )
{
    GIVEN("A semi realistic game state and engine")
    {
        auto state = std::make_shared<GameState>();
        
        place_worm(true, 1, {10,10}, state);
        place_worm(true, 2, {11,10}, state); //friendly E of us
        place_worm(true, 3, {8,10}, state); //friendly W of us
        place_worm(false, 1, {11,9}, state); //enemy NE 1 step
        place_worm(false, 2, {13,13}, state); //enemy SE 2 step
        place_worm(false, 3, {10,15}, state); //just out of range

        THEN("GetSensibleShootsForWorm returns correct")
        {
            auto ret = NextTurn::GetSensibleShootsForWorm(true, state);
            REQUIRE(ret.size() == 2);
            auto expected_move = std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NE);

            INFO(ret[0]->GetCommandString());
            INFO(ret[1]->GetCommandString());

            REQUIRE( ( (ret[0]->GetCommandString() == "shoot NE") || (ret[0]->GetCommandString() == "shoot SE") ) );
            REQUIRE( ( (ret[1]->GetCommandString() == "shoot NE") || (ret[1]->GetCommandString() == "shoot SE") ) );
            REQUIRE( ret[0]->GetCommandString() != ret[1]->GetCommandString() );
        }
    }
}

TEST_CASE( "Handle no available moves", "[no_available_moves]" )
{
    GIVEN("A game state where the worm has no valid moves")
    {
        auto state = std::make_shared<GameState>();
        
        place_worm(true, 1, {10,10}, state);
        state->Cell_at({9, 9})->type = CellType::DEEP_SPACE;
        state->Cell_at({9, 10})->type = CellType::DEEP_SPACE;
        state->Cell_at({9, 11})->type = CellType::DEEP_SPACE;
        state->Cell_at({10, 9})->type = CellType::DEEP_SPACE;
        state->Cell_at({11, 9})->type = CellType::DEEP_SPACE;
        state->Cell_at({11, 10})->type = CellType::DEEP_SPACE;
        state->Cell_at({11, 11})->type = CellType::DEEP_SPACE;
        state->Cell_at({10, 11})->type = CellType::DEEP_SPACE;


        THEN("We return the donothing command (if we trim stupid shoots)")
        {
            auto ret = NextTurn::GetRandomValidMoveForWorm(true, state, true);
            REQUIRE(ret->GetCommandString() == "nothing");
        }
    }
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
        state->Cell_at({5, 4})->type = CellType::DIRT;
        state->Cell_at({4, 5})->type = CellType::DIRT;
        state->Cell_at({4, 6})->type = CellType::DIRT;
        state->Cell_at({7, 5})->type = CellType::DEEP_SPACE;

        THEN("Valid moves for player 1 are as expected")
        {
            std::vector<std::shared_ptr<Command>> moves = NextTurn::GetValidTeleportDigsForWorm(true, state);
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
                INFO("Don't have expected move " << i << " (" << expected_moves[i]->GetCommandString() << ")" );
                CHECK(containsExactlyOne);
            }
        }

        THEN("Valid moves for player 2 are as expected")
        {
            std::vector<std::shared_ptr<Command>> moves = NextTurn::GetValidTeleportDigsForWorm(false, state);
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
                std::vector<std::shared_ptr<Command>> moves = NextTurn::GetValidTeleportDigsForWorm(true, state);
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
                std::vector<std::shared_ptr<Command>> moves = NextTurn::GetValidTeleportDigsForWorm(true, state);
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
                std::vector<std::shared_ptr<Command>> moves = NextTurn::GetValidTeleportDigsForWorm(true, state, true);
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