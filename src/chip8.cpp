#include "chip8.h"
// Error Output
#include <iostream>
// Timer
#include <chrono>
// File Loading & Savestate
#include <fstream>

// For ImGui Menus
#include <GLFW/glfw3.h>
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/fonts/OpenSans.h"
#include "imgui/fonts/RobotoMono.h"

SaveStates::SaveStates() {
	for (int i = 0; i < 10; i++) {
		States[i] = new State();
	}
}

void SaveStates::CreateState(Chip8* c, State* s) {

	for (int i = 0; i < 4096; i++) {
		s->ram[i] = c->ram[i];
	}
	for (int i = 0; i < 2048; i++) {
		s->video[i] = c->video[i];
	}
	for (int i = 0; i < 2048; i++) {
		s->display[i] = c->display[i];
	}
	for (int i = 0; i < 16; i++) {
		s->V[i] = c->V[i];
	}
	for (int i = 0; i < 16; i++) {
		s->stack[i] = c->stack[i];
	}
	for (int i = 0; i < 16; i++) {
		s->keypad[i] = c->keypad[i];
	}
	s->opcode = c->opcode;
	s->I = c->I;
	s->pc = c->pc;
	s->sp = c->sp;
	s->delayTimer = c->delayTimer;
	s->soundTimer = c->soundTimer;

	// TODO: Save savestate to filesys
}

