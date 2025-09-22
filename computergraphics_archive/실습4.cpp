#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <vector>
#include <random>
#include <deque>

std::deque<std::pair<float, float>> path;  // 머리 이동 기록 저장
int delay = 20;  // 블록마다 지연 프레임 수

#define SIZEW 800
#define SIZEH 800
#define TimerSpeed 60

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<float> randcolor(0.0f, 1.0f);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);

void init(bool b);

struct NEMO {
	float firstx, firsty;
	float x, y;
	float size;
	float r, g, b;
	int b1 = 0; //0좌상 1좌하 2우상 2우하
	int b2 = 0; //0좌상 1좌하 2우상 2우하
	int anime = 5;
	bool b3 = false;
};
std::vector <NEMO> nemo;
bool is1 = false, is2 = false, is3 = false, is4 = false, is5 = false;
bool followMode = false;

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(100, 100); // 윈도우의 위치 지정
	glutInitWindowSize(SIZEW, SIZEH); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) { // glew 초기화 
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(1000 / TimerSpeed, Timer, 1);
	glutMouseFunc(Mouse);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() { //--- 콜백 함수: 출력 콜백 함수 {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f); 
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기

	for (int i = 0; i < nemo.size();i++) {
		glColor3f(nemo[i].r, nemo[i].g, nemo[i].b);
		glRectf(nemo[i].x - nemo[i].size, nemo[i].y + nemo[i].size, nemo[i].x + nemo[i].size, nemo[i].y - nemo[i].size);
	}

	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) { //--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {

	switch (key) {
	case '1':
		init(is1);
		is1 = !is1;
		break;
	case '2':
		init(is2);
		is2 = !is2;
		break;
	case '3':
		init(is3);
		is3 = !is3;
		followMode = !followMode;
		break;
	case '4':
		init(is4);
		is4 = !is4;
		followMode = !followMode;
		break;
	case'5':
		/*init(is5);
		is5 = !is5;*/
		followMode = !followMode;
		break;
	case's':
		init(NULL);
		break;
	case'm':
		for (int i = 0; i < nemo.size();i++) {
			nemo[i].x = nemo[i].firstx;
			nemo[i].y = nemo[i].firsty;
		}
		break;
	case 'r':
		init(NULL);
		while (!nemo.empty()) {
			nemo.pop_back();
		}
		break;
	case 'q':
		exit(0);
		break;
	}
}

GLvoid Mouse(int button, int state, int x, int y) {
	float mx = (float)x / (SIZEW / 2) - 1.0f;
	float my = (SIZEH - (float)y) / (SIZEH / 2) - 1.0f;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		nemo.push_back({ mx,my, mx,my,0.1f,randcolor(mt),randcolor(mt) ,randcolor(mt) });
	}
}

