#include <cstdint>

class State {
public:
	State();

	// Monochrome B/W Display
	uint32_t video[32 * 64];

	// 2-Color Display
	uint32_t display[32 * 64];

	// Input
	uint8_t keypad[16];

	// Main Memory 
	uint8_t ram[4096];

	// Registers
	uint16_t opcode;
	uint8_t V[16];
	uint16_t I;
	uint16_t pc;
	uint16_t sp;
	uint16_t stack[16];

	// Timers
	uint8_t delayTimer;
	uint8_t soundTimer;
	int cycleDelay;
};