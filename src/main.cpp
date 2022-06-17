// C/C++
#include <vector>
#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// ImGui
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

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

int main(int argc, char* argv[]) {
	#pragma region Initialization
	// Initialize GLFW, and set version + profile
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	std::string glsl_version = "#version 330";
	#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	int windowWidth = 1280, windowHeight = 720;

	// Create GLFW window context
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "XCHIP8", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	else
		std::cout << "GLFW sucessfully initialized!" << std::endl;
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	else
		std::cout << "GLAD sucessfully initialized!" << std::endl;

	//std::string glsl_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	// Set OGL Viewport
	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(0);
	glfwMaximizeWindow(window);

	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
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

	//Init Chip8 Sys
	Chip8 chip8;
	// Gets the current time as a high resolution clock
	auto lastCycle = std::chrono::high_resolution_clock::now();
	int width = 0, height = 0, controls_width = 0;

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// Input
		processInput(window);
		// Gets the current time as a high resolution clock
		auto currTime = std::chrono::high_resolution_clock::now();
		// Compares the clock to the clock of the last cycle
		float deltaTime = std::chrono::duration<float, std::chrono::microseconds::period>(currTime - lastCycle).count();

		if (chip8.isLoaded) {
			// cycle delay
			if (deltaTime > chip8.cycleDelay) {
				lastCycle = currTime;
				chip8.RunCycle();
			}
		}
		// start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		glClearColor(background.x, background.y, background.z, background.w);
		glClear(GL_COLOR_BUFFER_BIT);

		// get the window size as a base for calculating widgets geometry		
		glfwGetWindowSize(window, &width, &height);
		chip8.RunMenu(width, height);

		// Render ImGui
		ImGui::Render();
		// Render the window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}
