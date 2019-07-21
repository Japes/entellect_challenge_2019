#include "GameState.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <cmath>

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
    std::memcpy(mapDeepSpaces, other.mapDeepSpaces, sizeof(mapDeepSpaces));
    std::memcpy(mapDirts, other.mapDirts, sizeof(mapDirts));

    healthPackPos = other.healthPackPos;

    roundNumber = other.roundNumber;
    healthPack = other.healthPack;

    UpdateRefs();
}

void GameState::UpdateRefs()
{
    player1.state = this;
    UpdateRefs(player1);

    player2.state = this;
    UpdateRefs(player2);
}

void GameState::UpdateRefs(Player& player)
{
    for(Worm &w : player.worms) {
        w.state = this;
        if(w.health > 0) {
            PlaceWormAt(w.position, &w); //so that cell's worm is updated
        }
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

    myPlayer.consecutiveDoNothingCount = roundJSON["consecutiveDoNothingCount"].GetInt();
}

void GameState::PopulatePlayer(Player& player, const rapidjson::Value& playerJson)
{
    player.command_score = playerJson["score"].GetInt();
    player.health = 0; //player.health = playerJson["health"].GetInt(); only myPlayer has this
    player.currentWormId = playerJson["currentWormId"].GetInt();

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

    if(wormJson.HasMember("bananaBombs")) {
        PopulateBanana(worm.banana_bomb, wormJson["bananaBombs"]);
        worm.banana_bomb_count = wormJson["bananaBombs"]["count"].GetInt();
    } //else it will just use the default, i guess WHICH IS AN ISSUE I NEED TO KEEP TRACK OF ENEMY BANANA BOMBS

}

void GameState::PopulateWeapon(Weapon& weapon, const rapidjson::Value& wJson)
{
    auto weaponJson = wJson.GetObject();
    weapon.damage = wJson["damage"].GetInt();
    weapon.range = wJson["range"].GetInt();
    weapon.diagRange = std::ceil(std::sqrt((weapon.range*weapon.range)/2)); //inverse of euclidian SEEMS TO BE A PROBLEM WITH THIS WHEN i SUBMIT...
}

void GameState::PopulateBanana(BananaBomb& banana, const rapidjson::Value& wJson)
{
    /*
    "bananaBombs": {
                    "damage": 20,
                    "range": 5,
                    "count": 3,
                    "damageRadius": 2
                }
                */
    auto weaponJson = wJson.GetObject();
    banana.damage = wJson["damage"].GetInt();
    banana.range = wJson["range"].GetInt();
    banana.damageRadius = wJson["damageRadius"].GetInt();
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
            Position pos(x,y);
            CellType type =  Cell::strToCellType((*colItr)["type"].GetString());

            SetCellTypeAt(pos, type);
            ClearPowerupAt(pos);

            if((*colItr).HasMember("occupier")) {
                int wormId = (*colItr)["occupier"].GetObject()["id"].GetInt();
                int playerId = (*colItr)["occupier"].GetObject()["playerId"].GetInt();
                Player& wormOwner = playerId == 1? player1 : player2;
                PlaceWormAt(pos, &wormOwner.worms[wormId - 1]);
            }
            if((*colItr).HasMember("powerup")) {
                //TODO this is where we'd distinguish between different types
                PlacePowerupAt(pos);
            }
        }
    }
}

Cell GameState::Cell_at(Position pos)
{
    Cell ret;
    if(mapDeepSpaces[pos.y] & (first_bit_set >> pos.x) ) {
        ret.type = CellType::DEEP_SPACE;
    } else if (mapDirts[pos.y] & (first_bit_set >> pos.x)) {
        ret.type = CellType::DIRT;
    } else {
        ret.type = CellType::AIR;
    }

    ForAllWorms([&] (Worm& w) {
        if(w.position == pos && !w.IsDead()) {
            ret.worm = &w;
        }
    });

    for(auto const& hpPos : healthPackPos) {
        if(hpPos == pos) {
            ret.powerup = &healthPack;
            break;
        } 
    }

    return ret;
}

void GameState::SetCellTypeAt(Position pos, CellType type)
{
    auto posField = (first_bit_set >> pos.x);

    switch(type) {
        case CellType::AIR:
            mapDeepSpaces[pos.y] &= ~posField;
            mapDirts[pos.y] &= ~posField;
        break;
        case CellType::DEEP_SPACE:
            mapDeepSpaces[pos.y] |= posField;
            mapDirts[pos.y] &= ~posField;
        break;
        case CellType::DIRT:
            mapDeepSpaces[pos.y] &= ~posField;
            mapDirts[pos.y] |= posField;
        break;
    }
}

void GameState::PlacePowerupAt(Position pos)
{
    healthPackPos.push_back(pos);
}

void GameState::ClearPowerupAt(Position pos)
{
    auto it = healthPackPos.begin();

    while(it != healthPackPos.end()) {
        if(*it == pos) {
            healthPackPos.erase(it);
            return;
        }
        ++it;
    }
}

void GameState::PlaceWormAt(Position pos, Worm* worm)
{
    worm->position = pos;
}

void GameState::Move_worm(Worm* worm, Position pos)
{
    worm->previous_position = worm->position;

    PlaceWormAt(pos, worm);
}

Player* GameState::GetPlayer(bool player1)
{
    return player1 ? &this->player1 : &player2;
}

void GameState::ForAllWorms(std::function<void(Worm&)> wormFn)
{
    for(auto & worm : player1.worms) {
        wormFn(worm);
    }
    for(auto & worm : player2.worms) {
        wormFn(worm);
    }
}

bool GameState::operator==(const GameState &other) const
{
    bool deepSpacesGood = (memcmp ( mapDeepSpaces, other.mapDeepSpaces, sizeof(mapDeepSpaces) ) == 0);
    bool dirtsGood = (memcmp ( mapDirts, other.mapDirts, sizeof(mapDirts) ) == 0);

    //std::cerr << "(" << __FUNCTION__ << ") deepSpacesGood: " << deepSpacesGood <<
    //" dirtsGood: " << dirtsGood << 
    //" healthPackPos == other.healthPackPos: " << (healthPackPos == other.healthPackPos) <<
    //" roundNumber == other.roundNumber: " << (roundNumber == other.roundNumber) <<
    //std::endl;

    return deepSpacesGood && 
            dirtsGood && 
            player1 == other.player1 && 
            player2 == other.player2 &&
            healthPackPos == other.healthPackPos &&
            roundNumber == other.roundNumber;
}
