#include "Evaluators.hpp"


    //NOTE: be careful evaluating things in terms of myPlayer->GetCurrentWorm().
    // this will use whichever worm is current at the tip of the playout, NOT the actuall current worm!
    //    Worm* worm = myPlayer->GetCurrentWorm();

    //NOTE: evaulators are not always zero-sum.
    //Calling classes need to call this separately for each player if they want both results.

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
    float max = 80.0f;
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

    float healthdiff_Score = (healthdiff/150.0f);
    float enemyWorms_Score = numWormsHim/3.0f;
    float scorediff_Score = scorediff / 500;

    float healthdiff_Factor = 1.0f;
    float enemyWorms_Factor = 1.0f;
    float scorediff_Factor = 0.5f;

    healthdiff_Score *= healthdiff_Factor;
    enemyWorms_Score *= enemyWorms_Factor;
    scorediff_Score *= scorediff_Factor;

    float total_score = healthdiff_Score + enemyWorms_Score + scorediff_Score;
    float denom = healthdiff_Factor + enemyWorms_Factor + scorediff_Factor;

    return total_score / denom;

}

float Evaluators::RushHealth (bool player1, GameStatePtr state)
{
    Player* myPlayer = state->GetPlayer(player1);

    Worm* worm = myPlayer->GetWormById(1);

    if(worm == nullptr) {
        return 0;
    }

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
