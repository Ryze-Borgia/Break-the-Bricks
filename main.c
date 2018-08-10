#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#define LEFT 1
#define RIGHT 30
#define TOP 1
#define BUTTON 22

typedef struct POINT
{
    int x;
    int y;
} Point;

struct BALL
{
    int x;
    int y;
    int x_Speed;
    int y_Speed;
    int mode; //0->normal；1->power；2->slow；
} Ball;

struct BOARD
{
    int x;
    int y;
    int length;
} Board;

struct TREASURE //Gem
{
    int x;
    int y;
    int type;
} Treasure;

enum color
{
    green,
    blue,
    red
};

enum mode
{
    run_Game,
    creat_Map
};

/*-----------------------------------------------------------*/
void gotoxy(int x, int y);
int startMenu();
void run();
void messageBar(enum mode mode);

void runGame();
void creatMap();
void seeTheHelp();
int selectMap();
void playGame();
int checkComplete();

void initBoard();
void gameShow();
void moveBoard(int orientation);
void moveBall();

void removeDiamond(int direction);
void removeColor(enum color, Point diamond);
void showTreasure(int intRand, Point point_diamond); 
void moveTreasure();
void getGem();
void timeTreasure();
/*------------------------------------------------------------------------------*/

HANDLE g_hConsoleOutput;

int modeSelect = 0;
short pointVal[40][30]; //the status of every point in the console
						//0->blank，1->normal bricks，2->hard bricks，3->diamonds，4->boundary

int live = 1;        //status of game process.
int isStop = 0;      //Be used to control the game process.
int speed = 1000;
int grade = 0;
int intTreasure = 0;
int getTreasure = 0;
int currentLevel = 0;
int maxLevel = 0;

char mapList[100][20];

clock_t treaTime_Last, treaTime_Now;

static const char *BALL_SHAPE[] = {"⊙", "◎", "●"};

int main()
{
    g_hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE); 
    SetConsoleTitleA("break the bricks");

    mciSendString("play music\\bgmc.mp3 repeat ", NULL, 0, NULL); //play the bgm
    mciSendString("setaudio music\\bgm.mp3 volume to 30", NULL, 0, NULL); //set the vol

	CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
	SetConsoleCursorInfo(g_hConsoleOutput, &cursorInfo); 

	FILE *fMap;

	if (fopen_s(&fMap, "maps\\MapInfo.dat", "r"))
	{
		gotoxy(LEFT + 4, TOP + 2);
		printf("Open the mapInfo.dat error!!!");
		gotoxy(LEFT + 4, TOP + 4);
		printf("Please check MapInfo.dat");
		exit(0);
	}

	while (fgets(mapList[maxLevel], 20, fMap) != NULL)
	{
		if (mapList[maxLevel][strlen(mapList[maxLevel]) - 1] == '\n')
		{
			mapList[maxLevel][strlen(mapList[maxLevel]) - 1] = 0;
		}
		maxLevel++;
	}

	fclose(fMap);

    while(1)
    {
		run();
    }

    system("pause>nul");
    return 0;
}

/*-------------------------------location--------------------------------*/
void gotoxy(int x, int y)
{
    static COORD cd;

    cd.X = (int)(x << 1);
    cd.Y = y;
    SetConsoleCursorPosition(g_hConsoleOutput, cd);
}

int startMenu()
{
	int ch;
	static const char *menu[] = { ">  START GAME  <", "> MAP WORKSHOP <", ">  SELECT MAP  <", ">     HELP     <" };

	system("cls");

	SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	gotoxy(15, 5);
	printf("┏━━━━━━━━━━━━━━━━━━━━━━━┓");
	gotoxy(15, 6);
	printf("┃%2s%s ┃", "", "★BREAK THE BRICKS★", "");
	gotoxy(15, 7);
	printf("┗━━━━━━━━━━━━━━━━━━━━━━━┛");

	SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
	gotoxy(16, 14);
	printf("%2s%s%2s", "", menu[0], "");

	currentLevel = 0; 

	do
	{
		ch = _getch();
		switch (ch)
		{
		case 's':
		case 'S':
		case '2':
		case 80: // ↓
		{
			modeSelect = (modeSelect + 1) % 4;
			gotoxy(16, 14);
			printf("%2s%s%2s", "", menu[modeSelect], "");
			break;
		}
		case 'w':
		case 'W':
		case '8':
		case 72: // ↑
		{
			if (modeSelect == 0)
				modeSelect = 3;
			else
				modeSelect--;
			gotoxy(16, 14);
			printf("%2s%s%2s", "", menu[modeSelect], "");
			break;
		}
		case ' ':
		case 13:
			return modeSelect;
			break;
		default:
			break;
		}
	} while (1);
	return 0;
}

