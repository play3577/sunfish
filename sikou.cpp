/* sikou.cpp
 * R.Kubo 2009-2010
 * �����̎v�l���[�`��
 */

#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <sstream>
#include "match.h"
#include "resource.h"
#include "sikou.h"
#include "wconf.h"

// ���W�ϊ�
// CSA -> Sunfish 
#define CSA2SUN(a)		(BanAddr(((a)-1)%9+1,((a)-1)/9+1))
// Sunfish -> CSA 
#define SUN2CSA(a)		(Addr2X(a)+(Addr2Y(a)-1)*9)

// �ǖ�
ShogiEval* pshogi = NULL;
Think* pthink = NULL;
Book*  pbook = NULL;
ResignController* presign = NULL;

// �v�l���t���O
static int sikou_flag = 0;

// ���f�t���O
static int chudan = 0;
static int chudanDfPn = 0;
static int chudanPonder = 0;
static int*	chudan_flag = NULL;

// ����Ԏv�l
HANDLE g_hPonder = INVALID_HANDLE_VALUE;

// window.cpp
unsigned __stdcall CreateMainWindow( LPVOID p );
void DestroyMainWindow();
void SetPlayMode( BOOL bEnable );
void InitProgress();
void AddProgress( const char* str );
void DisplayResult( THINK_INFO *info );
void DisplayBook( int num, Moves& moves );
extern BOOL bWindowCreated;

WConf wconf;
int limit = 10;
int thread_num = 1;
int resign_value = RESIGN_VALUE_DEF;
int hash_mbytes = 40;
BOOL bResult = TRUE;
BOOL bPonder = TRUE;
BOOL bWide = FALSE;

// �w����񋓗p
Moves moves;

static long mutex;
#define MUTEX_CREATE		do{ mutex = 0; }while( 0 )
#define MUTEX_CLOSE			do{ }while( 0 )
#define MUTEX_LOCK			do{ \
long l; \
while( 1 ){ \
l = InterlockedExchange( &mutex, 1 ); \
if( l == 0 ){ break; } \
while( mutex != 0 ) \
	; \
} }while( 0 )
#define MUTEX_UNLOCK		do{ mutex = 0; }while( 0 )

// �T����񑗐M�p
typedef void (*FUNC_INF)( int, int, int, int, int, int );
static FUNC_INF func_inf = NULL;

static const char sjis_kansuji[][3] = {
	"��", "��", "�O", "�l", "��", "�Z", "��", "��", "��"
};

static const char sjis_koma[][5] = {
	"��", "��",   "�j",   "��",   "��", "�p", "��", "��",
	"��", "����", "���j", "����", "",   "�n", "��",
};

string Shogi::GetStrMoveSJIS( const MOVE& move, bool eol ){
/************************************************
�w����𕶎���ɕϊ��B
************************************************/
	ostringstream str;

	if( move.from & PASS ){
		str << "PASS";

		return str.str();
	}

	str << Addr2X(move.to);
	str << sjis_kansuji[Addr2Y(move.to)-1];

	if( move.koma & SENTE )
		str << sjis_koma[move.koma-SENTE-1];
	else if( move.koma & GOTE )
		str << sjis_koma[move.koma-GOTE-1];
	else
		str << sjis_koma[move.koma-1];

	if( move.from & DAI ){
		// �������łꍇ
		str << "�� ";
	}
	else{
		// �Տ�̋�𓮂����ꍇ
		if( move.nari ){ str << "��"; }

		str << '(';
		str << Addr2X(move.from);
		str << sjis_kansuji[Addr2Y(move.from)-1];
		str << ") ";
	}

	if( eol ){
		str << '\n';
	}

	return str.str();
}

static void dispatch(){
/**************************************************************
dispatch
**************************************************************/
	MSG	Message;

	if(::PeekMessage(&Message,NULL,0,0,PM_REMOVE))
	{
		::TranslateMessage(&Message);
		::DispatchMessage (&Message);
	}
}

