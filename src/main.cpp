// Windows Exclusive Includes
#if defined(_WIN64) or defined(_WIN32)
#include <Windows.h>
#else
//#include <ncurses.h> // Add the curses lib so that linux can use it
#endif

// C/C++
#include <vector>
#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// ImGui
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

// Program
#include "chip8.h"

void XBeep() {
#if defined(_WIN64) or defined(_WIN32)
	Beep(500, 64);
#else
	//beep();
#endif
}

// \brief Callback for GLFW to resize the viewport whenever the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// \brief Process input for GLFW Window
void processInput(GLFWwindow* window, Chip8* c) {
	// Exit Process
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == 1)
			glfwSetWindowShouldClose(window, true);

	// 1 2 3 C -> 1 2 3 4
	// 4 5 6 D -> Q W E R
	// 7 8 9 E -> A S D F
	// A 0 C F -> Z X C V

	if (glfwGetKey(window, GLFW_KEY_X) == 1)
		c->keypad[0] = 1;
	else if(glfwGetKey(window, GLFW_KEY_X) == 0)
		c->keypad[0] = 1;

	if (glfwGetKey(window, GLFW_KEY_1) == 1)
		c->keypad[1] = 1;
	else if (glfwGetKey(window, GLFW_KEY_1) == 0)
		c->keypad[1] = 0;

	if (glfwGetKey(window, GLFW_KEY_2) == 1)
		c->keypad[2] = 1;
	else if (glfwGetKey(window, GLFW_KEY_2) == 0)
		c->keypad[2] = 0;

	if (glfwGetKey(window, GLFW_KEY_3) == 1)
		c->keypad[3] = 1;
	else if (glfwGetKey(window, GLFW_KEY_3) == 0)
		c->keypad[3] = 0;

	if (glfwGetKey(window, GLFW_KEY_Q) == 1)
		c->keypad[4] = 1;
	else if (glfwGetKey(window, GLFW_KEY_Q) == 0)
		c->keypad[4] = 0;

	if (glfwGetKey(window, GLFW_KEY_W) == 1)
		c->keypad[5] = 1;
	else if (glfwGetKey(window, GLFW_KEY_W) == 0)
		c->keypad[5] = 0;

	if (glfwGetKey(window, GLFW_KEY_E) == 1)
		c->keypad[6] = 1;
	else if (glfwGetKey(window, GLFW_KEY_E) == 0)
		c->keypad[6] = 0;

	if (glfwGetKey(window, GLFW_KEY_A) == 1)
		c->keypad[7] = 1;
	else if (glfwGetKey(window, GLFW_KEY_A) == 0)
		c->keypad[7] = 0;

	if (glfwGetKey(window, GLFW_KEY_S) == 1)
		c->keypad[8] = 1;
	else if (glfwGetKey(window, GLFW_KEY_S) == 0)
		c->keypad[8] = 0;

	if (glfwGetKey(window, GLFW_KEY_D) == 1)
		c->keypad[9] = 1;
	else if (glfwGetKey(window, GLFW_KEY_D) == 0)
		c->keypad[9] = 0;

	if (glfwGetKey(window, GLFW_KEY_Z) == 1)
		c->keypad[0xA] = 1;
	else if (glfwGetKey(window, GLFW_KEY_Z) == 0)
		c->keypad[0xA] = 0;

	if (glfwGetKey(window, GLFW_KEY_C) == 1)
		c->keypad[0xB] = 1;
	else if (glfwGetKey(window, GLFW_KEY_C) == 0)
		c->keypad[0xB] = 0;

	if (glfwGetKey(window, GLFW_KEY_4) == 1)
		c->keypad[0xC] = 1;
	else if (glfwGetKey(window, GLFW_KEY_4) == 0)
		c->keypad[0xC] = 0;

	if (glfwGetKey(window, GLFW_KEY_R) == 1)
		c->keypad[0xD] = 1;
	else if (glfwGetKey(window, GLFW_KEY_R) == 0)
		c->keypad[0xD] = 0;

	if (glfwGetKey(window, GLFW_KEY_F) == 1)
		c->keypad[0xE] = 1;
	else if (glfwGetKey(window, GLFW_KEY_F) == 0)
		c->keypad[0xE] = 0;

	if (glfwGetKey(window, GLFW_KEY_V) == 1)
		c->keypad[0xF] = 1;
	else if (glfwGetKey(window, GLFW_KEY_V) == 0)
		c->keypad[0xF] = 0;
}

void GameThread(Chip8* c) {
	// Gets the current time as a high resolution clock
	auto lastCycle = std::chrono::high_resolution_clock::now();

	while (true) { // Keep Thread Alive
		while (c->isLoaded && c->isRunning) {
			// Gets the current time as a high resolution clock
			auto currTime = std::chrono::high_resolution_clock::now();
			// Compares the clock to the clock of the last cycle
			float deltaTime = std::chrono::duration<float, std::chrono::microseconds::period>(currTime - lastCycle).count();

			if (deltaTime > c->cycleDelay) {
				lastCycle = currTime;
				c->RunCycle();
			}
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void TimerThread(Chip8* c)  {
	// Gets the current time as a high resolution clock
	auto lastCycle = std::chrono::high_resolution_clock::now();

	while (true) { // Keep Thread Alive
		while (c->isLoaded && c->isRunning) {
			// Gets the current time as a high resolution clock
			auto currTime = std::chrono::high_resolution_clock::now();
			// Compares the clock to the clock of the last cycle
			float deltaTime = std::chrono::duration<float, std::chrono::microseconds::period>(currTime - lastCycle).count();
			
			if (c->soundTimer == 1)
				c->shouldBeep = true;
			else if (c->soundTimer == 0)
				c->shouldBeep = false;

			if (deltaTime > 16330) { // 16.33ms
				lastCycle = currTime;
				c->RunTimers();
			}
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void SoundThread(Chip8* c) {
	while (true) { // Keep Thread Alive
		while (c->isLoaded && c->isRunning) {
			if (c->shouldBeep)
				XBeep();
		}
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


int run() {
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

	int windowWidth = 1600, windowHeight = 800;

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
	glfwSwapInterval(1);
	//glfwMaximizeWindow(window);

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
	ImVec4 clearColor = ImVec4(35 / 255.0f, 35 / 255.0f, 35 / 255.0f, 1.00f);

	//Init Chip8 Sys
	Chip8 chip8 = Chip8();
	int width = 0, height = 0, controls_width = 0;

	std::thread game(GameThread, &chip8);
	std::thread timers(TimerThread, &chip8);
	std::thread sound(SoundThread, &chip8);

	// Render loop
	while (!glfwWindowShouldClose(window)) {
		// Input - Old, replaced with callback
		processInput(window, &chip8);

		// start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
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
	game.detach();
	timers.detach();
	sound.detach();

	return 0;
}

#if defined(_WIN32) || defined(_WIN64)
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
#ifdef _DEBUG
	FILE* stream;
	AllocConsole();
	freopen_s(&stream, "conin$", "r", stdin);
	freopen_s(&stream, "conout$", "w", stdout);
	freopen_s(&stream, "conout$", "w", stderr);
#endif
	return run();
}
#else
int main() {
	return run();
}
#endif