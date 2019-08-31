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
    static uint64_t Get_ns_since_epoch();

    //assumes fraction is between -1 and 1
    //-1 should return min
    //+1 should return max
    static inline float NormaliseTo(float fraction, float min, float max)
    {
        if(fraction > 1) {
            return max;
        }
        if(fraction < -1) {
            return min;
        }
        float halfRange = (max-min)/2;
        float halfway = halfRange + min;
        return (fraction*(halfRange)) + halfway;
    }
};

#endif