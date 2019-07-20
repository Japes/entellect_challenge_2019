#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "GameConfig.hpp"
#include "Player.hpp"
#include "Cell.hpp"
#include "./rapidjson/document.h"
#include <functional>

#define first_bit_set (0x8000000000000000)

//current state of the game
class GameState
{
	public:
    Player player1;
    Player player2;
    int roundNumber;
    PowerUp healthPack; //just here so that cells can reference something static

    GameState();
    GameState(const GameState& p);
    GameState(rapidjson::Document& roundJSON);

    Cell Cell_at(Position pos);
    void SetCellTypeAt(Position pos, CellType type);
    void PlacePowerupAt(Position pos);
    void ClearPowerupAt(Position pos);
    void PlaceWormAt(Position pos, Worm* worm);

    void Move_worm(Worm* worm, Position pos);

    Player* GetPlayer(bool player1);
    void ForAllWorms(std::function<void(Worm&)> wormFn);

    bool operator==(const GameState &other) const;

    private:
    
    //TODO can use 32bits here if I treat some of the deep space blocks as blocks in the rows that have more than 32 spaces
    uint64_t mapDeepSpaces[MAP_SIZE] = {0}; //TODO no need to store this actually, it is always the same
    uint64_t mapDirts[MAP_SIZE] = {0};
    std::vector<Position> healthPackPos;

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

    public:

    inline CellType CellType_at(Position pos)
    {
        if(mapDeepSpaces[pos.y] & (first_bit_set >> pos.x) ) {
            return CellType::DEEP_SPACE;
        } else if (mapDirts[pos.y] & (first_bit_set >> pos.x)) {
            return CellType::DIRT;
        }
            
        return CellType::AIR;
    }

    inline PowerUp* PowerUp_at(Position pos)
    {
        for(auto & hpPos : healthPackPos) {
            if(hpPos == pos) {
                return &healthPack;
            } 
        }

        return nullptr;
    }

    inline Worm* Worm_at(Position pos)
    {
        Worm* ret = nullptr;

        for(auto & w : player1.worms) {
            if(w.position == pos && !w.IsDead()) {
                ret = &w;
            }
        }
        for(auto & w : player2.worms) {
            if(w.position == pos && !w.IsDead()) {
                ret = &w;
            }
        }

    /*
        ForAllWorms([&] (Worm& w) {
            if(w.position == pos && !w.IsDead()) {
                ret = &w;
            }
        });
        */

        return ret;
    }

    inline int DirtsBananaWillHit(const Position& pos) {
        return 0;
        /*
        int ret = 0;
        if(!pos.IsOnMap() || CellType_at(pos) == CellType::DEEP_SPACE ) {
            return ret;
        }
        auto dirtsHit = mapDirts[pos.y] & pos.x;
        //ret += 
        uint64_t mapDirts[MAP_SIZE] = {0};
        */
    }
};

#endif
