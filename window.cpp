/* window.cpp
 * R.Kubo 2009-2010
 * �q�E�B���h�E�̐���
 */

#include <windows.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include "match.h"
#include "resource.h"
#include "sikou.h"

enum{
	_ID_VALUE,
	ID_VALUE,
	_ID_NODE,
	ID_NODE,
	_ID_NPS,
	ID_NPS,
	_ID_DEPTH,
	ID_DEPTH,
	_ID_OUT_TIME,
	ID_OUT_TIME,
	_ID_IN_TIME,
	ID_IN_TIME,
	_ID_THREADS,
	ID_THREADS,
	_ID_HASH,
	ID_HASH,
	_ID_RESIGN,
	ID_RESIGN,
	ID_PONDER,
	ID_RESULT,
	ID_WIDE,
	ID_INFO,
};

// ������������������A���enum�ƍs��(W)��ς��邱�ƁB
OBJECT Object[] = {
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(0), STATIC_WIDTH, STATIC_HEIGHT,   STATIC_LEFT, STATIC_TOP(0), STATIC_WIDTH, STATIC_HEIGHT, 0, "Value" },
	{ NULL, "EDIT"  , ST_EDIT_R,   EDIT_LEFT,   EDIT_TOP(0),   EDIT_WIDTH,   EDIT_HEIGHT,     EDIT_LEFT,   EDIT_TOP(0),   EDIT_WIDTH,   EDIT_HEIGHT, 0, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(1), STATIC_WIDTH, STATIC_HEIGHT,   STATIC_LEFT, STATIC_TOP(1), STATIC_WIDTH, STATIC_HEIGHT, 0, "Node" },
	{ NULL, "EDIT"  , ST_EDIT_R,   EDIT_LEFT,   EDIT_TOP(1),   EDIT_WIDTH,   EDIT_HEIGHT,     EDIT_LEFT,   EDIT_TOP(1),   EDIT_WIDTH,   EDIT_HEIGHT, 0, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(2), STATIC_WIDTH, STATIC_HEIGHT,   STATIC_LEFT, STATIC_TOP(2), STATIC_WIDTH, STATIC_HEIGHT, 0, "NPS" },
	{ NULL, "EDIT"  , ST_EDIT_R,   EDIT_LEFT,   EDIT_TOP(2),   EDIT_WIDTH,   EDIT_HEIGHT,     EDIT_LEFT,   EDIT_TOP(2),   EDIT_WIDTH,   EDIT_HEIGHT, 0, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(3), STATIC_WIDTH, STATIC_HEIGHT,   STATIC_LEFT, STATIC_TOP(3), STATIC_WIDTH, STATIC_HEIGHT, 0, "Depth" },
	{ NULL, "EDIT"  , ST_EDIT_R,   EDIT_LEFT,   EDIT_TOP(3),   EDIT_WIDTH,   EDIT_HEIGHT,     EDIT_LEFT,   EDIT_TOP(3),   EDIT_WIDTH,   EDIT_HEIGHT, 0, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(4), STATIC_WIDTH, STATIC_HEIGHT,   STATIC_LEFT, STATIC_TOP(4), STATIC_WIDTH, STATIC_HEIGHT, 0, "Time(Used)" },
	{ NULL, "EDIT"  , ST_EDIT_R,   EDIT_LEFT,   EDIT_TOP(4),   EDIT_WIDTH,   EDIT_HEIGHT,     EDIT_LEFT,   EDIT_TOP(4),   EDIT_WIDTH,   EDIT_HEIGHT, 0, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(5), STATIC_WIDTH, STATIC_HEIGHT,   STATIC_LEFT, STATIC_TOP(5), STATIC_WIDTH, STATIC_HEIGHT, 0, "Time(Limit)" },
	{ NULL, "EDIT"  , ST_EDIT  ,   EDIT_LEFT,   EDIT_TOP(5),   EDIT_WIDTH,   EDIT_HEIGHT,     EDIT_LEFT,   EDIT_TOP(5),   EDIT_WIDTH,   EDIT_HEIGHT, 1, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(6), STATIC_WIDTH, STATIC_HEIGHT, STATIC_LEFT_W, STATIC_TOP(0), STATIC_WIDTH, STATIC_HEIGHT, 0, "Threads" },
	{ NULL, "EDIT"  , ST_EDIT  ,   EDIT_LEFT,   EDIT_TOP(6),   EDIT_WIDTH,   EDIT_HEIGHT,   EDIT_LEFT_W,   EDIT_TOP(0),   EDIT_WIDTH,   EDIT_HEIGHT, 1, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(7), STATIC_WIDTH, STATIC_HEIGHT, STATIC_LEFT_W, STATIC_TOP(1), STATIC_WIDTH, STATIC_HEIGHT, 0, "Hash[MB]" },
	{ NULL, "EDIT"  , ST_EDIT  ,   EDIT_LEFT,   EDIT_TOP(7),   EDIT_WIDTH,   EDIT_HEIGHT,   EDIT_LEFT_W,   EDIT_TOP(1),   EDIT_WIDTH,   EDIT_HEIGHT, 1, "" },
	{ NULL, "STATIC", ST_STATIC, STATIC_LEFT, STATIC_TOP(8), STATIC_WIDTH, STATIC_HEIGHT, STATIC_LEFT_W, STATIC_TOP(2), STATIC_WIDTH, STATIC_HEIGHT, 0, "Resign" },
	{ NULL, "EDIT"  , ST_EDIT  ,   EDIT_LEFT,   EDIT_TOP(8),   EDIT_WIDTH,   EDIT_HEIGHT,   EDIT_LEFT_W,   EDIT_TOP(2),   EDIT_WIDTH,   EDIT_HEIGHT, 1, "" },
	{ NULL, "BUTTON", ST_CHECK , STATIC_LEFT,   EDIT_TOP(9),  CHECK_WIDTH,   EDIT_HEIGHT, STATIC_LEFT_W,   EDIT_TOP(3),  CHECK_WIDTH,   EDIT_HEIGHT, 1, "Ponder" },
	{ NULL, "BUTTON", ST_CHECK ,  CHECK_LEFT,   EDIT_TOP(9),  CHECK_WIDTH,   EDIT_HEIGHT, STATIC_LEFT_W,   EDIT_TOP(4),  CHECK_WIDTH,   EDIT_HEIGHT, 0, "Result" },
	{ NULL, "BUTTON", ST_CHECK , STATIC_LEFT,  EDIT_TOP(10),  CHECK_WIDTH,   EDIT_HEIGHT, STATIC_LEFT_W,   EDIT_TOP(5),  CHECK_WIDTH,   EDIT_HEIGHT, 0, "Wide" },
	{ NULL, "LISTBOX",ST_LIST  , STATIC_LEFT,  EDIT_TOP(11),  LEDIT_WIDTH,  LEDIT_HEIGHT,   STATIC_LEFT,   EDIT_TOP(6),LEDIT_WIDTH_W,LEDIT_HEIGHT_W, 0,  NULL },
};
#define W				11 // �s��

