#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <thread>
#include <iostream>
using namespace std;

//Hằng số
#define MAX_CAR 17
#define MAX_CAR_LENGTH 5
#define MAX_SPEED 3
#define MAX_MAN WIDTH_CONSOLE - 1 //biến chứa tối đa vị trí những người đến đích = chiều rộng của màn hình - 1

//Biến toàn cục
POINT** X; //Mảng chứa MAX_CAR xe
POINT Y; // Đại diện người qua đường
int cnt = 0;//Biến hỗ trợ trong quá trình tăng tốc độ xe di chuyển
int MOVING;//Biến xác định hướng di chuyển của người
int SPEED;// Tốc độ xe chạy (xem như level)
int HEIGH_CONSOLE = 20, WIDTH_CONSOLE = 70;// Độ dài và độ rộng của màn hình console
bool STATE; // Trạng thái sống/chết của người qua đường
int* FinishLine;	//mảng chứa vị trí ở đích mà người có thể ở đó hoặc không

void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void TaoMangFinish(int* &FinishLine)
{
	//Tạo mảng chứa vị trí ở đích mà xe có thể ở đó hoặc không
	if (FinishLine == NULL)
	{
		FinishLine = new int[MAX_MAN];
		for (int i = 0; i < MAX_MAN; i++)	//khi nhân vật chưa đến đích thì vị trí ở đích có giá trị bằng 0, nhân vật đến đích thì giá trị sẽ bằng 1
		{
			FinishLine[i] = 0;
		}
	}
}

//Hàm khởi tạo dữ liệu mặc định ban đầu
void ResetData() {
	MOVING = 'D'; // Ban đầu cho người di chuyển sang phải
	SPEED = 1; // Tốc độ lúc đầu
	Y = { 18,19 }; // Vị trí lúc đầu của người
				   // Tạo mảng xe chạy
	if (X == NULL) {
		X = new POINT*[MAX_CAR];
		for (int i = 0; i < MAX_CAR; i++)
			X[i] = new POINT[MAX_CAR_LENGTH];
		for (int i = 0; i < MAX_CAR; i++)
		{
			int temp1 = (rand() % (WIDTH_CONSOLE - MAX_CAR_LENGTH)) + 1;
			for (int j = 0; j < MAX_CAR_LENGTH; j++)
			{
				X[i][j].x = temp1 + j;
				X[i][j].y = 2 + i;
			}
		}
	}
	//Tạo mảng chứa vị trí ở đích mà xe có thể ở đó hoặc không
	TaoMangFinish(FinishLine);
}

void DrawBoard(int x, int y, int width, int height, int curPosX = 0, int curPosY = 0)
{
	GotoXY(x + 1, y);
	for (int i = 1; i < width; i++) cout << '_';
	GotoXY(x, height + y); cout << '|';
	for (int i = 1; i < width; i++) cout << '_';
	cout << '|';
	for (int i = y + 1; i < height + y; i++)
	{
		GotoXY(x, i); cout << '|';
		GotoXY(x + width, i); cout << '|';
	}
	GotoXY(curPosX, curPosY);

}

void DrawMenu()
{
	GotoXY(0, HEIGH_CONSOLE + 1);
	cout << "Huong dan tuy chon:";
	GotoXY(0, HEIGH_CONSOLE + 2);
	cout << "-Nhan nut P: dung tam thoi game";
	GotoXY(0, HEIGH_CONSOLE + 3);
	cout << "-Nhan nut L: nhap ten tap tin ma ban muon save game";
	GotoXY(0, HEIGH_CONSOLE + 4);
	cout << "-Nhan nut T: nhap ten tap tin ma ban muon load game";
}

void StartGame() {
	system("cls");
	ResetData(); // Khởi tạo dữ liệu gốc
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE); // Vẽ màn hình game
	DrawMenu();	//vẽ Menu lựa chọn
	STATE = true;//Bắt đầu cho Thread chạy
}

//Hàm dọn dẹp tài nguyên
void GabageCollect()
{
	for (int i = 0; i < MAX_CAR; i++)
	{
		delete[] X[i];
	}
	delete[] X;
	delete[] FinishLine;
}

//Hàm thoát game
void ExitGame(HANDLE t) {
	system("cls");
	TerminateThread(t, 0);
	GabageCollect();
}

//Hàm dừng game
void PauseGame(HANDLE t) {
	SuspendThread(t);
}

//Hàm xử lý khi người đụng xe
void ProcessDead() {
	STATE = 0;
	GotoXY(WIDTH_CONSOLE + 2, 2);
	printf("Dead, type Y to continue or anykey to exit");
}

//Hàm xử lý khi người băng qua đường thành công
void ProcessFinish(POINT& p) {
	SPEED == MAX_SPEED ? SPEED = 1 : SPEED++;
	p = { 18,19 }; // Vị trí lúc đầu của người
	MOVING = 'D'; // Ban đầu cho người di chuyển sang phải
}

