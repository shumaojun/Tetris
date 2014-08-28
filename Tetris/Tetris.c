#include<windows.h>
#include<Windows.h>
#include"resource.h"
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include "block.h"
#define ID_GAMETIMER 1

/************游戏区结构体***************
* 用于记录游戏区每个小方块的状态及颜色
**************************************/
struct BOARD
{
	int Map[22][13];
	int Colour[22][13];//小方块的颜色
	int top;//当前情况下游戏区方块的最高位置的上面一行，类似于栈顶
}gameBoard;
/**************************************END*/

/**************全局变量区***************/
static TCHAR szAppName[] = TEXT("Tetris--俄罗斯方块(舒茂军)");
static int xBricks = 20;
static int yBricks = 24;			//纵横坐标上共有多少个方块(每个方块为24*24)
HBITMAP hBitMap[10];		//用于存储位图句柄的数组
RECT rectGame;					//游戏区
RECT rectNext;						//用于显示下一个出现的方块的区域		
RECT rectDownBlock;			//下落的方块所在的矩形区域
RECT rectScoreAndLevel;   //绘制游戏等级和分数的矩形区域
int gameSpeed = 200;
struct BLOCK curBlock;					//指向当前的方块
struct BLOCK nextBlock;				//指向下一个结构体
struct BOARD gameBoard;              //保存游戏区的状态记录
int Score=0;										//游戏的得分
int Level =1;										//游戏的等级
BOOL Pause = TRUE;						//用于暂停游戏
int Lines[4];										//纪录当前的满行的行号
int gameState;									//纪录游戏状态，0:未开始，1:游戏中，2:游戏结束

unsigned char Styles[][8] ={{0x0F,0x00,0x44,0x44},//长条的两种形状
											 {0x0E,0x80,0xC4,0x40,0x2E,0x00,0x44,0x60},//L型的4种形状
											 {0x0E,0x20,0x44,0xC0,0x8E,0x00,0x64,0x40},//反L型的4种形状
											 {0x0C,0x60,0x4C,0x80},//Z型的两种形状
											 {0x06,0xC0,0x46,0x20},//反Z型的两种形状
										     {0x0E,0x40,0x4C,0x40,0x4E,0x00,0x46,0x40},//凸型的四种形状
											 {0x06,0x60}};//田型的一种形状

/***********************************END*/


/**************函数声明区***************/
LRESULT CALLBACK  windProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK AboutProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
void SetClientRect(HWND	hwnd,int width,int height);//设置客户区的大小
void DrawWall(HWND hwnd,HDC hdc,int index,RECT rectClient);//绘制砖墙
void ProduceBlock(struct BLOCK *block);
void DrawBrick(HWND hwnd,HDC hdc,struct BLOCK block,RECT rect,HBITMAP hBitMap[],int index);
void DrawGameRect(HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int index);
void DrawScoreAndLevel(HWND hwnd);//绘制分数和等级
BOOL TouchDown();//判断方块是否到达底部
POINT Translate(RECT rect,int x,int y);//将rectDownBlock中的i,j转换为gameBoard中的i,j
BOOL RightShift();//判断右移是否碰到墙或者障碍物
BOOL LeftShift();//左移是否碰到墙或者障碍物
BOOL Up();//判断是否可以旋转
void GameBoardValue();//保存游戏版的状态
int FindTop();//找到当前最高点所在的行
int FullLines();//查找当前所有满行所在的位置，存储在数组a中
void  DeleteFull(int fullLines);//消行处理
void DeleteLine(int lines);//消除一行的处理操作
void Rotation();//旋转操作
void SetBlockMap(int classifier,int style);//旋转过程中重新设置curBlock.Map的值
BOOL GameOver();//判断游戏结束
void DrawGameOver();
void LevelControl();//等级控制
void DrawGameOver(HWND hwnd);
/************************************END*/