void run()
{
	startMenu();
	switch (modeSelect)
	{
	case 0:
		playGame();
		break;
	case 1:
		creatMap();
		break;
	case 2:
		if (selectMap())
			playGame();
		else
			return;
		break;
	case 3:
		seeTheHelp();
		break;
	}
}

/*------------------the main process in this project-------------------*/
void playGame()
{
    char ch;

    do
    {
        runGame(mapList[currentLevel]);

        if (currentLevel < maxLevel && checkComplete())
        {
            currentLevel++;
            gotoxy(LEFT + 2, BUTTON - 3);
            printf("Congratulations！！！");
            gotoxy(LEFT + 2, BUTTON - 2);
            printf("Press the enter to continue OR any other keys to return to the start menu");
        }
        else
        {
			printf("nothing can be show to you at all!");
            break;
        }
    } while ((ch = _getch()) == 13);
}

/*--------------------------process function-------------------------*/
void runGame()
{
    int ch;
    clock_t clockLast, clockNow; 
    clock_t treasureLast, treasureNow;

    clockLast = treasureLast = clock();

    messageBar(run_Game);

    gameShow();

    while (live)
    {
        while (!isStop && live)
        {
            while (_kbhit())
            {
                ch = _getch();
                switch (ch)
                {
                case 27:
                    return;
                    break;
                case 'a':
                case 'A':
                case '4':
                case 75:
                    moveBoard(1);
                    break;
                case 'd':
                case 'D':
                case '6':
                case 77:
                    moveBoard(2);
                    break;
                case 32:
                    isStop = 1;
					break;
				default:
					break;
                }
            }

            clockNow = clock();
            if (clockNow - clockLast > 0.2F * speed)  //control the speed of ball
            {
                clockLast = clockNow;
                moveBall(0);
            }

            if (intTreasure == 1)
            {
                treasureNow = clock();
                if (treasureNow - treasureLast > 0.5F * CLOCKS_PER_SEC)
                {
                    treasureLast = treasureNow;
                    moveTreasure();
                }
            }

            if (getTreasure == 1)
            {
                treaTime_Now = clock();
                if (treaTime_Now - treaTime_Last > 1.0F * CLOCKS_PER_SEC)
                {
                    if (treaTime_Now - treaTime_Last < 1.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 27, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 2.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 26, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 3.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 25, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 4.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 24, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 5.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 23, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 6.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 22, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 7.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 21, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 8.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 20, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 9.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 19, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 10.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 18, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 11.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 17, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 12.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 16, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 13.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 15, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 14.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 14, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 15.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 13, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 16.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 12, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 17.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 11, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 18.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 10, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 19.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 9, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 20.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 8, BUTTON - 2);
                        printf("%2s", "");
                    }
                    else if (treaTime_Now - treaTime_Last < 21.1F * CLOCKS_PER_SEC)
                    {
                        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                        gotoxy(LEFT + 2, BUTTON - 2);
                        printf("%50s", "");
                        timeTreasure();
                        getTreasure = 0;
                    }
                }
            }
            if (checkComplete())
            {
                return;
            }
        }

        if (live)
        {
            if ((ch = _getch()) == 32)
                isStop = 0;
        }
    }
}

void seeTheHelp()
{
    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    system("cls");

    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    gotoxy(LEFT, 2);
    printf("====================Items=====================");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 4);
    printf("■:normal bricks,can be broken easily");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 6);
    printf("■:hard bricks,may cost you some effort");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 8);
    printf("■:diamond,it's not a brick!!");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 10);
    printf("★:Gem of Power,nothing can stop the ball");

    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    gotoxy(LEFT, 12);
    printf("▓:Gem of Frost,the ball will be slow down");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 14);
    printf("◆:Gem of Greed,the board get longer");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 16);
    printf("◆:Gem of Evil,the board get shorter,best wishes~");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 18);
    printf("▼:Gen of Wind：Speed Up!!!");

    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    gotoxy(LEFT, 20);
    printf("★:Crazy Stone：Bonus Time,nothing should you do but watch the show!!");

    system("pause>nul");
}

