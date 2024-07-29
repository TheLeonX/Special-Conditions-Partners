#include "MovesetPlus.h"
#include "Main.h"
#include <iostream>
#include <vector>

#include <WinSock2.h>
#include "PatternScan.h"
#include <windows.h>
#include <future>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <format>
#include "Common.h"
using namespace std;



#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)
int prevFrame = 0;
int prevBattle = 0;

void me_EnemyDispOff(__int64 p);
void me_PlayBGM(int snd);

__int64 __fastcall MovesetPlus::meTest(__int64 a1, __int64 a2)
{
	int character = *((int*)(a1 + 0xE64));
	short param1 = *((short*)(a2 + 0x24));
	short param2 = *((short*)(a2 + 0x26));
	float param3 = *((float*)(a2 + 0x2C));

	switch (param1)
	{
		default:
			me_EnemyDispOff(a1);
			break;
		case 1:
			me_PlayBGM(param2);
			break;
	}

	return 1;
}
void me_EnemyDispOff(__int64 a1)
{
	__int64 v1; // rax  

	v1 = (*(__int64(__fastcall**)(__int64))(*(__int64*)a1 + 3512i64))(a1);
	if (v1)
		(*(void(__fastcall**)(__int64))(*(__int64*)v1 + 3000i64))(v1);


}

__int64 MovesetPlus::BgmOffFunctionAdress = 0;
__int64 MovesetPlus::BgmOnFunctionAdress = 0;
__int64 MovesetPlus::BgmOff2FunctionAdress = 0;

int MovesetPlus::BGM_ID;
void me_PlayBGM(int snd)
{
	//find ccAdvPlayStageBgm or check offset 0xB2B10 in 1.01 version of file
	__int64 offset = 0x142187BF0;
	if (snd == 0) {
		MovesetPlus::BGM_ID = 0;
		typedef signed  __int64(__fastcall* IsPlayBgm)(__int64 a1, int a2);
		IsPlayBgm IsPlayBgm_f = (IsPlayBgm)(MovesetPlus::BgmOnFunctionAdress); //8E68B0
		do {
			MovesetPlus::BGM_ID++;
			//cout << "first = ";
			//cout << MovesetPlus::BGM_ID << endl;
		} while ((unsigned int)IsPlayBgm_f(Common::GetQword(offset), MovesetPlus::BGM_ID) != 1 && MovesetPlus::BGM_ID != 200); //qword_1420F3A20
		if (MovesetPlus::BGM_ID != -1 && MovesetPlus::BGM_ID != 200) {
			typedef signed  __int64(__fastcall* StopBgm)(__int64 a1, unsigned int a2);
			StopBgm StopBgm_f = (StopBgm)(MovesetPlus::BgmOffFunctionAdress); //8E93C0
			StopBgm_f(Common::GetQword(offset), 0); //qword_1420F3A20
		}
	}
	else if (snd == 1) {
		if (MovesetPlus::BGM_ID != 0) {
			//cout << "second = ";
			//cout << MovesetPlus::BGM_ID << endl;
			typedef void(__fastcall* sub_14073C2E8)(__int64 a1, int a2, int a3, float a4, int a5);
			sub_14073C2E8 sub_14073C2E8_f = (sub_14073C2E8)(MovesetPlus::BgmOff2FunctionAdress); //8E6C10
			sub_14073C2E8_f(Common::GetQword(offset), MovesetPlus::BGM_ID, 0, -1, 0);
		}

	}


}
