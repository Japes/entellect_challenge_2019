#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "GameConfig.hpp"
#include "Player.hpp"
#include "Cell.hpp"
#include "./rapidjson/document.h"
#include <functional>

//current state of the game
class GameState
{
	public:
    Player player1;
    Player player2;
    int roundNumber;
    Cell map[MAP_SIZE][MAP_SIZE];
    PowerUp healthPack; //just here so that cells can reference something static


    //TODO can use 32bits here if I tread some of the deep space blocks as blocks in the rows that have more than 32 spaces
    uint64_t mapDirts[MAP_SIZE];
    Position healPackPos[2];

    GameState();
    GameState(const GameState& p);
    GameState(rapidjson::Document& roundJSON);

//    const Cell* Cell_at(Position pos) const;
    Cell* Cell_at(Position pos);
    void SetCellTypeAt(Position pos, CellType type);
    void PlacePowerupAt(Position pos, int powerupIndex);
    void ClearPowerupAt(Position pos);
    void PlaceWormAt(Position pos, Worm* worm);
    void ClearWormAt(Position pos);

    void Move_worm(Worm* worm, Position pos);

    Player* GetPlayer(bool player1);
    void ForAllWorms(std::function<void(Worm&)> wormFn);

    bool operator==(const GameState &other) const;

    private:
    void UpdateRefs();
    void UpdateRefs(Player& player);

    void PrintJson(const rapidjson::Value& json);

    void PopulatePlayers(rapidjson::Document& roundJSON);
    void PopulatePlayer(Player& player, const rapidjson::Value& playerJson);
    void PopulateWorm(Worm& worm, const rapidjson::Value& wormJson);
    void PopulateWeapon(Weapon& weapon, const rapidjson::Value& weaponJson);
    void PopulateBanana(BananaBomb& banana, const rapidjson::Value& wJson);

    void PopulateMap(rapidjson::Document& roundJSON);

    void PopulatePosition(Position& pos, const rapidjson::Value& posJson);
};

#endif
