#pragma once
#include <Windows.h>

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 32

#define WORLD_WIDTH CHUNK_WIDTH*8
#define WORLD_HEIGHT CHUNK_HEIGHT

#define TERRAIN_SURFACE 127

#define SAVE_FILE_PATH "game_data/terrain"

enum {
	CELL_AIR = 0,
	CELL_DIRT,
	CELL_WATER
};

struct Cell {
	BYTE terrain;
};
