#include "game.h"

void Player::addShip(const Ship &ship)
{
    if (this->shipCount < Player::maxShipCount)
    {
        this->ships[this->shipCount++] = ship;
    }
}

int Player::tryHit(glm::ivec2 position)
{
    for (int i = 0; i < this->shipCount; i++)
    {
        Ship *ship = &this->ships[i];
        
        bool horizontalHit = ship->position.p &&
            ship->position.y == position.y &&
            ship->position.x <= position.x && position.x < ship->position.x + ship->position.p;
        bool verticalHit = ship->position.q &&
            ship->position.x == position.x &&
            ship->position.y <= position.y && position.y < ship->position.y + ship->position.q;

        // if the p value is set, it's a horizontal ship
        if (horizontalHit || verticalHit)
        {
            int result = TARGET_HIT;
            ship->health--;
            if (ship->health <= 0)
            {
                result |= TARGET_SINK;
            }
            return result;
        }
    }

    return TARGET_MISS;
}