extern "C"
BOOL WINAPI DllMain( HINSTANCE hInst, DWORD fdwReason, PVOID pvReserved ){
/**************************************************************
���C��
**************************************************************/
	switch( fdwReason ){
	case DLL_PROCESS_ATTACH:
		// DLL�ǂݍ��ݎ�
		init_gen_rand( (unsigned int)time( NULL ) );

		wconf.Read();
		wconf.getValueInt( "TimeLimit", &limit );
		wconf.getValueInt( "NumberOfThreads", &thread_num );
		wconf.getValueInt( "ResignValue", &resign_value );
		wconf.getValueInt( "HashSize", &hash_mbytes );
		wconf.getValueInt( "Ponder", &bPonder );
		wconf.getValueInt( "Result", &bResult );
		wconf.getValueInt( "Wide", &bWide );

		_beginthreadex( NULL, 0, CreateMainWindow, (LPVOID)hInst, 0, NULL );

		if( pshogi == NULL ){
			pshogi = new ShogiEval();
		}
		if( pthink == NULL ){
			pthink = new Think();
		}
		if( pbook == NULL ){
			pbook = new Book();
		}
		if( presign == NULL ){
			presign = new ResignController();
		}

		break;
	case DLL_PROCESS_DETACH:
		// DLL�����
		chudan = 1;
		chudanDfPn = 1;
		while( sikou_flag ){
			dispatch();
			Sleep( 10 );
		}

		// ����Ԏv�l�X���b�h�̏I����҂B
		if( g_hPonder != INVALID_HANDLE_VALUE ){
			chudanPonder = 1;
			WaitForSingleObject( g_hPonder, INFINITE );
			CloseHandle( g_hPonder );
			g_hPonder = INVALID_HANDLE_VALUE;
		}

		if( pshogi ){
			delete pshogi;
			pshogi = NULL;
		}

		if( pthink ){
			delete pthink;
			pthink = NULL;
		}

		if( pbook ){
			delete pbook;
			pbook = NULL;
		}

		if( presign ){
			delete presign;
			presign = NULL;
		}

		wconf.setValueInt( "TimeLimit", limit );
		wconf.setValueInt( "NumberOfThreads", thread_num );
		wconf.setValueInt( "ResignValue", resign_value );
		wconf.setValueInt( "HashSize", hash_mbytes );
		wconf.setValueInt( "Ponder", bPonder );
		wconf.setValueInt( "Result", bResult );
		wconf.setValueInt( "Wide", bWide );
		wconf.Write();

		DestroyMainWindow();

		break;
	}

	return TRUE;
}

static VOID CALLBACK timelimit( UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 ){
/************************************************
�������Ԍo��
************************************************/
	chudan = 1; // �ł��؂�t���O
}

static VOID CALLBACK timelimit2( UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 ){
/************************************************
�������Ԍo��
************************************************/
	chudanDfPn = 1; // �ł��؂�t���O(�l�ݒT��)
}

unsigned __stdcall PonderSearchThread( LPVOID p ){
/************************************************
����ԂŒT��������X���b�h
************************************************/
	THINK_INFO info;

	if( pshogi != NULL && pthink != NULL ){
		pthink->Search( pshogi, NULL, 32, &info, &chudanPonder, 1 );
	}

	return 0;
}

unsigned __stdcall ThinkThread( LPVOID _p ){
/**************************************************************
�v�l���[�`�����Ăяo���X���b�h
**************************************************************/
	int ret = 0;
	THINK_INFO info;
	MOVE* pmove = (MOVE*)_p;
	MOVE move;
	MMRESULT tid;
	int num;
	int msec_mate = 0;

	memset( &info, 0, sizeof(THINK_INFO) );

	// ��Ղ𒲂ׂ�B
	if( pbook != NULL ){
		ret = pbook->GetMove( pshogi, move );
		if( ret != 0 ){
			// ��Ղ̕\��
			moves.Init();
			num = pbook->GetMoveAll( pshogi, moves );
			DisplayBook( num, moves );
		}
	}

	if( ret == 0 ){
		// �l�ݒT��
		sikou_flag = 1;

		// �^�C�}�[�I��
		if( limit > 0 ){
			// �l�݂𒲂ׂ鎞��(�ő�1�b)
			msec_mate = limit * 1000 / 10;
			if( msec_mate > 1000 ){
				msec_mate = 1000;
			}

			tid = timeSetEvent( msec_mate, 10, timelimit2, 0, TIME_ONESHOT );
		}

		ret = pthink->DfPnSearch( pshogi, &move, NULL, &info, &chudanDfPn );

		// �^�C�}�[�I�t
		if( limit > 0 ){
			timeKillEvent( tid );
		}

		if( ret ){
			DisplayResult( &info );
			AddProgress( pshogi->GetStrMove( move ).c_str() );
		}
	}

	if( ret == 0 ){
		// �ʏ�T��
		sikou_flag = 1;

		// �^�C�}�[�I��
		if( limit > 0 ){
			tid = timeSetEvent( limit * 1000 - msec_mate, 10, timelimit, 0, TIME_ONESHOT );
		}

		// �����[���T��
		ret = pthink->Search( pshogi, &move, 32, &info, &chudan, 1 );

		// �^�C�}�[�I�t
		if( limit > 0 ){
			timeKillEvent( tid );
		}

		DisplayResult( &info );

		// ����
		if( ret != 0 && presign->bad( info ) ){
			ret = 0;
		}
	}

	sikou_flag = 0;

	if( ret == 0 )
		// �w���肪�Ȃ�
		return (DWORD)0;

	*(pmove) = move;

	return (DWORD)1;
}

