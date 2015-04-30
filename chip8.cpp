#include "chip8.h"
#include <cstdio>
#include <iostream>
#include <cstdlib>
#define DEBUG_MODE

using namespace std;

void chip8::init(){
	this->I = 0;
	this->opcode = 0;
	this->PC = 0x200;
	this->SP = 0;
	this->DF = 0;

	unsigned char fonts[] = {
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

	// load font set
	for(int i=0x040; i<0x0a0; i++)
		this->mem[i] = fonts[i-0x040];

	// clear flags
	for(int i=0; i<16; i++)
		this->V[i] = 0;

	// clear
	for(int i=0; i<4096; i++)
		this->mem[i] = 0;

	this->clearScreen();
}

void chip8::clearScreen(){
	// Clear the screen
	for(int i=0; i<32; i++){
		for(int j=0; j<64; j++){
			this->display[i][j] = 0;
		}
	}
}

void chip8::loadProgram(const char* filename){
	FILE* f = fopen(filename,"r");
	unsigned char buffer[4096-512]; // max rom size
	size_t bytes_read = 0;
	bytes_read = fread(buffer, sizeof(unsigned char),4096-512,f);

	for(int i=0; i<bytes_read; i++)
	{
		this->mem[i+0x200] = buffer[i];
	}
}

void chip8::fetchOp(){
	this->opcode = this->mem[this->PC] << 8 | this->mem[this->PC + 1];
}

// Print regs

void chip8::executeOp(){
	// execute this->opcode
	//
#ifdef DEBUG_MODE
	this->disasmOp();
#endif
	switch(this->opcode & 0xf000)
	{
		case 0x0:
			switch(this->opcode & 0x00ff){
				case 0x00ee: // return from subroutine
					this->PC = this->stack[this->SP];
					this->SP-=1;
					break;
				case 0x00e0: // clear screen
					this->clearScreen();
					break;
			}
			break;

			// Jumps to address at NNN
		case 0x1000:
			this->PC = this->opcode &0xfff;
			break;

			// execute subroutine
		case 0x2000:
			this->SP+=1;
			this->stack[this->SP] = this->PC;
			this->PC = this->opcode & 0xfff;
			break;

			// skip next instruction if this->VX == NN
		case 0x3000:
			if(this->V[(this->opcode & 0x0f00)>>8] == (this->opcode & 0x0ff))
				this->PC += 2;
			break;

		case 0x4000:
			if(this->V[(this->opcode & 0x0f00)>>8] != (this->opcode & 0x0ff))
				this->PC += 2;
			break;

		case 0x5000:
			if(this->V[(this->opcode & 0x0f00)>>8] == this->V[(this->opcode & 0x0f0)>>4])
				this->PC += 2;
			break;

		case 0x6000:
			// Seems correct
			this->V[(this->opcode & 0x0f00) >> 8] = (this->opcode & 0x0ff);
			break;

		case 0x7000:
			//  Apparently we need to handle number wraps here...
			//  So, we add together the two unsigned integers and store them in an int
			//  then if we are larger than 255, we subtract %255 and store them in an unsigned char
            /*
			int tempresult = (this->V[(this->opcode&0x0f00)>>8] + this->opcode&0x00ff);
			if(tempresult > 255)
			{
				unsigned char result = (unsigned char)(tempresult - (tempresult%255));
				this->V[(this->opcode&0x0f00)>>8] += result;
			}
			else{
				this->V[(this->opcode&0x0f00)>>8] += (this->opcode&0x00ff);
			}
            */
			break;
		case 0x8000:
			switch(this->opcode&0x000f){
				case 0: // 8xy0
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x00f0)>>4];
					break;
				case 1:
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]|this->V[(this->opcode&0x00f0)>>4];
					break;
				case 2:
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]&this->V[(this->opcode&0x00f0)>>4];
					break;
				case 3:
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]^this->V[(this->opcode&0x00f0)>>4];
					break;
				case 4:
					// Looks correct, added check for overflow
					/*int temp = this->V[(this->opcode&0x0f00)>>8]+this->V[(this->opcode&0x00f0)>>4];
					if(temp > 255){
						this->V[0x0f] = 1;
						temp = temp - temp%255;
					}
					else
						this->V[0x0f] = 0;

					this->V[(this->opcode&0x0f00)>>8] = (unsigned char) temp;
                    */
					break;
				case 5:
					// Looks correct
					if(this->V[(this->opcode&0x0f00)>>8] < this->V[(this->opcode&0x00f0)>>4])
						this->V[0x0f] = 0;
					else
						this->V[0x0f] = 1;
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]-this->V[(this->opcode&0x00f0)>>4];
					break;
				case 6:
					this->V[0x0f] = this->V[(this->opcode&0x0f00)]&0x0001;
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]>>1;
					break;
				case 7:
					if(this->V[(this->opcode&0x00f0)>>4] < this->V[(this->opcode&0x0f00)>>8])
						this->V[0x0f] = 0;
					else
						this->V[0x0f] = 1;
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x00f0)>>4] - this->V[(this->opcode&0x0f00)>>8];
					break;
				case 0xe:
					this->V[0x0f] = (this->V[(this->opcode&0x0f00)]&0x8000)>>15;
					this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]<<1;
					break;
			}
			break;

		case 0x9000:
			if(this->V[(this->opcode&0x0f00)>>8] != this->V[(this->opcode&0x00f0)>>4])
				this->PC+=2;
			break;
		case 0xa000:
			this->I = this->opcode&0x0fff;
			break;
		case 0xb000:
			this->PC = (this->opcode&0x0fff)+this->V[0];
			break;
		case 0xc000:
			this->V[(this->opcode&0x00f0)>>4] = rand()%1000&(this->opcode&0x00ff);
		case 0xd000:
			{
				// draw a sprite at the specified coordinate
				// "Each row of 8 pixels is read as bit-coded, with the MSBit of each byte displayed on the left starting from memory location I"
				// "I doesn't change after execution of this instruction"
				int y1 = this->V[(this->opcode&0x0f00)>>8];
				int x1 = this->V[(this->opcode&0x00f0)>>4];
				int height = this->opcode&0x0f;
				int index = this->I;

				{
					bool changed = true;

					for(int i=0; i<height; i++){
						unsigned char halfrowA = this->mem[index+i];
						for(int j=0; j<7; j++)
						{
							unsigned short oldval = this->display[y1+i][x1+j];
							unsigned short newval = halfrowA&(2<<6-j) > 0 ? 1 : 0;
							this->display[y1+i][x1+j] = newval;
							if(oldval != newval){
								changed = false;
							}
						}
						unsigned short oldval = this->display[y1+i-1][x1+7];
						unsigned short newval = halfrowA&(0x01) > 0 ? 1 : 0;
						this->display[y1+i-1][x1+7] = halfrowA&(0x01) > 0 ? 1 : 0;
						if(oldval != newval)
							changed = false;
						index += 8;
					}
					if(changed == false)
						this->V[0xf] = 1;
					else
						this->V[0xf] = 0;
					this->DF = true;
				}
			}
			break;
		case 0xe000:
			switch(opcode&0x000f){
				//TODO not implemented
				case 0x0e:
					if(this->V[(this->opcode&0x0f00)>>8] == 1) // if key VX is pressed, skip next instruction
						this->PC += 2;
					break;
				case 0x01:
					if(this->V[(this->opcode&0x0f00)>>8] != 1) // " not pressed
						this->PC += 2;
					break;
			}
			break;

		case 0xf000:
			switch(this->opcode&0xff){

				case 0x07:
					this->V[(this->opcode&0x0f00)>>8] = this->delay_timer;
					break;
				case 0x0a:
					// TODO: await keypress then store in vx
					//
					break;
				case 0x15:
					//TODO not implemented
					this->delay_timer = this->V[(this->opcode&0x0f00)>>8];
					break;
				case 0x18:
					//TODO not implemented
					this->sound_timer = this->V[(this->opcode&0x0f00)>>8];
					break;
				case 0x1e:
					this->I = this->I + this->V[(this->opcode&0x0f00)>>8];
				case 0x29:
					// TODO: "Sets I to the location of the sprite for the character in VX"
					// "Characters 0-F are represented by a 4x5 font"
					//
					this->I = this->V[(this->opcode&0x0f00)>>8];

					break;
				case 0x33:  // BCD rep of this->VX at addr this->I
					mem[this->I] = this->V[(this->opcode &0x0f00) << 8]/100;
					mem[this->I+1] = (this->V[(this->opcode &0x0f00) << 8]/10)%10;
					break;

				case 0x55:
					for(int i=0; i<((this->opcode&0x0f00)>>8); i+=1){
						mem[(this->I+i)] = this->V[i];
					}
					this->I = this->I + ((this->opcode&0x0f00)>>8) + 1;
					break;
				case 0x65:
					for(int i=0; i<((this->opcode&0x0f00)>>8); i+=1){
						this->V[i] = this->mem[(this->I+i)];
					}
					this->I = this->I + ((this->opcode&0x0f00)>>8) + 1;
					break;
			}
			break;
		default:
			break;

	}
	this->PC += 2;
}