int selectMap()
{
    char ch;

    currentLevel = 0;
    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    gotoxy(LEFT + 15, TOP + 11);
    printf(">----Map  List----<");

    SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
    gotoxy(LEFT + 15, TOP + 13);
    printf("   %10s     ", mapList[currentLevel]);
	gotoxy(LEFT + 12, TOP + 15);
	printf("current map:No.%d  Total List:%d", currentLevel + 1, maxLevel);
	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    do
    {
        ch = _getch();

        switch (ch)
        {
        case 'w':
        case 'W':
        case '8':
        case 72: // ↑
		{
			if (currentLevel == 0)
				currentLevel = maxLevel - 1;
			else
				currentLevel--;
			SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
			gotoxy(LEFT + 15, TOP+13);
			printf("    %10s    ", mapList[currentLevel]);
			SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
			break;
		}
        case 's':
        case 'S':
        case '2':
        case 80: // ↓
		{
			currentLevel = (currentLevel + 1) % maxLevel;
			SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
			gotoxy(LEFT + 15, TOP+13);
			printf("    %10s    ", mapList[currentLevel]);
			SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
			break;
		}
        case ' ':
        case 13:
            return 1;
            break;
        case 27:
            currentLevel = 0;
            return 0;
            break;
        }
    } while (1);
	return 0;
}

void creatMap()
{
    int i, j;
    char ch;
    Point pot_map;
    char map_Name[20] = "maps//";
    char temp_Name[20];

    FILE *fp, *fMap;

    CONSOLE_CURSOR_INFO cursorInfo = {1, TRUE};
    SetConsoleCursorInfo(g_hConsoleOutput, &cursorInfo);

    messageBar(creat_Map);

    pot_map.x = LEFT + 1;
    pot_map.y = TOP + 1;
    gotoxy(pot_map.x, pot_map.y);

    do
    {
        switch (ch = _getch())
        {
        case 'w':
        case 'W':
        case 72: //↑
		{
			if (pot_map.y > TOP + 1)
			{
				pot_map.y -= 1;
				gotoxy(pot_map.x, pot_map.y);
			}
			break;
		}
        case 's':
        case 'S':
        case 80: //↓
		{
			if (pot_map.y < BUTTON - 10)
			{
				pot_map.y += 1;
				gotoxy(pot_map.x, pot_map.y);
			}
			break;
		}
        case 'a':
        case 'A':
        case 75: //←
		{
			if (pot_map.x > LEFT + 1)
			{
				pot_map.x -= 1;
				gotoxy(pot_map.x, pot_map.y);
			}
			break;
		}
        case 'd':
        case 'D':
        case 77: //→
		{
			if (pot_map.x < RIGHT - 1)
			{
				pot_map.x += 1;
				gotoxy(pot_map.x, pot_map.y);
			}
			break;
		}
        case '0':
		{
			SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
			gotoxy(pot_map.x, pot_map.y);
			printf("%2s", "");
			pointVal[pot_map.x][pot_map.y] = 0;
			break;
		}
        case '1':
		{
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			gotoxy(pot_map.x, pot_map.y);
			printf("■");
			pointVal[pot_map.x][pot_map.y] = 1;
			break;
		}
        case '2':
		{
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			gotoxy(pot_map.x, pot_map.y);
			printf("■");
			pointVal[pot_map.x][pot_map.y] = 2;
			break;
		}
        case '3':
		{
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
			gotoxy(pot_map.x, pot_map.y);
			printf("■");
			pointVal[pot_map.x][pot_map.y] = 3;
			break;
		}
        case 32:
            if (pointVal[pot_map.x][pot_map.y] == 0)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                gotoxy(pot_map.x, pot_map.y);
                printf("■");
                pointVal[pot_map.x][pot_map.y] = 1;
            }
            else if (pointVal[pot_map.x][pot_map.y] == 1)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                gotoxy(pot_map.x, pot_map.y);
                printf("■");
                pointVal[pot_map.x][pot_map.y] = 2;
            }
            else if (pointVal[pot_map.x][pot_map.y] == 2)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(pot_map.x, pot_map.y);
                printf("■");
                pointVal[pot_map.x][pot_map.y] = 3;
            }
            else if (pointVal[pot_map.x][pot_map.y] == 3)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(pot_map.x, pot_map.y);
                printf("%2s", "");
                pointVal[pot_map.x][pot_map.y] = 0;
            }
            break;

        case 27:
            return;
            break;
        }

        if (ch == 13)
        {
            gotoxy(LEFT + 2, BUTTON - 5);
            printf("Press enter to continue to save this map");
            _getch();
            ch = _getch();
            if (ch == 13)
            {
                gotoxy(LEFT + 5, BUTTON - 3);
                printf("save as-->  ");
                scanf_s("%s", temp_Name, sizeof(temp_Name));
                gotoxy(LEFT + 5, BUTTON - 2);
                printf("saving...");
                Sleep(100);
                break;
            }
            else
            {
                gotoxy(LEFT + 2, BUTTON - 5);
                printf("%53s", "");
            }
        }

        gotoxy(pot_map.x, pot_map.y);
    } while (1);

    strcat_s(map_Name, sizeof(map_Name), temp_Name);
    strcat_s(map_Name, sizeof(map_Name), ".map");

    if (fopen_s(&fp, map_Name, "wb") || fopen_s(&fMap, "maps\\MapInfo.dat", "a"))
    {
        gotoxy(LEFT + 12, BUTTON - 3);
        printf("Unsaved!");
        Sleep(1000);
        exit(0);
    }
	else
	{
		for (i = TOP + 1; i < BUTTON - 9; i++)
		{
			for (j = LEFT + 1; j < RIGHT; j++)
			{
				fwrite(&pointVal[j][i], sizeof(short), 1, fp);
			}
		}

		strcat_s(temp_Name, sizeof(temp_Name), "\n");
		fputs(temp_Name, fMap);

		fclose(fMap);
		fclose(fp);
	}
    gotoxy(LEFT + 12, BUTTON - 2);
    printf("save successfully!");
    Sleep(1000);
}

