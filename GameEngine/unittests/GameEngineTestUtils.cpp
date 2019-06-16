#include "GameEngineTestUtils.hpp"
#include <fstream>
#include <sstream>

Worm* place_worm(bool player1, int wormNumber, Position pos, std::shared_ptr<GameState> state)
{
    Worm* worm_under_test;
    if(player1) {
        worm_under_test = &state->player1.worms[wormNumber - 1];
    } else {
        worm_under_test = &state->player2.worms[wormNumber - 1];
    }
    worm_under_test->position = worm_under_test->previous_position = pos;
    state->Cell_at(pos)->worm = worm_under_test;

    return worm_under_test;
};

void place_powerup(Position pos, std::shared_ptr<GameState> state)
{
    state->Cell_at(pos)->powerup = &state->healthPack;
};

rapidjson::Document ReadJsonFile(std::string filePath)
{
    std::ifstream dataIn;
    dataIn.open(filePath, std::ifstream::in);
    if(!dataIn.is_open()) {
        throw std::runtime_error("Problem loading state file in unit test");
    }

    std::stringstream buffer;
    buffer << dataIn.rdbuf();
    std::string stateJson = buffer.str();
    rapidjson::Document roundJSON;
    const bool parsed = !roundJSON.Parse(stateJson.c_str()).HasParseError();
    if(!parsed) {
        throw std::runtime_error("Problem parsing state file in unit test");
    }

    return roundJSON;
}