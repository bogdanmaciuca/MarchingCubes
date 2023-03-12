#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "vendor/shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

typedef GLFWwindow* Window;
namespace gfx {
	Window Init(const char* name, int width, int height) {
		if (!glfwInit()) return 0;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		Window window = glfwCreateWindow(width, height, name, 0, 0);
		if (!window) return 0;
		glfwMakeContextCurrent(window);
		glewInit();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		return window;
	}
	bool WindowShouldClose(Window window) { return glfwWindowShouldClose(window); }
	void BeginFrame() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); }
	void EndFrame(Window window) {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	void SetBkgColour(float r, float g, float b) { glClearColor(r, g, b, 1); }

	void BindVAO(unsigned int VAO) { glBindVertexArray(VAO); }
	unsigned int CreateVAO() {
		unsigned int vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		return vao;
	}
	unsigned int CreateVBO(float* vertices, unsigned int num) {
		unsigned vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, num * sizeof(float), vertices, GL_STATIC_DRAW);
		return vbo;
	}
	unsigned int CreateIBO(unsigned int* indices, unsigned int num) {
		unsigned ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, num * sizeof(unsigned int), indices, GL_STATIC_DRAW);
		return ibo;
	}
	void InitLayout() {
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	unsigned int LoadTexture(const char* filename) {
		unsigned int result;
		glGenTextures(1, &result);
		glBindTexture(GL_TEXTURE_2D, result);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
		return result;
	}
	unsigned int CreateSSBO(const ComputeShader& compute_shader, const char* name, const int& binding) {
		GLuint SSBO;
		glGenBuffers(1, &SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
		int block_index = glGetProgramResourceIndex(compute_shader.ID, GL_SHADER_STORAGE_BLOCK, name);
		glShaderStorageBlockBinding(compute_shader.ID, block_index, binding);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, SSBO);
		return SSBO;
	}
	void SetSSBOData(unsigned int ssbo, void* data, int size) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
}