//主函数
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd )
{
	HWND	hwnd;
	MSG		msg;
	WNDCLASS		wndClass;
	HICON hIcon;
	wndClass.style					= CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra			= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.hCursor				= LoadCursor(NULL,IDC_ARROW);
	wndClass.hIcon					= LoadIcon(NULL,MAKEINTRESOURCE(IDI_APPLICATION));//修改应用程序图标时使用
	wndClass.hInstance			= hInstance;
	wndClass.lpfnWndProc	=	windProc;
	wndClass.lpszClassName	=  szAppName;
	wndClass.lpszMenuName	=	MAKEINTRESOURCE(IDR_TERIS);

	//注册窗口类
	if(!RegisterClass(&wndClass))
	{
		MessageBox(NULL,TEXT("This program requires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,szAppName,
		WS_OVERLAPPEDWINDOW& ~WS_MAXIMIZEBOX &~WS_MINIMIZEBOX&~WS_THICKFRAME,//去除最大化最小化和缩放功能
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		LoadMenu(hInstance,MAKEINTRESOURCE(IDR_TERIS)),
		hInstance,
		NULL);

	//修改窗口左上角的图标

	hIcon = LoadIcon((HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE) ,MAKEINTRESOURCE(IDI_APP));
	SendMessage(hwnd, WM_SETICON, TRUE,  (LPARAM)hIcon);
	ShowWindow(hwnd,nShowCmd);
	UpdateWindow(hwnd);
	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK  windProc(HWND	hwnd,UINT		Message,WPARAM		wParam,LPARAM		lParam)
{
	int result;
	HINSTANCE hInstance;
	static HDC hdc;
	static RECT rectClient;
	static PAINTSTRUCT ps;
	static HBRUSH hBrush;
	static int lines;
	hBrush = CreateSolidBrush(RGB(0,0,0));//用黑色填充

	switch (Message)
	{
	case WM_CREATE:
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		SetClientRect(hwnd,xBricks*24,yBricks*24);//设置客户区大小
		srand((unsigned)time(NULL));
		hBitMap[0] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BRICKWALL));//砖墙
		hBitMap[1] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BLOCKCOLOR));//小方块
		hBitMap[2] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SCOREANDLEVEL));//等级和分数的位图
		hBitMap[3] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_NUMBER));//数字位图，用于绘制分数和等级
		hBitMap[4] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMEOVER));//游戏结束的图片
		hBitMap[5] =  LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_INTRODUCTION));//游戏说明图片
		gameState = 0;
		memset(&gameBoard.Map,0,sizeof(gameBoard.Map));
		memset(&gameBoard.Colour,0,sizeof(gameBoard.Colour));
		gameBoard.top = 21;
		ProduceBlock(&curBlock);
		ProduceBlock(&nextBlock);
		break;
	case WM_SIZE:
		GetClientRect(hwnd,&rectClient);

		rectGame.left = rectClient.left+24;
		rectGame.right = rectGame.left+13*24;
		rectGame.top = rectClient.top+24;
		rectGame.bottom = rectClient.bottom-24;

		rectNext.left = rectClient.right-5*24;
		rectNext.top = rectClient.top+24;
		rectNext.bottom = rectNext.top+4*24;
		rectNext.right = rectClient.right-24;

		rectDownBlock.left = rectGame.left+5*24;
		rectDownBlock.top = rectGame.top-24;
		rectDownBlock.right = rectDownBlock.left+4*24;
		rectDownBlock.bottom = rectDownBlock.top+4*24;

		rectScoreAndLevel.left = rectNext.left;
		rectScoreAndLevel.top = rectNext.bottom+24;
		rectScoreAndLevel.right = rectNext.right;
		rectScoreAndLevel.bottom = rectScoreAndLevel.top+9*24;

		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case ID_ABOUT:
			DialogBox(NULL,MAKEINTRESOURCE(IDD_ABOUT),NULL,AboutProc);
			break;
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:			//按esc键退出
			result = MessageBox(hwnd,TEXT("确定要退出吗?"),TEXT("Warning"),MB_YESNO | MB_ICONWARNING);
			switch (result)
			{
			case IDYES:
				DestroyWindow(hwnd);
				KillTimer(hwnd,ID_GAMETIMER);
				break;
			case IDNO:
				break;
			}
			break;
		case VK_SPACE:
			Pause = !Pause;
			if(Pause)
				KillTimer(hwnd,ID_GAMETIMER);
			else
			{
				SetTimer(hwnd,ID_GAMETIMER,gameSpeed,NULL);
				if(gameState ==2)
				{
					memset(&gameBoard.Map,0,sizeof(gameBoard.Map));
					memset(&gameBoard.Colour,0,sizeof(gameBoard.Colour));
					gameBoard.top = 21;
					ProduceBlock(&curBlock);
					ProduceBlock(&nextBlock);
					gameSpeed = 200;
					gameState = 1;
					Score = 0;
					Level = 1;
				}
			}
			break;
		case VK_DOWN://加速下降
			if(!Pause)
			{
				if(!TouchDown())
				{
					curBlock.rectBlock.left =  curBlock.rectBlock.left;
					curBlock.rectBlock.top += 24;
					curBlock.rectBlock.right = curBlock.rectBlock.right;
					curBlock.rectBlock.bottom = curBlock.rectBlock.bottom+24;
				}
				/*else 
				{
					GameBoardValue();
					lines = FullLines(Lines);
					if(lines)//如果有满行的话进行消行处理
						DeleteFull(lines);
					LevelControl();
					curBlock = nextBlock;
					ProduceBlock(&nextBlock);
					gameBoard.top = FindTop();
					if(GameOver())
					{
						Pause = TRUE;
						KillTimer(hwnd,ID_GAMETIMER);
						gameState = 2;
					}
				}*/
			}
			break;
		case VK_LEFT://左移
			if(!LeftShift()&&!Pause)
			{
				curBlock.rectBlock.left -=  24;
				curBlock.rectBlock.top = curBlock.rectBlock.top;
				curBlock.rectBlock.right = curBlock.rectBlock.right - 24;
				curBlock.rectBlock.bottom = curBlock.rectBlock.bottom;
			}
			break;
		case VK_RIGHT://右移
			if(!RightShift()&&!Pause)
			{
				curBlock.rectBlock.left +=  24;
				curBlock.rectBlock.top = curBlock.rectBlock.top;
				curBlock.rectBlock.right = curBlock.rectBlock.right + 24;
				curBlock.rectBlock.bottom = curBlock.rectBlock.bottom;
			}		
		
			break;
		case VK_UP://旋转变形
			if(Up()&&!Pause)
			{
				Rotation();
			}
			break;
		}
		InvalidateRect(hwnd,&rectGame,FALSE);
		InvalidateRect(hwnd,&rectNext,FALSE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd,&ps);
		FillRect(hdc,&rectScoreAndLevel,hBrush);
		DrawScoreAndLevel(hwnd);
		DrawWall(hwnd,hdc,0,rectClient);
		if(gameState !=2)
		{
			FillRect(hdc,&rectGame,hBrush);	
			DrawBrick(hwnd,hdc,curBlock,curBlock.rectBlock,hBitMap,1);
			DrawGameRect(hwnd,hdc,rectGame,hBitMap,1);
		}	
		FillRect(hdc,&rectNext,hBrush);
		DrawBrick(hwnd,hdc,nextBlock,rectNext,hBitMap,1);
		if(gameState == 2)
		{
			FillRect(hdc,&rectGame,hBrush);
			DrawGameOver(hwnd);
		}
		EndPaint(hwnd,&ps);
		break;
	case WM_TIMER:
		if(!TouchDown())
		{
			curBlock.rectBlock.left =  curBlock.rectBlock.left;
			curBlock.rectBlock.top += 24;
			curBlock.rectBlock.right = curBlock.rectBlock.right;
			curBlock.rectBlock.bottom = curBlock.rectBlock.bottom+24;
		}
		else 
		{
			GameBoardValue();
			gameBoard.top =FindTop();
			lines = FullLines();
			if(lines)//如果有满行的话进行消行处理即lines不等于0的
				DeleteFull(lines);
			gameBoard.top =FindTop();
			LevelControl();
			curBlock = nextBlock;
			ProduceBlock(&nextBlock);
			if(GameOver())
			{
				Pause = TRUE;
				gameState = 2;
				KillTimer(hwnd,ID_GAMETIMER);
			}
		}
			
		InvalidateRect(hwnd,&rectGame,FALSE);
		InvalidateRect(hwnd,&rectNext,FALSE);
		break;
	case WM_CLOSE:
		result = MessageBox(hwnd,TEXT("确定要退出吗?"),TEXT("Warning"),MB_YESNO | MB_ICONWARNING);
		switch (result)
		{
		case IDYES:
			DestroyWindow(hwnd);
			break;
		case IDNO:
			return 0;
		default:return DefWindowProc(hwnd,Message,wParam,lParam);
		}
		break;
	case WM_DESTROY:
		KillTimer(hwnd,ID_GAMETIMER);
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd,Message,wParam,lParam);
}