GLvoid Timer(int value) {
	while (nemo.size() > 5) {
		nemo.pop_back();
	}

	if (followMode) {
		if (!nemo.empty()) {
			path.push_front({ nemo[0].x, nemo[0].y });
			while (path.size() > (nemo.size() - 1) * delay + 1) {
				path.pop_back();
			}
		}
	}

	if (!followMode) {
		if (is1) {
			for (int i = 0; i < nemo.size(); i++) {
				if (nemo[i].b1 == 0) {
					nemo[i].x -= 0.01f;
					nemo[i].y += 0.01f;
					if (nemo[i].x - nemo[i].size <= -1.0f) nemo[i].b1 = 2;
					if (nemo[i].y + nemo[i].size >= 1.0f) nemo[i].b1 = 1;
				}
				if (nemo[i].b1 == 1) {
					nemo[i].x -= 0.01f;
					nemo[i].y -= 0.01f;
					if (nemo[i].x - nemo[i].size <= -1.0f) nemo[i].b1 = 3;
					if (nemo[i].y - nemo[i].size <= -1.0f) nemo[i].b1 = 0;
				}
				if (nemo[i].b1 == 2) {
					nemo[i].x += 0.01f;
					nemo[i].y += 0.01f;
					if (nemo[i].x + nemo[i].size >= 1.0f) nemo[i].b1 = 0;
					if (nemo[i].y + nemo[i].size >= 1.0f) nemo[i].b1 = 3;
				}
				if (nemo[i].b1 == 3) {
					nemo[i].x += 0.01f;
					nemo[i].y -= 0.01f;
					if (nemo[i].x + nemo[i].size >= 1.0f) nemo[i].b1 = 1;
					if (nemo[i].y - nemo[i].size <= -1.0f) nemo[i].b1 = 2;
				}
			}
		}
		if (is2) {
			for (int i = 0; i < nemo.size(); i++) {
				if (nemo[i].b2 == 0) {
					if (nemo[i].anime == 5)nemo[i].x -= 0.01f;
					if (nemo[i].x - nemo[i].size <= -1.0f) {
						if (nemo[i].anime == 5)nemo[i].anime = 0;
						if (nemo[i].anime < 5) {
							nemo[i].y += (nemo[i].size * 2) / 5;
							nemo[i].anime++;
							if (nemo[i].y + nemo[i].size >= 1.0f) {
								nemo[i].y = 1.0f - nemo[i].size;
								nemo[i].b2 = 3;
								nemo[i].anime = 5;
								continue;
							}
							if (nemo[i].anime != 5)continue;
						}
						nemo[i].b2 = 2;
					}
				}
				if (nemo[i].b2 == 1) {
					if (nemo[i].anime == 5)nemo[i].x -= 0.01f;
					if (nemo[i].x - nemo[i].size <= -1.0f) {
						if (nemo[i].anime == 5)nemo[i].anime = 0;
						if (nemo[i].anime < 5) {
							nemo[i].y -= (nemo[i].size * 2) / 5;
							nemo[i].anime++;
							if (nemo[i].y - nemo[i].size <= -1.0f) {
								nemo[i].y = -1.0f + nemo[i].size;
								nemo[i].b2 = 2;
								nemo[i].anime = 5;
								continue;
							}
							if (nemo[i].anime != 5)continue;
						}
						nemo[i].b2 = 3;
					}
				}
				if (nemo[i].b2 == 2) {
					if (nemo[i].anime == 5)nemo[i].x += 0.01f;
					if (nemo[i].x + nemo[i].size >= 1.0f) {
						if (nemo[i].anime == 5)nemo[i].anime = 0;
						if (nemo[i].anime < 5) {
							nemo[i].y += (nemo[i].size * 2) / 5;
							nemo[i].anime++;
							if (nemo[i].y + nemo[i].size >= 1.0f) {
								nemo[i].y = 1.0f - nemo[i].size;
								nemo[i].b2 = 1;
								nemo[i].anime = 5;
								continue;
							}
							if (nemo[i].anime != 5)continue;
						}
						nemo[i].b2 = 0;
					}
				}
				if (nemo[i].b2 == 3) {
					if (nemo[i].anime == 5)nemo[i].x += 0.01f;
					if (nemo[i].x + nemo[i].size >= 1.0f) {
						if (nemo[i].anime == 5)nemo[i].anime = 0;
						if (nemo[i].anime < 5) {
							nemo[i].y -= (nemo[i].size * 2) / 5;
							nemo[i].anime++;
							if (nemo[i].y - nemo[i].size <= -1.0f) {
								nemo[i].y = -1.0f + nemo[i].size;
								nemo[i].b2 = 0;
								nemo[i].anime = 5;
								continue;
							}
							if (nemo[i].anime != 5)continue;
						}
						nemo[i].b2 = 1;
					}
				}
			}
		}
		if (is3) {
			for (int i = 0; i < nemo.size(); i++) {
				std::uniform_real_distribution<float> rdsize(0.005f, 0.02f);
				float addsize = rdsize(mt);

				if (nemo[i].b3 == false) {
					nemo[i].size += addsize;
					if (nemo[i].size >= 0.2f) nemo[i].b3 = true;
				}
				else {
					nemo[i].size -= addsize;
					if (nemo[i].size <= 0.01f) nemo[i].b3 = false;
				}
			}
		}
		if (is4) {
			for (int i = 0; i < nemo.size(); i++) {
				nemo[i].r = randcolor(mt);
				nemo[i].g = randcolor(mt);
				nemo[i].b = randcolor(mt);
			}
		}
	}
	else {
		if (is1) {
			if (nemo[0].b1 == 0) {
				nemo[0].x -= 0.01f;
				nemo[0].y += 0.01f;
				if (nemo[0].x - nemo[0].size <= -1.0f) nemo[0].b1 = 2;
				if (nemo[0].y + nemo[0].size >= 1.0f) nemo[0].b1 = 1;
			}
			if (nemo[0].b1 == 1) {
				nemo[0].x -= 0.01f;
				nemo[0].y -= 0.01f;
				if (nemo[0].x - nemo[0].size <= -1.0f) nemo[0].b1 = 3;
				if (nemo[0].y - nemo[0].size <= -1.0f) nemo[0].b1 = 0;
			}
			if (nemo[0].b1 == 2) {
				nemo[0].x += 0.01f;
				nemo[0].y += 0.01f;
				if (nemo[0].x + nemo[0].size >= 1.0f) nemo[0].b1 = 0;
				if (nemo[0].y + nemo[0].size >= 1.0f) nemo[0].b1 = 3;
			}
			if (nemo[0].b1 == 3) {
				nemo[0].x += 0.01f;
				nemo[0].y -= 0.01f;
				if (nemo[0].x + nemo[0].size >= 1.0f) nemo[0].b1 = 1;
				if (nemo[0].y - nemo[0].size <= -1.0f) nemo[0].b1 = 2;
			}
			for (int i = 1; i < nemo.size(); i++) {
				int way = i * delay;
				if (way < path.size()) {
					nemo[i].x = path[way].first;
					nemo[i].y = path[way].second;
				}
			}
		}
		if (is2) {
			if (nemo[0].b2 == 0) {
				if (nemo[0].anime == 5) nemo[0].x -= 0.01f;
				if (nemo[0].x - nemo[0].size <= -1.0f) {
					if (nemo[0].anime == 5) nemo[0].anime = 0;
					if (nemo[0].anime < 5) {
						nemo[0].y += (nemo[0].size * 2) / 5;
						nemo[0].anime++;
						if (nemo[0].y + nemo[0].size >= 1.0f) {
							nemo[0].y = 1.0f - nemo[0].size;
							nemo[0].b2 = 3;
							nemo[0].anime = 5;
						}
						else if (nemo[0].anime == 5) nemo[0].b2 = 2;
					}
				}
			}
			else if (nemo[0].b2 == 1) {
				if (nemo[0].anime == 5) nemo[0].x -= 0.01f;
				if (nemo[0].x - nemo[0].size <= -1.0f) {
					if (nemo[0].anime == 5) nemo[0].anime = 0;
					if (nemo[0].anime < 5) {
						nemo[0].y -= (nemo[0].size * 2) / 5;
						nemo[0].anime++;
						if (nemo[0].y - nemo[0].size <= -1.0f) {
							nemo[0].y = -1.0f + nemo[0].size;
							nemo[0].b2 = 2;
							nemo[0].anime = 5;
						}
						else if (nemo[0].anime == 5) nemo[0].b2 = 3;
					}
				}
			}
			else if (nemo[0].b2 == 2) {
				if (nemo[0].anime == 5) nemo[0].x += 0.01f;
				if (nemo[0].x + nemo[0].size >= 1.0f) {
					if (nemo[0].anime == 5) nemo[0].anime = 0;
					if (nemo[0].anime < 5) {
						nemo[0].y += (nemo[0].size * 2) / 5;
						nemo[0].anime++;
						if (nemo[0].y + nemo[0].size >= 1.0f) {
							nemo[0].y = 1.0f - nemo[0].size;
							nemo[0].b2 = 1;
							nemo[0].anime = 5;
						}
						else if (nemo[0].anime == 5) nemo[0].b2 = 0;
					}
				}
			}
			else if (nemo[0].b2 == 3) {
				if (nemo[0].anime == 5) nemo[0].x += 0.01f;
				if (nemo[0].x + nemo[0].size >= 1.0f) {
					if (nemo[0].anime == 5) nemo[0].anime = 0;
					if (nemo[0].anime < 5) {
						nemo[0].y -= (nemo[0].size * 2) / 5;
						nemo[0].anime++;
						if (nemo[0].y - nemo[0].size <= -1.0f) {
							nemo[0].y = -1.0f + nemo[0].size;
							nemo[0].b2 = 0;
							nemo[0].anime = 5;
						}
						else if (nemo[0].anime == 5) nemo[0].b2 = 1;
					}
				}
			}
			for (int i = 1; i < nemo.size(); i++) {
				int way = i * delay;
				if (way < path.size()) {
					nemo[i].x = path[way].first;
					nemo[i].y = path[way].second;
				}
			}
		}
		if (is3) {
			for (int i = 0; i < nemo.size(); i++) {
				std::uniform_real_distribution<float> rdsize(0.005f, 0.02f);
				float addsize = rdsize(mt);

				if (nemo[i].b3 == false) {
					nemo[i].size += addsize;
					if (nemo[i].size >= 0.2f) nemo[i].b3 = true;
				}
				else {
					nemo[i].size -= addsize;
					if (nemo[i].size <= 0.01f) nemo[i].b3 = false;
				}
			}

		}
		if (is4) {
			for (int i = 0; i < nemo.size(); i++) {
				nemo[i].r = randcolor(mt);
				nemo[i].g = randcolor(mt);
				nemo[i].b = randcolor(mt);
			}
		}
	}

	glutTimerFunc(1000 / TimerSpeed, Timer, 1); //--- 타이머 콜백함수 지정 (60 FPS)
	glutPostRedisplay();
}

void init(bool b) {
	if (b != is1) is1 = false;
	if (b != is2) is2 = false;
	if (b != is3) is3 = false;
	if (b != is4) is4 = false;
	if (b != is5) is5 = false;
}