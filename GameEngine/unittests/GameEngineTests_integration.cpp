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
#include <bitset>
#include <dirent.h>
#include "Utilities.hpp"
#include "MonteCarlo.hpp"
#include <thread>
#include <mutex>

uint64_t Get_ns_since_epoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}

TEST_CASE( "Performance tests - just advance state", "[.performance]" ) {

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

uint64_t gameCount = 0;
uint64_t turnCount = 0;
std::mutex mtx;

void runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> mc, std::shared_ptr<GameState> state1, bool ImPlayer1, unsigned playthroughDepth)
{
    while(Get_ns_since_epoch() < stopTime) {
        for(unsigned i = 0; i < 100; ++i) {
            mtx.lock();
            //choose next node
            auto next_node = mc->NextNode();
            mtx.unlock();

            //load the state
            auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            //auto nextMoveFn = [] (bool player1, std::shared_ptr<GameState> gs) -> std::shared_ptr<Command> {return std::make_shared<DoNothingCommand>();};
            int numplies{0};
            int thisScore = eng.Playthrough(ImPlayer1, next_node->command, nextMoveFn, EvaluationFunctions::ScoreComparison, -1, playthroughDepth, numplies);

            mtx.lock();
            turnCount += numplies;
            ++gameCount;

            next_node->score += thisScore;
            next_node->w += thisScore > 0? 1 : 0;
            ++next_node->n;

            mc->UpdateNumSamples();
            mtx.unlock();
        }
    }
}

TEST_CASE( "Performance tests - realistic loop", "[.performance][trim]" ) {

    gameCount = 0;
    turnCount = 0;
    uint64_t num_milliseconds = 3000;
    auto start_time = Get_ns_since_epoch();

    auto roundJSON = Utilities::ReadJsonFile("./Test_files/state22.json"); //todo need to make sure there are bots in range
    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;
    auto state1 = std::make_shared<GameState>(roundJSON);

    NextTurn::Initialise();


//from the bot---------------------------------------------------------

    float c = std::sqrt(2);
    auto mc = std::make_shared<MonteCarlo>(NextTurn::AllValidMovesForPlayer(ImPlayer1, state1, true), c);

    unsigned playthroughDepth = 24;

    uint64_t stopTime = start_time + (num_milliseconds * 1000000LL);

    std::thread t1(runMC, stopTime, mc, state1, ImPlayer1, playthroughDepth);
    std::thread t2(runMC, stopTime, mc, state1, ImPlayer1, playthroughDepth);
    t1.join();
    t2.join();

//from the bot---------------------------------------------------------


    INFO("Moves per second: " << (turnCount*1000)/num_milliseconds << ", Moves per game: " << turnCount/gameCount << 
                " (" << turnCount << " moves in " << gameCount << " games in " << num_milliseconds/1000.0f << " seconds)");
    CHECK(false);

    //700000 moves per second, with just donothings, 2.5M per second
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
    } else if (moveType == "banana") {
        std::size_t secondSpace = cmd.find(" ", firstSpace + 1);
        int x = std::stoi(cmd.substr(firstSpace, secondSpace - firstSpace));
        int y = std::stoi(cmd.substr(secondSpace, cmd.length() - secondSpace));
        return std::make_shared<BananaCommand>(Position(x,y));
    } else if (moveType == "nothing") {
        return std::make_shared<DoNothingCommand>();
    } else if (moveType == "No") { //"No Command" - invalid
        return std::make_shared<TeleportCommand>(Position(-10,-10)); //always invalid
    } else {
        std::stringstream msg;
        msg << "dont understand this move type: " << moveType;
        throw std::runtime_error(msg.str());
    }
}

