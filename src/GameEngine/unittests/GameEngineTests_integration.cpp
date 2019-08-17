#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameState/GameStateLoader.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "NextTurn.hpp"
#include "EvaluationFunctions.hpp"
#include "../../Bot/Bot.hpp"
#include <sstream>
#include <chrono>
#include <fstream>
#include <bitset>
#include <dirent.h>
#include "Utilities.hpp"
#include "MonteCarlo.hpp"
#include <thread>
#include <mutex>
#include <cmath>

uint64_t Get_ns_since_epoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}

TEST_CASE( "Performance tests - realistic loop", "[.performance]" ) {

    int playThroughDepth{24};
    int dirtsForBanana{10};
    int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
    //uint64_t mcTime_ns{3000000000000};
    uint64_t mcTime_ns{3000000000};
    float mc_c{std::sqrt(2)};
    int mc_runsBeforeClockCheck{50};

    Bot bot(playThroughDepth, dirtsForBanana, clearSpaceForHeuristic, mcTime_ns, mc_c, mc_runsBeforeClockCheck);

    //auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
    auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapFight.json");
    //auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapBanana.json");
    //auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapSnowball.json");
    bot.runStrategy(roundJSON);

    INFO("Moves per second: " << (bot.GetNumPlies()*1000000000)/mcTime_ns << " m/s)");
    CHECK(false);
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

    return GameStateLoader::GetCommandFromString(cmdString);
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

    std::vector<std::string> matches = GetFoldersInFolder("Test_files/matches");

    //std::vector<std::string> matches;
    //matches.push_back("Test_files/matches/2019.08.08.22.29.04/");
    //matches.push_back("Test_files/matches/2019.08.09.16.35.29/");
    //matches.push_back("Test_files/matches/2019.08.10.09.34.19/");
    //matches.push_back("Test_files/matches/2019.08.10.09.52.21/");
    //matches.push_back("Test_files/matches/2019.08.10.09.52.25/");
    //matches.push_back("Test_files/matches/2019.08.10.10.19.24/");
    //matches.push_back("Test_files/matches/2019.08.10.12.07.57/"); //snowball before banana pls
    //matches.push_back("Test_files/matches/2019.08.11.06.06.54/"); //dig points for simultaneous dig/banana
    //matches.push_back("Test_files/matches/2019.08.10.11.41.52/");

    for(auto & match: matches) {
        match = match + std::string("/");

        INFO(match);
        std::cerr << "(" << __FUNCTION__ << ") match: " << match << std::endl;

        std::string botAFolder, botBFolder;
        GetBotFolders(match + GetRoundFolder(1), botAFolder, botBFolder);
        INFO("botAFolder: " << botAFolder << " botBFolder: " << botBFolder);
        unsigned numRounds = GetNumRounds(match);
        INFO("numRounds: " << numRounds );

        unsigned round = 1;
        auto roundJSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
        auto original_state = GameStateLoader::LoadGameStatePtr(roundJSON);
        GameEngine eng(original_state);

        while(round <= numRounds) {
            INFO("progressing from round " << round << " to " << round + 1 
            << ". Current worms are 1" << original_state->player1.GetCurrentWorm()->id
            << " and 2" << original_state->player2.GetCurrentWorm()->id );

            //todo compare freshly loaded each round as well?

            std::shared_ptr<Command> p1Command = GetCommandFromFile(match + GetRoundFolder(round) + botAFolder + "PlayerCommand.txt");
            std::shared_ptr<Command> p2Command = GetCommandFromFile(match + GetRoundFolder(round) + botBFolder + "PlayerCommand.txt");

            INFO("(" << __FUNCTION__ << ") round: " << round << " p1Command: " << p1Command->GetCommandString() << " p2Command: " << p2Command->GetCommandString() << 
                " p1 score: " << original_state->player1.command_score  << " p2 score: " << original_state->player2.command_score);
            eng.AdvanceState(*p1Command, *p2Command);

            if(p1Command->Order() == static_cast<int>(Command::CommandType::TELEPORT) && 
                p2Command->Order() == static_cast<int>(Command::CommandType::TELEPORT) && 
                p1Command->GetCommandString() == p2Command->GetCommandString()) {

                std::cerr << "(" << __FUNCTION__ << ") COLLISION DETECTED, SKIPPING VALIDATION OF ROUND " << round << std::endl;

                ++round;
                if(round <= numRounds) {
                    roundJSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
                    original_state = GameStateLoader::LoadGameStatePtr(roundJSON);
                    eng = GameEngine(original_state);
                }
                continue; //collisions are non-deterministic, can't test
            }

            if(round != numRounds) {
                ++round;
                auto round2JSON = Utilities::ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
                auto next_state = GameStateLoader::LoadGameStatePtr(round2JSON);

                REQUIRE(original_state->player1 == next_state->player1);
                REQUIRE(original_state->player2 == next_state->player2);
                REQUIRE(*original_state == *next_state);

                //cant just compare lavas directly, coz in my engine things can be lava and something else at the same time
                for(int x = 0; x < GameConfig::mapSize; ++x) {
                    for(int y = 0; y < GameConfig::mapSize; ++y) {
                        Position pos(x,y);
                        if(original_state->LavaAt(pos)) {
                            INFO(pos << " next_state->LavaAt(pos): " << next_state->LavaAt(pos) << " CellType_at(pos): " << (int)next_state->CellType_at(pos));
                            bool couldBeALava = next_state->LavaAt(pos) || next_state->CellType_at(pos) == CellType::DIRT || next_state->CellType_at(pos) == CellType::DEEP_SPACE;
                            REQUIRE(couldBeALava);
                        }
                    }
                }
                
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

                if ( (playerAHealth <= 0 && playerBHealth > 0) ||
                    (playerBHealth <= 0 && playerAHealth > 0) ) {
                    REQUIRE(result.result == GameEngine::ResultType::FINISHED_KO);
                } else {
                    REQUIRE(result.result == GameEngine::ResultType::FINISHED_POINTS);
                }

                INFO(" original_state->player1.GetScore() " << original_state->player1.GetScore() << " playerAScore: " << playerAScore); 
                CHECK(original_state->player1.GetScore() == playerAScore);
                INFO(" original_state->player2.GetScore() " << original_state->player2.GetScore() << " playerBScore: " << playerBScore); 
                CHECK(original_state->player2.GetScore() == playerBScore );
                CHECK( original_state->player1.health == playerAHealth);
                CHECK( original_state->player2.health == playerBHealth);

                ++round;
            }
        }
    }
}

TEST_CASE( "Playthroughs from map", "[playthrough_map]" )
{
    GIVEN("A realistic game state and engine")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
        auto state = GameStateLoader::LoadGameStatePtr(roundJSON);
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
            auto roundJSON = Utilities::ReadJsonFile("./Test_files/JsonMapV3.json");
            auto state1 = GameStateLoader::LoadGameStatePtr(roundJSON);
            auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int depth = 20;
            int plies = 0;
            eng.Playthrough(true, nextMoveFn(true, state.get()), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);
        }
    }
}
