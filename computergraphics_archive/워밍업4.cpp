#include <iostream>
#include <random>
#include <Windows.h>

using namespace std;

random_device rd;
mt19937 mt(rd());
uniform_int_distribution<int> rdchar('a', 'm');

int main()
{
	char board[5][5];
	for (int i = 0; i < 5;i++) {
		for (int j = 0; j < 5;j++) {
			board[i][j] = '*';
		}
	}

	char answer[5][5];
	int alphacnt[13] = { 0 };

	for (int i = 0; i < 5;i++) {
		for (int j = 0; j < 5;j++) {
			char rdc = rdchar(rd);
			if (rdc != 'm' && alphacnt[rdc - 'a'] < 2) {
				answer[i][j] = rdc;
				alphacnt[rdc - 'a']++;
			}
			else if (rdc == 'm' && alphacnt[rdc - 'a'] < 1) {
				answer[i][j] = 'z';
				alphacnt[rdc - 'a']++;
			}
			else {
				j--;
			}
		}
	}

	while (1) {
		cout << "  a b c d e" << endl;
		for (int i = 0; i < 5; i++) {
			cout << i + 1 << " ";
			for (int j = 0; j < 5;j++) {
				if (board[i][j] == '*') {
					cout << board[i][j] << " ";
				}
				else {
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), board[i][j] - 'A' + 1);
					cout << board[i][j] << " ";
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
				}
			}
			cout << endl;
		}

		char cmd;
		cout << "명령을 원하는가 yes-> r/h/q, no-> a \n대답: ";
		cin >> cmd;
		if (cmd == 'a') {
			char a1, a2;
			int n1, n2;
			cout << "첫번쨰 좌표 입력 ex) a 1: ";
			cin >> a1 >> n1;
			cout << "두번쨰 좌표 입력 ex) a 1: ";
			cin >> a2 >> n2;

			int x1 = a1 - 'a';
			int x2 = a2 - 'a';
			n1--;
			n2--;

			if (a1 < 'a' || a1 > 'e' || a2 < 'a' || a2 > 'e' || n1 < 0 || n1 > 5 || n2 < 0 || n2 > 5) {
				cout << "거긴 보드 밖이지렁~~~";
				Sleep(2000);
				system("cls");
				continue;
			}
			else if (board[n1][x1] != '*' || board[n2][x2] != '*') {
				cout << "이미 선택했던 곳이지렁~~~~";
				Sleep(2000);
				system("cls");
				continue;
			}

			system("cls");
			cout << "  a b c d e" << endl;
			for (int i = 0; i < 5;i++) {
				cout << i + 1 << " ";
				for (int j = 0; j < 5;j++) {
					if ((x1 == j && n1 == i) || (x2 == j && n2 == i)) {
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), answer[i][j] - 'a' + 1);
						cout << answer[i][j] << " ";
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					}
					else {
						if (board[i][j] == '*') cout << board[i][j] << " ";
						else {
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), board[i][j] - 'A' + 1);
							cout << board[i][j] << " ";
							SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
						}
					}
				}
				cout << endl;
			}
			Sleep(2000);
			if (answer[n1][x1] == answer[n2][x2]) {
				char pair = answer[n1][x1];
				pair -= 32;
				board[n1][x1] = pair;
				board[n2][x2] = pair;
			}
			if (answer[n1][x1] == 'z') {
				char pair = answer[n2][x2];
				pair -= 32;
				for (int i = 0; i < 5;i++) {
					for (int j = 0; j < 5;j++) {
						if (answer[i][j] == answer[n2][x2]) {
							board[i][j] = pair;
						}
					}
				}
				board[n1][x1] = 'Z';
			}
			if (answer[n2][x2] == 'z') {
				char pair = answer[n1][x1];
				pair -= 32;
				for (int i = 0; i < 5;i++) {
					for (int j = 0; j < 5;j++) {
						if (answer[i][j] == answer[n1][x1]) {
							board[i][j] = pair;
						}
					}
				}
				board[n2][x2] = 'Z';
			}
		}
		else if (cmd == 'r') {
			for (int i = 0; i < 5;i++) {
				for (int j = 0; j < 5;j++) {
					board[i][j] = '*';
				}
			}

			for (int i = 0; i < 13;i++) {
				alphacnt[i] = 0;
			}

			for (int i = 0; i < 5;i++) {
				for (int j = 0; j < 5;j++) {
					char rdc = rdchar(rd);
					if (rdc != 'm' && alphacnt[rdc - 'a'] < 2) {
						answer[i][j] = rdc;
						alphacnt[rdc - 'a']++;
					}
					else if (rdc == 'm' && alphacnt[rdc - 'a'] < 1) {
						answer[i][j] = 'z';
						alphacnt[rdc - 'a']++;
					}
					else {
						j--;
					}
				}
			}
		}
		else if (cmd == 'h') {
			cout << "  a b c d e" << endl;
			for (int i = 0; i < 5; i++) {
				cout << i + 1 << " ";
				for (int j = 0; j < 5;j++) {
					if (board[i][j] == '*') {
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), answer[i][j] - 'a' + 1);
						cout << answer[i][j] << " ";
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					}
					else {
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), board[i][j] - 'A' + 1);
						cout << board[i][j] << " ";
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
					}
				}
				cout << endl;
			}
			Sleep(2000);
		}
		else if (cmd == 'q') {
			exit(0);
		}
		else {
			cout << "그것은 명령이 아니다잉";
			Sleep(2000);
			system("cls");
			continue;
		}

		system("cls");
	}
}