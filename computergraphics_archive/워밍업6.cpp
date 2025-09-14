#include <iostream>
#include <Windows.h>
#include <random>

using namespace std;

#define BOARD_X 30
#define BOARD_Y 30

struct PLAYER {
	int x = 0;
	int y = 0;
}player;

int main()
{
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<int> boxrd(0, 29);

	int board[BOARD_Y][BOARD_X];
	for (int i = 0; i < BOARD_Y;i++) {
		for (int j = 0; j < BOARD_X;j++) {
			board[i][j] = 0;
		}
	}

	int box[30];
	for (int i = 0;i < BOARD_Y;i++) {
		box[i] = boxrd(rd);
	}

	bool step = true;
	while (1) {
		if(step)board[player.y][player.x]++;
		step = true;

		for (int i = 0; i < BOARD_Y;i++) {
			for (int j = 0; j < BOARD_X;j++) {
				if (i == player.y && j == player.x) {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 9);
					cout << "*" << " ";
				}
				else if (j == box[i]) {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
					cout << 'X' << " ";
				}
				else if (board[i][j] != 0) {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
					cout << board[i][j] << " ";
				}
				else {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					cout << board[i][j] << " ";
				}
			}
			cout << endl;
		}
		
		char input;
		cout << "ют╥б: ";
		cin >> input;

		switch (input) {
		case 'w':
			if (player.y > 0)player.y--;
			else step = false;
			if (box[player.y] == player.x) {
				player.y++;
				step = false;
			}
			break;
		case 'a':
			if (player.x > 0)player.x--;
			else step = false;
			if (box[player.y] == player.x) {
				player.x++;
				step = false;
			}
			break;
		case's':
			if (player.y < 29)player.y++;
			else step = false;
			if (box[player.y] == player.x) {
				player.y--;
				step = false;
			}
			break;
		case'd':
			if (player.x < 29)player.x++;
			else step = false;
			if (box[player.y] == player.x) {
				player.x--;
				step = false;
			}
			break;
		case'r':
			for (int i = 0;i < BOARD_Y;i++) {
				box[i] = boxrd(rd);
			}
			for (int i = 0; i < BOARD_Y;i++) {
				for (int j = 0; j < BOARD_X;j++) {
					board[i][j] = 0;
				}
			}
			player.y = 0;
			player.x = 0;
			break;
		}


		system("cls");
	}

}