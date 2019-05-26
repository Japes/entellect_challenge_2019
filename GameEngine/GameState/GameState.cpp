#include "GameState.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

GameState::GameState() :
    player1(this),
    player2(this),
    roundNumber{1}
{
    player1.id = 1;
    player2.id = 2;
}

//deep copy of state
GameState::GameState(const GameState& other) :
    player1{other.player1},
    player2{other.player2}
{
    for(unsigned i = 0; i < MAP_SIZE; i++){
        for(unsigned j = 0; j < MAP_SIZE; j++){
            map[i][j] = other.map[i][j];
        }
    }

    roundNumber = other.roundNumber;
    healthPack = other.healthPack;

    //update internal references
    player1.state = this;
    for(Worm &w : player1.worms) {
        w.state = this;
        Cell_at(w.position)->worm = &w;
    }

    player2.state = this;
    for(Worm &w : player2.worms) {
        w.state = this;
        Cell_at(w.position)->worm = &w;
    }
}


GameState::GameState(rapidjson::Document& roundJSON) : GameState()
{
    roundNumber = roundJSON["currentRound"].GetInt();
    PopulatePlayers(roundJSON);
    PopulateMap(roundJSON);
}

void GameState::PrintJson(const rapidjson::Value& json)
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    json.Accept(writer);
    auto str = sb.GetString();
    printf("%s\n", str);
}

void GameState::PopulatePlayers(rapidjson::Document& roundJSON)
{
    const rapidjson::Value& myPlayerJson = roundJSON["myPlayer"].GetObject();
    const rapidjson::Value& otherPlayerJson = roundJSON["opponents"].GetArray()[0].GetObject(); //assuming 1 opponent for now

    Player& myPlayer    = (myPlayerJson["id"].GetInt() == 1)?       player1 : player2;
    Player& otherPlayer = (otherPlayerJson["id"].GetInt() == 1)?    player1 : player2;

    PopulatePlayer(myPlayer, myPlayerJson);
    PopulatePlayer(otherPlayer, otherPlayerJson);

    myPlayer.currentWormId = roundJSON["currentWormId"].GetInt();
    myPlayer.consecutiveDoNothingCount = roundJSON["consecutiveDoNothingCount"].GetInt();
}

void GameState::PopulatePlayer(Player& player, const rapidjson::Value& playerJson)
{
    player.command_score = playerJson["score"].GetInt();
    player.health = 0; //player.health = playerJson["health"].GetInt(); only myPlayer has this

    auto wormsJson = playerJson.GetObject()["worms"].GetArray();
    for (rapidjson::Value::ConstValueIterator itr = wormsJson.Begin(); itr != wormsJson.End(); ++itr) {
        const rapidjson::Value& wormJson = *itr;
        int wormIndex = wormJson["id"].GetInt() - 1;
        PopulateWorm(player.worms[wormIndex], wormJson);
    }

    player.RecalculateHealth(); 

    player.command_score -= player.GetAverageWormHealth();
}

void GameState::PopulateWorm(Worm& worm, const rapidjson::Value& wormJson)
{
    worm.id = wormJson["id"].GetInt();
    worm.health = wormJson["health"].GetInt();
    worm.movementRange = wormJson["movementRange"].GetInt();
    worm.diggingRange = wormJson["diggingRange"].GetInt();
    PopulatePosition(worm.position, wormJson["position"]);
    worm.previous_position = worm.position;

    if(wormJson.HasMember("weapon")) {
        PopulateWeapon(worm.weapon, wormJson["weapon"]);
    } //else it will just use the default, i guess
}

void GameState::PopulateWeapon(Weapon& weapon, const rapidjson::Value& wJson)
{
    auto weaponJson = wJson.GetObject();
    weapon.damage = wJson["damage"].GetInt();
    weapon.range = wJson["range"].GetInt();
    weapon.diagRange = std::ceil(std::sqrt((weapon.range*weapon.range)/2)); //inverse of euclidian
}

void GameState::PopulatePosition(Position& pos, const rapidjson::Value& posJson)
{
    auto positionJson = posJson.GetObject();
    pos.x = positionJson["x"].GetInt();
    pos.y = positionJson["y"].GetInt();
}


void GameState::PopulateMap(rapidjson::Document& roundJSON)
{
    if(roundJSON["mapSize"].GetInt() != GameConfig::mapSize) {
        std::cerr << "(GameState::PopulateMap) map size in this json is " << roundJSON["mapSize"].GetInt() << ", expected " << GameConfig::mapSize << ". Not loading map." << std::endl;
        return;
    }

    for (rapidjson::Value::ConstValueIterator rowItr = roundJSON["map"].Begin(); rowItr != roundJSON["map"].End(); ++rowItr) {
        for (rapidjson::Value::ConstValueIterator colItr = (*rowItr).Begin(); colItr != (*rowItr).End(); ++colItr) {
            int x = (*colItr)["x"].GetInt();
            int y = (*colItr)["y"].GetInt();
            CellType type = strToCellType((*colItr)["type"].GetString());

            Cell& thisCell = map[x][y];
            thisCell.type = type;
            thisCell.worm = nullptr;
            thisCell.powerup = nullptr;

            if((*colItr).HasMember("occupier")) {
                int wormId = (*colItr)["occupier"].GetObject()["id"].GetInt();
                int playerId = (*colItr)["occupier"].GetObject()["playerId"].GetInt();
                Player& wormOwner = playerId == 1? player1 : player2;
                thisCell.worm = &wormOwner.worms[wormId - 1];
            }
            if((*colItr).HasMember("powerup")) {
                //TODO this is where we'd distinguish between different types
                thisCell.powerup = &healthPack;
            }
        }
    }
}

Cell* GameState::Cell_at(Position pos)
{
    if(pos.IsOnMap()) {
        return &map[pos.x][pos.y];
    }

    return nullptr;
}

void GameState::Move_worm(Worm* worm, Position pos)
{
    worm->previous_position = worm->position;

    Cell_at(worm->position)->worm = nullptr;
    worm->position = pos;
    Cell_at(worm->position)->worm = worm;
}
