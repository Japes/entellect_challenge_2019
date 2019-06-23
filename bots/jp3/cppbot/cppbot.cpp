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

//a monte carlo node
struct MCNode
{
    std::shared_ptr<Command> command;
    float w;
    float n;
    int score;
    float UCT;

};

std::ostream & operator << (std::ostream &out, const MCNode &move)
{
    out << move.command->GetCommandString() << ", " <<  move.w << "/" << move.n << ", " << move.score << ", " << move.UCT;
    return out;
}

//expects command string to be returned e.g. "dig 5 6"
std::string runStrategy(rapidjson::Document& roundJSON)
{
    uint64_t startTime = Get_ns_since_epoch();

    bool ImPlayer1 = roundJSON["myPlayer"].GetObject()["id"].GetInt() == 1;

    auto state1 = std::make_shared<GameState>(roundJSON);

    NextTurn::Initialise();

    std::vector<MCNode> nodes;
    
    for(auto const& move: NextTurn::AllValidMovesForPlayer(ImPlayer1, state1, true)) {
        nodes.push_back({move, 0, 0, 0});
    }

    int N = 0;
    float c = std::sqrt(2);

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

    while(Get_ns_since_epoch() < (startTime + 880000000)) {
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

        next_node->score += thisScore;
        next_node->w += thisScore > 0? 1 : 0;
        ++next_node->n;
        ++N;
    }

    //choose the best move and do it
    auto best_move_it = std::max_element(std::begin(nodes), std::end(nodes), 
        [] (MCNode const lhs, MCNode const rhs) -> bool { return (lhs.w/lhs.n) < (rhs.w/rhs.n); });

    std::cerr << "MC results: " << std::endl;
    for(auto const & move : nodes) {
        std::cerr << move << std::endl;
    }
    std::cerr << " best move is " << best_move_it->command->GetCommandString() << std::endl;
    std::cerr << "N: " << N << std::endl;

    return best_move_it->command->GetCommandString();
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
