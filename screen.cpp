#include "screen.h"
#include <unistd.h>
#include <curses.h>
#include <sstream>

using namespace std;

void screen::init(){
	initscr();
	cbreak();
	refresh();
	this->window = newwin(33,64,0,0);
}

void screen::clear(){
	wrefresh(this->window);
}

void screen::test(){
	mvaddstr(13, 33, "Hello, world!");
	refresh();
	getch();
}

void screen::blit(unsigned short display[][64], unsigned short pc)
{
	for(int i=0; i<32; i++){
		for(int j=0; j<64; j++){
			if(display[i][j] == 0)
				mvwaddch(this->window, i, j, '0');
			else
				mvwaddch(this->window, i, j, '1');
			wrefresh(this->window);
		}
	}
	/*
	   stringstream ss;//create a stringstream
	   ss << pc;//add number to the stream
	   mvwaddstr(this->window, 33, 33, ss.str().c_str());
	   wrefresh(this->window);
	   */
}

void screen::cleanup(){
	delwin(this->window);
	endwin();
}