/*************根据需要设置客户区的大小**************
* 参数:width，height分别表示想要设置的客户区的大小
* 
* 
***************************************************/
void SetClientRect(HWND	hwnd,int width,int height)
{
	RECT rectWindow;
	RECT rectClient;
	int windowSizeX,windowSizeY;

	GetWindowRect(hwnd,&rectWindow);
	GetClientRect(hwnd,&rectClient);

	windowSizeX = rectWindow.right-rectWindow.left-(rectClient.right-rectClient.left)+width;
	windowSizeY = rectWindow.bottom-rectWindow.top-(rectClient.bottom-rectClient.top)+height;

	MoveWindow(hwnd,
							GetSystemMetrics(SM_CXSCREEN)/2-windowSizeX/2,
							GetSystemMetrics(SM_CYSCREEN)/2-windowSizeY/2-100,
							windowSizeX,
							windowSizeY,
							FALSE);

}

/**************绘制砖墙******************
*
*
***************************************/
void DrawWall(HWND hwnd,HDC hdc,int index,RECT rectClient)
{
	BITMAP bitMap;
	HDC hdcDes;
	int i;
	int x,y;
	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[index]);
	GetObject(hBitMap[index],sizeof(BITMAP),&bitMap);
	x = bitMap.bmWidth;
	y = bitMap.bmHeight;

	for(i = 0;i<xBricks;i++)//绘制上下砖墙
	{
		BitBlt(hdc,rectClient.left+i*x,rectClient.top,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.left+i*x,rectClient.bottom-y,x,y,hdcDes,0,0,SRCCOPY);
	}
	for(i = 0;i<4;i++)
	{
		BitBlt(hdc,rectClient.right-5*x+i*x,rectClient.top+5*x,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.right-5*x+i*x,rectClient.top+10*x,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.right-5*x+i*x,rectClient.top+15*x,x,y,hdcDes,0,0,SRCCOPY);
	}
	
	for(i =1;i<yBricks-1;i++)//绘制左右砖墙
	{
		BitBlt(hdc,rectClient.left,rectClient.top+i*y,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.right-x,rectClient.top+i*y,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.left+y+13*x,rectClient.top+i*y,x,y,hdcDes,0,0,SRCCOPY);
	}
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}

