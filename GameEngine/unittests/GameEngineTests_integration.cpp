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


//a monte carlo node
struct MCNode
{
    std::shared_ptr<Command> command;
    float w;
    float n;
    int score;
    float UCT;

};

TEST_CASE( "Performance tests - realistic loop", "[.performance][trim]" ) {

    unsigned gameCount = 0;
    int turnCount = 0;
    unsigned num_seconds = 3;
    auto start_time = Get_ns_since_epoch();

    auto roundJSON = Utilities::ReadJsonFile("./Test_files/state22.json"); //todo need to make sure there are bots in range
    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;
    auto state1 = std::make_shared<GameState>(roundJSON);

    NextTurn::Initialise();


//from the bot---------------------------------------------------------

    std::vector<MCNode> nodes;

    auto movesChar = NextTurn::GetValidTeleportDigs (ImPlayer1, state1, true);
    std::bitset<8> moves = std::bitset<8>(movesChar);
    for(unsigned i = 0; i < 8; ++i ) {
        if(moves[i]) {
            nodes.push_back({NextTurn::GetTeleportDig(ImPlayer1, state1, i), 0, 0, 0});
        }
    }

    auto possible_shootsChar = NextTurn::GetValidShoots (ImPlayer1, state1, true);
    std::bitset<8> possible_shoots = std::bitset<8>(possible_shootsChar);
    for(unsigned i = 0; i < 8; ++i ) {
        if(possible_shoots[i]) {
            nodes.push_back({NextTurn::_playerShoots[i], 0, 0, 0});
        }
    }

    int N = 0;
    float c = std::sqrt(2);

    unsigned playthroughDepth = -1;

    while(Get_ns_since_epoch() < start_time + (num_seconds * 1000000000)) {
    //while(true) {

        //choose next node
        for(auto & node:  nodes) {
            if(node.n == 0) {
                node.UCT = std::numeric_limits<decltype(node.UCT)>::max();
            } else {
                node.UCT = (node.w / node.n) + c*std::sqrt(std::log(N)/node.n );
            }
        }
        auto next_node = std::max_element(std::begin(nodes), std::end(nodes), [] (MCNode const lhs, MCNode const rhs) -> bool { return lhs.UCT < rhs.UCT; });

        //load the state
        auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
        GameEngine eng(state);

        auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
        int numplies{0};
        int thisScore = eng.Playthrough(ImPlayer1, next_node->command, nextMoveFn, EvaluationFunctions::ScoreComparison, -1, playthroughDepth, numplies);
        turnCount += numplies;
        ++gameCount;

        next_node->score += thisScore;
        next_node->w += thisScore > 0? 1 : 0;
        ++next_node->n;
        ++N;
    }

//from the bot---------------------------------------------------------


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

    std::string match = "Test_files/matches/2019.06.15.13.50.08/"; //this one is not from the latest engine
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

        //std::cerr << "(" << __FUNCTION__ << ") round: " << round << " p1Command: " << p1Command->GetCommandString() << 
        //" p2Command: " << p2Command->GetCommandString() << std::endl;
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
            auto roundJSON = Utilities::ReadJsonFile("./Test_files/state_move_occupied1.json");
            auto state = std::make_shared<GameState>(roundJSON);
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int depth = 20;
            int plies = 0;
            eng.Playthrough(true, nextMoveFn(true, state), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth, plies);
        }
    }
}