void gameShow()
{
    int i, j;

    FILE *fp;  
    short file_temp; 
    char url_Map[30] = "maps//"; 

    strcat_s(url_Map, sizeof(url_Map), mapList[currentLevel]);
    strcat_s(url_Map, sizeof(url_Map), ".map");
    if (fopen_s(&fp, url_Map, "rb"))
    {
        printf("open the map failed!!");
        exit(0);
    }

    gotoxy(LEFT + 2, TOP + 2);

    for (i = TOP + 1; i < BUTTON - 9; i++)
    {
        for (j = LEFT + 1; j < RIGHT; j++)
        {
            gotoxy(j, i);

            fread(&file_temp, sizeof(short), 1, fp);

            switch (file_temp)
            {
            case 1:
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                pointVal[j][i] = 1;
                printf("■");
                break;
            case 2:
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                pointVal[j][i] = 2;
                printf("■");
                break;
            case 3:
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
                pointVal[j][i] = 3;
                printf("■");
                break;
            default:
                pointVal[j][i] = 0;
                break;
            }
            Sleep(5);
        }
    }

    fclose(fp);

    initBoard();
}

void initBoard()
{
    //print the ball
    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    Ball.x = 19;
    Ball.y = 18;
    gotoxy(Ball.x, Ball.y);
    printf("%s", BALL_SHAPE[0]);

    //print the board
    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    Board.x = 18;
    Board.y = 19;
    gotoxy(Board.x, Board.y);
    pointVal[Board.x][Board.y] = pointVal[Board.x + 1][Board.y] = pointVal[Board.x + 2][Board.y] = 4;
    printf("▔▔▔");
}

void moveBoard(int orientation)
{
    if (orientation == 1)
    {
		if (Board.x > LEFT + 1)
		{
			Board.x = Board.x - 1;
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			gotoxy(Board.x, Board.y);
			if (Board.length == 1)
			{
				printf("▔  ");
				pointVal[Board.x][Board.y] = 4;
				pointVal[Board.x + 1][Board.y] = 0;
			}
			else if (Board.length == 3)
			{
				printf("▔▔▔  ");
				pointVal[Board.x][Board.y] = 4;
				pointVal[Board.x + 3][Board.y] = 0;
			}
			else if (Board.length == 5)
			{
				printf("▔▔▔▔▔  ");
				pointVal[Board.x][Board.y] = 4;
				pointVal[Board.x + 5][Board.y] = 0;
			}
		}
    }
    else
    {
        if (Board.length == 1)
        {
            if ((Board.x + 1) < RIGHT)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                gotoxy(Board.x, Board.y);
                printf("  ▔");
                Board.x = Board.x + 1;
                pointVal[Board.x][Board.y] = 4;
                pointVal[Board.x - 1][Board.y] = 0;
            }
        }
        else if (Board.length == 3)
        {
            if ((Board.x + 3) < RIGHT)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                gotoxy(Board.x, Board.y);
                printf("  ▔▔▔");
                Board.x = Board.x + 1;
                pointVal[Board.x + 2][Board.y] = 4;
                pointVal[Board.x - 1][Board.y] = 0;
            }
        }
        else if (Board.length == 5)
        {
            if ((Board.x + 5) < RIGHT)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                gotoxy(Board.x, Board.y);
                printf("  ▔▔▔▔▔");
                Board.x = Board.x + 1;
                pointVal[Board.x + 4][Board.y] = 4;
                pointVal[Board.x - 1][Board.y] = 0;
            }
        }
    }
}

