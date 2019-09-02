#include "PatternDetector.hpp"

PatternDetector::PatternDetector(unsigned maxSequenceSize) :
    _maxSequenceSize(maxSequenceSize)
{

}
    
void PatternDetector::AddCommand(std::shared_ptr<Command> cmd)
{
    _cmds.push_front(cmd);
    if(_cmds.size() > (_maxSequenceSize*2 + 1)) {
        _cmds.pop_back();
    }
}

//can return null
std::shared_ptr<Command> PatternDetector::Prediction()
{
    // latest                       first                       2nd
    //                              seq                         seq
    //                              marker                      mark
    // x      ->     o      ->      x     ->     o     ->       x

    if(_cmds.size() == 0) {
        return nullptr;
    }

    auto latest = _cmds.front();

    //get sequence length
    unsigned sequenceLength = 0;
    auto firstSeqMarker = _cmds.end();
    for(auto it = _cmds.begin() + 1; it != _cmds.end(); ++it) {
        ++sequenceLength;
        if(AreEqual(latest, (*it))) {
            firstSeqMarker = it;
            break;
        }
    }

    //check if I could have 2 sequences
    if( sequenceLength == 0 || (_cmds.size() - 1) < sequenceLength*2) {
        return nullptr;
    }
    auto secondSeqMarker = firstSeqMarker + sequenceLength;
    if(!AreEqual(*firstSeqMarker, *secondSeqMarker) ) {
        return nullptr;
    }

    //check if sequences are the same
    for(auto it = _cmds.begin() + 1; it != firstSeqMarker; ++it) {
        if( !AreEqual(*it, *(it + sequenceLength)) ) {
            return nullptr;
        }
    }

    //we have a sequence!
    return *(firstSeqMarker-1);
}

bool PatternDetector::AreEqual(std::shared_ptr<Command> l, std::shared_ptr<Command> r)
{
    return l->GetCommandString() == r->GetCommandString();
}
