#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "NextTurn.hpp"
#include "EvaluationFunctions.hpp"
#include <sstream>
#include <chrono>
#include <fstream>
#include <dirent.h>
#include "Utilities.hpp"

uint64_t Get_ns_since_epoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}

TEST_CASE( "Performance tests", "[.performance]" ) {

    unsigned gameCount = 0;
    unsigned turnCount = 0;
    unsigned num_seconds = 3;
    auto start_time = Get_ns_since_epoch();

    auto roundJSON = Utilities::ReadJsonFile("./Test_files/state22.json");
    auto original_state = std::make_shared<GameState>(roundJSON);

    while(Get_ns_since_epoch() < start_time + (num_seconds * 1000000000)) {
    //while(true) {

        auto state = std::make_shared<GameState>(*original_state); //no idea why it needs to be done this way
        GameEngine eng(state);

        while(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS) {
            eng.AdvanceState(*NextTurn::GetRandomValidMoveForPlayer(true, state, false).get(), *NextTurn::GetRandomValidMoveForPlayer(false, state, false).get());
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

    auto roundJSON = Utilities::ReadJsonFile("./Test_files/state22.json");
    auto original_state = std::make_shared<GameState>(roundJSON);

    while(Get_ns_since_epoch() < start_time + (num_seconds * 1000000000)) {
    //while(true) {

        auto state = std::make_shared<GameState>(*original_state); //no idea why it needs to be done this way
        GameEngine eng(state);

        while(eng.GetResult().result == GameEngine::ResultType::IN_PROGRESS) {
            eng.AdvanceState(*NextTurn::GetRandomValidMoveForPlayer(true, state, true).get(), *NextTurn::GetRandomValidMoveForPlayer(false, state, true).get());
            ++turnCount;
        }
        ++gameCount;
    }

    INFO("Moves per second: " << turnCount/num_seconds << ", Moves per game: " << turnCount/gameCount << " (" << turnCount << " moves in " << gameCount << " games in " << num_seconds << " seconds)");
    CHECK(false);
}

std::shared_ptr<Command> GetCommandFromString(std::string cmd)
{
    std::size_t firstSpace = cmd.find(" ");
    std::size_t endLine = cmd.find("\n");
    std::string moveType = cmd.substr(0, firstSpace);

    if(moveType == "move") {
        std::size_t secondSpace = cmd.find(" ", firstSpace + 1);
        int x = std::stoi(cmd.substr(firstSpace, secondSpace - firstSpace));
        int y = std::stoi(cmd.substr(secondSpace, cmd.length() - secondSpace));
        return std::make_shared<TeleportCommand>(Position(x,y));
    } else if (moveType == "dig") {
        std::size_t secondSpace = cmd.find(" ", firstSpace + 1);
        int x = std::stoi(cmd.substr(firstSpace, secondSpace - firstSpace));
        int y = std::stoi(cmd.substr(secondSpace, cmd.length() - secondSpace));
        return std::make_shared<DigCommand>(Position(x,y));
    } else if (moveType == "shoot") {
        std::string dirString = cmd.substr(firstSpace + 1, (endLine - firstSpace));
        ShootCommand::ShootDirection dir;

        if(dirString == "N") { dir = ShootCommand::ShootDirection::N;}
        else if(dirString == "NE") { dir = ShootCommand::ShootDirection::NE;}
        else if(dirString == "E") { dir = ShootCommand::ShootDirection::E;}
        else if(dirString == "SE") { dir = ShootCommand::ShootDirection::SE;}
        else if(dirString == "S") { dir = ShootCommand::ShootDirection::S;}
        else if(dirString == "SW") { dir = ShootCommand::ShootDirection::SW;}
        else if(dirString == "W") { dir = ShootCommand::ShootDirection::W;}
        else if(dirString == "NW") { dir = ShootCommand::ShootDirection::NW;}
        else {throw std::runtime_error("don't understand this shoot string");}

        auto ret = std::make_shared<ShootCommand>(dir);
        return ret;

    } else if (moveType == "nothing") {
        return std::make_shared<DoNothingCommand>();
    } else {
        throw std::runtime_error("dont understand this move type");
    }
}

void GetEndGameState(std::string path, char& winningPlayer, int& playerAScore, int& playerAHealth, int& playerBScore, int& playerBHealth)
{
    std::string fileContents = Utilities::ReadFile(path);
}

std::shared_ptr<Command> GetCommandFromFile(std::string path)
{
    std::string fileContents = Utilities::ReadFile(path);

    //get the move string
    std::size_t firstColon = fileContents.find(":");
    if (firstColon == std::string::npos) {
        throw std::runtime_error("Problem loading command in unit test");
    }
    std::size_t endOfLine = fileContents.find("\n", firstColon + 1);
    if (endOfLine == std::string::npos) {
        throw std::runtime_error("Problem loading command in unit test");
    }
    std::string cmdString = fileContents.substr(firstColon + 2, endOfLine - (firstColon + 2));

    return GetCommandFromString(cmdString);
}

std::string GetRoundFolder(unsigned round)
{
    std::string padding = "";
    if(round < 100) {
        padding += "0";
    }
    if(round < 10) {
        padding += "0";
    }

    return "Round " + padding + std::to_string(round) + "/";
}

void GetBotFolders(std::string roundFolder, std::string& botA, std::string& botB)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (roundFolder.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string directory = ent->d_name;
            if(directory[0] == 'A') {
                botA = directory + "/";
            } else if(directory[0] == 'B') {
                botB = directory + "/";
            }
        }
        closedir (dir);
    } else {
        throw std::runtime_error("Problem getting bot folders in unit test");
    }
}

