#ifndef SCREEN_H
#define SCREEN_H
#include <curses.h>

class screen{
	private:
		WINDOW * window;

	public:
		void init();
		void clear();
		void test();
		void cleanup();
		void blit(unsigned short display[][64],unsigned short pc);
};

#endif
