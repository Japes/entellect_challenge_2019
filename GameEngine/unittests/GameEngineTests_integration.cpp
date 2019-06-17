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

    auto roundJSON = ReadJsonFile("./Test_files/state2.json");
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
    }
}

std::shared_ptr<Command> GetCommandFromFile(std::string path)
{
    //read in file
    std::ifstream dataIn;
    dataIn.open(path, std::ifstream::in);
    if(!dataIn.is_open()) {
        throw std::runtime_error("Problem loading command in unit test");
    }

    std::stringstream buffer;
    buffer << dataIn.rdbuf();
    std::string fileContents = buffer.str();

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

    std::string match = "../../starter-pack/match-logs/2019.06.15.13.50.08/";
    std::string botAFolder, botBFolder;
    GetBotFolders(match + GetRoundFolder(1), botAFolder, botBFolder);
    unsigned numRounds = GetNumRounds(match);

    unsigned round = 1;
    auto roundJSON = ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
    auto original_state = std::make_shared<GameState>(roundJSON);
    GameEngine eng(original_state);

    while(round <= numRounds) {

        std::shared_ptr<Command> p1Command = GetCommandFromFile(match + GetRoundFolder(round) + botAFolder + "PlayerCommand.txt");
        std::shared_ptr<Command> p2Command = GetCommandFromFile(match + GetRoundFolder(round) + botBFolder + "PlayerCommand.txt");

        std::cerr << "(" << __FUNCTION__ << ") round: " << round << " p1Command: " << p1Command->GetCommandString() << 
        " p2Command: " << p2Command->GetCommandString() << std::endl;
        eng.AdvanceState(*p1Command, *p2Command);

        if(round > numRounds) {

        } else {
            ++round;
            auto round2JSON = ReadJsonFile(match + GetRoundFolder(round) + botBFolder + "JsonMap.json");
            auto next_state = std::make_shared<GameState>(round2JSON);

            REQUIRE(original_state->player1 == next_state->player1);
            REQUIRE(original_state->player2 == next_state->player2);
            REQUIRE(*original_state == *next_state);
        }
    }

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
            auto roundJSON = ReadJsonFile("./Test_files/state_move_occupied1.json");
            auto state = std::make_shared<GameState>(roundJSON);
            GameEngine eng(state);

            auto nextMoveFn = std::bind(NextTurn::GetRandomValidMoveForPlayer, std::placeholders::_1, std::placeholders::_2, true);
            int depth = 20;
            eng.Playthrough(true, nextMoveFn(true, state), nextMoveFn, EvaluationFunctions::ScoreComparison, -1, depth);
        }
    }
}
