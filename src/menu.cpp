#include "menu.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include <GLFW/glfw3.h>

bool showMenu = true;
bool showDemo = false;
char buf[128] = "roms/test.ch8";

bool Menu::runMenus(Chip8 *chip8, float width, float height) {
	if (showMenu) { // a window is defined by Begin/End pair
		float controls_width = width;
		// make controls widget width to be 1/3 of the main window width
		if ((controls_width /= 3) < 300) {
			controls_width = 300;
		}

		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
		// here we set the calculated width and also make the height to be
		// be the height of the main window also with some margin
		ImGui::SetNextWindowSize(ImVec2(300, 165), ImGuiCond_Always);
		// create a window and append into it
		ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoResize);

		// buttons and most other widgets return true when clicked/edited/activated
		ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);
		ImGui::InputText("filename", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank);
		if (ImGui::Button("Load ROM")) {
			chip8->LoadRom((const char*)buf);
		}
		ImGui::End();


		if (chip8->isLoaded) {
			// Debug Window
			ImGui::SetNextWindowPos(ImVec2(width - static_cast<float>(controls_width) - 5, 10), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(static_cast<float>(controls_width), 500.0f), ImGuiCond_Always);
			ImGui::Begin("CHIP-8 Debug Window", NULL, ImGuiWindowFlags_NoResize);

			ImGui::BeginChild("DebugL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::Text("opcode: %x", chip8->opcode);
			ImGui::Text("PC: %hu", chip8->pc);
			ImGui::Text("I: %hu", chip8->I);
			for (int i = 0; i < 16; i++) {
				ImGui::Text("V[%0i]: %x", i, chip8->V[0]);
			}
			ImGui::EndChild(); ImGui::SameLine();

			ImGui::BeginChild("DebugR", ImVec2(ImGui::GetContentRegionAvail().x - 30, ImGui::GetContentRegionAvail().y), false, ImGuiWindowFlags_NoResize);

			ImGui::Text("SP: %hu", chip8->sp);
			for (int i = 0; i < 16; i++) {
				ImGui::Text("S[%i]: %x", i, chip8->stack[i]);
			}
			ImGui::EndChild();
			ImGui::End();

			// Game Window
			ImGui::SetNextWindowSize(ImVec2((64 * 10), (32 * 10)), ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(5, height - 5 - (32 * 10)), ImGuiCond_Once);
			ImGui::Begin("CHIP-8 Viewer", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			ImGui::End();
		}

		// Render chip8 video memory as pixels via OpenGL
		/*if (chip8.shouldDraw) {
			chip8.shouldDraw = false;

			uint32_t pixels[2048];

			for (int i = 0; i < 2048; i++) {
				pixels[i] = (chip8.video[i] & 0xFFFFFF00) | 0xFF;
			}
		}*/
	}
	return true;
}