void moveBall()
{
    if (Ball.x_Speed == 1 && Ball.y_Speed == 1)
    {
        if (pointVal[Ball.x][Ball.y - 1] == 4)
        {
            Ball.y_Speed = -1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x + 1][Ball.y] == 4)
        {
            Ball.x_Speed = -1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x][Ball.y - 1] != 0)
        {
            removeDiamond(1);
        }
        else if (pointVal[Ball.x + 1][Ball.y] != 0)
        {
            removeDiamond(3);
        }
        else if (pointVal[Ball.x + 1][Ball.y - 1] != 0)
        {
            removeDiamond(2);
        }
        else
        {
            if (Ball.mode == 0)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x + 1;
                Ball.y = Ball.y - 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[0]);
            }
            else if (Ball.mode == 1)
            {
                gotoxy(Ball.x, Ball.y);
                if (pointVal[Ball.x][Ball.y] == 1)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("■");
                }
                else if (pointVal[Ball.x][Ball.y] == 0)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("%2s", "");
                }

                Ball.x = Ball.x + 1;
                Ball.y = Ball.y - 1;

                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[1]);
            }
            else
            {
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x + 1;
                Ball.y = Ball.y - 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[2]);
            }
        }
    }

    if (Ball.x_Speed == 1 && Ball.y_Speed == -1)
    {

        if (pointVal[Ball.x][Ball.y + 1] == 4)
        {
            Ball.y_Speed = 1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x + 1][Ball.y] == 4)
        {
            Ball.x_Speed = -1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x + 1][Ball.y + 1] == 4)
        {
            Ball.x_Speed = -1;
            Ball.y_Speed = 1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x + 1][Ball.y] != 0)
        {
            removeDiamond(3);
        }
        else if (pointVal[Ball.x][Ball.y + 1] != 0)
        {
            removeDiamond(5);
        }
        else if (pointVal[Ball.x + 1][Ball.y + 1] != 0)
        {
            removeDiamond(4);
        }
        else
        {
            if (Ball.mode == 0)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x + 1;
                Ball.y = Ball.y + 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[0]);
            }
            else if (Ball.mode == 1)
            {

                gotoxy(Ball.x, Ball.y);
                if (pointVal[Ball.x][Ball.y] == 1)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("■");
                }
                else if (pointVal[Ball.x][Ball.y] == 0)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("%2s", "");
                }

                Ball.x = Ball.x + 1;
                Ball.y = Ball.y + 1;

                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[1]);
            }
            else
            {
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x + 1;
                Ball.y = Ball.y + 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[2]);
            }
        }
    }

    if (Ball.x_Speed == -1 && Ball.y_Speed == -1)
    {
        if (pointVal[Ball.x][Ball.y + 1] == 4)
        {
            Ball.y_Speed = 1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x - 1][Ball.y] == 4)
        {
            Ball.x_Speed = 1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x - 1][Ball.y + 1] == 4)
        {
            Ball.x_Speed = 1;
            Ball.y_Speed = 1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x][Ball.y + 1] != 0)
        {
            removeDiamond(5);
        }
        else if (pointVal[Ball.x - 1][Ball.y] != 0)
        {
            removeDiamond(7);
        }
        else if (pointVal[Ball.x - 1][Ball.y + 1] != 0)
        {
            removeDiamond(6);
        }
        else
        {
            if (Ball.mode == 0)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x - 1;
                Ball.y = Ball.y + 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[0]);
            }
            else if (Ball.mode == 1)
            {

                gotoxy(Ball.x, Ball.y);
                if (pointVal[Ball.x][Ball.y] == 1)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("■");
                }
                else if (pointVal[Ball.x][Ball.y] == 0)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("%2s", "");
                }

                Ball.x = Ball.x - 1;
                Ball.y = Ball.y + 1;

                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[1]);
            }
            else
            {
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x - 1;
                Ball.y = Ball.y + 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[2]);
            }
        }
    }

    if (Ball.x_Speed == -1 && Ball.y_Speed == 1)
    {
        if (pointVal[Ball.x][Ball.y - 1] == 4)
        {
            Ball.y_Speed = -1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x - 1][Ball.y] == 4)
        {
            Ball.x_Speed = 1;
            PlaySound("music\\knock_1.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (pointVal[Ball.x][Ball.y - 1] != 0)
        {
            removeDiamond(1);
        }
        else if (pointVal[Ball.x - 1][Ball.y] != 0)
        {
            removeDiamond(7);
        }
        else if (pointVal[Ball.x - 1][Ball.y - 1] != 0)
        {
            removeDiamond(8);
        }
        else
        {
            if (Ball.mode == 0)
            {
                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x - 1;
                Ball.y = Ball.y - 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[0]);
            }
            else if (Ball.mode == 1)
            {

                gotoxy(Ball.x, Ball.y);
                if (pointVal[Ball.x][Ball.y] == 1)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("■");
                }
                else if (pointVal[Ball.x][Ball.y] == 0)
                {
                    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                    printf("%2s", "");
                }

                Ball.x = Ball.x - 1;
                Ball.y = Ball.y - 1;

                SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[1]);
            }
            else
            {
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(Ball.x, Ball.y);
                printf("%2s", "");
                Ball.x = Ball.x - 1;
                Ball.y = Ball.y - 1;
                gotoxy(Ball.x, Ball.y);
                printf("%s", BALL_SHAPE[2]);
            }
        }
    }

    if (Ball.y > BUTTON)
        live = 0;
}

