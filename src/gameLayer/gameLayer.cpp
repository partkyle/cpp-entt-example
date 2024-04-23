#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include <gl2d/gl2d.h>
#include <platformTools.h>

#include "board.h"
#include "game.h"

const int BOARD_WIDTH = 8;
const int BOARD_HEIGHT = 8;
const int BOARD_SIZE = BOARD_WIDTH * BOARD_HEIGHT;

struct GameData
{
	int padding = 1;

	Board target;
	Board ocean;

	Player players[2];
	int currentPlayer;

	int winner = -1;
	glm::ivec2 textPos = {0,0};
	float textSize = 1.5f;
	float accTextSize = 0;
} gameData;

gl2d::Renderer2D renderer;

gl2d::Font font;

bool initGame()
{
	// initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	// loading the saved data. Loading an entire structure like this makes savind game data very easy.
	// platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

	gameData.target = Board{{BOARD_WIDTH, BOARD_HEIGHT}, {0, 0, 200, 200}, 1, Colors_Gray};
	gameData.ocean = Board{{BOARD_WIDTH, BOARD_HEIGHT}, {0, 200, 200, 200}, 1, Colors_Green};

	gameData.players[0].shipCount = 0;
	gameData.players[0].addShip(Ship{{0, 0, 4, 0}, 4});
	gameData.players[0].addShip(Ship{{0, 4, 0, 3}, 3});

	gameData.players[1].shipCount = 0;
	gameData.players[1].addShip(Ship{{0, 0, 0, 4}, 4});
	gameData.players[1].addShip(Ship{{4, 4, 3, 0}, 3});

	gameData.currentPlayer = 0;

	font = gl2d::Font(RESOURCES_PATH "press-start-2p.ttf");

	return true;
}

// IMPORTANT NOTICE, IF YOU WANT TO SHIP THE GAME TO ANOTHER PC READ THE README.MD IN THE GITHUB
// https://github.com/meemknight/cmakeSetup
// OR THE INSTRUCTION IN THE CMAKE FILE.
// YOU HAVE TO CHANGE A FLAG IN THE CMAKE SO THAT RESOURCES_PATH POINTS TO RELATIVE PATHS
// BECAUSE OF SOME CMAKE PROGBLMS, RESOURCES_PATH IS SET TO BE ABSOLUTE DURING PRODUCTION FOR MAKING IT EASIER.

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0;
	int h = 0;
	w = platform::getFrameBufferSizeX(); // window w
	h = platform::getFrameBufferSizeY(); // window h

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT); // clear screen

	renderer.updateWindowMetrics(w, h);
#pragma endregion

	if (platform::isButtonPressedOn(platform::Button::Escape))
	{
		return false;
	}

	int currentPlayer = gameData.currentPlayer;
	int otherPlayer = (gameData.currentPlayer + 1) % 2;

	if (platform::isLMousePressed())
	{
		glm::ivec2 coords;
		if (gameData.target.getRowColFromMousePosition(platform::getRelMousePosition(), &coords))
		{
			if (gameData.players[currentPlayer].hits[std::make_pair(coords.x, coords.y)] == 0)
			{
				int result = gameData.players[otherPlayer].tryHit(coords);
				gameData.players[currentPlayer].hits[std::make_pair(coords.x, coords.y)] = result;
				if (result & TARGET_SINK)
				{
					gameData.players[currentPlayer].sinkCount++;
					if (gameData.players[currentPlayer].sinkCount >= gameData.players[otherPlayer].shipCount)
					{
						gameData.winner = currentPlayer;
					}
				}

				gameData.currentPlayer = (gameData.currentPlayer + 1) % 2;
			}
		}
	}

	ImGui::Begin("Debug");
	ImGui::Text("current player: %d", gameData.currentPlayer);

	ImGui::Text("Player 0 Map");
	for (const auto &[key, value] : gameData.players[0].hits)
	{
		ImGui::Text("[%d,%d] = %d\n", key.first, key.second, value);
	}

	ImGui::Text("Player 1 Map");
	for (const auto &[key, value] : gameData.players[1].hits)
	{
		ImGui::Text("[%d,%d] = %d\n", key.first, key.second, value);
	}

	ImGui::DragInt("padding", &gameData.target.padding, 1, 0, min(w, h));
	ImGui::DragFloat("textPos", &gameData.textSize, 1, 0, 100);
	ImGui::DragInt2("textPos", (int *)&gameData.textPos, 1, 0, max(w, h));
	ImGui::DragInt4("target board rect", (int *)&gameData.target.bounds, 1, 0, max(w, h));
	ImGui::DragInt4("ocean board rect", (int *)&gameData.ocean.bounds, 1, 0, max(w, h));
	ImGui::End();

	Board *boards[] = {&gameData.target, &gameData.ocean};

	for (int i = 0; i < 2; i++)
	{
		Board *board = boards[i];

		for (int row = 0; row < board->boardSize.y; row++)
		{
			for (int col = 0; col < board->boardSize.x; col++)
			{
				glm::vec4 color = board->color;
				if (i == 0)
				{
					auto key = std::make_pair(col, row);
					if (gameData.players[currentPlayer].hits.find(key) != gameData.players[currentPlayer].hits.end())
					{
						int hitResult = gameData.players[currentPlayer].hits[key];
						if (hitResult & TARGET_MISS)
						{
							color = Colors_White;
						}
						else if (hitResult & TARGET_HIT)
						{
							color = Colors_Red;
						}
					}
				}

				renderer.renderRectangle(board->cellPosToRect(col, row), color);
			}
		}
	}

	for (int i = 0; i < gameData.players[currentPlayer].shipCount; i++)
	{
		auto ship = gameData.players[currentPlayer].ships[i];
		for (int j = ship.position.x; j < ship.position.x + ship.position.z; j++)
		{
			renderer.renderRectangle(gameData.ocean.cellPosToRect(j, ship.position.y), Colors_Yellow);
		}
		for (int j = ship.position.y; j < ship.position.y + ship.position.w; j++)
		{
			renderer.renderRectangle(gameData.ocean.cellPosToRect(ship.position.x, j), Colors_Yellow);
		}
	}

	if (gameData.winner == -1)
	{
		glm::ivec4 rect;
		if (gameData.target.getCellRectFromMousePosition(platform::getRelMousePosition(), &rect))
		{
			renderer.renderRectangle(rect, Colors_Blue);
		}
	}
	else
	{
		char banner[80];
		sprintf(banner, "player %d wins!", gameData.winner);
		gameData.accTextSize += deltaTime;
		float textSize = .45 + (.85 - .45) * (sin(gameData.accTextSize) * 0.5 + 1);
		renderer.renderText({w/2, h/2}, banner, font, Colors_White, textSize);
	}

	renderer.flush();

	return true;
#pragma endregion
}

// This function might not be be called if the program is forced closed
void closeGame()
{
	// saved the data.
	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));
}
