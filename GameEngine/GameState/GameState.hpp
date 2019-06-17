#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "GameConfig.hpp"
#include "Player.hpp"
#include "Cell.hpp"
#include "./rapidjson/document.h"

//current state of the game
class GameState
{
	public:
    Player player1;
    Player player2;
    Cell map[MAP_SIZE][MAP_SIZE];
    int roundNumber;
    PowerUp healthPack; //just here so that cells can reference something static

    GameState();
    GameState(const GameState& p);
    GameState(rapidjson::Document& roundJSON);

    Cell* Cell_at(Position pos);

    void Move_worm(Worm* worm, Position pos);

    bool operator==(const GameState &other) const;

    private:
    void UpdateRefs();
    void UpdateRefs(Player& player);

    void PrintJson(const rapidjson::Value& json);

    void PopulatePlayers(rapidjson::Document& roundJSON);
    void PopulatePlayer(Player& player, const rapidjson::Value& playerJson);
    void PopulateWorm(Worm& worm, const rapidjson::Value& wormJson);
    void PopulateWeapon(Weapon& weapon, const rapidjson::Value& weaponJson);

    void PopulateMap(rapidjson::Document& roundJSON);

    void PopulatePosition(Position& pos, const rapidjson::Value& posJson);
};

#endif
