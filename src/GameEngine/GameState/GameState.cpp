#include "GameState.hpp"
#include "AllCommands.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <cmath>

std::bitset<MAP_SIZE*MAP_SIZE> GameState::bananaBombOverlay;
int GameState::bananaBombOverlayCentre;

GameState::GameState() :
    player1(this),
    player2(this),
    roundNumber{1}
{
    player1.id = 1;
    player2.id = 2;

    if(bananaBombOverlay.none()) {
        //initialise to the shadow of a bomb in the topish leftish corner
        bananaBombOverlay.set(2);

        bananaBombOverlay.set(MAP_SIZE + 1);
        bananaBombOverlay.set(MAP_SIZE + 2);
        bananaBombOverlay.set(MAP_SIZE + 3);

        bananaBombOverlay.set(MAP_SIZE*2);
        bananaBombOverlay.set(MAP_SIZE*2 + 1);
        bananaBombOverlay.set(MAP_SIZE*2 + 2);
        bananaBombOverlay.set(MAP_SIZE*2 + 3);
        bananaBombOverlay.set(MAP_SIZE*2 + 4);

        bananaBombOverlay.set(MAP_SIZE*3 + 1);
        bananaBombOverlay.set(MAP_SIZE*3 + 2);
        bananaBombOverlay.set(MAP_SIZE*3 + 3);

        bananaBombOverlay.set(MAP_SIZE*4 + 2);

        bananaBombOverlayCentre = MAP_SIZE*2 + 2;

        //std::cerr << "(" << __FUNCTION__ << ") bananaBombOverlay: " << bananaBombOverlay << std::endl;
        //std::cerr << "(" << __FUNCTION__ << ") bananaBombOverlay << 1: " << (bananaBombOverlay << 1) << std::endl;
        //std::cerr << "(" << __FUNCTION__ << ") bananaBombOverlay << -1: " << (bananaBombOverlay << -1) << std::endl;
    }
}

//deep copy of state
GameState::GameState(const GameState& other) :
    player1{other.player1},
    player2{other.player2},
    roundNumber{other.roundNumber},
    healthPack{other.healthPack},
    mapDeepSpaces{other.mapDeepSpaces},
    mapDirts{other.mapDirts},
    mapLavas{other.mapLavas},
    healthPackPos{other.healthPackPos}
{
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
    player.remainingWormSelections = playerJson["remainingWormSelections"].GetInt();

    auto wormsJson = playerJson.GetObject()["worms"].GetArray();
    for (rapidjson::Value::ConstValueIterator itr = wormsJson.Begin(); itr != wormsJson.End(); ++itr) {
        const rapidjson::Value& wormJson = *itr;
        int wormIndex = wormJson["id"].GetInt() - 1;
        PopulateWorm(player.worms[wormIndex], wormJson);
    }

    player.RecalculateHealth(); 

    player.command_score -= player.GetAverageWormHealth();
    player.previousCommand = Str2Cmd(playerJson["previousCommand"].GetString());
}

//gets the x/y part of commands that use it
Position GameState::GetCommandPosition(std::string str)
{
    std::size_t firstSpace = str.find(" ", 0);
    std::size_t secondSpace = str.find(" ", firstSpace + 1);

    int x = std::stoi(str.substr(firstSpace, secondSpace - firstSpace));
    int y = std::stoi(str.substr(secondSpace + 1, str.size() - secondSpace + 1));

    return {x,y};
}

std::shared_ptr<Command> GameState::Str2Cmd(std::string str)
{
    if(str.find("select") != std::string::npos){
        str = str.substr(9, str.length() - 9); //ignore the select bit
    }

    if(str.find("move") != std::string::npos){
        return std::make_shared<TeleportCommand>(GetCommandPosition(str));
    }
    if(str.find("dig") != std::string::npos){
        return std::make_shared<DigCommand>(GetCommandPosition(str));
    }
    if(str.find("banana") != std::string::npos){
        return std::make_shared<BananaCommand>(GetCommandPosition(str));
    }
    if(str.find("shoot") != std::string::npos){
        std::string dirStr = str.substr(6, str.size() - 6);
        ShootCommand::ShootDirection dir;
        if(dirStr == "N") { dir = ShootCommand::ShootDirection::N; }
        if(dirStr == "NE") { dir = ShootCommand::ShootDirection::NE; }
        if(dirStr == "E") { dir = ShootCommand::ShootDirection::E; }
        if(dirStr == "SE") { dir = ShootCommand::ShootDirection::SE; }
        if(dirStr == "S") { dir = ShootCommand::ShootDirection::S; }
        if(dirStr == "SW") { dir = ShootCommand::ShootDirection::SW; }
        if(dirStr == "W") { dir = ShootCommand::ShootDirection::W; }
        if(dirStr == "NW") { dir = ShootCommand::ShootDirection::NW; }
        return std::make_shared<ShootCommand>(dir);
    }
    if(str.find("nothing") != std::string::npos || str.find("invalid") != std::string::npos){
        return std::make_shared<DoNothingCommand>();
    }
    if(str.find("snowball") != std::string::npos){
        return std::make_shared<SnowballCommand>(GetCommandPosition(str));
    }

    return nullptr;
}