//Hàm vẽ các toa xe
void DrawCars(char* s)
{
	for (int i = 0; i < MAX_CAR; i++) {
		for (int j = 0; j < MAX_CAR_LENGTH; j++)
		{
			GotoXY(X[i][j].x, X[i][j].y);
			printf("%s", s);
		}
	}
}

//Hàm vẽ người qua đường
void DrawSticker(const POINT& p, char* s) {
	GotoXY(p.x, p.y);
	printf("%s", s);
}

//Hàm kiểm tra xem người qua đường có đụng xe không
bool IsImpact(const POINT& p, int d)
{
	if (d == 1 || d == 19)return false;
	for (int i = 0; i < MAX_CAR_LENGTH; i++)
	{
		if (p.x == X[d - 2][i].x && p.y == X[d - 2][i].y) return true;
	}
	return false;
}

void MoveCars() {
	for (int i = 1; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do {
			cnt++;
			for (int j = 0; j < MAX_CAR_LENGTH - 1; j++) {
				X[i][j] = X[i][j + 1];
			}
			X[i][MAX_CAR_LENGTH - 1].x + 1 == WIDTH_CONSOLE ? X[i][MAX_CAR_LENGTH - 1].x = 1 : X[i][MAX_CAR_LENGTH - 1].x++; // Kiểm tra xem xe có đụng màn hình không
		} while (cnt < SPEED);
	}
	for (int i = 0; i < MAX_CAR; i += 2)
	{
		cnt = 0;
		do {
			cnt++;
			for (int j = MAX_CAR_LENGTH - 1; j > 0; j--)
			{
				X[i][j] = X[i][j - 1];
			}
			X[i][0].x - 1 == 0 ? X[i][0].x = WIDTH_CONSOLE - 1 : X[i][0].x--;// Kiểm tra xem xe có đụng màn hình không
		} while (cnt < SPEED);
	}
}

// Hàm xóa xe (xóa có nghĩa là không vẽ)
void EraseCars()
{
	for (int i = 0; i < MAX_CAR; i += 2) {
		cnt = 0;
		do {
			GotoXY(X[i][MAX_CAR_LENGTH - 1 - cnt].x, X[i][MAX_CAR_LENGTH - 1 - cnt].y);
			printf(" ");
			cnt++;
		} while (cnt < SPEED);
	}
	for (int i = 1; i < MAX_CAR; i += 2) {
		cnt = 0;
		do {
			GotoXY(X[i][0 + cnt].x, X[i][0 + cnt].y);
			printf(" ");
			cnt++;
		} while (cnt < SPEED);
	}
}

void MoveRight() {
	if (Y.x < WIDTH_CONSOLE - 1)
	{
		DrawSticker(Y, " ");
		Y.x++;
		DrawSticker(Y, "Y");
	}
}
void MoveLeft() {
	if (Y.x > 1) {
		DrawSticker(Y, " ");
		Y.x--;
		DrawSticker(Y, "Y");
	}
}
void MoveDown() {
	if (Y.y < HEIGH_CONSOLE - 1)
	{
		DrawSticker(Y, " ");
		Y.y++;
		DrawSticker(Y, "Y");
	}
}
void MoveUp() {
	if (Y.y > 1) {
		DrawSticker(Y, " ");
		Y.y--;
		DrawSticker(Y, "Y");
	}
}

void SubThread()
{

	while (1) {
		if (STATE) //Nếu người vẫn còn sống
		{
			switch (MOVING) //Kiểm tra biến moving
			{
			case 'A':
				MoveLeft();
				break;
			case 'D':
				MoveRight();
				break;
			case 'W':
				MoveUp();
				break;
			case 'S':
				MoveDown();
				break;
			}
			MOVING = ' ';// Tạm khóa không cho di chuyển, chờ nhận phím từ hàm main
			EraseCars();
			MoveCars();
			DrawCars("0");
			if (IsImpact(Y, Y.y))
			{
				ProcessDead(); // Kiểm tra xe có đụng không
			}
			//xử lí khi nhân vật đến đích
			if (Y.y == 1)
			{
				for (int i = 0; i < MAX_MAN; i++)
				{
					if (Y.x - 1 == i && FinishLine[i] == 1)
					{
						ProcessDead();
					}
				}
				FinishLine[Y.x - 1] = 1; //lưu lại vị trí vào mảng khi nhân vật đến đích
				ProcessFinish(Y); // Kiểm tra xem về đích chưa
			}
			Sleep(50);//Hàm ngủ theo tốc độ SPEED
		}
	}
}

void TextColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void SaveGame()
{
	char NameSave[30];	//biến chứa tên mà người chơi muốn save game
	GotoXY(WIDTH_CONSOLE + 2, 4);
	cout << "Moi ban nhap ten tap tin can luu lai";
	GotoXY(WIDTH_CONSOLE + 1, 5);
	cout << "(VD: abc.txt) :";
	GotoXY(WIDTH_CONSOLE + 17, 5);
	cout << "                                            ";
	GotoXY(WIDTH_CONSOLE + 2, 6);
	cout << "                                            ";
	GotoXY(WIDTH_CONSOLE + 17, 5);
	gets_s(NameSave);
	FILE* fsave = fopen(NameSave, "wt");
	if (fsave != NULL)
	{
		for (int i = 0; i < MAX_MAN; i++)
		{
			fprintf(fsave, "%d ", FinishLine[i]);
		}
		GotoXY(WIDTH_CONSOLE + 2, 6);
		cout << ".....Tro choi da duoc luu thanh cong";
		fclose(fsave);
	}
	else
	{
		GotoXY(WIDTH_CONSOLE + 2, 6);
		cout << ".....Luu tro choi that bai";
	}
}

