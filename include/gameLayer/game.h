#include <map>

#include <glm/glm.hpp>

const int TARGET_MISS = 1;
const int TARGET_HIT = 1 << 1;
const int TARGET_SINK = 1 << 2;

class Ship
{
public:
    glm::ivec4 position;
    int health;

    Ship()
    {
    }

    Ship(glm::ivec4 position, int health) : position(position), health(health)
    {
    }
};

struct KeyFuncs
{
    size_t operator()(const glm::ivec2 &k) const
    {
        return std::hash<int>()(k.x) ^ std::hash<int>()(k.y);
    }

    bool operator()(const glm::ivec2 &a, const glm::ivec2 &b) const
    {
        return a.x == b.x && a.y == b.y;
    }
};


class Player
{
public:
    static const int maxShipCount = 10;
    Ship ships[maxShipCount];
    int shipCount = 0;
    int sinkCount = 0;

    std::map<std::pair<int,int>, int> hits;

    void addShip(const Ship &ship);

    int tryHit(glm::ivec2 position);
};