void SaveStates::Loadstate(Chip8* c, State* s) {
	// TODO: Load savestate from filesys
	for (int i = 0; i < 4096; i++) {
		c->ram[i] = s->ram[i];
	}
	for (int i = 0; i < 2048; i++) {
		c->video[i] = s->video[i];
	}
	for (int i = 0; i < 2048; i++) {
		c->display[i] = s->display[i];
	}
	for (int i = 0; i < 16; i++) {
		c->V[i] = s->V[i];
	}
	for (int i = 0; i < 16; i++) {
		c->stack[i] = s->stack[i];
	}
	for (int i = 0; i < 16; i++) {
		c->keypad[i] = s->keypad[i];
	}
	c->opcode = s->opcode;
	c->I = s->I;
	c->pc = s->pc;
	c->sp = s->sp;
	c->delayTimer = s->delayTimer;
	c->soundTimer = s->soundTimer;

	glBindTexture(GL_TEXTURE_2D, c->TEX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 32, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, c->display);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Fixed font address at $50
const unsigned int FONTSET_START_ADDRESS = 0x50;
// Fixed start address at $200
const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;

// Used mattmikolay's Mastering Chip8 github wiki
uint8_t fontset[FONTSET_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// randGen part is from austinmorlan
Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
	isRunning = true;
	// Zero out memory for registers
	memset(video, 0, sizeof(video));
	memset(display, 0, sizeof(display));
	memset(ram, 0, sizeof(ram));
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(keypad, 0, sizeof(keypad));

	pc = START_ADDRESS;
	opcode = 0;
	I = 0;
	sp = 0;
	delayTimer = 0;
	soundTimer = 0;
	cycleDelay = 2000; // 500Hz, ish
	videoScale = 10;

	// Initialize RNG
	randByte = std::uniform_int_distribution<uint16_t>(0, 255U);

	// Load fonts into memory
	for (int i = 0; i < FONTSET_SIZE; ++i) {
		Chip8::ram[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	savestates = SaveStates();

	// Set up function pointer table, thanks to austinmorlan for the tutorial
	// Master Table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xnn;
	table[0x4] = &Chip8::OP_4xnn;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xnn;
	table[0x7] = &Chip8::OP_7xnn;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxbb;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	// Table 0
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	// Table 8
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	OpenSans = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_data, OpenSans_size, 18.0f, NULL, NULL);
	RobotoMono = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMono_data, RobotoMono_size, 18.0f, NULL, NULL);

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // beta flags, not in master
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	glGenTextures(1, &TEX);
	glBindTexture(GL_TEXTURE_2D, TEX);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Chip8::Reset() {
	isRunning = true;
	// Zero out memory for registers
	memset(video, 0, sizeof(video));
	memset(display, 0, sizeof(display));
	memset(ram, 0, sizeof(ram));
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(keypad, 0, sizeof(keypad));

	pc = START_ADDRESS;
	opcode = 0;
	I = 0;
	sp = 0;
	delayTimer = 0;
	soundTimer = 0;
	//cycleDelay = 875;

	// Initialize RNG
	randByte = std::uniform_int_distribution<uint16_t>(0, 255U);

	// Load fonts into memory
	for (int i = 0; i < FONTSET_SIZE; ++i) {
		Chip8::ram[FONTSET_START_ADDRESS + i] = fontset[i];
	}
}

void Chip8::LoadRom(const char* filename) {
	// ensure that if we load a new rom, the CPU is reset to boot state
	Reset();
	std::ifstream is(filename, std::ios::in | std::ios::binary);
	std::vector<char> prog(
		(std::istreambuf_iterator<char>(is)),
		std::istreambuf_iterator<char>()
	);
	is.close();
	//copy program into memory
	for (int i = 0; i < prog.size(); ++i) {
		ram[START_ADDRESS + i] = prog[i];
	}
	isLoaded = true;
}

void Chip8::RunCycle() {
	// Fetch
	opcode = ram[pc] << 8 | ram[pc + 1];

	// Increment the PC
	pc += 2;

	// Decode & Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();
}

void Chip8::RunTimers() {
	// Decrement the delay timer if it's been set
	if (delayTimer > 0) {
		--delayTimer;
	}
	// Decrement the sound timer if it's been set
	if (soundTimer > 0) {
		--soundTimer;
	}
}

// Get color byte and replace it with our custom colors.
// This whole process also flips it from RGBA to ABGR. Because endianess, or something.
// From 0xRRGGBBAA to 0xAABBGGRR, for example. I don't know a better way to do this tbh.
// Grab red byte, and don't shift.
uint32_t Chip8::GetColoredPixel(uint32_t video, ImVec4 fgCol, ImVec4 bgCol) {
	uint8_t pixel = 0U; uint32_t newPixel = 0U;

	// Storing the bitwised variable is required, it can't be done in the if.
	pixel = (video & 0xFF000000) >> 24;
	if (pixel == 0xFF) {
		newPixel |= static_cast<uint8_t>(fgCol.x * 255);
	} else {
		newPixel |= static_cast<uint8_t>(bgCol.x * 255);
	}
	pixel = (video & 0xFF0000) >> 16;
	if (pixel == 0xFF) {
		newPixel |= (static_cast<uint8_t>(fgCol.y * 255) << 8);
	} else {
		newPixel |= (static_cast<uint8_t>(bgCol.y * 255) << 8);
	}
	pixel = (video & 0xFF00) >> 8;
	if (pixel == 0xFF) {
		newPixel |= (static_cast<uint8_t>(fgCol.z * 255) << 16);
	} else {
		newPixel |= (static_cast<uint8_t>(bgCol.z * 255) << 16);
	}
	pixel = (video & 0xFF);
	if (pixel == 0xFF) {
		newPixel |= (static_cast<uint8_t>(fgCol.w * 255) << 24);
	} else {
		newPixel |= (static_cast<uint8_t>(bgCol.w * 255) << 24);
	}

	return newPixel;
}

void Chip8::RunMenu(int screenWidth, int screenHeight) {
	if (showMenu) {
		// Menu Window
		ImGui::SetNextWindowPos(ImVec2(5, 5));
		ImGui::SetNextWindowSize(ImVec2(300, 325));
		ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoResize);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		if (ImGui::Button("Load ROM")) {
			LoadRom((const char*)buf);
		}
		ImGui::SameLine();
		ImGui::InputText("##", buf, sizeof(buf), ImGuiInputTextFlags_CharsNoBlank);
		if (ImGui::Button("Resume"))
			isRunning = true;
		ImGui::SameLine();
		if (ImGui::Button("Pause"))
			isRunning = false;
		ImGui::SameLine();
		if (ImGui::Button("Step")) {
			isRunning = true;
			RunCycle();
			isRunning = false;
		}
		ImGui::InputInt("State Number", &whichState, 1, 1);
		if (whichState <= 0)
			whichState = 0;
		else if (whichState >= 9)
			whichState = 9;
		if (ImGui::Button("Save State")) {
			isRunning = false; // Pause the other thread while creating state.
			savestates.CreateState(this, savestates.States[whichState]);
			isRunning = true; // Resume the other thread.
		}
		ImGui::SameLine();
		if (ImGui::Button("Load State")) {
			isRunning = false;
			savestates.Loadstate(this, savestates.States[whichState]);
		}
		
		ImGui::InputInt("Video Scale", &videoScale, 1, 5);
		if (videoScale <= 1)
			videoScale = 1;
		ImGui::InputInt("Clock Delay", &cycleDelay, 5, 25);
		if (cycleDelay <= 5)
			cycleDelay = 5;
		ImGui::ColorEdit3("FG Color", (float*)&foreground);
		ImGui::ColorEdit3("BG Color", (float*)&background);
		if (ImGui::Button("Swap Color Palette")) {
			auto temp = foreground; // classic buffer swap, would be faster with pointers though
			foreground = background;
			background = temp;
		}
		ImGui::End();

		// Interpreter Window
		ImGui::SetNextWindowPos(ImVec2(305, 5));
		// adds small window buffer required for imgui window to function properly
		int gameW = 32 + (64 * videoScale); int gameH = 48 + (32 * videoScale);
		ImGui::Begin("Interpreter", NULL, ImGuiWindowFlags_NoResize);
		ImGui::SetWindowSize(ImVec2(static_cast<float>(gameW), static_cast<float>(gameH)));
		if (updateDrawImage) {
			// Convert Monochrome B/W to custom palette
			for (int i = 0; i < 2048; i++) {
				display[i] = GetColoredPixel(video[i], foreground, background);
			}

			glBindTexture(GL_TEXTURE_2D, TEX);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 32, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, display);
			glBindTexture(GL_TEXTURE_2D, 0);
			updateDrawImage = false;
		}
		ImGui::Image(reinterpret_cast<ImTextureID>(TEX), ImVec2(static_cast<float>(64 * videoScale), static_cast<float>(32 * videoScale)));
		ImGui::End();

		// Debugger Windows
		ImGui::SetNextWindowPos(ImVec2(5, 330));
		ImGui::Begin("Debugger", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
		ImGui::PushFont(RobotoMono); // Proper push/pop
		ImGui::BeginChild("DebugL", ImVec2(140, 425), false);
		ImGui::Text("opcode: %x", opcode);
		ImGui::Text("PC: %hu", pc);
		ImGui::Text("I: %hu", I);
		for (int i = 0; i < 16; i++) {
			ImGui::Text("V[%0i]: %x", i, V[i]);
		}
		ImGui::EndChild(); ImGui::SameLine(); //DebugL
		
		ImGui::BeginChild("DebugR", ImVec2(135, 425), false);
		ImGui::Text("Delay Timer: %x", delayTimer);
		ImGui::Text("Sound Timer: %x", soundTimer);
		ImGui::Text("Stack Ptr: %hu", sp);
		for (int i = 0; i < 16; i++) {
			ImGui::Text("S[%i]: %x", i, stack[i]);
		}
		ImGui::EndChild(); ImGui::SameLine(); // DebugR
		ImGui::PopFont(); // Proper push/pop
		ImGui::End(); ImGui::SameLine();
		
		// RAM Contents Window
		ImGui::SetNextWindowPos(ImVec2(305, static_cast<float>(gameH + 5)));
		ImGui::Begin("RAM", NULL);
		ImGui::PushFont(RobotoMono); // Proper push/pop
		ramViewer.Cols = 16;
		ramViewer.OptShowAscii = true;
		ramViewer.ReadOnly = true;
		ramViewer.DrawContents(&ram, sizeof(ram), 0);
		ImGui::PopFont(); // Proper push/pop
		ImGui::End();
	}
}

void Chip8::Table0() {
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8() {
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE() {
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF() {
	((*this).*(tableF[opcode & 0x00FFu]))();
}

// NO OPERATION
void Chip8::OP_NULL() {

}

// Clear screen
void Chip8::OP_00E0() {
	memset(video, 0, sizeof(video));
}

// Return from subroutine
void Chip8::OP_00EE() {
	--sp;
	pc = stack[sp];
}

// Jump to a machine code routine at nnn.
void Chip8::OP_0nnn() {
	OP_NULL();
}

// Jump to address at nnn
void Chip8::OP_1nnn() {
	uint16_t addr = opcode & 0x0FFFu;
	pc = addr;
}

// Call address at nnn
void Chip8::OP_2nnn() {
	uint16_t addr = opcode & 0x0FFFu;
	stack[sp] = pc;
	++sp;
	pc = addr;
}

// Skips next instruction if Vx equals byte at nn
void Chip8::OP_3xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (V[x] == byte) {
		pc += 2;
	}
}

// Skips next instruction if Vx does not equals byte at nn
void Chip8::OP_4xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (V[x] != byte) {
		pc += 2;
	}
}

// Skips next instruction if Vx equals Vy
void Chip8::OP_5xy0() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[x] == V[y]) {
		pc += 2;
	}
}

