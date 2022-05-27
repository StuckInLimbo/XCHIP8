// C/C++
#include <vector>
#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>

// SDL
#include <glad/glad.h>
#include <SDL.h>

// ImGui
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/opensans.h"

// Program
#include "chip8.h"

int main(int argc, char* argv[]) {
	int windowWidth = 960, windowHeight = 480;
	bool showMenu = true;
	bool showDemo = false;
	char buf[128] = "roms/pong.ch8";
	uint32_t pixels[2048];

	// init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("[ERROR] %s\n", SDL_GetError());
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	std::string glsl_version = "";

#ifdef __APPLE__
	// GL 3.2 Core + GLSL 150
	glsl_version = "#version 150";
	SDL_GL_SetAttribute( // required on Mac OS
		SDL_GL_CONTEXT_FLAGS,
		SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
	);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
	// GL 3.2 Core + GLSL 150
	glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
	// GL 3.0 + GLSL 130
	glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(
		SDL_WINDOW_OPENGL
		| SDL_WINDOW_RESIZABLE
		| SDL_WINDOW_ALLOW_HIGHDPI
		);
	SDL_Window* window = SDL_CreateWindow(
		"CHIP-8 Interpreter",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		window_flags
	);
	// limit to which minimum size user can resize the window
	SDL_SetWindowMinimumSize(window, 64, 32);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);

	// enable VSync
	SDL_GL_SetSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "[ERROR] Couldn't initialize glad" << std::endl;
	} else {
		std::cout << "[INFO] glad initialized\n";
	}

	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 18.0f, NULL, NULL);


	// setup Dear ImGui style
	ImGui::StyleColorsDark();

	// setup platform/renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version.c_str());

	// colors are set in RGBA, but as float
	ImVec4 background = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.00f);

	glClearColor(background.x, background.y, background.z, background.w);

	//Init Chip8 Sys
	Chip8 chip8;
	auto lastCycle = std::chrono::high_resolution_clock::now();

	bool loop = true;
	while (loop) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		if(chip8.isLoaded)
			chip8.RunCycle();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// without it you won't have keyboard input and other things
			ImGui_ImplSDL2_ProcessEvent(&event);
			// you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
			// before processing events

			switch (event.type) {
			case SDL_QUIT:
				loop = false;
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					windowWidth = event.window.data1;
					windowHeight = event.window.data2;
					glViewport(0, 0, windowWidth, windowHeight);
					break;
				}
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					loop = false;
					break;
				case SDLK_INSERT:
					showMenu = !showMenu;
					break;
				#ifdef _DEBUG
				case SDLK_HOME:
					showDemo = !showDemo;
				#endif
				}
				
				break;
			}
		}

		// start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		if (showMenu) { // a window is defined by Begin/End pair
			if (showDemo) {
				ImGui::ShowDemoWindow();
			}
			// get the window size as a base for calculating widgets geometry
			int sdl_width = 0, sdl_height = 0, controls_width = 0;
			SDL_GetWindowSize(window, &sdl_width, &sdl_height);
			controls_width = sdl_width;
			// make controls widget width to be 1/3 of the main window width
			if ((controls_width /= 3) < 300) { controls_width = 300; }

			{
				// position the controls widget in the top-right corner with some margin
				ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
				// here we set the calculated width and also make the height to be
				// be the height of the main window also with some margin
				ImGui::SetNextWindowSize(
					ImVec2(static_cast<float>(controls_width), 165),
					ImGuiCond_Always
				);
				// create a window and append into it
				ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize);

				ImGui::Dummy(ImVec2(0.0f, 1.0f));
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform"); ImGui::SameLine();
				ImGui::Text("%s", SDL_GetPlatform());
				ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
				ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);

				// buttons and most other widgets return true when clicked/edited/activated
				ImGui::InputText("filename", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank);
				if (ImGui::Button("Load ROM")) {
					chip8.LoadRom((const char*)buf);
				}
				ImGui::End();
			}

			if (chip8.isLoaded) {
				ImGui::SetNextWindowPos(ImVec2(static_cast<float>(controls_width) + 20, 10), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(static_cast<float>(controls_width), static_cast<float>(sdl_height - 20)), ImGuiCond_Always);
				ImGui::Begin("CHIP-8 DEBUG", NULL, ImGuiWindowFlags_NoResize);

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
		}

		// Render chip8 video memory as pixels via OpenGL
		if (chip8.shouldDraw) {
			chip8.shouldDraw = false;

			// Store pixels in temporary buffer, technically should be not needed. 
			for (int i = 0; i < 2048; ++i) {
				uint8_t pixel = chip8.video[i];
				pixels[i] = (0xFFFFFF00 * pixel) | 0x000000FF;
			}

			// prepare chip8 framebuffer
			GLuint rendTex;
			glGenTextures(1, &rendTex);
			// Give an empty image to OpenGL ( the last "0" )
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0,  GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8, pixels);
			// bind to nerly greated texture : all future texture functions will modify this texture
			glBindTexture(GL_TEXTURE_2D, rendTex);
			// Poor filtering. Needed !
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			// Always check that our framebuffer is ok
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				return -2;

			// Render to our framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, rendTex);
		}

		// rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		SDL_GL_SwapWindow(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
