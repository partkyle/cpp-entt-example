#include "board.h"

glm::vec4 Board::cellPosToRect(int col, int row)
{
    int cellWidth = this->bounds.z / this->boardSize.x;
    int cellHeight = this->bounds.w / this->boardSize.y;

    return {this->bounds.x + cellWidth * col + this->padding, this->bounds.y + cellHeight * row + this->padding, cellWidth - 2 * this->padding, cellHeight - 2 * this->padding};
}

bool Board::getRowColFromMousePosition(const glm::ivec2 &mousePos, glm::ivec2 *coords)
{
    int cellWidth = this->bounds.z / this->boardSize.x;
    int cellHeight = this->bounds.w / this->boardSize.y;

    if (mousePos.x < this->bounds.x - this->padding)
    {
        return false;
    }

    if (mousePos.y < this->bounds.y - this->padding)
    {
        return false;
    }

    if (mousePos.x >= this->bounds.x + this->bounds.z - this->padding * 2)
    {
        return false;
    }

    if (mousePos.y >= this->bounds.y + this->bounds.w - this->padding * 2)
    {
        return false;
    }

    int col = (mousePos.x - this->bounds.x) / cellWidth;
    int row = (mousePos.y - this->bounds.y) / cellHeight;

    *coords = {col, row};

    return true;
}

bool Board::getCellRectFromMousePosition(const glm::ivec2 &mousePos, glm::ivec4 *rect)
{
    glm::ivec2 coords;
    if (!this->getRowColFromMousePosition(mousePos, &coords))
    {
        return false;
    }

    *rect = cellPosToRect(coords.x, coords.y);

    return true;
}