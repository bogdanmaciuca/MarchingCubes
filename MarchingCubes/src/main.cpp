#pragma comment(lib, "glfw3_mt")
#pragma comment(lib, "glew32s")
#pragma comment(lib, "opengl32")

#include "renderer.h"
#include "camera.h"
#include "marching_cubes.h"
#include "game.h"
#include <Windows.h>
#include <chrono>
#include "vendor/noise/perlin.h"
extern "C" {
#include "vendor/noise/noise1234.h"
}

struct Chunk {
	MarchingCubes mc;
	unsigned int vao, vbo;
	void Init(const World grid, int x, int y) {
		mc.CreateMeshData(grid, x * CHUNK_WIDTH, y * CHUNK_WIDTH);
		vao = gfx::CreateVAO();
		vbo = gfx::CreateVBO((float*)mc.vertices.data(), mc.vertices.size() * 3);
		gfx::InitLayout();
	}
	void Draw(const gfx::Shader& shader) {
		gfx::BindVAO(vao);
		glDrawArrays(GL_TRIANGLES, 0, mc.vertices.size());
	}
	void UpdateGeom() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, mc.vertices.size() * sizeof(glm::vec3), mc.vertices.data(), GL_DYNAMIC_DRAW);
	}
};

void CreateWorld(World &world) {
	// Allocate memory
	world = (Cell***)malloc(sizeof(Cell**) * (WORLD_WIDTH + 1));
	for (int i = 0; i < WORLD_WIDTH + 1; i++) {
		world[i] = (Cell**)malloc(sizeof(Cell*) * (WORLD_HEIGHT + 1));
		for (int j = 0; j < WORLD_HEIGHT + 1; j++) {
			world[i][j] = (Cell*)malloc(sizeof(Cell) * (WORLD_WIDTH + 1));
			memset(world[i][j], 0, sizeof(Cell) * (WORLD_WIDTH + 1));
		}
	}

	// Terrain generation
	for (int x = 0; x < WORLD_WIDTH + 1; x++) {
		for (int z = 0; z < WORLD_WIDTH + 1; z++) {
			for (int y = 0; y < WORLD_HEIGHT + 1; y++) {
				//float height = 10*noise3(x / 12.0f, y / 12.0f, z/ 12.0f);
				float height = 20+ 4*noise2(x / 11.0f, z/ 11.0f) - y;
				height = glm::clamp(height, -1.0f, 1.0f);
				BYTE value = (BYTE)((float)(height+1) * 127.0);
				world[x][y][z].terrain = value;
			}
		}
	}
}

int main() {
	Window window = gfx::Init("Hey biatchhh", 800, 600);
	gfx::Shader shader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");

	World grid;
	CreateWorld(grid);

	const int chunk_num = WORLD_WIDTH / CHUNK_WIDTH;
	Chunk chunks[chunk_num][chunk_num];
	for (int i = 0; i < chunk_num; i++) {
		for (int j = 0; j < chunk_num; j++) {
			chunks[i][j].Init(grid, i, j);
		}
	}

	Camera camera;
	camera.Position = glm::vec3(WORLD_WIDTH / 2.0f, CHUNK_HEIGHT * 0.75f, WORLD_WIDTH / 2.0f);

	glm::mat4 model(1.0f);
	glm::mat4 view(1.0f);
	glm::mat4 proj = glm::perspective(77.0f, 4 / 3.0f, 0.1f, 100.0f);
	shader.Use();
	shader.SetMat4("uProj", proj);
	shader.SetVec3("lightColor", glm::vec3(1.0f));
	shader.SetBool("uTransparent", 0);
	shader.SetVec3("objectColor", glm::vec3(92 / 255.0f, 42 / 255.0f, 4 / 255.0f));

	gfx::SetBkgColour(0.2, 0.3, 0.4);
	double mouseX = 0, mouseY = 0;
	float lastMouseX = 0, lastMouseY = 0;
	float deltaTime = 0.0f;
	while (!gfx::WindowShouldClose(window)) {
		auto frameStart = std::chrono::high_resolution_clock::now();

		lastMouseX = mouseX;
		lastMouseY = mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		float deltaMouseX = mouseX - lastMouseX;
		float deltaMouseY = -(mouseY - lastMouseY);

		if (GetAsyncKeyState('W'))
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (GetAsyncKeyState('S'))
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (GetAsyncKeyState('A'))
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (GetAsyncKeyState('D'))
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (GetAsyncKeyState(VK_SHIFT))
			camera.MovementSpeed = SPEED * 2.0f;
		else
			camera.MovementSpeed = SPEED;
		camera.ProcessMouseMovement(deltaMouseX, deltaMouseY);
		view = camera.GetViewMatrix();

		gfx::BeginFrame();
		shader.SetMat4("uView", view);
		shader.SetMat4("uModel", glm::mat4(1));
		shader.SetVec3("lightPos", camera.Position);
		shader.SetVec3("viewPos", camera.Position);
		// Chunks
		for (int i = 0; i < chunk_num; i++) {
			for (int j = 0; j < chunk_num; j++) {
				chunks[i][j].Draw(shader);
			}
		}
		gfx::EndFrame(window);
		auto frameEnd = std::chrono::high_resolution_clock::now();
		deltaTime = ((std::chrono::duration<float, std::milli>)(frameEnd - frameStart)).count();
		std::cout << deltaTime << "\n";
	}

	glfwTerminate();
	return 0;
}