void LoadGame(char* NameLoad, char t)
{
	FILE* fload = fopen(NameLoad, "rt");
	if (fload != NULL)
	{
		for (int i = 0; i < MAX_MAN; i++)
		{
			fscanf_s(fload, "%d", &FinishLine[i]);
		}
		if (t == 2)
		{
			GotoXY(WIDTH_CONSOLE + 2, 11);
			cout << ".....Tai tro choi da luu thanh cong";
		}
		else
		{
			GotoXY(2, 8);
			cout << "  .....Tai tro choi da luu thanh cong. Nhan phim bat ki de vao game.";
		}
		GotoXY(0, HEIGH_CONSOLE + 5);
		for (int i = 0; i < MAX_MAN; i++)
		{
			printf("%d ", FinishLine[i]);
		}
		fclose(fload);
	}
	else
	{
		if (t == 2)
		{
			GotoXY(WIDTH_CONSOLE + 2, 11);
			cout << ".....Tai tro choi that bai";
		}
		else
		{
			GotoXY(2, 8);
			cout << "  .....Tai tro choi that bai. Nhan phim bat ki de choi moi.";
		}
	}
}

void GioiThieu()
{
	GotoXY(0, 4);
	cout << "Huong dan thao tac :";
	GotoXY(2, 5);
	cout << "- Bam phim bat ki de bat dau choi moi";
	GotoXY(2, 6);
	cout << "- Bam phim T de tai lai game. De tai lai game vui long nhap ten tap tin can tai (VD: abc.txt)";
	while (1)
	{
		GotoXY(40, 1);
		TextColor(8 + rand() % 8);
		cout << "CHAO MUNG CAC BAN DEN VOI TRO CHOI BANG QUA DUONG" << endl;
		Sleep(100);
	}
}

void main()
{
	int temp;	//lưu trữ phím bấm của người chơi
	char NameLoad[30];	//biến chứa tên mà người chơi muốn Load game
	srand(time_t(NULL));
	char t = 1;	//nhận biêt tiểu trình: 1 là tiểu trình giơi thiệu, 2 là tiểu trình chạy game

				//xử lí màn hình chính trước khi vào game
	thread t2(GioiThieu);
	TaoMangFinish(FinishLine);
	temp = toupper(_getch());
	if (temp == 'T')
	{
		PauseGame(t2.native_handle());
		TextColor(15);
		GotoXY(2, 7);
		cout << "  Moi ban nhap ten tap tin can load :";
		GotoXY(40, 7);
		gets_s(NameLoad);
		LoadGame(NameLoad, t);
		_getch();
		ResumeThread((HANDLE)t2.native_handle());
	}
	TerminateThread(t2.native_handle(), 0);
	t2.join();
	TextColor(15);

	//bắt đầu chơi game
	t = 2;
	FixConsoleWindow();
	StartGame();
	thread t1(SubThread);
	while (1)
	{
		temp = toupper(_getch());
		if (STATE == 1)
		{
			if (temp == 27) {
				ExitGame(t1.native_handle());
				t1.join();
				return;
			}
			else if (temp == 'P') {
				PauseGame(t1.native_handle());
			}
			else if (temp == 'L')
			{
				PauseGame(t1.native_handle());
				SaveGame();
				ResumeThread((HANDLE)t1.native_handle());
			}
			else if (temp == 'T')
			{
				PauseGame(t1.native_handle());
				GotoXY(WIDTH_CONSOLE + 2, 9);
				cout << "Moi ban nhap ten tap tin da duoc luu ";
				GotoXY(WIDTH_CONSOLE + 2, 10);
				cout << "(VD: abc.txt) :";
				GotoXY(WIDTH_CONSOLE + 17, 10);
				cout << "                            ";
				GotoXY(WIDTH_CONSOLE + 2, 11);
				cout << "                                    ";
				GotoXY(WIDTH_CONSOLE + 17, 10);
				gets_s(NameLoad);
				LoadGame(NameLoad, t);
				ResumeThread((HANDLE)t1.native_handle());
			}
			else {
				ResumeThread((HANDLE)t1.native_handle());
				if (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S')
				{
					MOVING = temp;
				}
			}
		}
		else
		{
			if (temp == 'Y')
			{
				//trả lại giá trị của vị trí đích là 0 khi nhấn Y để tiếp tục chơi
				for (int i = 0; i < MAX_MAN; i++)
				{
					FinishLine[i] = 0;
				}
				StartGame();
			}
			else {
				ExitGame(t1.native_handle());
				t1.join();
				return;
			}
		}
	}
}