// �E�B���h�E�̃n���h��
HWND hSikouWnd = NULL;

extern int limit;
extern int thread_num;
extern int resign_value;
extern int hash_mbytes;
extern BOOL bResult;
extern BOOL bPonder;
extern BOOL bWide;

void search_inf0();

void CreateChildWindow( HWND hwnd );
LRESULT CALLBACK SikouWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
void DisplayString( const char* str = NULL );

BOOL bWindowCreated = FALSE;

unsigned __stdcall CreateMainWindow( LPVOID p ){
/**************************************************************
���C���E�B���h�E�̐���
**************************************************************/
	WNDCLASS winc;
	HINSTANCE hInst = (HINSTANCE)p;
	MSG msg;
	int ret;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = SikouWndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInst;
	winc.hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_ICON01 ) );
	winc.hCursor = LoadCursor( NULL, IDC_ARROW );
	winc.hbrBackground = CreateSolidBrush( RGB( 236, 233, 216 ) );
	winc.lpszMenuName = NULL;
	winc.lpszClassName = "Sikou";

	if( !RegisterClass( &winc ) )
		return (DWORD)0;

	hSikouWnd = CreateWindow(
		"Sikou", "Sunfish SIKOU.DLL",
		WS_OVERLAPPED | WS_CAPTION |
		WS_MINIMIZEBOX | WS_VISIBLE |
		WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WINDOW_SIZE_X    + GetSystemMetrics( SM_CXFIXEDFRAME ) * 2 ,
		WINDOW_SIZE_Y(W) + GetSystemMetrics( SM_CXFIXEDFRAME ) * 2
		+ GetSystemMetrics( SM_CYCAPTION ),
		NULL, NULL, hInst, NULL );

	while( ret = GetMessage( &msg, NULL, 0, 0 ) ){
		if( ret == -1 ){ break; }
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return (DWORD)1;
}