// Sets Vx to byte nn
void Chip8::OP_6xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[x] = byte;
}

// Adds byte nn to Vx
void Chip8::OP_7xnn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[x] += byte;
}

#pragma region 8XY Instructions
// Sets Vx to the value of Vy
void Chip8::OP_8xy0() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] = V[y];
}

// Sets Vx to (Vx OR Vy)
void Chip8::OP_8xy1() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] |= V[y];
}

// Sets Vx to (Vx AND Vy)
void Chip8::OP_8xy2() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] &= V[y];
}

// Sets Vx to (Vx XOR VY)
void Chip8::OP_8xy3() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	V[x] ^= V[y];
}

// Adds VY to Vx. VF is set to 1 when there's a carry, and to 0 when there isn't.
void Chip8::OP_8xy4() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = V[x] + V[y];

	if (sum > 255u) {
		V[0xF] = 1;
	} else {
		V[0xF] = 0;
	}

	V[x] = sum & 0xFFu;
}

// VY is subtracted from Vx. VF is set to 0 when there's a borrow, and 1 when there isn't.
void Chip8::OP_8xy5() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[x] > V[y]) {
		V[0xF] = 1;
	} else {
		V[0xF] = 0;
	}

	V[x] -= V[y];
}

// Shifts Vx right by one. VF is set to the value of the least significant bit of Vx before the shift.
void Chip8::OP_8xy6() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	V[0xF] = (V[x] & 0x1u);

	V[x] >>= 1;
}

