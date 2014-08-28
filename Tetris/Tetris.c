#include<windows.h>
#include<Windows.h>
#include"resource.h"
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include "block.h"
#define ID_GAMETIMER 1

/************��Ϸ���ṹ��***************
* ���ڼ�¼��Ϸ��ÿ��С�����״̬����ɫ
**************************************/
struct BOARD
{
	int Map[22][13];
	int Colour[22][13];//С�������ɫ
	int top;//��ǰ�������Ϸ����������λ�õ�����һ�У�������ջ��
}gameBoard;
/**************************************END*/

/**************ȫ�ֱ�����***************/
static TCHAR szAppName[] = TEXT("Tetris--����˹����(��ï��)");
static int xBricks = 20;
static int yBricks = 24;			//�ݺ������Ϲ��ж��ٸ�����(ÿ������Ϊ24*24)
HBITMAP hBitMap[10];		//���ڴ洢λͼ���������
RECT rectGame;					//��Ϸ��
RECT rectNext;						//������ʾ��һ�����ֵķ��������		
RECT rectDownBlock;			//����ķ������ڵľ�������
RECT rectScoreAndLevel;   //������Ϸ�ȼ��ͷ����ľ�������
int gameSpeed = 200;
struct BLOCK curBlock;					//ָ��ǰ�ķ���
struct BLOCK nextBlock;				//ָ����һ���ṹ��
struct BOARD gameBoard;              //������Ϸ����״̬��¼
int Score=0;										//��Ϸ�ĵ÷�
int Level =1;										//��Ϸ�ĵȼ�
BOOL Pause = TRUE;						//������ͣ��Ϸ
int Lines[4];										//��¼��ǰ�����е��к�
int gameState;									//��¼��Ϸ״̬��0:δ��ʼ��1:��Ϸ�У�2:��Ϸ����

unsigned char Styles[][8] ={{0x0F,0x00,0x44,0x44},//������������״
											 {0x0E,0x80,0xC4,0x40,0x2E,0x00,0x44,0x60},//L�͵�4����״
											 {0x0E,0x20,0x44,0xC0,0x8E,0x00,0x64,0x40},//��L�͵�4����״
											 {0x0C,0x60,0x4C,0x80},//Z�͵�������״
											 {0x06,0xC0,0x46,0x20},//��Z�͵�������״
										     {0x0E,0x40,0x4C,0x40,0x4E,0x00,0x46,0x40},//͹�͵�������״
											 {0x06,0x60}};//���͵�һ����״

/***********************************END*/


/**************����������***************/
LRESULT CALLBACK  windProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK AboutProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
void SetClientRect(HWND	hwnd,int width,int height);//���ÿͻ����Ĵ�С
void DrawWall(HWND hwnd,HDC hdc,int index,RECT rectClient);//����שǽ
void ProduceBlock(struct BLOCK *block);
void DrawBrick(HWND hwnd,HDC hdc,struct BLOCK block,RECT rect,HBITMAP hBitMap[],int index);
void DrawGameRect(HWND hwnd,HDC hdc,RECT rect,HBITMAP hBitMap[],int index);
void DrawScoreAndLevel(HWND hwnd);//���Ʒ����͵ȼ�
BOOL TouchDown();//�жϷ����Ƿ񵽴�ײ�
POINT Translate(RECT rect,int x,int y);//��rectDownBlock�е�i,jת��ΪgameBoard�е�i,j
BOOL RightShift();//�ж������Ƿ�����ǽ�����ϰ���
BOOL LeftShift();//�����Ƿ�����ǽ�����ϰ���
BOOL Up();//�ж��Ƿ������ת
void GameBoardValue();//������Ϸ���״̬
int FindTop();//�ҵ���ǰ��ߵ����ڵ���
int FullLines();//���ҵ�ǰ�����������ڵ�λ�ã��洢������a��
void  DeleteFull(int fullLines);//���д���
void DeleteLine(int lines);//����һ�еĴ������
void Rotation();//��ת����
void SetBlockMap(int classifier,int style);//��ת��������������curBlock.Map��ֵ
BOOL GameOver();//�ж���Ϸ����
void DrawGameOver();
void LevelControl();//�ȼ�����
void DrawGameOver(HWND hwnd);
/************************************END*/