void removeDiamond(int direction)
{
    Point point_diamond;

    switch (direction)
    {
    case 1: //↑

        if (Ball.mode != 1)
        {
            Ball.y_Speed = -1;
        }

        point_diamond.x = Ball.x;
        point_diamond.y = Ball.y - 1;

        break;

    case 2: //upper right
        if (Ball.mode != 1)
        {
            Ball.x_Speed = -1;
            Ball.y_Speed = -1;
        }

        point_diamond.x = Ball.x + 1;
        point_diamond.y = Ball.y - 1;

        break;

    case 3: //→
        if (Ball.mode != 1)
        {
            Ball.x_Speed = -1;
        }

        point_diamond.x = Ball.x + 1;
        point_diamond.y = Ball.y;

        break;

    case 4: //low right
        if (Ball.mode != 1)
        {
            Ball.x_Speed = -1;
            Ball.y_Speed = 1;
        }

        point_diamond.x = Ball.x + 1;
        point_diamond.y = Ball.y + 1;

        break;

    case 5:
        if (Ball.mode != 1)
        {
            Ball.y_Speed = 1;
        }

        point_diamond.x = Ball.x;
        point_diamond.y = Ball.y + 1;

        break;

    case 6:
        if (Ball.mode != 1)
        {
            Ball.x_Speed = 1;
            Ball.y_Speed = 1;
        }

        point_diamond.x = Ball.x - 1;
        point_diamond.y = Ball.y + 1;

        break;

    case 7:
        if (Ball.mode != 1)
        {
            Ball.x_Speed = 1;
        }

        point_diamond.x = Ball.x - 1;
        point_diamond.y = Ball.y;

        break;

    case 8:
        if (Ball.mode != 1)
        {
            Ball.x_Speed = 1;
            Ball.y_Speed = -1;
        }

        point_diamond.x = Ball.x - 1;
        point_diamond.y = Ball.y - 1;

        break;
    }

    if (pointVal[point_diamond.x][point_diamond.y] == 3)
    {
        removeColor(red, point_diamond);
    }
    else if (pointVal[point_diamond.x][point_diamond.y] == 2)
    {
        removeColor(blue, point_diamond);
    }
    else if (pointVal[point_diamond.x][point_diamond.y] == 1)
    {
        removeColor(green, point_diamond);
    }
}