// Sets Vx to VY minus Vx. VF is set to 0 when there's a borrow, and 1 when there isn't
void Chip8::OP_8xy7() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[y] > V[x]) {
		V[0xF] = 1;
	} else {
		V[0xF] = 0;
	}

	V[x] = V[y] - V[x];
}

// Shifts Vx left by one. VF is set to the value of the most significant bit of Vx before the shift.
void Chip8::OP_8xyE() {
	uint8_t x = (opcode & 0xF00u) >> 8u;
	V[0xF] = (V[x] & 0x80u) >> 7u;

	V[x] <<= 1;
}
#pragma endregion

// Skips the next instruction if Vx doesn't equal Vy.
void Chip8::OP_9xy0() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;

	if (V[x] != V[y]) {
		pc += 2;
	}
}

// Sets I to the address nnn
void Chip8::OP_Annn() {
	uint16_t addr = opcode & 0x0FFFu;
	I = addr;
}

// Jumps to the address nnn plus V0
void Chip8::OP_Bnnn() {
	uint16_t addr = opcode & 0x0FFFu;
	pc = V[0] + addr;
}

// Sets Vx to a random number, masked by byte nn
void Chip8::OP_Cxbb() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	V[x] = randByte(randGen) & byte;
}

// Draws a sprite at coordinate (Vx, Vy) that has a width of 8 pixels and a height of N pixels.
// Each row of 8 pixels is read as bit-coded starting from memory location I;
// I value doesn't change after the execution of this instruction.
// VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
// and to 0 if that doesn't happen.
void Chip8::OP_Dxyn() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t y = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = V[x] % VIDEO_WIDTH;
	uint8_t yPos = V[y] % VIDEO_HEIGHT;

	V[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t pixel = ram[I + row];

		for (unsigned int col = 0; col < 8; ++col) {
			// Weird math to AND together pixel bytes with bitwise
			uint8_t spritePixel = pixel & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// Sprite pixel is on
			if (spritePixel) {
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF) {
					V[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
	// Small optimization that allows us to only process a new image when we have new data.
	updateDrawImage = true;
}

// Skips the next instruction if the key stored in VX is pressed
void Chip8::OP_Ex9E() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t key = V[x];

	if (keypad[key]) {
		pc += 2;
	}
}

// Skips the next instruction if the key stored in VX isn't pressed
void Chip8::OP_ExA1() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t key = V[x];

	if (!keypad[key]) {
		pc += 2;
	}
}

// Sets VX to the value of the delay timer
void Chip8::OP_Fx07() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;

	V[x] = delayTimer;
}

