#include "Worm.hpp"
#include "GameState.hpp"

Worm::Worm(GameState* _state, bool agent) : state{_state}, id{0}, position{-1,-1}, movedThisRound{0}
{
    if(agent) {
        proffession = Proffession::AGENT;
        health = GameConfig::agentWorms.initialHp;
        diggingRange = GameConfig::agentWorms.diggingRange;
        movementRange = GameConfig::agentWorms.movementRange;
        banana_bomb_count = GameConfig::agentWorms.banana.count;
    } else {
        proffession = Proffession::COMMANDO;
        health = GameConfig::commandoWorms.initialHp;
        diggingRange = GameConfig::commandoWorms.diggingRange;
        movementRange = GameConfig::commandoWorms.movementRange;
        banana_bomb_count = 0;
    }
}

bool Worm::IsDead() const
{
    return (health <= 0);
}

void Worm::TakeDamage(int dmgAmount)
{
    health -= dmgAmount;
    state->player1.RecalculateHealth();
    state->player2.RecalculateHealth();
}

bool Worm::operator==(const Worm &other) const
{
    //std::cerr << "(" << __FUNCTION__ << ") " 
    //" id: " << (id == other.id) << "(" << id << ", " << other.id << ")" <<
    //" health: " << (health == other.health) << "( " << health << ", " << other.health << ")" <<
    //" position: " << (position == other.position) <<
    //" previous_position: " << (previous_position == other.previous_position) <<
    //" weapon: " << (weapon == other.weapon) <<
    //" banana_bomb: " << (banana_bomb == other.banana_bomb) <<
    //" banana_bomb_count: " << (banana_bomb_count == other.banana_bomb_count) <<
    //" diggingRange: " << (diggingRange == other.diggingRange) <<
    //" movementRange: " << (movementRange == other.movementRange) <<
    //std::endl;

    return (id == other.id &&
            proffession == other.proffession &&
            health == other.health &&
            position == other.position &&
            //previous_position == other.previous_position && //not loaded from state (this operator only used for unit testing)
            weapon == other.weapon &&
            banana_bomb == other.banana_bomb &&
            //banana_bomb_count == other.banana_bomb_count && //not loaded from state (this operator only used for unit testing)
            diggingRange == other.diggingRange &&
            movementRange == other.movementRange);
}
