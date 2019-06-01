#include <iostream>
#include "./rapidjson/document.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <cmath>
#include <chrono>
#include <random>
#include "GameEngine.hpp"
#include "AllCommands.hpp"

static std::string dirt = "DIRT";
static std::string air = "AIR";
static std::string space = "DEEP_SPACE";

struct POINT
{
    int x;
    int y;
};

enum DIRECTIONS { E = 0, NE, N, NW, W, SW, S, SE };
std::vector<POINT> directions = { {1,0},{1,-1},{0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1} };
std::vector<std::string> directionNames = { "E", "NE", "N", "NW","W","SW","S","SE" };

uint64_t Get_ns_since_epoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
}

POINT GetMyCurrentWormPoint(const rapidjson::Document& roundJSON)
{
    const auto currentWormId = roundJSON["currentWormId"].GetInt();
    const auto curWorm = roundJSON["myPlayer"].GetObject()["worms"].GetArray()[currentWormId - 1].GetObject()["position"].GetObject();
    return { curWorm["x"].GetInt() , curWorm["y"].GetInt() };
}

/**
 * Build a list of all cells in a specific direction withing shooting range of my active worm
 */
std::vector<POINT> buildDirectionLine(rapidjson::Document& roundJSON, DIRECTIONS direction)
{
    const int weaponRange = roundJSON["myPlayer"].GetObject()["worms"].GetArray()[roundJSON["currentWormId"].GetInt()-1].GetObject()["weapon"].GetObject()["range"].GetInt();
    std::vector<POINT> currentDirectionLine;
    for (int i = 1; i <= weaponRange; i++) 
    {
        currentDirectionLine.push_back({ i * directions[direction].x, i * directions[direction].y });
    }
    return currentDirectionLine;
}

/**
 * Get the lines the active worm can shoot in
 */
auto getShootTemplates(rapidjson::Document& roundJSON)
{
    std::vector<std::vector<POINT>> shootTemplates;

    for (int direction = E; direction <= SE; direction++) 
    {
        shootTemplates.push_back(buildDirectionLine(roundJSON, static_cast<DIRECTIONS>(direction)));
    }
    return shootTemplates;
}

/**
 * Add the x and y values of two coordinates together
 * @return Position
 */
POINT sumCoordinates(const POINT coordinateA, const POINT coordinateB) {
    return {
            coordinateA.x + coordinateB.x,
            coordinateA.y + coordinateB.y
    };
}

/**
 * Check if a coordinate is in the map bounds
 * @param coordinateToCheck {Point}
 * @param mapSize
 * @return {boolean}
 */
bool coordinateIsOutOfBounds(const POINT coordinateToCheck, const int mapSize) {
    return coordinateToCheck.x < 0
        || coordinateToCheck.x >= mapSize
        || coordinateToCheck.y < 0
        || coordinateToCheck.y >= mapSize;
}

/**
 * Calculate the distance between two points
 * https://en.wikipedia.org/wiki/Euclidean_distance
 *
 * @param positionA {Point}
 * @param positionB {Point}
 * @return {number}
 */
int euclideanDistance(POINT positionA, POINT positionB) {
    return static_cast<int>(std::floor(
        std::sqrt(std::pow(positionA.x - positionB.x, 2) + std::pow(positionA.y - positionB.y, 2))));
}

std::string RandomStrategy(rapidjson::Document& roundJSON)
{
    std::random_device rd;    //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(-1, 1);
    
    POINT dest = GetMyCurrentWormPoint(roundJSON);
    dest.x += dis(gen);
    dest.y += dis(gen);
    int maxMap = roundJSON["mapSize"].GetInt() - 1;
    if (dest.x < 0) dest.x = 0;
    if (dest.x > maxMap) dest.x = maxMap;

    if (dest.y < 0) dest.y = 0;
    if (dest.y > maxMap) dest.y = maxMap;

    auto cell2 = roundJSON["map"].GetArray()[dest.y].GetArray()[dest.x].GetObject();
    auto str = cell2["type"].GetString();
    if (dirt == str)
    {
        return "dig " + std::to_string(dest.x) + " " + std::to_string(dest.y);
    }
    if (air == str)
    {
        return "move " + std::to_string(dest.x) + " " + std::to_string(dest.y);
    }
    if (space == str)
    {
        return "nothing";
    }
    return "nothing";
}

//expects command string to be returned e.g. "dig 5 6"
std::string runStrategy(rapidjson::Document& roundJSON)
{
    uint64_t startTime = Get_ns_since_epoch();

    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;

    //TODO only consider sensible moves
    auto state1 = std::make_shared<GameState>(roundJSON);
    GameEngine eng1(state1);
    auto possible_moves = eng1.GetValidMovesForWorm (ImPlayer1);
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::S));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::E));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::W));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NW));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::NE));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SW));
    possible_moves.push_back(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::SE));

    std::vector<int> movescores(possible_moves.size(), 0);

    while(Get_ns_since_epoch() < (startTime + 900000000)) { //900ms
        for(unsigned i = 0; i < movescores.size(); ++i) {
            //load the state
            auto state = std::make_shared<GameState>(*state1); //no idea why it needs to be done this way
            GameEngine eng(state);
            //TODO use the selection formula here
            movescores[i] += eng.Playthrough(ImPlayer1, possible_moves[i], 20);
        }
    }

    //choose the best move and do it
    auto best_move_it = std::max_element(std::begin(movescores), std::end(movescores));
    unsigned best_move_index = best_move_it - std::begin(movescores);

    std::cerr << "move scores: ";
    for(auto const & move : movescores) {
        std::cerr << move << ", ";
    }
    std::cerr << " best move is at index: " << best_move_index << ": " << possible_moves[best_move_index]->GetCommandString() << std::endl;

    return possible_moves[best_move_index]->GetCommandString();
}

std::string executeRound(std::string& roundNumber)
{
    std::string ret;
    const std::string filePath = "./rounds/" + roundNumber + "/state.json";
    std::ifstream dataIn;
    dataIn.open(filePath, std::ifstream::in);
    if (dataIn.is_open())
    {
        std::stringstream buffer;
        buffer << dataIn.rdbuf();
        std::string stateJson = buffer.str();
        rapidjson::Document roundJSON;
        const bool parsed = !roundJSON.Parse(stateJson.c_str()).HasParseError();
        if (parsed)
        {
            ret = "C;" + roundNumber + ";" + runStrategy(roundJSON) + "\n";
        }
        else
        {
            ret = "C;" + roundNumber + ";error executeRound \n";
        }
    }

    return ret;
}

int main(int argc, char** argv)
{
    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(roundNumber);
    }
}