void removeColor(enum color color_diamond, Point point_diamond)
{
    PlaySound("music\\knock_2.wav", NULL, SND_FILENAME | SND_ASYNC);

    switch (color_diamond)
    {
    case green:
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(point_diamond.x, point_diamond.y);
        pointVal[point_diamond.x][point_diamond.y] = 0;
        printf("%2s", "");
        break;

    case blue:
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(point_diamond.x, point_diamond.y);
        pointVal[point_diamond.x][point_diamond.y] = 1;
        printf("■");
        break;

    case red:
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(point_diamond.x, point_diamond.y);
        pointVal[point_diamond.x][point_diamond.y] = 2;
        printf("■");
        break;
    }

    grade += 10;
    SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    gotoxy(RIGHT + 5, 2);
    printf("%d", grade);

    srand((int)time(NULL));
    if (rand() % 100 < 60 && intTreasure == 0 && getTreasure == 0) //generate a rand treasure
    {
        showTreasure(rand() % 100, point_diamond);
    }
}

void showTreasure(int intRand, Point point_diamond)
{
    int y = Board.y;
    intTreasure = 1;

    if (intRand < 15) //Power
    {
        Treasure.type = 1;
        Treasure.x = point_diamond.x;
    }
    else if (intRand < 25) //Frost
    {
        Treasure.type = 2;
        Treasure.x = point_diamond.x;
    }
    else if (intRand < 40) //grow
    {
        Treasure.type = 3;
        Treasure.x = point_diamond.x;
    }
    else if (intRand < 65) //shorten
    {
        Treasure.type = 4;
        Treasure.x = point_diamond.x;
    }
    else if (intRand < 75) //speed up
    {
        Treasure.type = 5;
        Treasure.x = point_diamond.x;
    }
    else //crazy
    {
        Treasure.type = 6;
        Treasure.x = point_diamond.x;
    }

    while (pointVal[point_diamond.x][y - 1] == 0 && y > point_diamond.y)
        y--;
    Treasure.y = y;
}

void moveTreasure()
{
    gotoxy(Treasure.x, Treasure.y);
    printf("%2s", "");

    if (Treasure.y + 1 == Board.y && pointVal[Treasure.x][Treasure.y + 1] == 4)
    {
        getGem();
        intTreasure = 0;
        return;
    }
    else if (Treasure.y + 1 < BUTTON)
    {
        Treasure.y++;
    }
    else
    {
        intTreasure = 0;
        return;
    }

    gotoxy(Treasure.x, Treasure.y);

    switch (Treasure.type)
    {
    case 1: //stick
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        printf("★");
        break;

    case 2: //slower
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        printf("▓");
        break;

    case 3: //grow
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        printf("◆");
        break;

    case 4: //shorten
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
        printf("◆");
        break;

    case 5: //speed up
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        printf("▼");
        break;

    case 6: //crazy
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("★");
        break;
    }
}

void getGem()
{
    int i;

    switch (Treasure.type)
    {
    case 1: //strengthen

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(LEFT + 2, BUTTON - 2);
        printf("★");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 4, BUTTON - 2);
        printf("Time: ■■■■■■■■■■■■■■■■■■■■");

        speed = 1000;
        Ball.mode = 1;

        break;

    case 2: //slower

        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 2, BUTTON - 2);
        printf("▓");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 4, BUTTON - 2);
        printf("Time: ■■■■■■■■■■■■■■■■■■■■");

        speed = 1600;
        Ball.mode = 2;

        break;

    case 3: //grow

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(LEFT + 2, BUTTON - 2);
        printf("◆");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 4, BUTTON - 2);
        printf("Time: ■■■■■■■■■■■■■■■■■■■■");

        Board.length = 5;
        pointVal[Board.x][Board.y] = pointVal[Board.x + 1][Board.y] = pointVal[Board.x + 2][Board.y] = 0;

        if (Board.x + 5 >= RIGHT)
        {
            Board.x = RIGHT - 5;
        }
        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(Board.x, Board.y);
        printf("▔▔▔▔▔");
        pointVal[Board.x][Board.y] = pointVal[Board.x + 1][Board.y] = pointVal[Board.x + 2][Board.y] = pointVal[Board.x + 3][Board.y] = pointVal[Board.x + 4][Board.y] = 4;

        break;

    case 4: //shorten

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(LEFT + 2, BUTTON - 2);
        printf("◆");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 4, BUTTON - 2);
        printf("Time: ■■■■■■■■■■■■■■■■■■■■");

        Board.length = 1;
        pointVal[Board.x + 1][Board.y] = pointVal[Board.x + 2][Board.y] = 0;

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(Board.x, Board.y);
        printf("▔%4s", "");

        break;

    case 5: //speed up

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(LEFT + 2, BUTTON - 2);
        printf("▼");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 4, BUTTON - 2);
        printf("Time: ■■■■■■■■■■■■■■■■■■■■");

        speed = 500;
        break;

    case 6: //crazy

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(LEFT + 2, BUTTON - 2);
        printf("★");
        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(LEFT + 4, BUTTON - 2);
        printf("Time: ■■■■■■■■■■■■■■■■■■■■");

        speed = 200;
        for (i = LEFT + 1; i <= RIGHT - 1; i++)
        {
            SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
            gotoxy(i, BUTTON - 4);
            printf("■");
            pointVal[i][BUTTON - 4] = 4;
            Sleep(10);
        }

        break;
    }

    getTreasure = 1;
    treaTime_Last = clock();
}