void GetEndGameState(std::string path, std::string& winningPlayer, int& playerAScore, int& playerAHealth, int& playerBScore, int& playerBHealth)
{
    std::string fileContents = Utilities::ReadFile(path);

    //get the winner
    std::size_t winnerLocation = fileContents.find("The winner is: ") + 15;
    std::size_t playerAScoreLocation = fileContents.find("score:") + 6;
    std::size_t playerAHealthLocation = fileContents.find("health:") + 7;
    std::size_t playerAEndOfLine = fileContents.find("\n", playerAHealthLocation);
    std::size_t playerBScoreLocation = fileContents.find("score:", playerAHealthLocation) + 6;
    std::size_t playerBHealthLocation = fileContents.find("health:", playerBScoreLocation) + 7;
    std::size_t playerBEndOfLine = fileContents.find("\n", playerBHealthLocation);

    if (winnerLocation == std::string::npos || 
            playerAScoreLocation == std::string::npos || playerAHealthLocation == std::string::npos ||
            playerBScoreLocation == std::string::npos || playerBHealthLocation == std::string::npos ||
            playerAEndOfLine == std::string::npos || playerBEndOfLine == std::string::npos) {
        throw std::runtime_error("Problem loading end game state in unit test");
    }

    winningPlayer = fileContents.substr(winnerLocation, 1);
    INFO("winningPlayer: " << winningPlayer);

    std::string playerAScoreStr = fileContents.substr(playerAScoreLocation, (playerAHealthLocation - 8) - playerAScoreLocation);
    INFO("playerAScoreStr: " << playerAScoreStr);
    playerAScore = std::stoi(playerAScoreStr);

    std::string playerAHealthStr = fileContents.substr(playerAHealthLocation, playerAEndOfLine - playerAHealthLocation);
    INFO("playerAHealthStr: " << playerAHealthStr);
    playerAHealth = std::stoi(playerAHealthStr);

    std::string playerBScoreStr = fileContents.substr(playerBScoreLocation, (playerBHealthLocation - 8) - playerBScoreLocation);
    INFO("playerBScoreStr: " << playerBScoreStr);
    playerBScore = std::stoi(playerBScoreStr);

    std::string playerBHealthStr = fileContents.substr(playerBHealthLocation, playerBEndOfLine - playerBHealthLocation);
    INFO("playerBHealthStr: " << playerBHealthStr);
    playerBHealth = std::stoi(playerBHealthStr);
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

    UNSCOPED_INFO("cmdString: " << cmdString );

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

std::vector<std::string> GetFoldersInFolder(std::string folder)
{
    std::vector<std::string> ret;

    const char* PATH = folder.c_str();
    DIR *dir = opendir(PATH);

    struct dirent *entry = readdir(dir);
    while (entry != NULL) {
        if (entry->d_type == DT_DIR) {
            //printf("%s\n", entry->d_name);
            std::string s = entry->d_name;
            if(s != "." && s != "..") {
                ret.push_back(folder + std::string("/") + s);
            }
        }

        entry = readdir(dir);
    }

    closedir(dir);

    return ret;
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

TEST_CASE( "Comparison with java engine", "[.comparison]" ) {

    std::vector<std::string> matches = GetFoldersInFolder("../../starter-pack/match-logs");
    matches.push_back("Test_files/matches/2019.06.15.13.50.08/"); //this one is not from the latest engine
    matches.push_back("Test_files/matches/2019.06.22.15.52.10/");
    matches.push_back("Test_files/matches/2019.06.22.15.46.18/");

    for(auto & match: matches) {
        match = match + std::string("/");

        INFO(match);

        std::string botAFolder, botBFolder;
        GetBotFolders(match + GetRoundFolder(1), botAFolder, botBFolder);
        INFO("botAFolder: " << botAFolder << " botBFolder: " << botBFolder);
        unsigned numRounds = GetNumRounds(match);
        INFO("numRounds: " << numRounds );

        unsigned round = 1;
        auto roundJSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
        auto original_state = std::make_shared<GameState>(roundJSON);
        GameEngine eng(original_state);

        while(round <= numRounds) {
            INFO("round " << round);

            //todo compare freshly loaded each round as well?

            std::shared_ptr<Command> p1Command = GetCommandFromFile(match + GetRoundFolder(round) + botAFolder + "PlayerCommand.txt");
            std::shared_ptr<Command> p2Command = GetCommandFromFile(match + GetRoundFolder(round) + botBFolder + "PlayerCommand.txt");

            //std::cerr << "(" << __FUNCTION__ << ") round: " << round << " p1Command: " << p1Command->GetCommandString() << " p2Command: " << p2Command->GetCommandString() << 
            //    " p1 score: " << original_state->player1.command_score  << " p2 score: " << original_state->player2.command_score << std::endl;
            eng.AdvanceState(*p1Command, *p2Command);

            if(round != numRounds) {
                ++round;
                auto round2JSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
                auto next_state = std::make_shared<GameState>(round2JSON);

                REQUIRE(original_state->player1 == next_state->player1);
                REQUIRE(original_state->player2 == next_state->player2);
                REQUIRE(*original_state == *next_state);
                
            } else {
                //check end state
                std::string winningPlayer;
                int playerAScore;            int playerAHealth;
                int playerBScore;            int playerBHealth;

                GetEndGameState(match + GetRoundFolder(round) + "endGameState.txt", winningPlayer, playerAScore, playerAHealth, playerBScore, playerBHealth);

                auto result = eng.GetResult();

                if(winningPlayer == "B") {
                    REQUIRE(result.winningPlayer == &original_state->player2);
                    REQUIRE(result.losingPlayer == &original_state->player1);
                } else {
                    REQUIRE(result.winningPlayer == &original_state->player1);
                    REQUIRE(result.losingPlayer == &original_state->player2);
                }

                if(playerAHealth == 0 || playerBHealth == 0) {
                    REQUIRE(result.result == GameEngine::ResultType::FINISHED_KO);
                } else {
                    REQUIRE(result.result == GameEngine::ResultType::FINISHED_POINTS);
                }

                REQUIRE(playerAScore == original_state->player1.GetScore());
                REQUIRE(playerBScore == original_state->player2.GetScore());
                REQUIRE(playerAHealth == original_state->player1.health);
                REQUIRE(playerBHealth == original_state->player2.health);

                ++round;
            }
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
            int plies = 0;
            eng.Playthrough(true, std::make_shared<DoNothingCommand>(), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);
        }
    }
}


TEST_CASE( "Debugging aid...", "[.debug]" )
{
    GIVEN("A realistic game state and engine")
    {
        for(unsigned i = 0; i < 30000; i++)
        {
            auto roundJSON = Utilities::ReadJsonFile("./Test_files/invalids.json");
            auto state1 = std::make_shared<GameState>(roundJSON);
            auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int depth = 20;
            int plies = 0;
            eng.Playthrough(true, nextMoveFn(true, state), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);
        }
    }
}