//������
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
	wndClass.hIcon					= LoadIcon(NULL,MAKEINTRESOURCE(IDI_APPLICATION));//�޸�Ӧ�ó���ͼ��ʱʹ��
	wndClass.hInstance			= hInstance;
	wndClass.lpfnWndProc	=	windProc;
	wndClass.lpszClassName	=  szAppName;
	wndClass.lpszMenuName	=	MAKEINTRESOURCE(IDR_TERIS);

	//ע�ᴰ����
	if(!RegisterClass(&wndClass))
	{
		MessageBox(NULL,TEXT("This program requires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,szAppName,
		WS_OVERLAPPEDWINDOW& ~WS_MAXIMIZEBOX &~WS_MINIMIZEBOX&~WS_THICKFRAME,//ȥ�������С�������Ź���
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		LoadMenu(hInstance,MAKEINTRESOURCE(IDR_TERIS)),
		hInstance,
		NULL);

	//�޸Ĵ������Ͻǵ�ͼ��

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
	hBrush = CreateSolidBrush(RGB(0,0,0));//�ú�ɫ���

	switch (Message)
	{
	case WM_CREATE:
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		SetClientRect(hwnd,xBricks*24,yBricks*24);//���ÿͻ�����С
		srand((unsigned)time(NULL));
		hBitMap[0] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BRICKWALL));//שǽ
		hBitMap[1] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BLOCKCOLOR));//С����
		hBitMap[2] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_SCOREANDLEVEL));//�ȼ��ͷ�����λͼ
		hBitMap[3] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_NUMBER));//����λͼ�����ڻ��Ʒ����͵ȼ�
		hBitMap[4] = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_GAMEOVER));//��Ϸ������ͼƬ
		hBitMap[5] =  LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_INTRODUCTION));//��Ϸ˵��ͼƬ
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
		case VK_ESCAPE:			//��esc���˳�
			result = MessageBox(hwnd,TEXT("ȷ��Ҫ�˳���?"),TEXT("Warning"),MB_YESNO | MB_ICONWARNING);
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
		case VK_DOWN://�����½�
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
					if(lines)//��������еĻ��������д���
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
		case VK_LEFT://����
			if(!LeftShift()&&!Pause)
			{
				curBlock.rectBlock.left -=  24;
				curBlock.rectBlock.top = curBlock.rectBlock.top;
				curBlock.rectBlock.right = curBlock.rectBlock.right - 24;
				curBlock.rectBlock.bottom = curBlock.rectBlock.bottom;
			}
			break;
		case VK_RIGHT://����
			if(!RightShift()&&!Pause)
			{
				curBlock.rectBlock.left +=  24;
				curBlock.rectBlock.top = curBlock.rectBlock.top;
				curBlock.rectBlock.right = curBlock.rectBlock.right + 24;
				curBlock.rectBlock.bottom = curBlock.rectBlock.bottom;
			}		
		
			break;
		case VK_UP://��ת����
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
			if(lines)//��������еĻ��������д���lines������0��
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
		result = MessageBox(hwnd,TEXT("ȷ��Ҫ�˳���?"),TEXT("Warning"),MB_YESNO | MB_ICONWARNING);
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

