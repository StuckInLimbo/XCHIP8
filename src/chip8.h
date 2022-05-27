#pragma once

#include <cstdint>
#include <random>

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8 {
public:
	Chip8();
	// File Functions
	void LoadRom(const char* filename);

	// Interpreter
	void RunCycle();

	// Reset
	void Reset();

	// Graphics
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT];

	// Input
	uint8_t keypad[KEY_COUNT];

	// Memory
	uint8_t memory[MEMORY_SIZE];

	// Registers
	uint16_t opcode;
	uint8_t V[REGISTER_COUNT];
	uint16_t I;
	uint16_t pc;
	uint16_t sp;
	uint16_t stack[STACK_LEVELS];

	// Timers
	uint8_t delayTimer;
	uint8_t soundTimer;

	// Booleans
	bool isLoaded = false;
	bool shouldDraw = false;

private:
	// Function Pointer Tables
	void Table0();
	void Table8();
	void TableE();
	void TableF();

#pragma region Opcodes
	// NOP
	void OP_NULL();
	// CLS
	void OP_00E0();
	// RET
	void OP_00EE();
	// SYS addr
	void OP_0nnn();
	// JMP addr
	void OP_1nnn();
	// CALL addr
	void OP_2nnn();
	// SE Vx, byte
	void OP_3xnn();
	// SNE Vx, byte
	void OP_4xnn();
	// SE Vx, Vy
	void OP_5xy0();
	// LD Vx, byte
	void OP_6xnn();
	// ADD Vx, byte
	void OP_7xnn();
	// LD Vx, Vy
	void OP_8xy0();
	// OR Vx, Vy
	void OP_8xy1();
	// AND Vx, Vy
	void OP_8xy2();
	// XOR Vx, Vy
	void OP_8xy3();
	// ADD Vx, Vy
	void OP_8xy4();
	// SUB Vx, Vy
	void OP_8xy5();
	// SHR Vx
	void OP_8xy6();
	// SUBN Vx, Vy
	void OP_8xy7();
	// SHL Vx
	void OP_8xyE();
	// SNE Vx, Vy
	void OP_9xy0();
	// LD I, addr
	void OP_Annn();
	// JMP V0, addr
	void OP_Bnnn();
	// RND Vx, byte
	void OP_Cxbb();
	// DRW Vx, Vy, nibble
	void OP_Dxyn();
	// SKP Vx
	void OP_Ex9E();
	// SKNP Vx
	void OP_ExA1();
	// LD Vx, DT
	void OP_Fx07();
	// LD Vx, K
	void OP_Fx0A();
	// LD DT, Vx
	void OP_Fx15();
	// LD ST, Vx
	void OP_Fx18();
	// ADD I, Vx
	void OP_Fx1E();
	// LD F, Vx
	void OP_Fx29();
	// LD BCD, Vx
	void OP_Fx33();
	// LD [I], Vx
	void OP_Fx55();
	// LD Vx, [I]
	void OP_Fx65();
#pragma endregion

	// RNG member vars
	std::default_random_engine randGen;
	std::uniform_int_distribution<uint16_t> randByte;

	typedef void (Chip8::* Chip8Func)();
	Chip8Func table[0xF + 1]{ &Chip8::OP_NULL };
	Chip8Func table0[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func table8[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableE[0xE + 1]{ &Chip8::OP_NULL };
	Chip8Func tableF[0x65 + 1]{ &Chip8::OP_NULL };
};