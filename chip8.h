#ifndef CHIP_8_H
#define CHIP_8_H

class chip8{
	private:
		/* [ memory map ]
		 * 0x000-0x1FF -> chip8 interpreter
		 * 0x040-0x0A0 -> font set
		 * 0x200-0xFFF -> program ROM and RAM (4096-512)
		 */
		unsigned char mem[4096];

		unsigned char V[16];
		unsigned char stack[16];
		bool VF;

		unsigned char delay_timer;
		unsigned char sound_timer;

	public:
		bool DF;
		unsigned short display[32][64];
		unsigned short opcode;
		unsigned short PC; // program counter
		unsigned int I; // index register
		unsigned short SP;

		void init();

		void executeOp();
		void disasmOp();
		void clearScreen();
		void loadProgram(const char* filename);
		void fetchOp();
};

#endif
