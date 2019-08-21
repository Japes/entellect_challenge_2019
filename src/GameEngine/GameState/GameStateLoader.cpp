#include "GameStateLoader.hpp"
#include "AllCommands.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <cmath>

std::shared_ptr<GameState> GameStateLoader::LoadGameStatePtr(rapidjson::Document& roundJSON)
{
    auto ret = std::make_shared<GameState>();
    ret->roundNumber = roundJSON["currentRound"].GetInt();
    PopulatePlayers(*ret, roundJSON);
    PopulateMap(*ret, roundJSON);
    return ret;
}

GameState GameStateLoader::LoadGameState(rapidjson::Document& roundJSON)
{
    std::shared_ptr<GameState> ret = LoadGameStatePtr(roundJSON);
    return *ret;
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
    player.previousCommand = GetCommandFromString(playerJson["previousCommand"].GetString());
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

//expects strings of form:
//`No Command`
//`move 25 27`
//`select 1;move 25 27`
std::shared_ptr<Command> GameStateLoader::GetCommandFromString(std::string cmd)
{
    const std::size_t firstSpace = cmd.find(" ");
    const std::size_t endLine = cmd.find("\n");
    const std::string moveType = cmd.substr(0, firstSpace);

    if(moveType == "move") {
        return std::make_shared<TeleportCommand>(GetCommandPosition(cmd));

    } else if (moveType == "dig") {
        return std::make_shared<DigCommand>(GetCommandPosition(cmd));

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
        else {return nullptr;}

        auto ret = std::make_shared<ShootCommand>(dir);
        return ret;

    } else if (moveType == "banana") {
        return std::make_shared<BananaCommand>(GetCommandPosition(cmd));

    } else if (moveType == "snowball") {
        return std::make_shared<SnowballCommand>(GetCommandPosition(cmd));

    } else if (moveType == "nothing") {
        return std::make_shared<DoNothingCommand>();

    } else if (moveType == "select") {
        std::string indexStr = cmd.substr(firstSpace + 1, 1); 
        int index = std::stoi(indexStr); //will always be 1 digit

        size_t startOfSelected = firstSpace + 4;
        std::string selectedCmdstr = cmd.substr(startOfSelected, cmd.length() - startOfSelected );
        std::shared_ptr<Command> selectedCmd = GetCommandFromString(selectedCmdstr);

        return std::make_shared<SelectCommand>(index, selectedCmd);
    } 

    return std::make_shared<TeleportCommand>(Position(-10,-10)); //it was invalid for some reason (timeout, invalid command, etc)
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
            std::string cellType = (*colItr)["type"].GetString();
            CellType type =  Cell::strToCellType(cellType);
            state.SetCellTypeAt(pos, type);

            if(cellType == "LAVA") {
                state.AddLavaAt(pos, state.roundNumber);
            }

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