/*************������Ҫ���ÿͻ����Ĵ�С**************
* ����:width��height�ֱ��ʾ��Ҫ���õĿͻ����Ĵ�С
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

/**************����שǽ******************
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

	for(i = 0;i<xBricks;i++)//��������שǽ
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
	
	for(i =1;i<yBricks-1;i++)//��������שǽ
	{
		BitBlt(hdc,rectClient.left,rectClient.top+i*y,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.right-x,rectClient.top+i*y,x,y,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectClient.left+y+13*x,rectClient.top+i*y,x,y,hdcDes,0,0,SRCCOPY);
	}
	ReleaseDC(hwnd,hdc);
	DeleteDC(hdcDes);
}

/****************����һ������************
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
	case 0:	//���� Ĭ��Ϊ����
		block->Colour = 0;

		for(i =0;i<4;i++)
			block->Map[1][i] = 1;
		break;
	case 1:	//L��,Ĭ��ΪL˳ʱ����ת90�����״
		block->Colour = 1;

		for(i =0;i<3;i++)
			block->Map[1][i] = 1;
		block->Map[2][0] = 1;
		break;
	case 2:	//����Ĭ�������Ϊ����ת270�����״
		block->Colour = 1;

		for(i =0;i<3;i++)
			block->Map[1][i] = 1;
		block->Map[2][2] = 1;
		break;
	case 3:	//Z��Ĭ�������ΪZ��
		block->Colour = 2;

		block->Map[1][0] = 1;
		block->Map[1][1] = 1;
		block->Map[2][1] = 1;
		block->Map[2][2] = 1;
		break;
	case 4:	//��Z��Ĭ�������Ϊ��Z��
		block->Colour = 2;

		block->Map[1][1] = 1;
		block->Map[1][2] = 1;
		block->Map[2][0] = 1;
		block->Map[2][1] = 1;
		break;
	case 5:	//͹��Ĭ�������Ϊ��͹��
		block->Colour = 3;

		for(i =0;i<3;i++)
			block->Map[1][i] = 1;
		block->Map[2][1] = 1;
		break;
	case 6:	//����
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

/**************���Ʒ���*************
* ���ڻ�����������ķ����Լ�����ʾ���ķ���
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

/*************������Ϸ�����Ѿ��źõķ���**********
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
/****************���Ʒ����͵ȼ�********
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
		//���Ʒ����͵ȼ�����
		BitBlt(hdc,rectScoreAndLevel.left,rectScoreAndLevel.top,bitMap.bmWidth,bitMap.bmHeight/2,hdcDes,0,0,SRCCOPY);
		BitBlt(hdc,rectScoreAndLevel.left,rectScoreAndLevel.top+120,bitMap.bmWidth,bitMap.bmHeight/2,hdcDes,0,48,SRCCOPY);

		SelectObject(hdcDes,hBitMap[3]);
		GetObject(hBitMap[3],sizeof(BITMAP),&bitMap);
		//���Ʒ�������Ϸ�ȼ�
		for(i=0;i<3;i++)
		{
			BitBlt(hdc,rectScoreAndLevel.left+i*32,rectScoreAndLevel.top+48,32,48,hdcDes,32*(temp/j),0,SRCCOPY);
			temp = temp%j;
			j = j/10;
		}
		BitBlt(hdc,rectScoreAndLevel.left+32,rectScoreAndLevel.top+168,32,48,hdcDes,Level*32,0,SRCCOPY);

		//������Ϸ˵������
		SelectObject(hdcDes,hBitMap[5]);
		GetObject(hBitMap[5],sizeof(BITMAP),&bitMap);
		BitBlt(hdc,rectScoreAndLevel.left,rectScoreAndLevel.bottom+24,bitMap.bmWidth,bitMap.bmHeight,hdcDes,0,0,SRCCOPY);
		ReleaseDC(hwnd,hdc);
		DeleteDC(hdcDes);
}
/************�ж��Ƿ񵽴�ײ�**********
* ����TRUE��ʾ����ײ� FALSE��ʾ���Լ��������ƶ�
*
*************************************/
BOOL TouchDown()
{
	int i,j;
	POINT point;
	int pointy;

	//��һ�����:��Ϸ�������һ��û�з��飬��ʱ�жϷ����������һ���Ƿ������˵ײ���Ե
	//������жϵ�ǰ�ķ�����·��ķ����Ƿ�ռ��
	for(i =3;i>=0;i--)
		for(j =0;j<4;j++)
			if(curBlock.Map[i][j])//�����ҵ�������ķ���
			{
				pointy = curBlock.rectBlock.top+(i+1)*24;//����������ķ�����±�Ե������
				if(pointy>=rectGame.bottom)
					return TRUE;//���±�Ե��������ڵ�����Ϸ���ı�Ե����TRUE	
				else //�����ж�������ķ����Ƿ�ռ����
				{
					point  = Translate(curBlock.rectBlock,i,j);
					if(gameBoard.Map[point.y+1][point.x])
						return TRUE;	
				}
			}
	return FALSE;
}