void DestroyMainWindow(){
/**************************************************************
���C���E�B���h�E�̔j��
**************************************************************/
	if( hSikouWnd != NULL ){
		if( IsWindow( hSikouWnd ) ){
			SendMessage( hSikouWnd, WM_MYCLOSE, 0, 0L );
		}
		hSikouWnd = NULL;
	}
}

void CreateChildWindow( HWND hWnd ){
/**************************************************************
�q�E�B���h�E�̐���
**************************************************************/
	HFONT hFont;
	HINSTANCE hInst = (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE );
	HWND hObj;
	char buf[16];
	int i;

	//�R���g���[���̔z�u�ƃt�H���g�̐ݒ�
	hFont = CreateFont( 
		FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		VARIABLE_PITCH | FF_ROMAN, "�S�V�b�N" );

	// �E�B���h�E����
	for( i = 0 ; i < sizeof(Object) / sizeof(OBJECT) ; i++ ){
		hObj = CreateWindow(
			Object[i].ClassName, Object[i].DefaultString, Object[i].style,
			Object[i].left, Object[i].top, Object[i].width, Object[i].height,
			hWnd, (HMENU)i, hInst, NULL
		);
		SendMessage( hObj, WM_SETFONT, (WPARAM)hFont, TRUE );
		Object[i].hWnd = hObj;
	}

	// ������
	sprintf( buf, "%d", limit );
	SetWindowText( Object[ID_IN_TIME].hWnd, buf );
	sprintf( buf, "%d", thread_num );
	SetWindowText( Object[ID_THREADS].hWnd, buf );
	sprintf( buf, "%d", resign_value );
	SetWindowText( Object[ID_RESIGN].hWnd, buf );
	sprintf( buf, "%d", hash_mbytes );
	SetWindowText( Object[ID_HASH].hWnd, buf );
	SendMessage( Object[ID_PONDER].hWnd, BM_SETCHECK, (WPARAM)bPonder, 0 );
	SendMessage( Object[ID_RESULT].hWnd, BM_SETCHECK, (WPARAM)bResult, 0 );
	SendMessage( Object[ID_WIDE].hWnd, BM_SETCHECK, (WPARAM)bWide, 0 );
	ShowWindow( Object[ID_INFO].hWnd, bResult );
}

void SetWideWindowSize( HWND hWnd ){
/**************************************************************
���C�h�\��
**************************************************************/
	int i;
	RECT rect;
	int wide_x = ( bWide != FALSE ? WINDOW_WIDE_X : 0 );
	int result_y = ( bResult != FALSE ? 0 :
		( bWide != FALSE ? Object[ID_INFO].heightW : Object[ID_INFO].height ) );

	// �T�C�Y�ύX(���C���E�B���h�E)
	GetWindowRect( hWnd, &rect );
	MoveWindow( hWnd, rect.left, rect.top,
		WINDOW_SIZE_X + wide_x + GetSystemMetrics( SM_CXFIXEDFRAME ) * 2,
		WINDOW_SIZE_Y(W)       + GetSystemMetrics( SM_CXFIXEDFRAME ) * 2
		+ GetSystemMetrics( SM_CYCAPTION ) - result_y,
		TRUE );

	// �T�C�Y�ύX
	for( i = 0 ; i < sizeof(Object) / sizeof(OBJECT) ; i++ ){
		if( bWide ){
			MoveWindow( Object[i].hWnd, Object[i].leftW, Object[i].topW,
				Object[i].widthW, Object[i].heightW, TRUE );
		}
		else{
			MoveWindow( Object[i].hWnd, Object[i].left, Object[i].top,
				Object[i].width, Object[i].height, TRUE );
		}
	}
}

