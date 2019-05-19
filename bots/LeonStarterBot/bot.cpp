//file io -----------------------------

#include <fstream>
#include <iostream>

//Find the json library used here:
//https://github.com/nlohmann/json
#include "json.hpp"
using json = nlohmann::json;

json Load_json(std::string filename)
{
	std::ifstream i(filename);
	if(!i.is_open()){
		std::cout << "file: " << filename << " doesn't exist." << std::endl;
		return {};
	}
	json j;
	i >> j;
	return j;
}

void Save_json(std::string filename, const json& j)
{
	std::ofstream o(filename);
	o << j;
}

//control io --------------------------

int Next_round()
{
	int i;
	std::cin >> i;
	return i;
}

//Action can be one of
//	nothing
//	move x y
//	dig x y
//	shoot direction
//Direction is one of { N, NE, E, SE, S, SW, W, NW }
void Perform_action(int round, std::string action)
{
	std::cout << "C;" << round << ";" << action << std::endl;
	std::cout.flush();
}

//state -------------------------------

enum class Block {space, air, dirt};

struct Worm
{
	int id;
	int diggingRange;
	int movementRange;
	int health;
	int x;
	int y;
};

struct Player
{
	int id;
	//int health;can get from worms and not given for opponent
	int score;
	std::vector<Worm> worms;
};

struct State
{
	int mapSize;
	int maxRounds;
	int currentRound;
	int currentWormId;
	int consecutiveDoNothingCount;
	Player self;
	Player opponent;
	std::vector<std::vector<Block>> map;
};

//state from json ---------------------

void from_json(const json& j, Block& o)
{
	std::string type = j.at("type");
	if(type == "DEEP_SPACE"){
		o = Block::space;
	}else if(type == "AIR"){
		o = Block::air;
	}else if(type == "DIRT"){
		o = Block::dirt;
	}
}

void from_json(const json& j, Worm& o)
{
	o.id = j.at("id").get<int>();
	o.diggingRange = j.at("diggingRange").get<int>();
	o.movementRange = j.at("movementRange").get<int>();
	o.health = j.at("health").get<int>();
	o.x = j.at("position").at("x").get<int>();
	o.y = j.at("position").at("y").get<int>();
}

void from_json(const json& j, Player& o)
{
	o.id = j.at("id").get<int>();
	//o.health = j.at("health").get<int>();
	o.score = j.at("score").get<int>();
	o.worms = j.at("worms").get<std::vector<Worm>>();
}

void from_json(const json& j, State& o)
{
	o.mapSize = j.at("mapSize").get<int>();
	o.maxRounds = j.at("maxRounds").get<int>();
	o.currentRound = j.at("currentRound").get<int>();
	o.currentWormId = j.at("currentWormId").get<int>();
	o.consecutiveDoNothingCount = j.at("consecutiveDoNothingCount").get<int>();
	o.self = j.at("myPlayer").get<Player>();
	o.opponent = j.at("opponents")[0].get<Player>();
	o.map = j.at("map").get<std::vector<std::vector<Block>>>();
}

//logic -------------------------------

#include <optional>
#include <stdlib.h>

bool In_range(const State& state, int x, int y, int dx, int dy)
{
	int adx = std::abs(dx);
	int ady = std::abs(dy);
	int count = 4;
	if(dx * dy != 0){
		if( adx != ady){
			return false;
		}
		count = 3;
	}
	if(dx != 0){
		dx /= adx;
	}
	if(dy != 0){
		dy /= ady;
	}
	for(int i = 1; i < count; ++i){
		x += dx;
		y += dy;
		if(x < 0 || y < 0 || x > state.mapSize || y > state.mapSize){
			return false;
		}
		if(state.map[y][x] != Block::air){
			return false;
		}
	}
	return true;
}

std::string Shoot(int dx, int dy)
{
	if(dx == 0){
		if(dy < 0){
			return "shoot N";
		}
		return "shoot S";
	}
	if(dy == 0){
		if(dx < 0){
			return "shoot E";
		}
		return "shoot W";
	}
	if(dx == dy){
		if(dy < 0){
			return "shoot NW";
		}
		return "shoot SE";
	}
	if(dy < 0){
		return "shoot NE";
	}
	return "shoot SW";
}

std::optional<std::string> Attack_command(const State& state)
{
	int id = state.currentWormId;
	int x = state.self.worms[id].x;
	int y = state.self.worms[id].y;
	for(auto& enemy : state.opponent.worms){
		int dx = x - enemy.x;
		int dy = y - enemy.y;
		if(In_range(state, x, y, dx, dy)){
			return Shoot(dx, dy);
		}
	}
	return std::nullopt;
}

std::optional<std::string> Move_command(const State& state)
{
	int move_index = rand() % 8;
	int xarr[] = {1,1, 1, 0,-1,-1,-1,0};
	int yarr[] = {1,0,-1,-1,-1, 0, 1,1};
	int dx = xarr[move_index];
	int dy = yarr[move_index];
	int id = state.currentWormId;
	int x = state.self.worms[id].x + dx;
	int y = state.self.worms[id].y + dy;
	if(x < 0 || y < 0 || x > state.mapSize || y > state.mapSize){
		return "nothing";
	}
	Block b = state.map[y][x];
	if(b == Block::air){
		return "move (" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
	if(b == Block::dirt){
		return "dig (" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
	if(b == Block::space){
		return "nothing";
	}
	return "nothing";
}

std::string Starter_bot_logic(const State& state)
{
	/*
	   If one of the opponent's worms is within range fire at it.
	   - Must be in range of current worm's weapon range.
	   - No obstacles can be in the path.

	   Otherwise choose a block in a random direction and do one of the following things
	   - If the chosen block is air, move to that block
	   - If the chosen block is dirt, dig out that block
	   - If the chosen block is deep space, do nothing

	   Commands in the format :
	   MOVE - move <x> <y>
	   DIG - dig <x> <y>
	   SHOOT - shoot <direction { N, NE, E, SE, S, SW, W, NW }>
	   DO NOTHING - nothing


	 ****THIS IS WHERE YOU CAN ADD OR CHANGE THE LOGIC OF THE BOT****
	 */
	auto attack = Attack_command(state);
	if(attack.has_value()){
		return attack.value();
	}
	auto move = Move_command(state);
	if(move.has_value()){
		return move.value();
	}
	return "nothing";
}

//main loop ---------------------------

int main()
{
	while(1){
		int round_number = Next_round();
		State state = Load_json("state.json");
		std::string action = Starter_bot_logic(state);
		Perform_action(round_number, action);
	}
	return 0;
}