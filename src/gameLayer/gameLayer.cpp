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

#include <entt/entt.hpp>

#include "board.h"
#include "game.h"

const int BOARD_WIDTH = 8;
const int BOARD_HEIGHT = 8;
const int BOARD_SIZE = BOARD_WIDTH * BOARD_HEIGHT;

struct position
{
	float x;
	float y;
};

struct velocity
{
	float dx;
	float dy;
};

struct rectsize
{
	float w;
	float h;
};

struct lifetime
{
	float t;
};

struct expiration
{
	float t;
};

struct GameData
{

	entt::registry registry;

	int spawnCount = 100;
	position defaultPosition = {0, 0};
	velocity defaultVelocity = {-100, 100};
	float fadeSpeed = 1.0;
	bool randomizeVelocity = true;
	bool useMousePosition = false;

} gameData;


const int MAX_PERFORMANCE_SLOTS = 1000;
struct PerformanceData
{
	float deltaTime[MAX_PERFORMANCE_SLOTS];
	float fps[MAX_PERFORMANCE_SLOTS];
	int idx = 0;
} perfData;

gl2d::Renderer2D renderer;

gl2d::Font font;

bool initGame()
{
	// initializing stuff for the renderer
	gl2d::init();
	renderer.create();

	// loading the saved data. Loading an entire structure like this makes savind game data very easy.
	// platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

	font = gl2d::Font(RESOURCES_PATH "press-start-2p.ttf");

	gameData.defaultPosition = {float(platform::getFrameBufferSizeX()) / 2, float(platform::getFrameBufferSizeY()) / 2};

	return true;
}

// IMPORTANT NOTICE, IF YOU WANT TO SHIP THE GAME TO ANOTHER PC READ THE README.MD IN THE GITHUB
// https://github.com/meemknight/cmakeSetup
// OR THE INSTRUCTION IN THE CMAKE FILE.
// YOU HAVE TO CHANGE A FLAG IN THE CMAKE SO THAT RESOURCES_PATH POINTS TO RELATIVE PATHS
// BECAUSE OF SOME CMAKE PROGBLMS, RESOURCES_PATH IS SET TO BE ABSOLUTE DURING PRODUCTION FOR MAKING IT EASIER.

float randf()
{
	return (float(rand()) / float(RAND_MAX));
}

float lerp(float lo, float hi, float p)
{
	return lo + (hi - lo) * p;
}

float randf_range(float lo, float hi)
{
	return lerp(lo, hi, randf());
}


void createRectangle(entt::registry &registry, position defaultPostiion, velocity vel, bool randomizeVelocity)
{
	const auto entity = registry.create();
	registry.emplace<position>(entity, defaultPostiion.x, defaultPostiion.y);
	if (randomizeVelocity)
	{
		glm::vec2 v = {randf_range(-1, 1), randf_range(-1, 1)};
		v = glm::normalize(v);
		v *= randf_range(vel.dx, vel.dy);
		registry.emplace<velocity>(entity, v.x, v.y);
	}
	else
	{
		registry.emplace<velocity>(entity, vel.dx, vel.dy);
	}
	registry.emplace<rectsize>(entity, 10.0f, 10.0f);
	registry.emplace<glm::vec4>(entity, randf(), randf(), randf(), 1.0f);

	registry.emplace<lifetime>(entity);
	registry.emplace<expiration>(entity, 2.0f);
}

void update(entt::registry &registry, float deltaTime)
{
	auto view = registry.view<position, velocity>();

	for (auto [entity, pos, vel] : view.each())
	{
		pos.x += vel.dx * deltaTime;
		pos.y += vel.dy * deltaTime;
	}
}

void updateLifetime(entt::registry &registry, float deltaTime)
{
	auto view = registry.view<lifetime>();

	for (auto [entity, life] : view.each())
	{
		life.t += deltaTime;
	}
}

void cullLifetime(entt::registry &registry, float fadeSpeed, float deltaTime)
{
	auto view = registry.view<const lifetime, const expiration, glm::vec4>();

	for (auto [entity, life, expiry, color] : view.each())
	{
		if (life.t >= expiry.t)
		{
			color.a -= fadeSpeed * deltaTime;
			if (color.a <= 0)
			{
				registry.destroy(entity);
			}
		}
	}
}

void draw(entt::registry &registry)
{
	auto view = registry.view<const position, const rectsize, const glm::vec4>();

	for (auto [entity, pos, s, color] : view.each())
	{
		renderer.renderRectangle(gl2d::Rect{pos.x, pos.y, s.w, s.h}, color);
	}
}

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

	perfData.deltaTime[perfData.idx] = deltaTime;
	perfData.fps[perfData.idx] = 1 / deltaTime;
	perfData.idx = (perfData.idx = (perfData.idx + 1) % MAX_PERFORMANCE_SLOTS);

	if (platform::isButtonPressedOn(platform::Button::Escape))
	{
		return false;
	}

	ImGui::Begin("Debug");
	ImGui::LabelText("deltaTime", "%0.4fms", deltaTime * 1000);
	ImGui::LabelText("fps", "%0.0f", 1 / deltaTime);
	ImGui::LabelText("entities", "%d", gameData.registry.view<position>().size());
	ImGui::DragInt("spawnCount", &gameData.spawnCount, 1, 1, 100);
	ImGui::Checkbox("useMousePosition", &gameData.useMousePosition);
	if (gameData.useMousePosition) {
		gameData.defaultPosition.x = float(platform::getRelMousePosition().x);
		gameData.defaultPosition.y = float(platform::getRelMousePosition().y);
	}
	ImGui::DragFloat2("position", (float *)&gameData.defaultPosition, 1, 0, max(w, h));
	ImGui::DragFloat2("velocity", (float *)&gameData.defaultVelocity, 1, -100, 100);
	ImGui::DragFloat("fadeSpeed", &gameData.fadeSpeed, 1, 0, 100);
	ImGui::Checkbox("randomize", &gameData.randomizeVelocity);

	ImGui::PlotLines("deltaTime", perfData.deltaTime, MAX_PERFORMANCE_SLOTS, perfData.idx - 1);
	ImGui::PlotLines("fps", perfData.fps, MAX_PERFORMANCE_SLOTS, perfData.idx - 1);

	if (ImGui::Button("create rectangle", ImVec2{0, 32}) || platform::isButtonHeld(platform::Button::Space))
	{
		for (int i = 0; i < gameData.spawnCount; i++)
		{
			createRectangle(gameData.registry, gameData.defaultPosition, gameData.defaultVelocity, gameData.randomizeVelocity);
		}
	}
	ImGui::End();

	update(gameData.registry, deltaTime);
	updateLifetime(gameData.registry, deltaTime);
	cullLifetime(gameData.registry, gameData.fadeSpeed, deltaTime);
	draw(gameData.registry);

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
