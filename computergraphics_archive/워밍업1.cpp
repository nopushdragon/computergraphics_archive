#include <iostream>
#include <fstream>
#include <random>
#include <Windows.h>

using namespace std;

int MIN1 = 9999;
int MIN2 = 9999;
int MAX1 = -9999;
int MAX2 = -9999;

int main()
{
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<int> rdnum(1, 9);

	int arr1[4][4];
	int arr2[4][4];

	for (int i = 0; i < 4;i++) {
		for (int j = 0; j < 4;j++) {
			arr1[i][j] = rdnum(rd);
			arr2[i][j] = rdnum(rd);
		}
	}

	bool isE = false;
	bool isF = false;
	while (1) {
		cout << "=================================================" << endl;
		for (int i = 0;i < 4;i++) {
			for (int j = 0; j < 4;j++)cout << arr1[i][j] << " ";
			cout << "	";
			for (int j = 0; j < 4;j++)cout << arr2[i][j] << " ";
			cout << endl;
		}


		char input;

		cout << "ÀÔ·Â: ";
		cin >> input;

		if (input >= 'a' && input <= 'z') {
			int answer[4][4];

			switch (input) {
			case 'm':
				for (int i = 0; i < 4;i++) {
					for (int j = 0; j < 4;j++) {
						int num = 0;
						for (int t = 0; t < 4;t++) {
							num += arr1[i][t] * arr2[t][j];
						}
						answer[i][j] = num;
					}
				}

				cout << "°á°ú:" << endl;
				for (int i = 0; i < 4;i++) {
					for (int j = 0;j < 4;j++) {
						cout << answer[i][j] << "  ";
					}
					cout << endl;
				}

				break;
			case 'a':

				for (int i = 0; i < 4;i++) {
					for (int j = 0; j < 4;j++) {
						int num;
						answer[i][j] = arr1[i][j] + arr2[i][j];
					}
				}

				cout << "°á°ú:" << endl;
				for (int i = 0; i < 4;i++) {
					for (int j = 0;j < 4;j++) {
						cout << answer[i][j] << "  ";
					}
					cout << endl;
				}

				break;
			case 'd':

				for (int i = 0; i < 4;i++) {
					for (int j = 0; j < 4;j++) {
						int num;
						answer[i][j] = arr1[i][j] - arr2[i][j];
					}
				}

				cout << "°á°ú:" << endl;
				for (int i = 0; i < 4;i++) {
					for (int j = 0;j < 4;j++) {
						cout << answer[i][j] << "  ";
					}
					cout << endl;
				}

				break;
			case 'r': {
				int det1 = 0, det2 = 0;
				int m[3][3];

				for (int k = 0; k < 4; k++) {
					int mi = 0, mj = 0;
					for (int i = 1; i < 4; i++) {
						mj = 0;
						for (int j = 0; j < 4; j++) {
							if (j == k) continue;
							m[mi][mj] = arr1[i][j];
							mj++;
						}
						mi++;
					}

					int det3 = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
					
					int plusminus;
					if (k % 2 == 0) plusminus = 1;
					else plusminus = -1;
					det1 += plusminus * arr1[0][k] * det3;
				}

				for (int k = 0; k < 4; k++) {
					int mi = 0, mj = 0;
					for (int i = 1; i < 4; i++) {
						mj = 0;
						for (int j = 0; j < 4; j++) {
							if (j == k) continue;
							m[mi][mj] = arr2[i][j];
							mj++;
						}
						mi++;
					}

					int det3 = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

					int plusminus;
					if (k % 2 == 0) plusminus = 1;
					else plusminus = -1;
					det2 += plusminus * arr2[0][k] * det3;
				}

				cout << "arr1 = " << det1 << endl;
				cout << "arr2 = " << det2 << endl;
				break;
			}
			case 't': {
				int jeonchi1[4][4], jeonchi2[4][4];

				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < 4; j++) {
						jeonchi1[i][j] = arr1[j][i];
						jeonchi2[i][j] = arr2[j][i];
					}
				}

				for (int i = 0; i < 4; i++) {
					for (int j = 0; j < 4; j++) cout << jeonchi1[i][j] << " ";
					cout << "	";
					for (int j = 0; j < 4; j++) cout << jeonchi2[i][j] << " ";
					cout << endl;
				}

				int det1 = 0, det2 = 0;
				int m[3][3];

				for (int k = 0; k < 4; k++) {
					int mi = 0, mj = 0;
					for (int i = 1; i < 4; i++) {
						mj = 0;
						for (int j = 0; j < 4; j++) {
							if (j == k) continue;
							m[mi][mj] = jeonchi1[i][j];
							mj++;
						}
						mi++;
					}

					int det3 = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

					int plusminus;
					if (k % 2 == 0) plusminus = 1;
					else plusminus = -1;
					det1 += plusminus * jeonchi1[0][k] * det3;
				}

				for (int k = 0; k < 4; k++) {
					int mi = 0, mj = 0;
					for (int i = 1; i < 4; i++) {
						mj = 0;
						for (int j = 0; j < 4; j++) {
							if (j == k) continue;
							m[mi][mj] = jeonchi2[i][j];
							mj++;
						}
						mi++;
					}

					int det3 = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

					int plusminus;
					if (k % 2 == 0) plusminus = 1;
					else plusminus = -1;
					det2 += plusminus * jeonchi2[0][k] * det3;
				}

				cout << "arr1 = " << det1 << endl;
				cout << "arr2 = " << det2 << endl;

				break;
			}
			case 'e':
				if (isE == false) {
					isE = true;

					for (int i = 0; i < 4;i++) {
						for (int j = 0; j < 4;j++) {
							if (MIN1 > arr1[i][j]) MIN1 = arr1[i][j];
							if (MIN2 > arr2[i][j]) MIN2 = arr2[i][j];
						}
					}

					for (int i = 0; i < 4;i++) {
						for (int j = 0; j < 4;j++) {
							arr1[i][j] -= MIN1;
							arr2[i][j] -= MIN2;
						}
					}
				}
				else {
					isE = false;
					for (int i = 0; i < 4;i++) {
						for (int j = 0; j < 4;j++) {
							arr1[i][j] += MIN1;
							arr2[i][j] += MIN2;
						}
					}
					MIN1 = 9999;
					MIN2 = 9999;
				}

				break;
			case 'f':
				if (isF == false) {
					isF = true;

					for (int i = 0; i < 4;i++) {
						for (int j = 0; j < 4;j++) {
							if (MAX1 < arr1[i][j]) MAX1 = arr1[i][j];
							if (MAX2 < arr2[i][j]) MAX2 = arr2[i][j];
						}
					}

					for (int i = 0; i < 4;i++) {
						for (int j = 0; j < 4;j++) {
							arr1[i][j] += MAX1;
							arr2[i][j] += MAX2;
						}
					}
				}
				else {
					isF = false;
					for (int i = 0; i < 4;i++) {
						for (int j = 0; j < 4;j++) {
							arr1[i][j] -= MAX1;
							arr2[i][j] -= MAX2;
						}
					}
					MAX1 = -9999;
					MAX2 = -9999;
				}
				break;
			case 's':
				for (int i = 0; i < 4;i++) {
					for (int j = 0; j < 4;j++) {
						arr1[i][j] = rdnum(rd);
						arr2[i][j] = rdnum(rd);
					}
				}
				break;
			case 'q':
				return 0;
				break;
			}
		}
		else if (input >= '1' && input <= '9') {
			int input_num = input - '0';

			for (int i = 0; i < 4;i++) {
				for (int j = 0; j < 4;j++) {
					if (arr1[i][j] % input_num == 0)cout << arr1[i][j] << " ";
					else cout << "»×" << " ";
				}
				cout << "	";
				for (int j = 0; j < 4;j++) {
					if (arr2[i][j] % input_num == 0)cout << arr2[i][j] << " ";
					else cout << "»×" << " ";
				}
				cout << endl;
			}
		}
	}
}