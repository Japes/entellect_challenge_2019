#include "catch.hpp"
#include "../GameState/GameState.hpp"
#include "../GameState/GameStateLoader.hpp"
#include "GameEngineTestUtils.hpp"
#include "Utilities.hpp"
#include "Command.hpp"
#include "GameEngine.hpp"

TEST_CASE( "GameState load worms from file", "[state_load][state_load_worms]" ) {
    GIVEN("A game state file with different kinds of worms")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
        GameState state = GameStateLoader::LoadGameState(roundJSON);

        THEN("We create the worms correctly")
        {
            REQUIRE(state.player1.worms[0].id == 1);
            REQUIRE(state.player1.worms[0].proffession == Worm::Proffession::COMMANDO);
            REQUIRE(state.player1.worms[0].roundsUntilUnfrozen == 3);
            REQUIRE(state.player1.worms[1].id == 2);
            REQUIRE(state.player1.worms[1].proffession == Worm::Proffession::AGENT);
            REQUIRE(state.player1.worms[1].roundsUntilUnfrozen == 0);
            REQUIRE(state.player1.worms[2].id == 3);
            REQUIRE(state.player1.worms[2].proffession == Worm::Proffession::TECHNOLOGIST);
            REQUIRE(state.player1.worms[2].roundsUntilUnfrozen == 0);
        }
    }
}

TEST_CASE( "GameState load previous command", "[state_load][state_load_previous_cmd]" ) {
    GIVEN("A game state file")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/previousCommands/move_nothing.json"); GameState state = GameStateLoader::LoadGameState(roundJSON);
        THEN("Previous command is read correctly")
        {
            REQUIRE(state.player1.previousCommand != nullptr); REQUIRE(state.player1.previousCommand->GetCommandString() == "move 26 13");
            REQUIRE(state.player2.previousCommand != nullptr); REQUIRE(state.player2.previousCommand->GetCommandString() == "nothing");
        }
    }
    GIVEN("A game state file")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/previousCommands/banana_banana.json"); GameState state = GameStateLoader::LoadGameState(roundJSON);
        THEN("Previous command is read correctly")
        {
            REQUIRE(state.player1.previousCommand != nullptr); REQUIRE(state.player1.previousCommand->GetCommandString() == "banana 10 26");
            REQUIRE(state.player2.previousCommand != nullptr); REQUIRE(state.player2.previousCommand->GetCommandString() == "banana 15 27");
        }
    }
    GIVEN("A game state file")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/previousCommands/dig_dig.json"); GameState state = GameStateLoader::LoadGameState(roundJSON);
        THEN("Previous command is read correctly")
        {
            REQUIRE(state.player1.previousCommand != nullptr); REQUIRE(state.player1.previousCommand->GetCommandString() == "dig 22 5");
            REQUIRE(state.player2.previousCommand != nullptr); REQUIRE(state.player2.previousCommand->GetCommandString() == "dig 7 26");
        }
    }
    GIVEN("A game state file")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/previousCommands/selectshoot_shoot.json"); GameState state = GameStateLoader::LoadGameState(roundJSON);
        THEN("Previous command is read correctly")
        {
            REQUIRE(state.player1.previousCommand != nullptr); REQUIRE(state.player1.previousCommand->GetCommandString() == "shoot W");
            REQUIRE(state.player2.previousCommand != nullptr); REQUIRE(state.player2.previousCommand->GetCommandString() == "select 1;shoot E");
        }
    }
    GIVEN("A game state file")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/previousCommands/snowball_move.json"); GameState state = GameStateLoader::LoadGameState(roundJSON);
        THEN("Previous command is read correctly")
        {
            REQUIRE(state.player1.previousCommand != nullptr); REQUIRE(state.player1.previousCommand->GetCommandString() == "snowball 6 18");
            REQUIRE(state.player2.previousCommand != nullptr); REQUIRE(state.player2.previousCommand->GetCommandString() == "move 30 16");
        }
    }
}

TEST_CASE( "GameState load lava", "[state_load][state_load_lava]" ) {
    GIVEN("A game state file")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
        GameState state = GameStateLoader::LoadGameState(roundJSON);

        THEN("Cell types are loaded correctly")
        {
            REQUIRE(state.CellType_at({6,0}) == CellType::DEEP_SPACE );
            REQUIRE(state.CellType_at({11,0}) == CellType::DIRT );
            REQUIRE(state.CellType_at({15,3}) == CellType::AIR );
            REQUIRE(state.LavaAt({21,3}));
        }
    }
}

TEST_CASE( "GameState static lava state", "[state_load][state_load_lava_static]" ) {
    GIVEN("A game state file loaded with lavas already")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
        GameState state = GameStateLoader::LoadGameState(roundJSON);
        REQUIRE(state.LavaAt({1,15}));

        WHEN("We create another game state")
        {
            GameState secondOne;

            THEN("Lavas in other rounds aren't affected")
            {
                REQUIRE(!secondOne.LavaAt({1,15}));
            }
        }
    }
}

TEST_CASE( "Convert string to command", "[GetCommandFromString]" ) {

    GIVEN("A string for a command...")
    {
        std::string move = GENERATE(
            std::string{"move 5 27"}, 
            std::string{"dig 22 13"}, 
            std::string{"shoot NE"}, 
            std::string{"banana 30 20"},
            std::string{"snowball 13 14"},
            std::string{"select 1;move 5 27"}, 
            std::string{"select 2;dig 22 13"}, 
            std::string{"select 3;shoot NE"}, 
            std::string{"select 1;banana 30 20"},
            std::string{"select 2; snowball 13 14"}
            );

        WHEN("We convert it to a command")
        {
            auto cmd = GameStateLoader::GetCommandFromString(move);
            THEN("It converts correctly...")
            {
                INFO(move);
                REQUIRE(cmd != nullptr);
                if(move != "select 2; snowball 13 14") {
                    REQUIRE(cmd->GetCommandString() == move);
                } else {
                    REQUIRE(cmd->GetCommandString() == "select 2;snowball 13 14");
                }
            }
        }
    }

    GIVEN("the format of a do nothing...")
    {
        std::string move{"nothing \"Player chose to do nothing\""};

        WHEN("We convert it to a command")
        {
            auto cmd = GameStateLoader::GetCommandFromString(move);
            THEN("It converts correctly...")
            {
                INFO(move);
                REQUIRE(cmd != nullptr);
                REQUIRE(cmd->GetCommandString() == "nothing");
            }
        }
    }

    GIVEN("the format of an invalid move...")
    {
        std::string move{"invalid"};

        WHEN("We convert it to a command")
        {
            auto cmd = GameStateLoader::GetCommandFromString(move);
            THEN("It converts correctly...")
            {
                INFO(move);
                REQUIRE(cmd != nullptr);
                GameState state;
                REQUIRE(!cmd->IsValid(true, &state));
            }
        }
    }
}
