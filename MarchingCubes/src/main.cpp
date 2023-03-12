/*
* TODO:
* - use compute shaders for creating the mesh and terrain generation
*   - figure out how tf SSBOs work
* - imgui implementation
* - better terrain
* - player controller
* - terraforming
* ...
* - loading chunks near the player only
*/

#pragma comment(lib, "glfw3_mt")
#pragma comment(lib, "glew32s")
#pragma comment(lib, "opengl32")

#include <Windows.h>
#include <chrono>
#include "renderer.h"
#include "camera.h"
#include "marching_cubes.h"
#include "game.h"

struct Chunk {
	MarchingCubes mc;
	unsigned int vao, vbo;
	void Init(int x, int y, bool generate_cells = 0) {
		mc.AllocCellMem();
		if (generate_cells)
			mc.GenerateCells(x * CHUNK_WIDTH, y * CHUNK_WIDTH);
		mc.CreateMeshData(x * CHUNK_WIDTH, y * CHUNK_WIDTH);
		vao = gfx::CreateVAO();
		vbo = gfx::CreateVBO((float*)mc.vertices.data(), mc.vertices.size() * 3);
		gfx::InitLayout();
	}
	void Draw(const gfx::Shader& shader) {
		gfx::BindVAO(vao);
		glDrawArrays(GL_TRIANGLES, 0, mc.vertices.size());
	}
	void UpdateGeom(int x, int y) {
		mc.CreateMeshData(x * CHUNK_WIDTH, y * CHUNK_WIDTH);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, mc.vertices.size() * sizeof(glm::vec3), mc.vertices.data(), GL_DYNAMIC_DRAW);
	}
};

void LoadChunks(Chunk* chunks) {
	FILE* file;
	fopen_s(&file, SAVE_FILE_PATH, "rb");
	if (!file) return;

	const int chunk_num_x = WORLD_WIDTH / CHUNK_WIDTH;
	const int chunk_num_y = WORLD_HEIGHT / CHUNK_HEIGHT;
	const int chunk_len = sizeof(Cell) * (CHUNK_WIDTH+1) * (CHUNK_HEIGHT+1) * (CHUNK_WIDTH+1); // refers to *cells size
	const int len = chunk_len * chunk_num_y * chunk_num_x * chunk_num_x;
	BYTE* buf = (BYTE*)malloc(len);
	if (!buf) return;

	fread(buf, 1, len, file);

	for (int i = 0; i < chunk_num_x; i++) {
		for (int j = 0; j < chunk_num_y; j++) {
			for (int k = 0; k < chunk_num_x; k++) {
				BYTE* this_chunk = buf + (k * chunk_num_x + i)*chunk_len;
				memcpy(chunks[k * chunk_num_x + i].mc.cells, this_chunk, chunk_len);
			}
		}
	}

	free(buf);
	fclose(file);
}

void SaveChunks(Chunk* chunks) {
	FILE* file;
	fopen_s(&file, SAVE_FILE_PATH, "wb");
	if (!file) return;
	
	const int chunk_num_x = WORLD_WIDTH / CHUNK_WIDTH;
	const int chunk_num_y = WORLD_HEIGHT / CHUNK_HEIGHT;
	
	std::vector<BYTE> data;

	for (int i = 0; i < chunk_num_x; i++)
		for (int j = 0; j < chunk_num_y; j++)
			for (int k = 0; k < chunk_num_x; k++)
				chunks[k*chunk_num_x+i].mc.SaveAsBinary(&data);
	//__debugbreak();
	fwrite(data.data(), 1, data.size(), file);
	fclose(file);
}

int main() {
	Window window = gfx::Init("Hey biatchhh", 800, 600);
	gfx::Shader shader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");

	const int chunk_num = WORLD_WIDTH / CHUNK_WIDTH;
	Chunk chunks[chunk_num*chunk_num];
	for (int i = 0; i < chunk_num; i++)
		for (int j = 0; j < chunk_num; j++)
			chunks[j*chunk_num+i].Init(i, j);
	LoadChunks(chunks);
	for (int i = 0; i < chunk_num; i++)
		for (int j = 0; j < chunk_num; j++)
			chunks[j * chunk_num + i].UpdateGeom(i, j);

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
		if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('S')) {
			SaveChunks(chunks);
		}

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
				chunks[j*chunk_num+i].Draw(shader);
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