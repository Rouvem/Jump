#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

wstring player;
string play;
unsigned char* pField = nullptr;
int nFieldWidth = 70; // map.dat file must match the nField dimensions!
int nFieldHeight = 40;
int nScreenWidth = 120;
int nScreenHeight = 100;

int timeSeconds = 0;
const int speed = 50;
const int secCycleCount = 1000 / speed;
int cycleCounter = 0;

bool DoesPieceFit(int nPosX, int nPosY) {
	for(int px = 0; px < 3; px++) {
		for (int py = 0; py < 3; py++) {
				
				// Get field index
				int fi = (nPosY + py) * nFieldWidth + (nPosX + px);
				
				// Check boundaries
				if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (player[(py*3)+px] != L'.' && pField[fi] != 0)
						return false;
				}
			}
		}
	}
	return true;
}
int main() {
	ifstream input("map.dat");
	int numLines = 0;
		pField = new unsigned char[nFieldWidth*nFieldHeight];

	if(input.is_open()) {
		while(!input.eof()) {
			string line;
			getline(input, line);
			if(line.length() != nFieldWidth) {
				cout << "map.dat file formatted incorrectly!" << endl;
				system("pause");
				return 0;
			}
			int numChar = 0;
			for(char& c : line) {
				//cout << ((numLines*nFieldWidth) + numChar) << endl;
				pField[numLines*nFieldWidth + numChar] = (c == '0') ? 0 : 1;
				numChar++;
			}
			numLines++;
		}
	}

	input.close();

	player.append(L".O.");
	player.append(L"/|\\");
	player.append(L"/ \\");
	
	// Make the *blank* screen
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	for(int i = 0; i < nScreenWidth*nScreenHeight; i++) {
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	bool bGameOver = false;
	bool bGameNotWon = true;
	bool bUpHold = false;
	bool allTheWayUp = false;
	bool alreadyJumped = false;
	int countUpHoldJump = 0;
	const int upJumpMax = 5;
	const int downJumpMax = 5;
	int countDownHoldJump = downJumpMax;

	const int winningX = 66;
	const int winningY = 1;

	int nCurrentX = 1; //nFieldWidth / 2;
	int nCurrentY = nFieldHeight - 4; //nFieldHeight / 2;
	bool bKey[3];

	while(!bGameOver && bGameNotWon) {
		// GAME TIMING =====================================================
		this_thread::sleep_for(chrono::milliseconds(speed)); // Game Tick

		// PROCESS USER INPUT =============================================
		
		for(int k = 0; k < 3; k++) {						  //  R    L    U
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x26"[k]))) != 0;

		}
		// GAME LOGIC =====================================================
		
		// left key
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentX - 1, nCurrentY)) ? 1 : 0;
		
		// right key
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentX + 1, nCurrentY)) ? 1 : 0;
				
		

		if(!allTheWayUp) {
			if(!DoesPieceFit(nCurrentX, nCurrentY - 1)) {
				countUpHoldJump = 0;
				bUpHold = false;
				allTheWayUp = true;
			}
			if(bKey[2]) {
					alreadyJumped = true;
					nCurrentY -= (!bUpHold && DoesPieceFit(nCurrentX, nCurrentY - 1)) ? 1 : 0;
					bUpHold = true;
					
					// if continue to hold key
					if(countUpHoldJump < upJumpMax) {
						nCurrentY -= (DoesPieceFit(nCurrentX, nCurrentY - 1)) ? 1 : 0;
						countUpHoldJump++;
					}
					if(countUpHoldJump == upJumpMax) {
						allTheWayUp = true;
						countUpHoldJump = 0;
						bUpHold = false;
					}

			} else { // executes once you let go of key
				if(bUpHold && countUpHoldJump < upJumpMax) {
						nCurrentY -= (DoesPieceFit(nCurrentX, nCurrentY - 1)) ? 1 : 0;
						countUpHoldJump++;
					} else if(bUpHold && countUpHoldJump == upJumpMax) {
						allTheWayUp = true;
						bUpHold = false;
						countUpHoldJump = 0;
					} else {
						bUpHold = false;
						countUpHoldJump = 0;
					}
			}
		} else { // all the way up, need to come down		
			if((DoesPieceFit(nCurrentX, nCurrentY + 1))) {
				nCurrentY += (DoesPieceFit(nCurrentX, nCurrentY + 1)) ? 1 : 0;
				countDownHoldJump--;
			} else {
				countDownHoldJump = downJumpMax;
				allTheWayUp = false;
				bUpHold = false;
				alreadyJumped = false;
			}
		}

		// Falling gravity
		if(!alreadyJumped)
			nCurrentY += (DoesPieceFit(nCurrentX, nCurrentY + 1)) ? 1 : 0;


		// RENDER OUTPUT ==================================================
		
		// Draw Field
		for (int x = 0; x < nFieldWidth; x++) {
			for (int y = 0; y < nFieldHeight; y++) {
				screen[(y + 2)*nScreenWidth + (x + 2)] = L" #"[pField[y*nFieldWidth + x]];
			}
		}
		 
		// Draw Current Piece
		for (int px = 0; px < 3; px++) {
			for (int py = 0; py < 3; py++) {
				if (player[(py*3) + px] != L'.') {
					screen[(nCurrentY + py + 2)*nScreenWidth + (nCurrentX + px + 2)] =  player[(py*3) + px];
				}
			}
		}

// DEBUG swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 2], 30, L"Position(x,y): %d, %d", nCurrentX, nCurrentY);

		swprintf_s(&screen[4 * nScreenWidth + nFieldWidth + 4], 30, L"<-- Finish");
		swprintf_s(&screen[8 * nScreenWidth + nFieldWidth + 4], 15, L"Time: %d", timeSeconds);
		
		WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0,0}, &dwBytesWritten);

		if(nCurrentX == winningX && nCurrentY == winningY) {
			bGameNotWon = false;
		}
		
		cycleCounter++;
		if(cycleCounter == secCycleCount) {
			timeSeconds += 1;
			cycleCounter = 0;
		}
	}
	CloseHandle(hConsole);
	if(!bGameNotWon) {
		cout << "You won the game!" << endl;
	} else {
		cout << "Game over!" << endl;
	}
	this_thread::sleep_for(chrono::seconds(5)); // Game Tick
	system("pause");
	return 0;
}