#include "chip8.h"
#include "screen.h"
#include <cstdlib>
#include <iostream>
#define DEBUG_MODE
#define SLOW_MO

chip8 c8;
screen sc;
using namespace std;

int main(int argc, char** argv){
    c8.init(); 
    c8.loadProgram("./games/Test128.ch8");

#ifndef DEBUG_MODE
    sc.init();
#endif
    
    while(1){
        c8.fetchOp();
        c8.executeOp();
#ifndef DEBUG_MODE
        if(c8.DF){
            sc.blit(c8.display, c8.PC);
            c8.DF = false;
        }
#endif
#ifdef SLOW_MO
    cin.ignore().get();
#endif
#ifdef SHOW_PC
        cout << "address: " << c8.PC << endl;
#endif
    }
}