/****************产生一个方块************
*
*
****************************************/
void ProduceBlock(struct BLOCK *block)
{
	int i;
	while(TRUE)
	{
		block->Classifier = rand()%7;
		if(curBlock.Classifier !=nextBlock.Classifier)break;
	}
	block->Style = 0;
	block->rectBlock = rectDownBlock;
	memset(&block->Map,0,sizeof(block->Map));

	switch (block->Classifier)
	{
	case 0:	//长条 默认为横条
		block->Colour = 0;

		for(i =0;i<4;i++)
			block->Map[1][i] = 1;
		break;
	case 1:	//L型,默认为L顺时针旋转90°的形状
		block->Colour = 1;

		for(i =0;i<3;i++)
			block->Map[1][i] = 1;
		block->Map[2][0] = 1;
		break;
	case 2:	//』型默认情况下为』旋转270°的形状
		block->Colour = 1;

		for(i =0;i<3;i++)
			block->Map[1][i] = 1;
		block->Map[2][2] = 1;
		break;
	case 3:	//Z型默认情况下为Z型
		block->Colour = 2;

		block->Map[1][0] = 1;
		block->Map[1][1] = 1;
		block->Map[2][1] = 1;
		block->Map[2][2] = 1;
		break;
	case 4:	//反Z型默认情况下为反Z型
		block->Colour = 2;

		block->Map[1][1] = 1;
		block->Map[1][2] = 1;
		block->Map[2][0] = 1;
		block->Map[2][1] = 1;
		break;
	case 5:	//凸型默认情况下为倒凸型
		block->Colour = 3;

		for(i =0;i<3;i++)
			block->Map[1][i] = 1;
		block->Map[2][1] = 1;
		break;
	case 6:	//田型
		block->Colour = 4;
		
		block->Map[1][1] = 1;
		block->Map[1][2] = 1;
		block->Map[2][1] = 1;
		block->Map[2][2] = 1;
		break;
	default:
		break;
	}
}

