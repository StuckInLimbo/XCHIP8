﻿// C/C++
#include <vector>
#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"

// ImGui
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/opensans.h"

// Program
#include "chip8.h"

// \brief Callback for GLFW to resize the viewport whenever the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// \brief Process input for GLFW Window
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	#pragma region Initialization
	// Initialize GLFW, and set version + profile
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::string glsl_version = "#version 130";
	#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	int windowWidth = 1280, windowHeight = 720;
	bool showMenu = true;
	bool showDemo = false;
	char buf[128] = "roms/test.ch8";

	// Create GLFW window context
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "LearnOpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Set OGL Viewport
	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 18.0f, NULL, NULL);

	// setup Dear ImGui style
	ImGui::StyleColorsDark();

	// setup platform/renderer bindings
	if (!ImGui_ImplOpenGL3_Init(glsl_version.c_str())) {
		std::cout << "Failed to Init ImGui for OpenGL3!" << std::endl;
		return -2;
	}
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
		std::cout << "Failed to Init ImGui GLFW for OpenGL!" << std::endl;
		return -2;
	}
	#pragma endregion

	// colors are set in RGBA, but as float
	ImVec4 background = ImVec4(35 / 255.0f, 35 / 255.0f, 35 / 255.0f, 1.00f);

	Shader shaders("shaders/vertex.glsl", "shaders/fragment.glsl");

	// Vertex array
	float vertices[] = {
		// positions          // colors           // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	GLuint VBO, VAO, EBO;
	// Create OGL Objects
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind to Vertex Array Object
	glBindVertexArray(VAO);
	// Set VBO as the current buffer we are working with
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Copies the vertex data into the VBO's memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	// Set EBO as the current buffer we are working with
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// Copies the index data into the EBO's memory
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	// Set vertex attribute pointers
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Create texture object
	GLuint rendTex;
	// Generate Texture ID context
	glGenTextures(1, &rendTex);

	// bind to nerly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, rendTex);

	// Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Init Chip8 Sys
	Chip8 chip8;
	auto lastCycle = std::chrono::high_resolution_clock::now();

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		processInput(window);

		glClearColor(background.x, background.y, background.z, background.w);
		glClear(GL_COLOR_BUFFER_BIT);

		if(chip8.isLoaded)
			chip8.RunCycle();

		// start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (true) { // a window is defined by Begin/End pair
			if (false) {
				ImGui::ShowDemoWindow();
			}
			// get the window size as a base for calculating widgets geometry
			int width = 0, height = 0, controls_width = 0;
			glfwGetWindowSize(window, &width, &height);
			controls_width = width;
			// make controls widget width to be 1/3 of the main window width
			if ((controls_width /= 3) < 300) { 
				controls_width = 300; 
			}
			
			{ 
				ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
				// here we set the calculated width and also make the height to be
				// be the height of the main window also with some margin
				ImGui::SetNextWindowSize(
					ImVec2(300, 165),
					ImGuiCond_Always
				);
				// create a window and append into it
				ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoResize);

				// buttons and most other widgets return true when clicked/edited/activated
				ImGui::InputText("filename", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank);
				if (ImGui::Button("Load ROM")) {
					chip8.LoadRom((const char*)buf);
				}
				ImGui::End();
			}

			if (chip8.isLoaded) {
				ImGui::SetNextWindowPos(ImVec2(width - static_cast<float>(controls_width) - 5, 10), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(static_cast<float>(controls_width), 450.0f), ImGuiCond_Always);
				ImGui::Begin("CHIP-8 Debug Window", NULL, ImGuiWindowFlags_NoResize);

				ImGui::BeginChild("DebugL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollWithMouse);
				ImGui::Text("opcode: %x", chip8.opcode);
				ImGui::Text("PC: %hu", chip8.pc);
				//ImGui::SameLine(); ImGui::Text("SP: %hu", chip8.sp);
				for (int i = 0; i < 16; i++) {
					ImGui::Text("V[%0i]: %x", i, chip8.V[0]);
				}
				ImGui::EndChild(); ImGui::SameLine();

				ImGui::BeginChild("DebugR", ImVec2(ImGui::GetContentRegionAvail().x - 30, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoResize);
				ImGui::Text("I: %hu", chip8.I);
				for (int i = 0; i < 16; i++) {
					ImGui::Text("S[%i]: %x", i, chip8.stack[i]);
				}
				ImGui::EndChild();
				ImGui::End();
			}

			// Render chip8 video memory as pixels via OpenGL
			if (chip8.shouldDraw && chip8.video) {
				chip8.shouldDraw = false;

				// Assign pixel buffer to texture
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, 
					GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, chip8.video);
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8, chip8.video);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		// Render ImGui
		ImGui::Render();

		glBindTexture(GL_TEXTURE_2D, rendTex);
		// Use created shader for all future calls
		shaders.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Render the window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();

	return 0;
}