void chip8::disasmOp(){
	// execute this->opcode
	//
	switch(this->opcode & 0xf000)
	{
		case 0x0:
			switch(this->opcode & 0x00ff){
				case 0x00ee: // return from subroutine
					cout << "OP(ret){" << endl;
					cout << "    PC = " << (this->stack[this->SP-1]) << endl;
					cout << "}" << endl;
					break;
				case 0x00e0: // clear screen

					cout << "OP(clrscr){}" << endl;
					break;
			}
			break;

			// Jumps to address at NNN
		case 0x1000:
			cout << "OP(jmp NNN){" << endl;
			cout << "    PC = " << (this->opcode &0xfff) << endl;
			cout << "}" << endl;
			break;

			// execute subroutine
		case 0x2000:
			cout << "OP(call NNN){" << endl;
			cout << "    SP + 1 = PC = " << this->PC << endl;
			cout << "}" << endl;
			break;
		case 0x3000:
			cout << "OP(skip.e VX, NN){" << endl;
			if(this->V[(this->opcode & 0x0f00)>>8] == (this->opcode & 0x0ff))
				cout << "    PC = PC + 2" << endl;
			cout << "}" << endl;
			break;

		case 0x4000:
			cout << "OP(skip.e VX, NN){" << endl;
			if(this->V[(this->opcode & 0x0f00)>>8] != (this->opcode & 0x0ff))
				cout << "    PC = PC + 2" << endl;
			cout << "}" << endl;
			break;

		case 0x5000:
			cout << "OP(skip.e VX, VY){" << endl;
			if(this->V[(this->opcode & 0xf00)>>8] == this->V[(this->opcode & 0x0f0)>>4])
				cout << "PC = PC + 2" << endl;
			cout << "}" << endl;
			break;
		case 0x6000:
			cout << "OP(mov VX, NN){" << endl;
			cout << "    V[" << ((this->opcode&0x0f00)>>8) << "] = " << (this->opcode&0x0ff) << endl;
			cout << "}" << endl;
			break;

		case 0x7000:
			cout << "OP(add VX, NN){" << endl;
			cout << "    V[" << ((this->opcode&0x0f00)>>8) << "] = " << this->V[(this->opcode&0x0f00)>>8]+(this->opcode&0x0ff) << endl;
			cout << "}" << endl;
			break;

		case 0x8000:
			switch(this->opcode&0x000f){
				case 0: // 8xy0
					cout << "OP(VX = VY){" << endl;
					cout << "    V[" << ((this->opcode&0x0f00)>>8) << "]" << " = V[" << ((this->opcode&0x0f0)>>4);
					cout << "] = " << this->V[(this->opcode&0x00f0)>>4] << endl;
					cout << "}";
					break;
					/*
					   case 1:
#ifdef DEBUG_MODE
cout << "mov V" << ((this->opcode&0x0f00)>>8) << ", V" << ((this->opcode&0x0f00)>>8) << " OR V" << ((this->opcode&0x0f0)>>4) << endl;
#endif
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]|this->V[(this->opcode&0x00f0)>>4];
break;
case 2:
#ifdef DEBUG_MODE
cout << "mov V" << ((this->opcode&0x0f00)>>8) << ", V" << ((this->opcode&0x0f00)>>8) << " AND V" << ((this->opcode&0x0f0)>>4) << endl;
#endif
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]&this->V[(this->opcode&0x00f0)>>4];
break;
case 3:
#ifdef DEBUG_MODE
cout << "mov V" << ((this->opcode&0x0f00)>>8) << ", V" << ((this->opcode&0x0f00)>>8) << " XOR V" << ((this->opcode&0x0f0)>>4) << endl;
#endif
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]^this->V[(this->opcode&0x00f0)>>4];
break;
case 4:
#ifdef DEBUG_MODE
cout << "addc V" << ((this->opcode&0x0f00)>>8) << ", V" << ((this->opcode&0x0f0)>>4) << endl;
#endif
if(this->V[(this->opcode&0x0f00)>>8]+this->V[(this->opcode&0x00f0)>>4] > 0xffff)
this->V[0x0f] = 1;
else
this->V[0x0f] = 0;
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]+this->V[(this->opcode&0x00f0)>>4];
break;
case 5:
if(this->V[(this->opcode&0x0f00)>>8] < this->V[(this->opcode&0x00f0)>>4])
this->V[0x0f] = 0;
else
this->V[0x0f] = 1;
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]-this->V[(this->opcode&0x00f0)>>4];
break;
case 6:
this->V[0x0f] = this->V[(this->opcode&0x0f00)]&0x0001;
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]>>1;
break;
case 7:
if(this->V[(this->opcode&0x00f0)>>4] < this->V[(this->opcode&0x0f00)>>8])
this->V[0x0f] = 0;
else
this->V[0x0f] = 1;
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x00f0)>>4] - this->V[(this->opcode&0x0f00)>>8];
break;
case 0xe:
this->V[0x0f] = (this->V[(this->opcode&0x0f00)]&0x8000)>>15;
this->V[(this->opcode&0x0f00)>>8] = this->V[(this->opcode&0x0f00)>>8]<<1;
break;
*/
			}
			break;

		case 0x9000:
			cout << "OP(skip.ne VX, VY){" << endl;
			if(this->V[(this->opcode&0x0f00)>>8] != this->V[(this->opcode&0x00f0)>>4])
				cout << "    I = I + 2" << endl;
			cout << "}" << endl;
			break;
		case 0xa000:
			cout << "OP(mv I, NNN){" << endl;
			cout << "    I = " << (this->opcode&0x0fff) << endl;
			cout << "}" << endl;
			break;
		case 0xb000:
			cout << "OP(jmp V0+NNN){"<< endl;
			cout << "    PC = " << (this->opcode&0x0fff) << " + V[0] = " << (this->opcode&0x0fff) + this->V[0] << endl;
			cout << "}";
			break;
		case 0xc000:
			cout << "OP(mv VX rand()&NN){"<< endl;
			cout << "    V[" << ((this->opcode&0x00f0)>>4) << "] = rand()&"  << (this->opcode&0x00ff) << endl;
			cout << "}";
			break;
		case 0xd000:
			{
				// draw a sprite at the specified coordinate
				// "Each row of 8 pixels is read as bit-coded, with the MSBit of each byte displayed on the left starting from memory location I"
				// "I doesn't change after execution of this instruction"
				int y1 = this->V[(this->opcode&0x0f00)>>8];
				int x1 = this->V[(this->opcode&0x00f0)>>4];
				int height = this->opcode&0x0f;
				int index = this->I;

				cout << "OP(draw){" << endl;
				cout << "    (" << x1 << "," << y1 << ")"<< endl;
				cout << "}" << endl;
			}
			break;
		case 0xe000:
			switch(opcode&0x000f){
				case 0x0e:
					cout << "OP(skip.keypressed){"<<endl;
					cout << "V[" << (this->V[(this->opcode&0x0f00)>>8]) << " = " << (this->V[(this->opcode&0x0f00)>>8]) << endl;
					if(this->V[(this->opcode&0x0f00)>>8] == 1) // " not pressed
						cout << "    PC = PC + 2" << endl;
					cout << "}" << endl;
					break;
				case 0x01:
					cout << "OP(skip.notkeypressed){"<<endl;
					cout << "V[" << (this->V[(this->opcode&0x0f00)>>8]) << " = " << (this->V[(this->opcode&0x0f00)>>8]) << endl;
					if(this->V[(this->opcode&0x0f00)>>8] != 1) // " not pressed
						cout << "    PC = PC + 2 " << endl;
					cout << "}" << endl;
					break;
			}
			break;

		case 0xf000:
			switch(this->opcode&0xff){
				case 0x07:
					cout << "OP(mv VX, delay_timer){"<<endl;
					cout <<    "    V[" << (this->V[(this->opcode&0x0f00)>>8]) << "] = delay_timer = " << (this->delay_timer) << endl;
					cout << "}" << endl;
					break;
				case 0x0a:
					cout << "OP(pause.keypress){"<<endl;
					cout << "    TODO";
					cout << "}" << endl;
					break;
				case 0x15:
					this->delay_timer = this->V[(this->opcode&0x0f00)>>8];
					cout << "OP(mv delaytimer, VX){"<<endl;
					cout << "   delay_timer = " << "V[" << (this->V[(this->opcode&0x0f00)>>8]) << "] =" << (this->V[(this->opcode&0x0f00)>>8]) << endl;
					cout << "}" << endl;
					break;
				case 0x18:
					cout << "OP(mv soundtimer, VX){"<<endl;
					cout << "   sound_timer = " << "V[" << (this->V[(this->opcode&0x0f00)>>8]) << "] =" << (this->V[(this->opcode&0x0f00)>>8]) << endl;
					cout << "}" << endl;
					break;
				case 0x1e:
					cout << "OP(add I,VX){"<<endl;
					cout <<  "    I = I + V[" << ((this->opcode&0x0f00)>>8) << "] = " << this->I  << " + " << this->V[((this->opcode&0x0f00)>>8)] << endl;
					cout << "}" << endl;
					break;
				case 0x29:
					cout << "OP(mv I,VX){"<<endl;
					cout << "    I = " << "V[" << ((this->opcode&0x0f00)>>8) << "] = " << this->V[(this->opcode&0x0f00)>>8] << endl;
					cout << "}" << endl;
					break;
				case 0x33:  // BCD rep of this->VX at addr this->I
					cout << "OP(BCD of VX at I){"<<endl;
					cout << "    BCD(V[" << (this->opcode&0x0f00) << "] = TODO" << endl;

					/*
					   this->V[(this->opcode &0x0f00) << 8]/100;
					   (this->V[(this->opcode &0x0f00) << 8]/10)%10;
					   */

					cout << "}"<<endl;
					break;

				case 0x55:
					cout << "OP(write regs){"<<endl;
					for(int i=0; i<((this->opcode&0x0f00)>>8); i+=1){
						cout <<  "I + " << ((this->opcode&0x0f00)>>8) << " = V[" << i << "]" << endl;
					}
					cout << "    I =  " << this->I + ((this->opcode&0x0f00)>>8)+1 << endl;
					break;
				case 0x65:
					cout << "OP(read regs) {"<<endl;
					for(int i=0; i<((this->opcode&0x0f00)>>8); i+=1){
						cout << "    V[" << i << "] =" << this->mem[(this->I+i)] << endl;
					}
					cout << "    I = " <<this->I + ((this->opcode&0x0f00)>>8) + 1 << endl;
					cout << "}" << endl;
					break;
			}
			break;
		default:
			cout << "OP(unknown opcode){}" << endl;
			break;
	}
}

