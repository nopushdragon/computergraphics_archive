#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int main() {
    fstream file("data.txt");
    string s;

    vector<string> str , tempstr;
    while (getline(file,s)){
        str.push_back(s);
    }

    int wordcnt[10];
    bool isvoid = false;
    for (int i = 0; i < str.size();i++) {
        for (int j = 0; j < str[i].size();j++) {
            if (str[i][j] != ' ' && isvoid == false) {
                wordcnt[i]++;
                isvoid = true;
            }
            else if(str[i][j] == ' '){
                isvoid = false;
            }
        }
    }

    bool isA,isB,isC,isD,isE,isF,isG,isH,isI,isJ = false;
    while (1) {
        copy(str.begin(), str.end(), back_inserter(tempstr));

        if (isA) {
            for (int i = 0; i < tempstr.size();i++) {
                for (int j = 0;j < tempstr[i].size();j++) {
                    if (tempstr[i][j] >= 'a' && tempstr[i][j] <= 'z') {
                        tempstr[i][j] -= 32;
                    }
                }
            }
        }

        for (int i = 0; i < tempstr.size();i++) {
            cout << tempstr[i] << endl;
        }
        
        char input;
        cout << "명령을 입력하세요: ";
        cin >> input;

        switch (input) {
        case 'a':
            isA = !isA;
            break;
        }
        system("cls");
    }

}
