#include "state.h"
#include <vcruntime_string.h>

State::State() {
	memset(video, 0, sizeof(video));
	memset(display, 0, sizeof(display));
	memset(ram, 0, sizeof(ram));
	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(keypad, 0, sizeof(keypad));
	opcode = 0;
	I = 0;
	pc = 0;
	sp = 0;
	delayTimer = 0;
	soundTimer = 0;
}