extern "C" __declspec(dllexport) void sikou_ini( int* c ){
/**************************************************************
������ (c�͒��f�t���O�ւ̃|�C���^)
**************************************************************/
	while( bWindowCreated == FALSE ){
		Sleep( 10 );
	}

	chudan_flag = c;
	Hash::SetSize( HashSize::MBytesToPow2( hash_mbytes, thread_num ) );
	if( pshogi == NULL )
		pshogi = new ShogiEval();
	if( pthink == NULL )
		pthink = new Match();
	if( pbook == NULL )
		pbook = new Book();
	if( presign == NULL )
		presign = new ResignController();
	SetPlayMode( FALSE );
	pthink->SetNumberOfThreads( thread_num );
	presign->set_limit( resign_value );
#if USE_QUICKCOPY
	pthink->PresetTrees( pshogi );
#endif
}

extern "C" __declspec(dllexport) int sikou( int tesu, unsigned char kifu[][2],
		int* timer_sec, int* i_moto, int* i_saki, int* i_naru ){
/**************************************************************
�v�l���[�`���̌Ăяo��
**************************************************************/
	HANDLE hThread;
	DWORD ret;
	MOVE move;
	int i;

	if( sikou_flag )
		return 0;

	// ����Ԏv�l�X���b�h�̏I����҂B
	if( g_hPonder != INVALID_HANDLE_VALUE ){
		chudanPonder = 1;
		WaitForSingleObject( g_hPonder, INFINITE );
		CloseHandle( g_hPonder );
		g_hPonder = INVALID_HANDLE_VALUE;
	}

	// �v�l�����\���̏�����
	InitProgress();

	if( pshogi == NULL ){
		if( ( pshogi = new ShogiEval ) == NULL )
			return 0;
	}

	if( pthink == NULL ){
		if( ( pthink = new Match ) == NULL )
			return 0;
	}

	while( pshogi->GoBack() )
		;

	// ���݂̋ǖʂ����B
	for( i = 0 ; i < tesu ; i++ ){
		// �ړ���
		if( kifu[i][1] <= 81 ){
			// �Տ�
			move.from = CSA2SUN( kifu[i][1] );
		}
		else{
			// ���
			move.from = kifu[i][1] - 100;
			move.from |= pshogi->GetSengo() | DAI;
		}

		// �ړ���
		if( kifu[i][0] <= 81 ){
			// ����Ȃ��ꍇ
			move.to = CSA2SUN( kifu[i][0] );
			move.nari = 0;
		}
		else{
			// ����ꍇ
			move.to = CSA2SUN( kifu[i][0] - 100 );
			move.nari = 1;
		}

		// ����������
		if( !pshogi->Move( move ) )
			return 0;
	}

	// �v�l�J�n
	chudan = 0;
	chudanDfPn = 0;
	hThread = (HANDLE)_beginthreadex( NULL, 0, ThinkThread, (LPVOID)&move, 0, NULL );
	for( ; ; ){
		GetExitCodeThread( hThread, &ret );
		if( ret != STILL_ACTIVE ){
			if( ret == 0 )
				return 0;
			break;
		}
		if( chudan_flag != NULL && *chudan_flag != 0 ){
			chudan = 1;
			chudanDfPn = 1;
		}
		dispatch();
		Sleep( 10 );
	}

	CloseHandle( hThread );

	// ���ʂ�Ԃ��B

	// �ړ���
	if( move.from & DAI ){
		// ���
		*i_moto = ( move.from & ~(DAI | SENGO) ) + 100;
	}
	else{
		// �Տ�
		*i_moto = SUN2CSA( move.from );
	}

	// �ړ���
	*i_saki = SUN2CSA( move.to );
	*i_naru = move.nari;

	// ����Ԏv�l���J�n
	if( bPonder && chudan_flag != NULL ){
		// �v�l�����\���̏�����
		InitProgress();

		chudanPonder = 0;
		pshogi->MoveD( move );
		g_hPonder = (HANDLE)_beginthreadex( NULL, 0, PonderSearchThread, (LPVOID)NULL, 0, NULL );
	}

	return 1;
}

extern "C" __declspec(dllexport) void sikou_end(){
/**************************************************************
�I��
**************************************************************/
	chudan_flag = NULL;

	// �v�l�̏I����҂�
	chudan = 1;
	chudanDfPn = 1;
	while( sikou_flag ){
		dispatch();
		Sleep( 10 );
	}

	// ����Ԏv�l�X���b�h�̏I����҂B
	if( g_hPonder != INVALID_HANDLE_VALUE ){
		chudanPonder = 1;
		WaitForSingleObject( g_hPonder, INFINITE );
		CloseHandle( g_hPonder );
		g_hPonder = INVALID_HANDLE_VALUE;
	}

	SetPlayMode( TRUE );
}
