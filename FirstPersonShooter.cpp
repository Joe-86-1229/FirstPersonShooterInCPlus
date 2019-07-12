// 3D First-Person-Shooter Engine
// https://github.com/OneLoneCoder/CommandLineFPS/blob/master/CommandLineFPS.cpp shoul take credit of this piece of code, I basically 
// followed most of the instructions and built-up this console 3D game just with little bit customization. The idea of engieen is 
// inpired by https://en.wikipedia.org/wiki/Wolfenstein_3D.
// Instruction:
//            Press W to move forward
//            Press S to move downward
//            Press A to rotate to left
//            Press D to rotate to right
// Initially, the player is facing downward in terms of 2D map

#include "pch.h"
#include "FirstPersonShooter.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <utility>

#include <Windows.h>


int main()
{
	// Create screen buffer
	wchar_t *screen = new wchar_t[SCREENWIDTH * SCREENHEIGHT];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;	// receives the number of character actually written

	
	// map in 2D
	std::wstring map;
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.........#....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	// Game loop
	while (1)
	{

		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> duration = tp2 - tp1;
		tp1 = tp2;
		float elapsedTime = duration.count();	// return ticks, 1 tick is 1 ms.

		// Handle rotation
		// press A, player rotates left by decrease player view angle.
		if (GetAsyncKeyState(static_cast<unsigned short>('A')) & 0x8000)
			// match the calling rate to make rotating more smoothly
			playerA -= (0.8f) * elapsedTime;	
		if (GetAsyncKeyState(static_cast<unsigned short>('D')) & 0x8000)
			playerA += (0.8f) * elapsedTime;

		// Handle forward movement, initially, player facing down of the map, so moving forward increase Y value.
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			playerX += sinf(playerA) * 5.0f * elapsedTime; // when player does not rotate, x coordinate won't change
			playerY += cosf(playerA) * 5.0f * elapsedTime;

			// collision detection
			if (map[static_cast<int>(playerY) * MAPWIDTH + static_cast<int>(playerX)] == '#')
			{
				playerX -= sinf(playerA) * 5.0f * elapsedTime; 
				playerY -= cosf(playerA) * 5.0f * elapsedTime;
			}
		}

		// Handle backward movement
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			playerX -= sinf(playerA) * 5.0f * elapsedTime;
			playerY -= cosf(playerA) * 5.0f * elapsedTime;

			if (map[static_cast<int>(playerY) * MAPWIDTH + static_cast<int>(playerX)] == '#')
			{
				playerX += sinf(playerA) * 5.0f * elapsedTime;
				playerY += cosf(playerA) * 5.0f * elapsedTime;
			}
		}

		for (int col = 0; col < SCREENWIDTH; col++)
		{ 
			float rayAngle = (playerA - FOV / 2.0f) + ((float)col / (float)SCREENWIDTH) * FOV;

			float distanceToWall = 0.0f;
			bool hitWall = false;	// Set true when ray hits wall blocks
			bool boundary = false;	// Set true when hits boundary bewteen wall blocks

			float eyeX = sinf(rayAngle); // Unit vector for ray in player space(normolize distance to wall)
			float eyeY = cosf(rayAngle);

			while (!hitWall)
			{
				distanceToWall += 0.1f;
				int coorX = (int)(playerX + eyeX * distanceToWall);
				int coorY = (int)(playerY + eyeY * distanceToWall);

				// if out of boundry
				if (coorX < 0 || coorY < 0 || coorX > MAPWIDTH || coorY > MAPHEIGHT)
				{
					hitWall = true;
					distanceToWall = maxRenderDistance;
				}
				else
				{	
					// convert 2d to 1d index; if the ray cell is a wall block
					if (map[coorY * MAPWIDTH + coorX] == '#')
					{
						hitWall = true;

						// detecting boundary. Further details, please go to https://www.youtube.com/watch?v=xW8skO7MFYw&t=1870s
						std::vector<std::pair<float, float>> disDotPair;
						for(int x = 0; x < 2; x++)
							for (int y = 0; y < 2; y++)
							{
								float lengthX = static_cast<float>(coorX) + x - playerX;
								float lengthY = static_cast<float>(coorY) + y - playerY;
								float distanceToCornor = sqrt(lengthX * lengthX + lengthY *lengthY);
								float dotValue = (eyeX * lengthX + eyeY * lengthY) / distanceToCornor; // Unit vector of cornor?
								disDotPair.push_back(std::make_pair(distanceToCornor, dotValue));
							}
						// Sort Pairs from closest to farthest
						std::sort(disDotPair.begin(), disDotPair.end(), [](const std::pair<float, float> &left, const std::pair<float, float> &right) {return left.first < right.first; });

						float fBound = 0.01;
						if (acos(disDotPair.at(0).second) < fBound) boundary = true;
						if (acos(disDotPair.at(1).second) < fBound) boundary = true;
					}
				}
			}
			int ceilLing = static_cast<float>(SCREENHEIGHT / 2.0) - SCREENHEIGHT / static_cast<float>(distanceToWall);
			int floor = SCREENHEIGHT - ceilLing;

			short shade = ' ';
			if (distanceToWall <= maxRenderDistance / 4.0f)			shade = 0x2588;		// fully shade, bright  very close	
			else if (distanceToWall < maxRenderDistance / 3.0f)		shade = 0x2593;		// dark shade	
			else if (distanceToWall < maxRenderDistance / 2.0f)		shade = 0x2592;		// medium shade
			else if (distanceToWall < maxRenderDistance)			shade = 0x2591;		// light shade, far away
			else shade = ' ';		// Too far away

			if (boundary) shade = ' ';

			for (int screenRow = 0; screenRow < SCREENHEIGHT; screenRow++)
			{
				if (screenRow <= ceilLing)
				{
					screen[screenRow * SCREENWIDTH + col] = ' ';
				}
				else if(screenRow > ceilLing && screenRow <= floor) //wall
				{
					screen[screenRow * SCREENWIDTH + col] = shade;
				}	
				else if(screenRow > floor)
				{
					// Shade floor based on distance, closer floor is #; further is -
					int a = screenRow;
					float b = 1.0f - ((static_cast<float>(screenRow) - SCREENHEIGHT / 2.0f) / (static_cast<float>(SCREENHEIGHT) / 2.0f));
					if (b < 0.25)
						shade = '#';
					else if (b < 0.5)
						shade = 'x';
					else if (b < 0.75)
						shade = '.';
					else if (b < 0.9)
						shade = '-';
					else
						shade = ' ';	
					screen[screenRow * SCREENWIDTH + col] = shade;
				}
			}
		}

		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ", playerX, playerY, playerA, 1.0f / elapsedTime);

		// Display Map
		for (int nx = 0; nx < MAPWIDTH; nx++)
			for (int ny = 0; ny < MAPHEIGHT; ny++)
			{
				// start at second row
				screen[(ny + 1)*SCREENWIDTH + nx] = map[ny * MAPWIDTH + nx];
			}
		screen[(static_cast<int>(playerY + 1) * SCREENWIDTH + static_cast<int>(playerX))] = 'P';
		
		screen[SCREENWIDTH * SCREENHEIGHT - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, SCREENWIDTH * SCREENHEIGHT, { 0, 0 }, &dwBytesWritten);
	}


	return 0;
}
