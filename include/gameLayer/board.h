#include <glm/glm.hpp>


class Board
{
public:
    glm::ivec2 boardSize;
    glm::ivec4 bounds;
    int padding;
    glm::vec4 color;

    Board()
    {
    }

    Board(glm::ivec2 boardSize, glm::ivec4 bounds, int padding, glm::vec4 color) : boardSize(boardSize), bounds(bounds), padding(padding), color(color)
    {
    }

    glm::vec4 cellPosToRect(int col, int row);

    bool getRowColFromMousePosition(const glm::ivec2& mousePos, glm::ivec2 *coords);

    bool getCellRectFromMousePosition(const glm::ivec2& mousePos, glm::ivec4 *rect);

private:
};