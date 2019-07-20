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
#include <limits>
#include "GameEngine.hpp"
#include "AllCommands.hpp"
#include "NextTurn.hpp"
#include "EvaluationFunctions.hpp"
#include "Utilities.hpp"
#include "MonteCarlo.hpp"
#include <thread>
#include <mutex>

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

std::mutex mtx;

void runMC(uint64_t stopTime, std::shared_ptr<MonteCarlo> mc, std::shared_ptr<GameState> state1, bool ImPlayer1, unsigned playthroughDepth)
{
    //get distance to closest enemy
    //pos of my worm:
    Player * me = state1->GetPlayer(ImPlayer1);
    Worm * worm = me->GetCurrentWorm();
    Position myWormPos = worm->position;

    //now calc
    Player * enemy = state1->GetPlayer(!ImPlayer1);
    int closestDist = 9999;
    for(auto const & worm : enemy->worms) {
        if(worm.IsDead()) {
            continue;
        }
        auto dist = myWormPos.EuclideanDistanceTo(worm.position);
        if(dist < closestDist){
            closestDist = dist;
        }
    }
    //int distToConsider = std::max(closestDist, 7);
    int distToConsider = -1;

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
            int numplies{0};
            auto thisScore = eng.Playthrough(ImPlayer1, next_node->command, nextMoveFn, EvaluationFunctions::ScoreComparison, distToConsider, playthroughDepth, numplies);

            mtx.lock();

            next_node->score += thisScore;
            next_node->w += thisScore;
            ++next_node->n;

            mc->UpdateNumSamples();
            mtx.unlock();
        }
    }
}

//expects command string to be returned e.g. "dig 5 6"
std::string runStrategy(rapidjson::Document& roundJSON)
{
    uint64_t start_time = Get_ns_since_epoch();

    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;

    auto state1 = std::make_shared<GameState>(roundJSON);

    NextTurn::Initialise();

    float c = std::sqrt(2);
    auto mc = std::make_shared<MonteCarlo>(NextTurn::AllValidMovesForPlayer(ImPlayer1, state1, true), c);

    //we'll adjust playthrough depth based on how many enemy worms are around us.
    unsigned playthroughDepth = 24;
    //Player* myPlayer = ImPlayer1? &state1->player1 : &state1->player2;
    //Player* enemyPlayer = ImPlayer1? &state1->player2 : &state1->player1;
    //for(auto const& enemyWorm: enemyPlayer->worms) {
    //    if(myPlayer->GetCurrentWorm()->position.MovementDistanceTo(enemyWorm.position) < 5) {
    //        playthroughDepth = 7;
    //    }
    //}
    //std::cerr << "playthroughDepth: " << playthroughDepth << std::endl;

    std::thread t1(runMC, start_time + 880000000, mc, state1, ImPlayer1, playthroughDepth);
    std::thread t2(runMC, start_time + 880000000, mc, state1, ImPlayer1, playthroughDepth);
    t1.join();
    t2.join();

    //choose the best move and do it
    auto best_move = mc->GetBestMove();
    std::cerr << "JP8:" << std::endl;
    mc->PrintState();

    return best_move->GetCommandString();
}

std::string executeRound(std::string& roundNumber)
{
    const std::string filePath = "./rounds/" + roundNumber + "/state.json";
    try {
        
        rapidjson::Document roundJSON = Utilities::ReadJsonFile(filePath);
        return "C;" + roundNumber + ";" + runStrategy(roundJSON) + "\n";

    } catch(...)        {
        return "C;" + roundNumber + ";error executeRound \n";
    }
}

int main(int argc, char** argv)
{
    for (std::string roundNumber; std::getline(std::cin, roundNumber);) 
    {
        std::cout << executeRound(roundNumber);
    }
}
