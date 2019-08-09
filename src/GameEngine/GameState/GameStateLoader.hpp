#ifndef GAMESTATELOADER_H
#define GAMESTATELOADER_H

#include "GameState.hpp"
#include "GameConfig.hpp"
#include "Player.hpp"
#include "Cell.hpp"
#include "./rapidjson/document.h"
#include <functional>
#include <bitset>

//class for loading a gamestate from json
class GameStateLoader
{
	public:
    static std::shared_ptr<GameState> LoadGameStatePtr(rapidjson::Document& roundJSON);
    static GameState LoadGameState(rapidjson::Document& roundJSON);

    static std::shared_ptr<Command> GetCommandFromString(std::string cmd);

    private:

    void PrintJson(const rapidjson::Value& json);

    static void PopulatePlayers(GameState& state, rapidjson::Document& roundJSON);
    static void PopulatePlayer(Player& player, const rapidjson::Value& playerJson);
    static void PopulateWorm(Worm& worm, const rapidjson::Value& wormJson);
    static void PopulateWeapon(Weapon& weapon, const rapidjson::Value& weaponJson);
    static void PopulateBanana(BananaBomb& banana, const rapidjson::Value& wJson);
    static void PopulateSnowBall(SnowBall& snowball, const rapidjson::Value& wJson);

    static void PopulateMap(GameState& state, rapidjson::Document& roundJSON);

    static void PopulatePosition(Position& pos, const rapidjson::Value& posJson);

    static Position GetCommandPosition(std::string str);

};

#endif
