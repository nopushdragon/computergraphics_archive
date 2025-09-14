#include <iostream>
#include <Windows.h>

using namespace std;

#define MAX_X 40
#define MAX_Y 40

struct NEMO {
    int x1, y1, x2, y2;
};

int main() {
    char board[MAX_Y][MAX_X];
    int BOARD_X = 30;
    int BOARD_Y = 30;
    NEMO nemo1, nemo2;

    cout << "첫번째 네모 (x1,y1) 입력: ";
    cin >> nemo1.x1 >> nemo1.y1;
    cout << "첫번째 네모 (x2,y2) 입력: ";
    cin >> nemo1.x2 >> nemo1.y2;

    cout << "두번째 네모 (x1,y1) 입력: ";
    cin >> nemo2.x1 >> nemo2.y1;
    cout << "두번째 네모 (x2,y2) 입력: ";
    cin >> nemo2.x2 >> nemo2.y2;

    while (1) {
        system("cls");

        for (int i = 0; i < BOARD_Y; i++) {
            for (int j = 0; j < BOARD_X; j++) {
                board[i][j] = '.';
            }
        }

        for (int y = nemo1.y1; y <= nemo1.y2; y++) {
            for (int x = nemo1.x1; x <= nemo1.x2; x++) {
                int draw_x = (x % BOARD_X + BOARD_X) % BOARD_X;
                int draw_y = (y % BOARD_Y + BOARD_Y) % BOARD_Y;
                board[draw_y][draw_x] = '@';
            }
        }

        for (int y = nemo2.y1; y <= nemo2.y2; y++) {
            for (int x = nemo2.x1; x <= nemo2.x2; x++) {
                int draw_x = (x % BOARD_X + BOARD_X) % BOARD_X;
                int draw_y = (y % BOARD_Y + BOARD_Y) % BOARD_Y;

                if (board[draw_y][draw_x] == '@') {
                    board[draw_y][draw_x] = '%';
                }
                else {
                    board[draw_y][draw_x] = '#';
                }
            }
        }

        for (int i = 0; i < BOARD_Y; i++) {
            for (int j = 0; j < BOARD_X; j++) {
                if (board[i][j] == '%') {
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
                    cout << board[i][j];
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
                }
                else cout << board[i][j];
            }
            cout << endl;
        }

        char input;
        cout << "명령어 입력: ";
        cin >> input;

        switch (input) {
        case 'x':
            nemo1.x1++; 
            nemo1.x2++;
            break;
        case'1':
            nemo2.x1++; 
            nemo2.x2++;
            break;
        case 'X':
            nemo1.x1--; 
            nemo1.x2--;
            break;
        case '2':
            nemo2.x1--; 
            nemo2.x2--;
            break;
        case 'y':
            nemo1.y1++; 
            nemo1.y2++;
            break;
        case '3':
            nemo2.y1++; 
            nemo2.y2++;
            break;
        case 'Y':
            nemo1.y1--; 
            nemo1.y2--;
            break;
        case '4':
            nemo2.y1--; 
            nemo2.y2--;
            break;
        case 's':
            if(nemo1.x2 != BOARD_X-1)nemo1.x2++; 
            if(nemo1.y2 != BOARD_Y-1)nemo1.y2++;
            break;
        case'5':
            if(nemo2.x2 != BOARD_X-1)nemo2.x2++; 
            if(nemo2.y2 != BOARD_Y-1)nemo2.y2++;
            break;
        case 'S':
            if(nemo1.x2 != nemo1.x1+1 && nemo1.x2 != 0 )nemo1.x2--;
            if(nemo1.y2 != nemo1.y1+1 && nemo1.y2 != 0 )nemo1.y2--;
            break;
        case '6':
            if(nemo2.x2 != nemo2.x1+1 && nemo2.x2 != 0)nemo2.x2--;
            if(nemo2.y2 != nemo2.y1+1 && nemo2.y2 != 0)nemo2.y2--;
            break;
        case 'i':
            nemo1.x2++;
            break;
        case '7':
            nemo2.x2++;
            break;
        case 'I':
            nemo1.x2--;
            break;
        case '8':
            nemo2.x2--;
            break;
        case 'j':
            nemo1.y2++;
            break;
        case '9':
            nemo2.y2++;
            break;
        case 'J': 
            nemo1.y2--;
            break;
        case '0':
            nemo2.y2--;
            break;
        case 'a':
            nemo1.x2++;
            nemo1.y2--;
            break;
        case'-':
            nemo2.x2++;
            nemo2.y2--;
            break;
        case 'A':
            nemo1.x2--;
            nemo1.y2++;
            break;
        case '=':
            nemo2.x2--;
            nemo2.y2++;
            break;
        case 'b':
            int nemo1_w, nemo1_h;
            if (nemo1.x1 <= nemo1.x2) {
                nemo1_w = abs(nemo1.x2 - nemo1.x1) + 1;
            }
            else {
                nemo1_w = (BOARD_X - nemo1.x1) + (nemo1.x2 + 1);
            }
            if (nemo1.y1 <= nemo1.y2) {
                nemo1_h = abs(nemo1.y2 - nemo1.y1) + 1;
            }
            else {
                nemo1_h = (BOARD_Y - nemo1.y1) + (nemo1.y2 + 1);
            }

            int nemo2_w, nemo2_h;
            if (nemo2.x1 <= nemo2.x2) {
                nemo2_w = abs(nemo2.x2 - nemo2.x1) + 1;
            }
            else {
                nemo2_w = (BOARD_X - nemo2.x1) + (nemo2.x2 + 1);
            }
            if (nemo2.y1 <= nemo2.y2) {
                nemo2_h = abs(nemo2.y2 - nemo2.y1) + 1;
            }
            else {
                nemo2_h = (BOARD_Y - nemo2.y1) + (nemo2.y2 + 1);
            }

            cout << "첫번쨰 네모 넓이: " << nemo1_w * nemo1_h << endl;
            cout << "두번째 네모 넓이: " << nemo2_w * nemo2_h << endl;
            Sleep(2000);
            break;
        case 'c':
            if (BOARD_X < 40 || BOARD_Y < 40) {
                BOARD_X++;
                BOARD_Y++;
            }
            else {
                cout << "더 커질 수 읎따" << endl;
                Sleep(2000);
            }
            break;
        case 'd':
            if (BOARD_X > 20 || BOARD_Y > 20) {
                BOARD_X--;
                BOARD_Y--;
            }
            else {
                cout << "더 작아질 수 읎따" << endl;
                Sleep(2000);
            }
            break;
        case 'r':
            BOARD_X = 30;
            BOARD_Y = 30;
            system("cls");
            cout << "첫번째 네모 (x1,y1) 입력: ";
            cin >> nemo1.x1 >> nemo1.y1;
            cout << "첫번째 네모 (x2,y2) 입력: ";
            cin >> nemo1.x2 >> nemo1.y2;

            cout << "두번째 네모 (x1,y1) 입력: ";
            cin >> nemo2.x1 >> nemo2.y1;
            cout << "두번째 네모 (x2,y2) 입력: ";
            cin >> nemo2.x2 >> nemo2.y2;
            break;
        case 'q':
            exit(0);
            break;
        }
    }
}