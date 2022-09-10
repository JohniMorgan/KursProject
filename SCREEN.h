#pragma once
#include <string>

const int MAXROW = 10, MAXCON = 79;
int offset[MAXROW/2 + 1];
std::string SCREEN[MAXROW];

void init_offset()
{
	offset[0] = MAXCON/2;
	for (int i = 1; i <= MAXROW / 2; ++i) {
		offset[i] = (offset[i - 1] >> i);
	}
}

void screen_init()
{
	for (int i = 0; i < MAXROW; ++i)
		SCREEN[i] = std::string(MAXCON, '.');
}

void screen_refresh() {
	for (int i = 0; i < MAXROW; ++i)
	{
		std::cout << SCREEN[i] << std::endl;
	}
}

void screen_clear() { screen_init(); }