/**************绘制方块*************
* 用于绘制下落区域的方块以及在提示区的方块
*
***********************************/
void DrawBrick(HWND hwnd,HDC hdc,struct BLOCK block,RECT rect,HBITMAP hBitMap[],int index)
{
	BITMAP bitMap;
	HDC hdcDes;
	int i,j;

	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[index]);
	GetObject(hBitMap[index],sizeof(BITMAP),&bitMap);
	for(i =0;i<4;i++)
		for(j =0;j<4;j++)
			if(block.Map[i][j] == 1)
			{
				BitBlt(hdc,rect.left+j*24,rect.top+i*24,bitMap.bmWidth,bitMap.bmHeight/6,hdcDes,0,block.Colour*24,SRCCOPY);
			}
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}

/*************绘制游戏区的已经放好的方块**********
* 
*
***********************************************/
void DrawGameRect(HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int index)
{
	BITMAP bitMap;
	HDC hdcDes;
	int i,j;

	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[index]);
	GetObject(hBitMap[index],sizeof(BITMAP),&bitMap);
	for(i =21;i>gameBoard.top;i--)
	{
		for(j =0;j<13;j++)
			if(gameBoard.Map[i][j])
			{
				BitBlt(hdc,rect.left+j*24,rect.top+i*24,bitMap.bmWidth,bitMap.bmHeight/6,hdcDes,0,gameBoard.Colour[i][j]*24,SRCCOPY);
			}
	}
			ReleaseDC(hwnd,hdc);
			DeleteDC(hdcDes);
}
/****************绘制分数和等级********
*
*************************************/
void DrawScoreAndLevel(HWND hwnd)
{
		HDC hdc;
		HDC hdcDes;
		BITMAP bitMap;
		int i,j;
		int temp;
		temp = Score;
		j = 100;
		hdc = GetDC(hwnd);
		hdcDes = CreateCompatibleDC(hdc);
		SelectObject(hdcDes,hBitMap[2]);
		GetObject(hBitMap[2],sizeof(BITMAP),&bitMap);
		//绘制分数和等级字样
		BitBlt(hdc,rectScoreAndLevel.left,rectScoreAndLevel.top,bitMap.bmWidth,bitMap.bmHeight/2,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectScoreAndLevel.left,rectScoreAndLevel.top+120,bitMap.bmWidth,bitMap.bmHeight/2,hdcDes,0,48,SRCCOPY);

		SelectObject(hdcDes,hBitMap[3]);
		GetObject(hBitMap[3],sizeof(BITMAP),&bitMap);
		//绘制分数和游戏等级
		for(i=0;i<3;i++)
		{
			BitBlt(hdc,rectScoreAndLevel.left+i*32,rectScoreAndLevel.top+48,32,48,hdcDes,32*(temp/j),0,SRCCOPY);
			temp = temp%j;
			j = j/10;
		}
		BitBlt(hdc,rectScoreAndLevel.left+32,rectScoreAndLevel.top+168,32,48,hdcDes,Level*32,0,SRCCOPY);

		//绘制游戏说明文字
		SelectObject(hdcDes,hBitMap[5]);
		GetObject(hBitMap[5],sizeof(BITMAP),&bitMap);
		BitBlt(hdc,rectScoreAndLevel.left,rectScoreAndLevel.bottom+24,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
		ReleaseDC(hwnd,hdc);
		DeleteDC(hdcDes);
}
/************判断是否到达底部**********
* 返回TRUE表示到达底部 FALSE表示可以继续向下移动
*
*************************************/
BOOL TouchDown()
{
	int i,j;
	POINT point;
	int pointy;

	//第一种情况:游戏区的最后一行没有方块，此时判断方块的最下面一块是否碰到了底部边缘
	//其次是判断当前的方块的下方的方块是否被占用
	for(i =3;i>=0;i--)
		for(j =0;j<4;j++)
			if(curBlock.Map[i][j])//首先找到最下面的方块
			{
				pointy = curBlock.rectBlock.top+(i+1)*24;//计算最下面的方块的下边缘的坐标
				if(pointy>=rectGame.bottom)
					return TRUE;//若下边缘的坐标大于等于游戏区的边缘返回TRUE	
				else //否则判断其下面的方块是否被占用了
				{
					point  = Translate(curBlock.rectBlock,i,j);
					if(gameBoard.Map[point.y+1][point.x])
						return TRUE;	
				}
			}
	return FALSE;
}

/*********将rectDownBlock中的i,j转换为gameBoard中的i,j******
* 
* 
******************************************************/
POINT Translate(RECT rect,int x,int y)
{
	POINT location;
	int pointx,pointy;

	pointx = rect.left+y*24;
	pointy = rect.top+ x*24;

	location.x = (pointx - rectGame.left)/24;
	location.y = (pointy - rectGame.top)/24;

	return location;
}

/******************左移****************************
* 判断是否可以继续左移
* 是否碰到墙或者障碍物
* 若碰到墙或者障碍则返回TRUE
**************************************************/

BOOL LeftShift()
{
	int i,j;
	int pointx;
	//POINT point;

	for(i =0;i<4;i++)
	{
		pointx = curBlock.rectBlock.left+i*24;
		for(j =3;j>=0;j--)
			if(curBlock.Map[j][i])
				if(pointx<=rectGame.left)return TRUE;
				//point  = Translate(curBlock.rectBlock,j,i);
				//if(gameBoard.Map[point.y][point.x-1])return TRUE;
	}
		return FALSE;
}

/******************右移****************************
* 判断是否可以继续右移
* 是否碰到墙或者现有的障碍
**************************************************/

BOOL RightShift()
{
	int i,j;
	int pointx;

	for(j =3;j>=0;j--)
		for(i =3;i>=0;i--)	
			if(curBlock.Map[i][j])
			{
				pointx = curBlock.rectBlock.left+(j+1)*24;
				if(pointx>=rectGame.right)return TRUE;
				//point  = Translate(curBlock.rectBlock,i,j);
				//if(gameBoard.Map[point.y][point.x+1])return TRUE;
			}
		return FALSE;
}

/*************判断当前情况是否可以进行变形操作*****
* 以curBlock.rectBlock.Map[1][1]为轴心进行旋转
* 返回false表示不可以变形，返回true表示可以变形
************************************************/

BOOL Up()
{
	switch (curBlock.Classifier)
	{
	case 0://长条情况下不可以变形的条件	
		if(curBlock.rectBlock.left<rectGame.left)return FALSE;
		if(curBlock.rectBlock.right>rectGame.right)return FALSE;
		if(curBlock.rectBlock.bottom>rectGame.bottom)return FALSE;
		break;
	case 1://L型
	case 2://』型	
		if(curBlock.rectBlock.left<rectGame.left)return FALSE;
		if(curBlock.rectBlock.right-rectGame.right>24)return FALSE;
		break;
	case 3://Z型
		if(curBlock.rectBlock.right-rectGame.right>24)return FALSE;
		break;
	case 4://反Z型
		if(rectGame.left>curBlock.rectBlock.left)return FALSE;
		break;
	case 5:
		if(curBlock.rectBlock.left<rectGame.left)return FALSE;
		if(curBlock.rectBlock.right-rectGame.right>24)return FALSE;
		break;
	}
	return TRUE;
}

/**************根据当前形状来进行旋转变形*********
*
*
************************************************/
void Rotation()
{
	switch (curBlock.Classifier)
	{
	case 0:
	case 3:
	case 4:
		curBlock.Style = (curBlock.Style+1)%2;
		break;
	case 1:
	case 2:
	case 5:
		curBlock.Style = (curBlock.Style+1)%4;
		break;
	}
	SetBlockMap(curBlock.Classifier,curBlock.Style);
}

/***根据当前的形状来重新设置curBlock.Map数组的值****
*
*
*************************************************/
void SetBlockMap(int classifier,int style)
{
	unsigned int Mask;
	int i,j;
	Mask = 128;
	memset(curBlock.Map,0,sizeof(curBlock.Map));

	for(i=0;i<2;i++)
		for(j=0;j<4;j++)
		{
			curBlock.Map[i][j] = (Styles[classifier][style*2] &Mask)/Mask;	
			Mask = Mask/2;
		}

		Mask = 128;
		for(i=2;i<4;i++)
			for(j=0;j<4;j++)
			{
				curBlock.Map[i][j] = (Styles[classifier][style*2+1] &Mask)/Mask;
				Mask = Mask/2;
			}
}

/*******************设置游戏区域的值**************
* 主要是对gameBoard结构体中的Map和colour数组进行设置赋值
*
*************************************************/
void GameBoardValue()
{
	int i,j;
	POINT point;
	for(i =0;i<4;i++)
		for(j =0;j<4;j++)
			if(curBlock.Map[i][j])
			{
				point = Translate(curBlock.rectBlock,i,j);
				gameBoard.Map[point.y][point.x] = 1;
				gameBoard.Colour [point.y][point.x] = curBlock.Colour;
			}
}

/*****************找到当前的最高点所在行*********
*
*
**********************************************/
int FindTop()
{
	int i,j;
	int count;
	for(i = 21;i>=0;i--)
	{
		count =0;
		for(j =0;j<13;j++)
			if(gameBoard.Map[i][j] == 0)count++;
		if(count == 13)
			return i;
	}
	return -1;//如果已经最高点已经到达最高处则游戏结束
}

/***************查找当前的所有的满行的行号******
* 返回当前满行的行数
* 同时记录满行的行号
*********************************************/
int FullLines()
{
	int i,j,k;
	int count;
	k=0;
	memset(Lines,0,sizeof(Lines));
	for(i=21;i>gameBoard.top;i--)
	{
		count =0;
		for(j =0;j<13;j++)
		{
			if(gameBoard.Map[i][j] ==1)count++;
		}
		if(count == 13)Lines[k++] = i;//纪录满行的行号
	}
	return k;//此时没有满行的话，返回0
}

/********************消行处理*******************
* 消行的删除过程
*
**********************************************/
void  DeleteFull(int fullLines)
{
	if(fullLines ==1)Score +=1;
	else if(fullLines == 2)Score+=2;
	else if(fullLines == 3)Score += 4;
	else if(fullLines == 4)Score +=6;
	while(fullLines)
	{
		gameBoard.top = FindTop();
		DeleteLine(fullLines-1);
		fullLines--;
	}
}

/*************对最高点到满的一行进行处理**********
* 将满行的值设置为0，同时上面的方块要下落下来
* 
************************************************/
void DeleteLine(int lines)
{
	int i,j;
	
	for(i=0;i<13;i++)
		gameBoard.Map[Lines[lines]][i] =0;
	for(i=Lines[lines]-1;i>gameBoard.top;i--)
		for(j =0;j<13;j++)
			if(gameBoard.Map[i][j])
			{
				gameBoard.Map[i+1][j]=1;
				gameBoard.Colour[i+1][j] = gameBoard.Colour[i][j];
				gameBoard.Map[i][j] = 0;
				gameBoard.Colour[i][j] = 0;
			}
}

/***********判断游戏是否结束**************
* 若最高点的行号为-1说明已经到达顶部，游戏结束
*
****************************************/
BOOL GameOver()
{
	if(gameBoard.top == -1)
		return TRUE;
	return FALSE;
}

/*****************根据分数设置当前等级*******
*
*******************************************/
void LevelControl()
{
	if(Score>10&&Score<30)
	{
		Level = 2;
		gameSpeed = 100;
	}
	else if(Score>=30&&Score<60)
	{
		Level = 3;
		gameSpeed = 50;
	}
	else if(Score>=60)
	{
		Level = 4;
		gameSpeed = 20;
	}
}

//对话框消息处理函数
BOOL CALLBACK AboutProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd,0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void DrawGameOver(HWND hwnd)
{
	HDC hdc;
	HDC hdcDes;
	BITMAP bitMap;
	hdc = GetDC(hwnd);
	hdcDes = CreateCompatibleDC(hdc);
	SelectObject(hdcDes,hBitMap[4]);
	GetObject(hBitMap[4],sizeof(BITMAP),&bitMap);
	BitBlt(hdc,rectGame.left+31,rectGame.top+180,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}