void timeTreasure()
{
    int i;

    switch (Treasure.type)
    {
    case 1: 
        Ball.mode = 0;
        break;

    case 2: 
        speed = 1000;
        Ball.mode = 0;
        break;

    case 3:

        Board.length = 3;
        pointVal[Board.x + 3][Board.y] = pointVal[Board.x + 4][Board.y] = 0;

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(Board.x, Board.y);
        printf("▔▔▔%4s", "");

        break;

    case 4: 

        Board.length = 3;
        if (Board.x + 3 >= RIGHT)
        {
            Board.x = RIGHT - 3;
        }
        pointVal[Board.x][Board.y] = pointVal[Board.x + 1][Board.y] = pointVal[Board.x + 2][Board.y] = 4;

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(Board.x, Board.y);
        printf("▔▔▔");

        break;

    case 5:
        speed = 1000;
        break;

    case 6: 

        speed = 1000;
        for (i = LEFT + 1; i <= RIGHT - 1; i++)
        {
            SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
            gotoxy(i, BUTTON - 4);
            printf("%2s", "");
            pointVal[i][BUTTON - 4] = 0;
            Sleep(10);
        }

        break;
    }
}

void messageBar(enum mode game_Mode)
{
    int i, j;

    for (i = LEFT + 1; i < RIGHT; i++)
    {
        for (j = TOP + 1; j < BUTTON; j++)
        {
            pointVal[i][j] = 0;
        }
    }

    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
    system("cls");
    SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);

    for (i = LEFT; i <= RIGHT; i++)
    {
        gotoxy(i, TOP);
        printf("%2s", "");
        pointVal[i][TOP] = 4;
    }

    for (i = TOP; i < BUTTON + 3; i++)
    {
        gotoxy(LEFT, i);
        printf("%2s", "");
        pointVal[LEFT][i] = 4;

        gotoxy(RIGHT, i);
        printf("%2s", "");
        pointVal[RIGHT][i] = 4;
    }

    SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);

    for (i = TOP + 1; i < BUTTON + 3; i++)
    {
        gotoxy(LEFT + 1, i);
        printf("%2s", "");

        gotoxy(RIGHT - 1, i);
        printf("%2s", "");
    }

    switch (game_Mode)
    {
    case run_Game:

        Ball.mode = 0;
        Ball.x_Speed = 1;
        Ball.y_Speed = 1;
        Board.length = 3;

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 2);
        printf("grade: 0");

        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(RIGHT + 2, 4);
        printf("===Items descption===");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 6);
        printf("■:normal bricks");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 8);
        printf("■:hard bricks");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 10);
        printf("■:diamonds");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 12);
        printf("★:power[stick]");

        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(RIGHT + 2, 14);
        printf("▓:Frost[slower]");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 16);
        printf("◆:Greed[grow]");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 18);
        printf("◆:Evil[shorten]");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 20);
        printf("▼:Wind[Speed up]");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 22);
        printf("★:Crazy[Auto]");

        break;

    case creat_Map:

        SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
        gotoxy(RIGHT + 2, 2);
        printf("tips：");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 4);
        printf("<0>-->undo");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 6);
        printf("<1>-->■");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 8);
        printf("<2>-->■");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 10);
        printf("<3>-->■");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 15);
        printf("<ENTER>save");

        SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED | FOREGROUND_INTENSITY);
        gotoxy(RIGHT + 2, 17);
        printf("<ESC> cancel");

        break;
    }
}

int checkComplete()
{
    int i, j;
    int intSurplus = 0;

    for (i = LEFT + 1; i < RIGHT; i++)
    {
        for (j = TOP + 1; j < Board.y; j++)
        {
            if (pointVal[i][j] != 0)
            {
                intSurplus++;
            }
            if (intSurplus > 3)
            {
                return 0;
            }
        }
    }

    return 1;
}