#include <iostream>
#include <deque>
#include <cmath>

using namespace std;

struct SPOT {
	int x, y, z;
};

int main()
{
	deque<pair<SPOT, bool>> dq;
	deque<pair<SPOT, bool>> dq2;
	for (int i = 0; i < 10; i++) {
		dq2.push_back({{ 0,0,0 }, false});
	}
	bool isF = false;

	while (1) {
		if (!isF) {
			for (int i = 9;i >= 0;i--) {
				cout << i << ": ";
				if (dq.size() >= i + 1 && dq[i].second == true) cout << dq[i].first.x << ", " << dq[i].first.y << ", " << dq[i].first.z;
				cout << endl;
			}
		}
		else {
			for (int i = 9;i >= 0;i--) {
				cout << i << ": ";
				if (dq2.size() >= i + 1 && dq2[i].second == true) cout << dq2[i].first.x << ", " << dq2[i].first.y << ", " << dq2[i].first.z;
				cout << endl;
			}
		}
		char input_char;
		int n1, n2, n3;
		cout << "명령어를 입력하시오: ";
		cin >> input_char;
		if(input_char == '+' || input_char == 'e') cin >> n1 >> n2 >> n3;

		switch (input_char) {
		case '+': {
			bool isvoid = false;
			for (int i = 0; i < dq.size();i++) {
				if (dq[i].second == false) {
					dq[i] = { {n1,n2,n3},true };
					isvoid = true;
					break;
				}
			}
			if(!isvoid) dq.push_back({ {n1,n2,n3},true });
			break;
		}
		case '-':
			dq.pop_back();
			break;
		case 'e':
			dq.push_front({ {n1,n2,n3},true });
			break;
		case 'd':
			for (int i = 0; i < dq.size();i++) {
				if (dq[i].second == true) {
					dq[i].second = false;
					break;
				}
			}
			break;
		case 'a': {
			int list_cnt = 0;
			for (int i = 0; i < dq.size();i++) {
				if (dq[i].second == true) {
					list_cnt++;
				}
			}
			cout << "리스트의 수: " << list_cnt << endl;
			break;
			}
		case 'b': {
			/*pair<SPOT, bool> temp;

			if (dq.size() < 10) {
				for (int i = dq.size();i < 10;i++) {
					dq.push_back({ { 0,0,0 }, false });
				}
			}

			temp = dq[0];
			
			for (int i = 0; i < 9;i++) {
				dq[i] = dq[i + 1];
			}
			dq[9] = temp;*/

			for (int i = 0; i < dq.size();i++) {
				dq[i].first.x--;
				dq[i].first.y--;
				dq[i].first.z--;
				if (dq[i].first.x < 0) dq[i].first.x = 9;
				if (dq[i].first.y < 0) dq[i].first.y = 9;
				if (dq[i].first.z < 0) dq[i].first.z = 9;
			}

			break;
			}
		case 'c':
			for (int i = 0; i < dq.size();i++) {
				dq[i].second = false;
			}
			break;
		case 'f':

			isF = !isF;

			if (isF) {
				dq2.clear();

				deque<pair<SPOT, bool>> temp;
				for (int i = 0; i < dq.size(); i++) {
					if (dq[i].second == true) temp.push_back(dq[i]);
				}

				for (int i = 0; i < temp.size(); i++) {
					for (int j = 1; j < temp.size(); j++) {
						int dist1 = temp[j].first.x * temp[j].first.x +
							temp[j].first.y * temp[j].first.y +
							temp[j].first.z * temp[j].first.z;
						int dist2 = temp[j - 1].first.x * temp[j - 1].first.x +
							temp[j - 1].first.y * temp[j - 1].first.y +
							temp[j - 1].first.z * temp[j - 1].first.z;
						if (dist1 < dist2) {
							swap(temp[j], temp[j - 1]);
						}
					}
				}

				for (int i = 0; i < temp.size(); i++) {
					dq2.push_back(temp[i]);
				}

			}
			break;
		case 'q':
			exit(0);
			break;
		}
	}

}