/*********��rectDownBlock�е�i,jת��ΪgameBoard�е�i,j******
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

/******************����****************************
* �ж��Ƿ���Լ�������
* �Ƿ�����ǽ�����ϰ���
* ������ǽ�����ϰ��򷵻�TRUE
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

/******************����****************************
* �ж��Ƿ���Լ�������
* �Ƿ�����ǽ�������е��ϰ�
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

/*************�жϵ�ǰ����Ƿ���Խ��б��β���*****
* ��curBlock.rectBlock.Map[1][1]Ϊ���Ľ�����ת
* ����false��ʾ�����Ա��Σ�����true��ʾ���Ա���
************************************************/

BOOL Up()
{
	switch (curBlock.Classifier)
	{
	case 0://��������²����Ա��ε�����	
		if(curBlock.rectBlock.left<rectGame.left)return FALSE;
		if(curBlock.rectBlock.right>rectGame.right)return FALSE;
		if(curBlock.rectBlock.bottom>rectGame.bottom)return FALSE;
		break;
	case 1://L��
	case 2://����	
		if(curBlock.rectBlock.left<rectGame.left)return FALSE;
		if(curBlock.rectBlock.right-rectGame.right>24)return FALSE;
		break;
	case 3://Z��
		if(curBlock.rectBlock.right-rectGame.right>24)return FALSE;
		break;
	case 4://��Z��
		if(rectGame.left>curBlock.rectBlock.left)return FALSE;
		break;
	case 5:
		if(curBlock.rectBlock.left<rectGame.left)return FALSE;
		if(curBlock.rectBlock.right-rectGame.right>24)return FALSE;
		break;
	}
	return TRUE;
}

/**************���ݵ�ǰ��״��������ת����*********
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

/***���ݵ�ǰ����״����������curBlock.Map�����ֵ****
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

/*******************������Ϸ�����ֵ**************
* ��Ҫ�Ƕ�gameBoard�ṹ���е�Map��colour����������ø�ֵ
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

/*****************�ҵ���ǰ����ߵ�������*********
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
	return -1;//����Ѿ���ߵ��Ѿ�������ߴ�����Ϸ����
}

/***************���ҵ�ǰ�����е����е��к�******
* ���ص�ǰ���е�����
* ͬʱ��¼���е��к�
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
		if(count == 13)Lines[k++] = i;//��¼���е��к�
	}
	return k;//��ʱû�����еĻ�������0
}

/********************���д���*******************
* ���е�ɾ������
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

/*************����ߵ㵽����һ�н��д���**********
* �����е�ֵ����Ϊ0��ͬʱ����ķ���Ҫ��������
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

/***********�ж���Ϸ�Ƿ����**************
* ����ߵ���к�Ϊ-1˵���Ѿ����ﶥ������Ϸ����
*
****************************************/
BOOL GameOver()
{
	if(gameBoard.top == -1)
		return TRUE;
	return FALSE;
}

/*****************���ݷ������õ�ǰ�ȼ�*******
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

//�Ի�����Ϣ������
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