LRESULT CALLBACK SikouWndProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp ){
/**************************************************************
���b�Z�[�W�̏���
**************************************************************/
	HMENU hMenu;

	switch( msg ){

	case WM_CREATE:
		// Close�{�^���𖳌���
		hMenu = GetSystemMenu( hWnd, FALSE );
		RemoveMenu( hMenu, SC_CLOSE, FALSE );

		// �e��E�B���h�E����
		CreateChildWindow( hWnd );

		SetWideWindowSize( hWnd );

		bWindowCreated = TRUE;

		return 0;

	case WM_COMMAND:
		switch( LOWORD(wp) ){
		case ID_IN_TIME:
			if( HIWORD( wp ) == EN_CHANGE ){
				char str[32];
				GetWindowText( Object[ID_IN_TIME].hWnd, str, sizeof(str) );
				if( str[0] != '\0' ){
					limit = strtol( str, NULL, 10 );
				}
			}
			break;

		case ID_THREADS:
			if( HIWORD( wp ) == EN_CHANGE ){
				char str[32];
				GetWindowText( Object[ID_THREADS].hWnd, str, sizeof(str) );
				if( str[0] != '\0' ){
					thread_num = strtol( str, NULL, 10 );
				}
			}
			break;

		case ID_RESIGN:
			if( HIWORD( wp ) == EN_CHANGE ){
				char str[32];
				GetWindowText( Object[ID_RESIGN].hWnd, str, sizeof(str) );
				if( str[0] != '\0' ){
					resign_value = strtol( str, NULL, 10 );
				}
			}
			break;

		case ID_HASH:
			if( HIWORD( wp ) == EN_CHANGE ){
				char str[32];
				GetWindowText( Object[ID_HASH].hWnd, str, sizeof(str) );
				if( str[0] != '\0' ){
					hash_mbytes = strtol( str, NULL, 10 );
				}
			}
			break;

		case ID_PONDER:
			bPonder = ( BST_CHECKED == SendMessage( Object[ID_PONDER].hWnd, BM_GETCHECK, 0, 0 ) );
			break;

		case ID_RESULT:
			bResult = ( BST_CHECKED == SendMessage( Object[ID_RESULT].hWnd, BM_GETCHECK, 0, 0 ) );
			ShowWindow( Object[ID_INFO].hWnd, bResult );
			SetWideWindowSize( hWnd );
			break;

		case ID_WIDE:
			// ���C�h�\�� / �ʏ�\��
			bWide = ( BST_CHECKED == SendMessage( Object[ID_WIDE].hWnd, BM_GETCHECK, 0, 0 ) );
			SetWideWindowSize( hWnd );
			break;
		}

		return 0;

	case WM_MYCLOSE:
		DestroyWindow( hWnd );

		return 0;

	case WM_CLOSE:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);

		return 0;
	}

	return DefWindowProc( hWnd, msg, wp, lp );
}

void SetPlayMode( BOOL bEnable ){
/**************************************************************
�΋ǒ��A�ҋ@���̐؂�ւ�
**************************************************************/
	int i;

	for( i = 0 ; i < sizeof(Object) / sizeof(OBJECT) ; i++ ){
		if( Object[i].change )
			EnableWindow( Object[i].hWnd, bEnable );
	}
}

void InitProgress(){
/**************************************************************
������
**************************************************************/
	SendMessage( Object[ID_INFO].hWnd, LB_RESETCONTENT, 0, 0 );
}

void AddProgress( const char* str ){
/**************************************************************
1�s�ǉ�
**************************************************************/
	int num;

	if( str != NULL ){
		SendMessage( Object[ID_INFO].hWnd, LB_INSERTSTRING, -1, (LPARAM)str );
		num = SendMessage( Object[ID_INFO].hWnd, LB_GETCOUNT, (WPARAM)0, (LPARAM)0 );
		SendMessage( Object[ID_INFO].hWnd, LB_SETCURSEL, num-1, (LPARAM)0 );
	}
}

void DisplayResult( THINK_INFO *info ){
/**************************************************************
���\��
**************************************************************/
	char str[32];

	wsprintf( str, "%d", (int)info->value );
	SetWindowText( Object[ID_VALUE].hWnd, str );

	wsprintf( str, "%d", (int)info->node );
	SetWindowText( Object[ID_NODE].hWnd, str );

	wsprintf( str, "%d", (int)info->nps );
	SetWindowText( Object[ID_NPS].hWnd, str );

	wsprintf( str, "%d", (int)info->depth );
	SetWindowText( Object[ID_DEPTH].hWnd, str );

	wsprintf( str, "%d", (int)( info->msec / 1000 ) );
	SetWindowText( Object[ID_OUT_TIME].hWnd, str );
}

void DisplayBook( int num, Moves& moves ){
/**************************************************************
��Վ��\��
**************************************************************/
	int i;

	for( i = 0 ; i < num ; i++ ){
		ostringstream str;
		str << Shogi::GetStrMoveSJIS( moves[i] );
		str << "(" << moves[i].value << "%)";
		AddProgress( str.str().c_str() );
	}
}
