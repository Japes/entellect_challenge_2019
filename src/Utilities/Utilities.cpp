#include "Utilities.hpp"

std::string Utilities::ReadFile(std::string path)
{
    //read in file
    std::ifstream dataIn;
    dataIn.open(path, std::ifstream::in);
    if(!dataIn.is_open()) {
        throw std::runtime_error("Problem loading command in unit test");
    }

    std::stringstream buffer;
    buffer << dataIn.rdbuf();
    std::string fileContents = buffer.str();
    return fileContents;
}

rapidjson::Document Utilities::ReadJsonFile(std::string filePath)
{
    std::string stateJson = ReadFile(filePath);

    rapidjson::Document roundJSON;
    const bool parsed = !roundJSON.Parse(stateJson.c_str()).HasParseError();
    if(!parsed) {
        throw std::runtime_error("Problem parsing state file in unit test");
    }

    return roundJSON;
}