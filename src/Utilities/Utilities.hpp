#ifndef UTILITIES_H
#define UTILITIES_H

#include <fstream>
#include <string>
#include <sstream>
#include "./rapidjson/document.h"

class Utilities
{
    public:
    static std::string ReadFile(std::string path);
    static rapidjson::Document ReadJsonFile(std::string filePath);
};

#endif