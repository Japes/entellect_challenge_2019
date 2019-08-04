#include "GameStateLoader.hpp"
#include "AllCommands.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <cmath>

GameState GameStateLoader::LoadGameState(rapidjson::Document& roundJSON)
{
    GameState ret;
    ret.roundNumber = roundJSON["currentRound"].GetInt();
    PopulatePlayers(ret, roundJSON);
    PopulateMap(ret, roundJSON);
    return ret;
}

void GameStateLoader::PrintJson(const rapidjson::Value& json)
{
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
    json.Accept(writer);
    auto str = sb.GetString();
    printf("%s\n", str);
}

void GameStateLoader::PopulatePlayers(GameState& state, rapidjson::Document& roundJSON)
{
    const rapidjson::Value& myPlayerJson = roundJSON["myPlayer"].GetObject();
    const rapidjson::Value& otherPlayerJson = roundJSON["opponents"].GetArray()[0].GetObject(); //assuming 1 opponent for now

    Player& myPlayer    = (myPlayerJson["id"].GetInt() == 1)?       state.player1 : state.player2;
    Player& otherPlayer = (otherPlayerJson["id"].GetInt() == 1)?    state.player1 : state.player2;

    PopulatePlayer(myPlayer, myPlayerJson);
    PopulatePlayer(otherPlayer, otherPlayerJson);

    myPlayer.consecutiveDoNothingCount = roundJSON["consecutiveDoNothingCount"].GetInt();
}

void GameStateLoader::PopulatePlayer(Player& player, const rapidjson::Value& playerJson)
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
Position GameStateLoader::GetCommandPosition(std::string str)
{
    std::size_t firstSpace = str.find(" ", 0);
    std::size_t secondSpace = str.find(" ", firstSpace + 1);

    int x = std::stoi(str.substr(firstSpace, secondSpace - firstSpace));
    int y = std::stoi(str.substr(secondSpace + 1, str.size() - secondSpace + 1));

    return {x,y};
}

std::shared_ptr<Command> GameStateLoader::Str2Cmd(std::string str)
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

void GameStateLoader::PopulateWorm(Worm& worm, const rapidjson::Value& wormJson)
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

    worm.roundsUntilUnfrozen = wormJson["roundsUntilUnfrozen"].GetInt();

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

void GameStateLoader::PopulateWeapon(Weapon& weapon, const rapidjson::Value& wJson)
{
    auto weaponJson = wJson.GetObject();
    weapon.damage = wJson["damage"].GetInt();
    weapon.range = wJson["range"].GetInt();
    weapon.diagRange = std::ceil(std::sqrt((weapon.range*weapon.range)/2)); //inverse of euclidian SEEMS TO BE A PROBLEM WITH THIS WHEN i SUBMIT...
}

void GameStateLoader::PopulateBanana(BananaBomb& banana, const rapidjson::Value& wJson)
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

void GameStateLoader::PopulateSnowBall(SnowBall& snowball, const rapidjson::Value& wJson)
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

void GameStateLoader::PopulatePosition(Position& pos, const rapidjson::Value& posJson)
{
    auto positionJson = posJson.GetObject();
    pos.x = positionJson["x"].GetInt();
    pos.y = positionJson["y"].GetInt();
}


void GameStateLoader::PopulateMap(GameState& state, rapidjson::Document& roundJSON)
{
    if(roundJSON["mapSize"].GetInt() != GameConfig::mapSize) {
        std::cerr << "(GameStateLoader::PopulateMap) map size in this json is " << roundJSON["mapSize"].GetInt() << ", expected " << GameConfig::mapSize << ". Not loading map." << std::endl;
        return;
    }

    for (rapidjson::Value::ConstValueIterator rowItr = roundJSON["map"].Begin(); rowItr != roundJSON["map"].End(); ++rowItr) {
        for (rapidjson::Value::ConstValueIterator colItr = (*rowItr).Begin(); colItr != (*rowItr).End(); ++colItr) {
            int x = (*colItr)["x"].GetInt();
            int y = (*colItr)["y"].GetInt();
            Position pos(x,y);
            CellType type =  Cell::strToCellType((*colItr)["type"].GetString());

            state.SetCellTypeAt(pos, type);
            state.ClearPowerupAt(pos);

            if((*colItr).HasMember("occupier")) {
                int wormId = (*colItr)["occupier"].GetObject()["id"].GetInt();
                int playerId = (*colItr)["occupier"].GetObject()["playerId"].GetInt();
                Player& wormOwner = playerId == 1? state.player1 : state.player2;
                state.PlaceWormAt(pos, &wormOwner.worms[wormId - 1]);
            }
            if((*colItr).HasMember("powerup")) {
                //TODO this is where we'd distinguish between different types
                state.PlacePowerupAt(pos);
            }
        }
    }
}
