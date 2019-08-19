#include "Worm.hpp"
#include "GameState.hpp"

Worm::Worm(GameState* _state, Worm::Proffession _proffession) : state{_state}, playerId{0}, id{0}, position{-1,-1}, 
                                                                movedThisRound{false}, diedByLavaThisRound{false}, frozenThisRound{false}, roundsUntilUnfrozen{0}
{
    lastAttackedBy.reserve(6);
    SetProffession(_proffession);
}

void Worm::SetProffession(Worm::Proffession _proffession)
{
    proffession = _proffession;

    switch (proffession) {
        case Proffession::COMMANDO:
        {
            health = GameConfig::commandoWorms.initialHp;
            diggingRange = GameConfig::commandoWorms.diggingRange;
            movementRange = GameConfig::commandoWorms.movementRange;
            banana_bomb_count = 0;
            snowball_count = 0;
        }
        break;
        case Proffession::AGENT:
        {
            health = GameConfig::agentWorms.initialHp;
            diggingRange = GameConfig::agentWorms.diggingRange;
            movementRange = GameConfig::agentWorms.movementRange;
            banana_bomb_count = GameConfig::agentWorms.banana.count;
            snowball_count = 0;
        }
        break;
        case Proffession::TECHNOLOGIST:
        {
            health = GameConfig::technologistWorms.initialHp;
            diggingRange = GameConfig::technologistWorms.diggingRange;
            movementRange = GameConfig::technologistWorms.movementRange;
            banana_bomb_count = 0;
            snowball_count = GameConfig::technologistWorms.snowball.count;
        }
        break;
    }
}

bool Worm::IsDead() const
{
    return (health <= 0);
}

bool Worm::IsFrozen() const
{
    return roundsUntilUnfrozen > 0;
}

void Worm::TakeDamage(int dmgAmount, Worm* attacker)
{
    health -= dmgAmount;
    if(attacker != nullptr && health <= 20) { //only need to add if he could still get killed this turn
        lastAttackedBy.push_back(attacker);
    }
    state->player1.RecalculateHealth();
    state->player2.RecalculateHealth();
}

bool Worm::operator==(const Worm &other) const
{
    /*
    std::cerr << "(" << __FUNCTION__ << ") " 
    " id: " << (id == other.id) << "(" << id << ", " << other.id << ")" <<
    " health: " << (health == other.health) << "( " << health << ", " << other.health << ")" <<
    " position: " << (position == other.position) <<
    " previous_position: " << (previous_position == other.previous_position) <<
    " weapon: " << (weapon == other.weapon) <<
    " banana_bomb: " << (banana_bomb == other.banana_bomb) <<
    " banana_bomb_count: " << (banana_bomb_count == other.banana_bomb_count) <<
    " diggingRange: " << (diggingRange == other.diggingRange) <<
    " movementRange: " << (movementRange == other.movementRange) <<
    std::endl;
    */

    return (playerId == other.playerId &&
            id == other.id &&
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