void GameState::PopulateWorm(Worm& worm, const rapidjson::Value& wormJson)
{
    std::string profString = wormJson["profession"].GetString();

    if(profString == "Commando") {
        worm.SetProffession(Worm::Proffession::COMMANDO);
    } else if(profString == "Agent") {
        worm.SetProffession(Worm::Proffession::AGENT);
    } else if(profString == "Technologist") {
        worm.SetProffession(Worm::Proffession::TECHNOLOGIST);
    } else {
        std::cerr << "(" << __FUNCTION__ << ") worm has no proffession!" << std::endl;
    }

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
    }
    
    if(wormJson.HasMember("snowballs")) {
        PopulateSnowBall(worm.snowball, wormJson["snowballs"]);
        worm.snowball_count = wormJson["snowballs"]["count"].GetInt();
    }
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

void GameState::PopulateSnowBall(SnowBall& snowball, const rapidjson::Value& wJson)
{
    /*
    "snowballs": {
                    "freezeDuration": 5,
                    "range": 5,
                    "count": 3,
                    "freezeRadius": 1
                },
                */
    auto weaponJson = wJson.GetObject();
    snowball.freezeDuration = wJson["freezeDuration"].GetInt();
    snowball.range = wJson["range"].GetInt();
    snowball.freezeRadius = wJson["freezeRadius"].GetInt();
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
    } else if (mapLavas[pos.y] & (first_bit_set >> pos.x)) {
        ret.type = CellType::LAVA;
    }else {
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
    auto posBit = (MAP_SIZE*pos.y + pos.x);

    switch(type) {
        case CellType::AIR:
            mapDeepSpaces.reset(posBit);
            mapDirts.reset(posBit);
            mapLavas.reset(posBit);
        break;
        case CellType::DEEP_SPACE:
            mapDirts.reset(posBit);
            mapLavas.reset(posBit);
            mapDeepSpaces.set(posBit);
        break;
        case CellType::DIRT:
            mapDeepSpaces.reset(posBit);
            mapLavas.reset(posBit);
            mapDirts.set(posBit);
        break;
        case CellType::LAVA:
            mapDeepSpaces.reset(posBit);
            mapDirts.reset(posBit);
            mapLavas.set(posBit);
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

//returns closest dirt in terms of move distance
//returns {-1, -1} if no dirts
Position GameState::Closest_dirt(const Position& fromPos) 
{
    //spiral around from worm and return the first dirt u find
    unsigned extents = 1;
    while(extents < (MAP_SIZE)) {

        Position NW = fromPos + Position(-extents,-extents);
        NW.ClampToMap();
        Position NE = fromPos + Position( extents,-extents);
        NE.ClampToMap();
        Position SE = fromPos + Position( extents, extents);
        SE.ClampToMap();
        Position SW = fromPos + Position(-extents, extents);
        SW.ClampToMap();

        for(Position pos = NW; pos != NE; ++pos.x) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }
        for(Position pos = NE; pos != SE; ++pos.y) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }
        for(Position pos = SE; pos != SW; --pos.x) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }
        for(Position pos = SW; pos != NW; --pos.y) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }

        ++extents;
    }

    return {-1,-1};
}

int GameState::Dist_to_closest_enemy(bool player1) 
{
    //get distance to closest enemy
    //pos of my worm:
    Player * me = GetPlayer(player1);
    Worm * worm = me->GetCurrentWorm();
    Position myWormPos = worm->position;

    //now calc
    Player * enemy = GetPlayer(!player1);
    int closestDist = 9999;
    for(auto const & worm : enemy->worms) {
        if(worm.IsDead() || !worm.position.IsOnMap()) {
            continue;
        }
        auto dist = myWormPos.EuclideanDistanceTo(worm.position);
        if(dist < closestDist){
            closestDist = dist;
        }
    }

    return closestDist;
}

//return all worms if dist is -1
std::vector<Worm*> GameState::WormsWithinDistance(Position pos, int dist)
{
    std::vector<Worm*> ret;

    ForAllWorms([&](Worm& worm) {
        if(dist < 0 || (!worm.IsDead() && pos.EuclideanDistanceTo(worm.position) <= dist)) {
            ret.push_back(&worm);
        }
    });

    return ret;
}

bool GameState::operator==(const GameState &other) const
{
    //std::cerr << "(" << __FUNCTION__ << ") deepSpacesGood: " << deepSpacesGood <<
    //" dirtsGood: " << dirtsGood << 
    //" healthPackPos == other.healthPackPos: " << (healthPackPos == other.healthPackPos) <<
    //" roundNumber == other.roundNumber: " << (roundNumber == other.roundNumber) <<
    //std::endl;

    return mapDeepSpaces == other.mapDeepSpaces && 
            mapDirts == other.mapDirts && 
            mapLavas == other.mapLavas && 
            player1 == other.player1 && 
            player2 == other.player2 &&
            healthPackPos == other.healthPackPos &&
            roundNumber == other.roundNumber;
}