unsigned GetNumRounds(std::string roundFolder)
{
    unsigned ret = 0;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (roundFolder.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string directory = ent->d_name;
            if(directory[0] == 'R') {
                ++ret;
            }
        }
        closedir (dir);
    } else {
        throw std::runtime_error("Problem getting bot folders in unit test");
    }

    return ret;
}

TEST_CASE( "Comparison with java engine", "[comparison]" ) {

    std::string match = "Test_files/2019.06.15.13.50.08/";
    std::string botAFolder, botBFolder;
    GetBotFolders(match + GetRoundFolder(1), botAFolder, botBFolder);
    unsigned numRounds = GetNumRounds(match);

    unsigned round = 1;
    auto roundJSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
    auto original_state = std::make_shared<GameState>(roundJSON);
    GameEngine eng(original_state);

    while(round <= numRounds) {

        //todo compare freshly loaded each round as well?

        std::shared_ptr<Command> p1Command = GetCommandFromFile(match + GetRoundFolder(round) + botAFolder + "PlayerCommand.txt");
        std::shared_ptr<Command> p2Command = GetCommandFromFile(match + GetRoundFolder(round) + botBFolder + "PlayerCommand.txt");

        std::cerr << "(" << __FUNCTION__ << ") round: " << round << " p1Command: " << p1Command->GetCommandString() << 
        " p2Command: " << p2Command->GetCommandString() << std::endl;
        eng.AdvanceState(*p1Command, *p2Command);

        if(round == numRounds) {
            //check end state
            ++round;
        } else {
            ++round;
            auto round2JSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
            auto next_state = std::make_shared<GameState>(round2JSON);

            REQUIRE(original_state->player1 == next_state->player1);
            REQUIRE(original_state->player2 == next_state->player2);
            REQUIRE(*original_state == *next_state);
        }
    }

}

TEST_CASE( "Playthroughs from map", "[playthrough_map]" )
{
    GIVEN("A realistic game state and engine")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/state22.json");
        auto state = std::make_shared<GameState>(roundJSON);
        GameEngine eng(state);
        
        WHEN("We do a playthrough to a depth -1")
        {
            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, false);
            int depth = -1;
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);
        }
    }
}


TEST_CASE( "Debugging aid...", "[.debug]" )
{
    GIVEN("A realistic game state and engine")
    {
        for(unsigned i = 0; i < 30000; i++)
        {
            auto roundJSON = Utilities::ReadJsonFile("./Test_files/state_move_occupied1.json");
            auto state = std::make_shared<GameState>(roundJSON);
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int depth = 20;
            eng.Playthrough(true, nextMoveFn(true, state), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);
        }
    }
}