// A key press is awaited, and then stored in VX
void Chip8::OP_Fx0A() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;

	if (keypad[0]) {
		V[x] = 0;
	} else if (keypad[1]) {
		V[x] = 1;
	} else if (keypad[2]) {
		V[x] = 2;
	} else if (keypad[3]) {
		V[x] = 3;
	} else if (keypad[4]) {
		V[x] = 4;
	} else if (keypad[5]) {
		V[x] = 5;
	} else if (keypad[6]) {
		V[x] = 6;
	} else if (keypad[7]) {
		V[x] = 7;
	} else if (keypad[8]) {
		V[x] = 8;
	} else if (keypad[9]) {
		V[x] = 9;
	} else if (keypad[10]) {
		V[x] = 10;
	} else if (keypad[11]) {
		V[x] = 11;
	} else if (keypad[12]) {
		V[x] = 12;
	} else if (keypad[13]) {
		V[x] = 13;
	} else if (keypad[14]) {
		V[x] = 14;
	} else if (keypad[15]) {
		V[x] = 15;
	} else {
		pc -= 2;
	}

}

// Sets the delay timer to VX
void Chip8::OP_Fx15() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	delayTimer = V[x];
}

// Sets the sound timer to VX
void Chip8::OP_Fx18() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	soundTimer = V[x];
}

//  Adds VX to I
void Chip8::OP_Fx1E() {
	uint8_t x = (opcode & 0x0F00) >> 8u;
	if (I + V[x] > 0xFFF)
		V[0xF] = 1;
	else
		V[0xF] = 0;

	I += V[(opcode & 0x0F00) >> 8];
}

// Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register VX
void Chip8::OP_Fx29() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = V[x];

	I = FONTSET_START_ADDRESS + (5 * digit);
}

// Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
void Chip8::OP_Fx33() {
	uint8_t x = (opcode & 0x0F00u) >> 8u;
	uint8_t value = V[x];

	// Ones-place
	ram[I + 2] = value % 10;
	value /= 10;

	// Tens-place
	ram[I + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	ram[I] = value % 10;

}

// Stores V0 to VX in memory starting at address I
void Chip8::OP_Fx55() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	for (int i = 0; i <= x; ++i) {
		ram[I + i] = V[i];
	}
}

// Fill registers V0 to VX inclusive with the values stored in memory starting at address I
void Chip8::OP_Fx65() {
	uint8_t x = (opcode & 0x0F00) >> 8u;

	for (int i = 0; i <= x; ++i) {
		V[i] = ram[I + i];
	}
}
