#ifndef PATTERNDETECTOR_H
#define PATTERNDETECTOR_H

#include <memory>
#include <deque>
#include "Commands/Command.hpp"

//takes in sequences of moves, tries to detect patterns, can predict next move and provide pattern sequence

class PatternDetector
{
    public:
    PatternDetector(unsigned maxSequenceSize);
    
    void AddCommand(std::shared_ptr<Command> cmd);
    std::shared_ptr<Command> Prediction(); //can return null
    //TODO access whole sequence

    private:
    unsigned _maxSequenceSize;
    std::deque<std::shared_ptr<Command>> _cmds;

    bool AreEqual(std::shared_ptr<Command> l, std::shared_ptr<Command> r);
};


#endif
