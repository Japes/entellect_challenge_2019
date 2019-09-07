#include "Evaluators.hpp"

float Evaluators::AveHpScore (bool player1, GameStatePtr state)
{
    float bestPossible = 120 + 1000/10; //rough estimate...ave hp + score/10

    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    int numliveWormsMe = 0;
    state->ForAllLiveWorms(player1, [&](Worm& worm) {
        ++numliveWormsMe;
    });

    int numliveWormsHim = 0;
    state->ForAllLiveWorms(!player1, [&](Worm& worm) {
        ++numliveWormsHim;
    });

    float healthdiff = (myPlayer->health/numliveWormsMe) - (otherPlayer->health/numliveWormsHim);
    float scorediff =  myPlayer->GetScore() - otherPlayer->GetScore();

    return ( healthdiff + scorediff/10 ) / bestPossible;
}

float Evaluators::AverageHealth (bool player1, GameStatePtr state)
{
    float bestPossible = 470.0f/3.0f; //rough estimate...3 full health worms + 2 health packs

    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    int numliveWormsMe = 0;
    state->ForAllLiveWorms(player1, [&](Worm& worm) {
        ++numliveWormsMe;
    });

    int numliveWormsHim = 0;
    state->ForAllLiveWorms(!player1, [&](Worm& worm) {
        ++numliveWormsHim;
    });

    return ( (myPlayer->health/numliveWormsMe) - (otherPlayer->health/numliveWormsHim) ) / bestPossible;
}

float Evaluators::Health (bool player1, GameStatePtr state)
{
    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    return (myPlayer->health - otherPlayer->health) / 470.0f;
}

//want this to be low if round is early and bombs are few
//max if roundnumber > 350
//max worth around 80 health
float Evaluators::GetBananaBonus(int numBananas, int roundNumber)
{
    float max = 160.0f;
    if(roundNumber > 350) {
        return 0;
    }

    float frac = ( numBananas * (350.0f - roundNumber) ) / (3.0f*350.0f);
    return max*frac;
}

float Evaluators::MaxHpScore (bool player1, GameStatePtr state)
{
    float bestPossible = (170*3) + (900/5); //rough estimate

    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    int maxHealthMe = 0;
    state->ForAllLiveWorms(player1, [&](Worm& worm) {
        if(worm.health > maxHealthMe) {
            maxHealthMe = worm.health;
        }
    });

    int maxHealthHim = 0;
    int numWormsHim = 0;
    state->ForAllLiveWorms(!player1, [&](Worm& worm) {
        ++numWormsHim;
        if(worm.health > maxHealthHim) {
            maxHealthHim = worm.health;
        }
    });

    float healthdiff = maxHealthMe - maxHealthHim;
    float scorediff =  myPlayer->GetScore() - otherPlayer->GetScore();

    //want to encourage holding on to the banana
    float bananaBonus = GetBananaBonus(myPlayer->worms[1].banana_bomb_count, state->roundNumber);

    return ((healthdiff*numWormsHim) + (scorediff/5) + bananaBonus) / bestPossible;
}

float Evaluators::RushHealth (bool player1, GameStatePtr state)
{
    Player* myPlayer = state->GetPlayer(player1);
    Worm* worm = myPlayer->GetCurrentWorm();

    float magic = 1000000;
    float minDist = magic;
    for(auto const& pos : state->GetHealthPackPos()) {
        int dist = worm->position.MovementDistanceTo(pos);
        if(dist < minDist) {
            minDist = dist;
        }
    }

    if(minDist == magic) {
        return 0;
    }

    float worstDist = 17.0f;
    return (worstDist - minDist)/17.0f;
}


float Evaluators::Score(bool player1, GameStatePtr state)
{
    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    return (myPlayer->GetScore() - otherPlayer->GetScore()) / 1000.0f;
}

//dance around without shooting the opponent
float Evaluators::Dance(bool player1, GameStatePtr state)
{
    Player* myPlayer = state->GetPlayer(player1);
    Player* otherPlayer = state->GetPlayer(!player1);

    float maxhealth = (100.0f*2 + 150.0f)*2;
    float scoreDiff = (myPlayer->GetScore() - otherPlayer->GetScore()) / 100.0f;

    return (myPlayer->health + otherPlayer->health + scoreDiff) / maxhealth;
}
