#pragma once
#include "Windows.h"
#include <iostream>

class WaterSim {
public:
	const int WIDTH = 32, HEIGHT = 64;
	
	const float FLOW = 0.01f;
	float*** grid;

	WaterSim() {
		// Allocating memory (and setting it to 0)
		grid = (float***)malloc(sizeof(float**) * (WIDTH + 1));
		for (int i = 0; i < WIDTH + 1; i++) {
			grid[i] = (float**)malloc(sizeof(float*) * (HEIGHT + 1));
			for (int j = 0; j < HEIGHT + 1; j++) {
				grid[i][j] = (float*)malloc(sizeof(float) * (WIDTH + 1));
				memset(grid[i][j], 0, sizeof(float) * (WIDTH + 1));
			}
		}
	}
	void DbgPlaceWater(int x, int y, int z) {
		grid[x][y][z] = MAX_PRESSURE;
		grid[x-1][y][z] = MAX_PRESSURE;
		grid[x + 1][y][z] = MAX_PRESSURE;
		grid[x - 1][y+1][z] = MAX_PRESSURE;
		grid[x - 1][y-1][z] = MAX_PRESSURE;
		grid[x - 1][y-1][z-1] = MAX_PRESSURE;
		grid[x + 1][y - 1][z - 1] = MAX_PRESSURE;
		grid[x - 1][y + 1][z - 1] = MAX_PRESSURE;
		grid[x - 1][y - 1][z + 1] = MAX_PRESSURE;
		grid[x + 1][y + 1][z - 1] = MAX_PRESSURE;
		grid[x - 1][y + 1][z + 1] = MAX_PRESSURE;
	}
	void UpdateParticle(int x, int y, int z) {
		// 1 (down)
		if (y != 0 && (grid[x][y][z] > grid[x][y - 1][z] || grid[x][y][z] + grid[x][y - 1][z] < MAX_PRESSURE)) {
			if (grid[x][y][z] + grid[x][y - 1][z] <= MAX_PRESSURE) {
				grid[x][y - 1][z] += grid[x][y][z];
				grid[x][y][z] = 0.0f;
			}
			else {
				grid[x][y][z] -= FLOW;
				grid[x][y - 1][z] += FLOW;
			}
		}
		// 2 (left, right, front, back)
		else {
			int r = rand() % 2;
			// left
			if (r == 0 && x < WIDTH - 1) {
				float average = (grid[x][y][z] + grid[x + 1][y][z]) / 2.0f;
				grid[x][y][z] = average;
				grid[x + 1][y][z] = average;
			}
			// right
			if (r == 1 && x > 0) {
				float average = (grid[x][y][z] + grid[x - 1][y][z]) / 2.0f;
				grid[x][y][z] = average;
				grid[x - 1][y][z] = average;
			}
			// front
			if (r == 0 && z < WIDTH - 1) {
				float average = (grid[x][y][z] + grid[x][y][z+1]) / 2.0f;
				grid[x][y][z] = average;
				grid[x][y][z+1] = average;
			}
			// back
			if (r == 1 && z > 0) {
				float average = (grid[x][y][z] + grid[x][y][z-1]) / 2.0f;
				grid[x][y][z] = average;
				grid[x][y][z-1] = average;
			}
		}
		// 3 (up)
		if (y < HEIGHT-1 && grid[x][y][z] > MAX_PRESSURE) {
			grid[x][y + 1][z]+= grid[x][y][z] - MAX_PRESSURE;
			grid[x][y][z] = MAX_PRESSURE;
		}
		if (grid[x][y][z].value)
	}
	void Update(int k = 5) {
		//std::cout << "NEW FRAME HAHHA\n";
		for(int k = 0; k < 5; k++)
			for (int x = 0; x < WIDTH; x++)
				for (int y = 0; y < HEIGHT; y++)
					for (int z = 0; z < WIDTH; z++)
						if (grid[x][y][z] != 0)
							UpdateParticle(x, y, z);
	}
};

/*
		BYTE average = c.water;
		BYTE k = 0;
		for (int i = 0; i < 4; i++) {
			if (indices[i][0]>=0 && indices[i][1]>=0
				&& indices[i][0] <= WORLD_WIDTH && indices[i][1] <= WORLD_WIDTH) {
				if (grid[indices[i][0]][y][indices[i][1]].type != CELL_DIRT) {
					average += grid[indices[i][0]][y][indices[i][1]].water;
					k++;
					indices[i][2] = 1;
				}
			}
		}
		if (k == 0) break;
		average = average / k;
		BYTE remainder = average % k;
		c.water = average + remainder;
		for (int i = 0; i < 4; i++) {
			if (indices[i][2]) {
				grid[indices[i][0]][y][indices[i][1]].water = average;
			}
		}
		break;
*/

/*
		BYTE volume = 0;
		BYTE flow = 0;
		if (c.water > WATER_HFLOW) flow = WATER_HFLOW;
		else if (c.water > WATER_HFLOW / 2) flow = WATER_HFLOW / 2;
		else if (c.water > 4) flow = 4;
		else {
			//c.water = 0; break;
		}
		for (int i = 0; i < 4; i++) {
			if (indices[i][0]>=0 && indices[i][2]>=0
				&& indices[i][0] <= WORLD_WIDTH && indices[i][2] <= WORLD_WIDTH) {
				BYTE room = GetCellRoom(grid[indices[i][0]][indices[i][1]][indices[i][2]]);
				grid[indices[i][0]][indices[i][1]][indices[i][2]].water += min(flow / 4, room);
				volume += min(flow / 4, room);
			}
		}
		c.water -= volume;
*/