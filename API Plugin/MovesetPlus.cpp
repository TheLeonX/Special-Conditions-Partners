#include "MovesetPlus.h"
#include <iostream>
#include <vector>

#include <WinSock2.h>
#include <windows.h>
#include <future>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <format>
#include "Common.h"
#include "Offsets.h"
#include "PatternScan.h"
#include "Main.h"

#include <cmath> // For fabs (absolute value for floats)
#include <cstdint>
#include <thread>
#include <chrono>
using namespace std;


__int64 MovesetPlus::CharacterCondition_funcAddress = 0;

#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)
int prevFrame = 0;
int prevBattle = 0;

uintptr_t me_FindHealthPointer(int characterId, int playerSide);
void me_EnemyDispOff(__int64 p);
void me_PlayBGM(int snd);
void me_test_switch_stage(__int64 function_param, __int64 a2, char* string_param, __int64 a1);
void me_character_param(__int64 p);
void me_change_skill(__int64 p, int jutsu, int index);
void me_change_speed(__int64 p, int enemy, float speed);
void me_change_speed_timed(__int64 char_p, int enemy, float speed, int duration_ms);
void me_play_blur (__int64 p);
void me_change_fov(__int64 p, float fov);
void me_change_walk_speed(__int64 p, float speed);
void me_change_jump_height(__int64 p, float jump_h);
void me_SetPlayerParam(__int64 s, int param2, float value);
void me_ChangePlayerParam(__int64 s, int param2, float value);
void me_SetPlayerVisibility(__int64 p, int cansee);
void me_enable_dpad_animation(__int64 a1, int param2);
void me_enable_control(__int64 a1, int enemy, int control);
void me_disable_control(__int64 a1, int enemy, int control);
void me_change_dpad_charge(__int64 a1, int enemy, int arrow, float charge);
void me_change_control_timed(__int64 a1, int enemy, int control, float time);

bool ReplaceBytes(void* address, const uint8_t* newBytes, size_t size);

int crc32(const std::string& input);

__int64 __fastcall MovesetPlus::meTest(__int64 a1, __int64 a2)
{
	int character = *((int*)(a1 + 0xE64));
	short param1 = *((short*)(a2 + 0x24));
	short param2 = *((short*)(a2 + 0x26));
	short param3 = *((short*)(a2 + 0x28));
	float param4 = *((float*)(a2 + 0x2C));
	int string_param_integer = *((int*)(a2));

	char string_param[31] = { 0 }; // Allocate 31 to include null terminator
	std::strncpy(string_param, reinterpret_cast<const char*>(a2), 30);
	string_param[30] = '\0'; // Ensure null termination

	__int64 enemy_pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	enemy_pointer = sub_1409B9FB0_f(a1);
	int side = 0;
	if (a1 < enemy_pointer)
	{
		side = 0; // Player 1 (left side)
	}
	else if (a1 > enemy_pointer)
	{
		side = 1; // Player 2 (right side)
	}

	switch (param1)
	{
		default:
			me_EnemyDispOff(a1);
			break;
		case 1:
			me_PlayBGM(param2);
			break;
		case 2:
			me_test_switch_stage(a2, param2, string_param, a1);
			break;
		case 3:
			me_change_skill(a1, param2, param3);
			break;
		case 4:
			me_change_speed(a1, param2, param4);
			break;
		case 5:
			me_change_speed_timed(a1, param2, param4, param3 * (1000/30));
			break;
		case 6:
			me_play_blur(a1);
			break;
		case 7:
			me_change_fov(a1, param4);
			break;
		case 8:
			me_change_walk_speed(a1, param4);
			break;
		case 9:
			me_change_jump_height(a1, param4);
			break;
		case 10:
			me_SetPlayerParam(me_FindHealthPointer(param3, side), param2, param4);
			break;
		case 11:
			me_ChangePlayerParam(me_FindHealthPointer(param3, side), param2, param4);
			break;
		case 12:
			me_SetPlayerVisibility(a1, param2);
			break;
		case 13:
			me_enable_dpad_animation(a1, param2);
			break;
		case 14:
			me_enable_control(a1, param2, param3);
			break;
		case 15:
			me_disable_control(a1, param2, param3);
			break;
		case 16:
			me_change_control_timed(a1, param2, param3, param4);
			break;
		case 17:
			me_change_dpad_charge(a1, param2, param3, param4);
			break;
		case 18:
			me_character_param(a1);
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

void me_enable_dpad_animation(__int64 a1, int param2) {
	// Address of the target bytes
	__int64 DPadHudAddress = PatternScan::Scan("83xxxxxxxxxxxx44xxxxxx44xxxxxx44xxxxxx0Fxxxxxxxxxx44xxxxxx66xxxxxxxxxxxxxxF3xxxxxxxxxx");
	void* targetAddress = (void*)(DPadHudAddress + 2); // Adjust offset if necessary

	// Original and new byte sequences
	uint8_t originalBytes[] = { 0x64, 0x0E, 0x00, 0x00, 0x7C };
	uint8_t newBytes[] = { 0x30, 0x0F, 0x00, 0x00, 0x01 };

	*(int*)(a1 + 0xF30) = param2;
	// Replace bytes and monitor changes
	ReplaceBytes(targetAddress, newBytes, sizeof(newBytes));

}


uintptr_t me_FindHealthPointer(int characterId, int playerSide)
{
	uintptr_t* p1 = nullptr;
	uintptr_t* p2 = nullptr;
	uintptr_t* p3 = nullptr;
	uintptr_t* p4 = nullptr;
	uintptr_t* p5 = nullptr;
	uintptr_t* p6 = nullptr;

	// Base address for the module
	p1 = (uintptr_t*)(plugin::moduleBase + 0x0219D808);
	if (p1 == nullptr || *p1 == 0) return 0;

	// Offset 0 (Player or Enemy selection based on playerSide)
	if (playerSide == 0) // Player 1 (left side)
	{
		p2 = (uintptr_t*)(*p1 + (characterId == 0 ? 0x80 : 0x88)); // Player or Enemy
	}
	else if (playerSide == 1) // Player 2 (right side)
	{
		p2 = (uintptr_t*)(*p1 + (characterId == 0 ? 0x88 : 0x80)); // Swap Player/Enemy for right side
	}
	else
	{
		std::cerr << "Invalid playerSide. Use 0 for Player 1 (left), 1 for Player 2 (right)." << std::endl;
		return 0;
	}
	if (p2 == nullptr || *p2 == 0) return 0;

	// Offset 1
	p3 = (uintptr_t*)(*p2 + 0xB8);
	if (p3 == nullptr || *p3 == 0) return 0;

	// Offset 2
	p4 = (uintptr_t*)(*p3 + 0xC0);
	if (p4 == nullptr || *p4 == 0) return 0;

	// Offset 3 (Player or Enemy specific)
	if (playerSide == 0) // Player 1 (left side)
	{
		p5 = (uintptr_t*)(*p4 + (characterId == 0 ? 0x10 : 0x88)); // Player or Enemy
	}
	else if (playerSide == 1) // Player 2 (right side)
	{
		p5 = (uintptr_t*)(*p4 + (characterId == 0 ? 0x88 : 0x10)); // Swap Player/Enemy for right side
	}
	if (p5 == nullptr || *p5 == 0) return 0;

	// Offset 4
	p6 = (uintptr_t*)(*p5 + 0x38);
	if (p6 == nullptr || *p6 == 0) return 0;

	// Debug output
	std::cout << "Health Pointer for CharacterId " << characterId << " on Side " << playerSide << ": " << p6 << std::endl;
	std::cout << "Health Value: " << *p6 << std::endl;

	return (uintptr_t)p6;
}


void me_change_dpad_charge(__int64 char_p, int enemy, int arrow, float charge)
{
	__int64 pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	if (enemy == 0)
		pointer = char_p;
	else
		pointer = sub_1409B9FB0_f(char_p);

	switch (arrow) {
	case 0://All Arrow
		*(float*)(pointer + 0x12B88) = charge; //Up Arrow
		*(float*)(pointer + 0x12B8C) = charge; //Down Arrow
		*(float*)(pointer + 0x12B90) = charge; //Left Arrow
		*(float*)(pointer + 0x12B94) = charge; //Right Arrow
		break;
	case 1:
		*(float*)(pointer + 0x12B88) = charge; //Up Arrow
		break;
	case 2:
		*(float*)(pointer + 0x12B8C) = charge; //Down Arrow
		break;
	case 3:
		*(float*)(pointer + 0x12B90) = charge; //Left Arrow
		break;
	case 4:
		*(float*)(pointer + 0x12B94) = charge; //Right Arrow
		break;
	}
}


void me_SetPlayerVisibility(__int64 p, int cancel)
{
	int* visible = (int*)(p + 0xE9C);
	*visible = cancel;
}

void me_SetPlayerParam(__int64 s, int param2, float value)
{
	switch (param2)
	{
	case 0: // health
		*(float*)(s + 0x00) = (value > *(float*)(s + 0x04)) ? *(float*)(s + 0x04) : value;
		break;

	case 1: // maxhealth
		*(float*)(s + 0x04) = value;
		*(float*)(s + 0x00) = min(*(float*)(s + 0x00), value);
		break;

	case 2: // chakra
		*(float*)(s + 0x08) = (value > *(float*)(s + 0x0C)) ? *(float*)(s + 0x0C) : value;
		break;

	case 3: // maxchakra
		*(float*)(s + 0x0C) = value;
		*(float*)(s + 0x08) = min(*(float*)(s + 0x08), value);
		break;

	case 4: // sub
		*(float*)(s + 0x10) = (value > *(float*)(s + 0x14)) ? *(float*)(s + 0x14) : value;
		break;

	case 5: // maxsub
		*(float*)(s + 0x14) = value;
		*(float*)(s + 0x10) = min(*(float*)(s + 0x10), value);
		break;
	}
}

void me_ChangePlayerParam(__int64 s, int param2, float value)
{
	switch (param2)
	{
	case 0: // health
		*(float*)(s + 0x00) = min(*(float*)(s + 0x00) + value, *(float*)(s + 0x04));
		break;

	case 1: // maxhealth
		*(float*)(s + 0x04) += value;
		*(float*)(s + 0x00) = min(*(float*)(s + 0x00), *(float*)(s + 0x04));
		break;

	case 2: // chakra
		*(float*)(s + 0x08) = min(*(float*)(s + 0x08) + value, *(float*)(s + 0x0C));
		break;

	case 3: // maxchakra
		*(float*)(s + 0x0C) += value;
		*(float*)(s + 0x08) = min(*(float*)(s + 0x08), *(float*)(s + 0x0C));
		break;

	case 4: // sub
		*(float*)(s + 0x10) = min(*(float*)(s + 0x10) + value, *(float*)(s + 0x14));
		break;

	case 5: // maxsub
		*(float*)(s + 0x14) += value;
		*(float*)(s + 0x10) = min(*(float*)(s + 0x10), *(float*)(s + 0x14));
		break;
	}
}


void me_change_skill(__int64 char_p, int jutsu, int index) {
	if (index > -1 && index <= 6) {
		if (jutsu == 0) {

			*(int*)(char_p + 0xE78) = index; //Jutsu 1
		}
		else if (jutsu == 1) {

			*(int*)(char_p + 0xE7C) = index; //Jutsu 2
		}
		else if (jutsu == 2) {

			*(int*)(char_p + 0xE80) = index; // Ultimate Jutsu
		}
	}
	
}
void me_play_blur(__int64 p) {
	*(int*)(p + 0x11998) = 1;

}
void me_change_fov(__int64 p, float FOV) {
	if (FOV > 0)
		*(float*)(p + 0x119A0) = FOV;
	else
		*(float*)(p + 0x119A0) = -1;

}

void me_change_walk_speed(__int64 p, float speed) {
	*(float*)(p + 0x10D44) = speed;
}
void me_change_jump_height(__int64 p, float jump_h) {
	*(float*)(p + 0x10D3C) = jump_h;
}
void me_change_speed(__int64 char_p, int enemy, float speed) {
	__int64 enemy_pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	enemy_pointer = sub_1409B9FB0_f(char_p);
	if (enemy == 0) {

		*(float*)(char_p + 0x214) = speed; //Player
	}
	else if (enemy == 1) {

		*(float*)(enemy_pointer + 0x214) = speed; //Enemy
	}
}

void me_change_speed_timed(__int64 char_p, int enemy, float speed, int duration_ms) {
	__int64 enemy_pointer;
	typedef signed __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress);

	// Get Enemy Pointer
	enemy_pointer = sub_1409B9FB0_f(char_p);

	// Save the original speed
	float* target_speed_ptr = (enemy == 0) ? (float*)(char_p + 0x214) : (float*)(enemy_pointer + 0x214);
	float original_speed = *target_speed_ptr;

	// Set the new speed
	*target_speed_ptr = speed;

	// Start a new thread to reset speed after the duration
	std::thread([=]() {
		// Wait for the specified duration
		std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));

		// Reset the speed back to the original value
		*target_speed_ptr = 1.0f;
		}).detach(); // Detach the thread to run independently
}


__int64 MovesetPlus::OriginalStageAddress = 0;
__int64 MovesetPlus::HandleStageChangeAddress = 0;
__int64 MovesetPlus::DefaultStageHandlerAddress = 0;
__int64 MovesetPlus::SpecificStageHandlerAddress = 0;
__int64 MovesetPlus::FixCharPositionAddress = 0;
__int64 MovesetPlus::VTableAddress = 0;

__int64 MovesetPlus::OriginalStageAddress2 = 0;




int MovesetPlus::STAGE_ID;
void me_test_switch_stage(__int64 function_param, __int64 param2, char* string_param, __int64 character_pointer )
{
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer

	typedef signed __int64(__fastcall* Sub_1408EAE00)(unsigned int);
	Sub_1408EAE00 sub_1408EAE00_f = (Sub_1408EAE00)(MovesetPlus::HandleStageChangeAddress);

	typedef signed __int64(__fastcall* Sub_1406F6800)(__int64);
	Sub_1406F6800 sub_1406F6800_f = (Sub_1406F6800)(MovesetPlus::DefaultStageHandlerAddress);

	typedef void(__fastcall* Sub_1406F6830)(__int64, unsigned int);
	Sub_1406F6830 sub_1406F6830_f = (Sub_1406F6830)(MovesetPlus::SpecificStageHandlerAddress);

	typedef void(__fastcall* sub_140643C10)(__int64);
	sub_140643C10 sub_140643C10_f = (sub_140643C10)(MovesetPlus::FixCharPositionAddress);

	//__int64(__fastcall * **__fastcall sub_141086E40(__int64 a1, __int64 a2))(void* Block)
	typedef __int64(__fastcall* BlockFunc)(void* Block);
	typedef BlockFunc* StageMoveVTable;
	typedef StageMoveVTable(__fastcall* Sub_141086E40Type)(__int64, __int64);

	//&xmmword_1422FB990
	__int64* pXmmword = reinterpret_cast<__int64*>(plugin::moduleBase + offset::VTableXmmwordOffset);

	//*((__int64 *)&xmmword_1422FB990 + 1)
	__int64 highPart = pXmmword[1];

	Sub_141086E40Type sub_141086E40_f = reinterpret_cast<Sub_141086E40Type>(MovesetPlus::VTableAddress);

	//v7 = (__int64)sub_141086E40(*((__int64 *)&xmmword_1422FB990 + 1), (__int64)"StageMove");
	StageMoveVTable vtable = sub_141086E40_f(highPart, reinterpret_cast<__int64>("StageMove"));

	//*(_QWORD *)(*(_QWORD *)(v7 + 8)
	__int64 innerPointer = *reinterpret_cast<__int64*>(reinterpret_cast<char*>(vtable) + 8);

	//*(_QWORD *)(*(_QWORD *)(v7 + 8) + 16i64);
	__int64 v9 = *reinterpret_cast<__int64*>(reinterpret_cast<char*>(reinterpret_cast<void*>(innerPointer)) + 16);

	
	if (param2 == 0) {
		sub_1406F6830_f(v9, crc32((std::string)string_param));
	}
	else {
		sub_1406F6800_f(v9);
	}
	cout << "Hash Stage: " << *(int*)(Common::GetQword(offset::stageOffset) + 8) << endl;

	sub_1408EAE00_f(*(int*)(Common::GetQword(offset::stageOffset) + 8));

	__int64 enemy_pointer = sub_1409B9FB0_f(character_pointer);
	sub_140643C10_f(character_pointer);
	sub_140643C10_f(enemy_pointer);
}



__int64 MovesetPlus::BgmOffFunctionAdress = 0;
__int64 MovesetPlus::BgmOnFunctionAdress = 0;
__int64 MovesetPlus::BgmOff2FunctionAdress = 0;

int MovesetPlus::BGM_ID;
void me_PlayBGM(int snd)
{
	//find ccAdvPlayStageBgm or check offset 0xB2B10 in 1.01 version of file
	__int64 offset = offset::bgmOffset;
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

__int64 MovesetPlus::EffectHandlerAddress = 0;
__int64 MovesetPlus::InitializeHandlerAddress = 0;
__int64 MovesetPlus::ResolveEffectAddress = 0;
__int64 MovesetPlus::ExecuteEffectAddress = 0;

__int64 __fastcall MovesetPlus::ApplyStatusEffect(__int64 a1, __int64 a2)
{
	__int64 v3; // rax
	__int64 v4; // rsi
	unsigned int v5; // eax
	int v6; // ebx
	unsigned int v7; // edi
	__int64 v8; // rax
	int param2_check; // ebx


	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	typedef signed  __int64(__fastcall* sub_14099A610)(__int64 a1);
	sub_14099A610 sub_14099A610_f = (sub_14099A610)(MovesetPlus::ResolveEffectAddress);
	typedef signed  __int64(__fastcall* sub_14099AEF0)(__int64 a1, unsigned int a2, int a3, float a4);
	sub_14099AEF0 sub_14099AEF0_f = (sub_14099AEF0)(MovesetPlus::ExecuteEffectAddress);



	param2_check = *(__int16*)(a2 + 38);

	if (param2_check == 1)
		v3 = a1;
	else
		v3 = sub_1409B9FB0_f(a1);
	v4 = v3;
	if (v3)
	{
		MovesetPlus::InitializeEffectHandler_f(v3);
		v5 = sub_14099A610_f(a2);
		v6 = *(__int16*)(a2 + 36);
		v7 = v5;
		v8 = MovesetPlus::InitializeEffectHandler_f(v4);
		sub_14099AEF0_f(v8, v7, v6, *(float*)(a2 + 44));
	}
	return 1;
}



__int64 __fastcall MovesetPlus::InitializeEffectHandler_f(__int64 a1)
{
	__int64 result; // rax

	result = 0i64;
	if (*(__int64*)(a1 + 71512))
		return *(__int64*)(a1 + 71512);
	return result;
}


__int64 __fastcall MovesetPlus::CancelActionFunction(__int64 a1, __int64 a2)
{
	__int64 result; // rax 


	// change 73712 value with update
	switch (*(int*)(a2 + 36))
	{
	case 0:
		*(int*)(a1 + 73728) = 1;
		result = 1i64;
		break;
	case 1:
		*(int*)(a1 + 73728) = 2; //JUMP CANCEL
		result = 1i64;
		break;
	case 2:
		*(int*)(a1 + 73728) = 4;
		result = 1i64;
		break;
	case 3:
		*(int*)(a1 + 73728) = 8; //DASH CANCEL
		result = 1i64;
		break;
	case 4:
		*(int*)(a1 + 73728) = 16; // COMBO CANCEL
		result = 1i64;
		break;
	case 5:
		*(int*)(a1 + 73728) = 32; // GRAB CANCEL
		result = 1i64;
		break;
	case 6:
		*(int*)(a1 + 73728) = 64; //JUTSU CANCEL
		result = 1i64;
		break;
	case 7:
		*(int*)(a1 + 73728) = 128;
		result = 1i64;
		break;
	case 8:
		*(int*)(a1 + 73728) = 256; //ULTIMATE JUTSU CANCEL
		result = 1i64;
		break;
	case 9:
		*(int*)(a1 + 73728) = 15; //WALK
		result = 1i64;
		break;
	case 0xA:
		*(int*)(a1 + 73728) = 208; //JUTSU, COMBO CANCEL
		result = 1i64;
		break;
	case 0xB:
		*(int*)(a1 + 73728) = 192;
		result = 1i64;
		break;
	case 0xC:
		*(int*)(a1 + 73728) = -1; //ALL CANCEL
		result = 1i64;
		break;
	case 0xD:
		*(int*)(a1 + 73728) = 512;
		result = 1i64;
		break;
	case 0xE:
		*(int*)(a1 + 73728) = 1024; //SHURIKEN CANCEL
		result = 1i64;
		break;
	case 0xF:
		*(int*)(a1 + 73728) = -1030; //COMBO, JUTSU, ULTIMATE, JUMP, GRAB, DASH
		result = 1i64;
		break;
	case 0x10:
		*(int*)(a1 + 73728) = -22; // SHURIKEN, JUTSU, ULTIMATE, JUMP, GRAB, DASH
		result = 1i64;
		break;
	case 0x11:
		*(int*)(a1 + 73728) = 1120; // GRAB, SHURIKEN, JUTSU
		result = 1i64;
		break;
	case 0x12:
		*(int*)(a1 + 73728) = -6; //COMBO, JUTSU, ULTIMATE, JUMP, SHURIKEN, GRAB, DASH
		result = 1i64;
		break;
	default:
		*(int*)(a1 + 73728) = 0;
		result = 1i64;
		break;
	}
	return result;
}

void me_enable_control(__int64 char_p, int enemy, int control) {
	__int64 pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	if (enemy == 0)
		pointer = char_p;
	else
		pointer = sub_1409B9FB0_f(char_p);

	switch (control) {
	case 0:
		*(int*)(pointer + 0x12A34) = 1; //enable PL_ANM_ATK
		break;
	case 1:
		*(int*)(pointer + 0x12A3C) = 1; //enable Ultimate Jutsu
		break;
	case 2:
		*(int*)(pointer + 0x12A44) = 1; //enable Jutsu 1 and Jutsu 2
		break;
	case 3:
		*(int*)(pointer + 0x12A4C) = 1; //enable PL_ANM_PRJ_LAND and PL_ANM_PRJ_ATK
		break;
	case 4:
		*(int*)(pointer + 0x12A54) = 1; //enable grab
		break;
	case 5:
		*(int*)(pointer + 0x12A58) = 1; //enable substitution jutsu
		break;
	case 6:
		*(int*)(pointer + 0x12A5C) = 1; //enable guard
		break;
	case 7:
		*(int*)(pointer + 0x12A60) = 1; //enable chakra load
		break;
	case 8:
		*(int*)(pointer + 0x12A64) = 1; //enable WASD and chakra
		break;
	case 9:
		*(int*)(pointer + 0x12A68) = 1; //enable jump
		break;
	case 10:
		*(int*)(pointer + 0x12A6C) = 1; //enable ninja movement
		break;
	case 11:
		*(int*)(pointer + 0x12A70) = 1; //enable air dash
		break;
	case 12:
		*(int*)(pointer + 0x12A74) = 1; //enable land dash
		break;
	case 13:
		*(int*)(pointer + 0x12A78) = 1; //enable D-Pad items
		break;
	case 14:
		*(int*)(pointer + 0x12A7C) = 1; //enable leader switch
		break;
	case 15:
		*(int*)(pointer + 0x12A80) = 1; //enable awakening
		break;
	case 16:
		*(int*)(pointer + 0x12A84) = 1; //enable supports
		break;
	case 17:
		*(int*)(pointer + 0x12A90) = 1; //enable counter attack
		break;
	}
}
void me_disable_control(__int64 char_p, int enemy, int control) {
	__int64 pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	if (enemy == 0)
		pointer = char_p;
	else
		pointer = sub_1409B9FB0_f(char_p);

	switch (control) {
	case 0:
		*(int*)(pointer + 0x12A34) = 0; //disable PL_ANM_ATK
		break;
	case 1:
		*(int*)(pointer + 0x12A3C) = 0; //disable Ultimate Jutsu
		break;
	case 2:
		*(int*)(pointer + 0x12A44) = 0; //disable Jutsu 1 and Jutsu 2
		break;
	case 3:
		*(int*)(pointer + 0x12A4C) = 0; //disable PL_ANM_PRJ_LAND and PL_ANM_PRJ_ATK
		break;
	case 4:
		*(int*)(pointer + 0x12A54) = 0; //disable grab
		break;
	case 5:
		*(int*)(pointer + 0x12A58) = 0; //disable substitution jutsu
		break;
	case 6:
		*(int*)(pointer + 0x12A5C) = 0; //disable guard
		break;
	case 7:
		*(int*)(pointer + 0x12A60) = 0; //disable chakra load
		break;
	case 8:
		*(int*)(pointer + 0x12A64) = 0; //disable WASD and chakra
		break;
	case 9:
		*(int*)(pointer + 0x12A68) = 0; //disable jump
		break;
	case 10:
		*(int*)(pointer + 0x12A6C) = 0; //disable ninja movement
		break;
	case 11:
		*(int*)(pointer + 0x12A70) = 0; //disable air dash
		break;
	case 12:
		*(int*)(pointer + 0x12A74) = 0; //disable land dash
		break;
	case 13:
		*(int*)(pointer + 0x12A78) = 0; //disable D-Pad items
		break;
	case 14:
		*(int*)(pointer + 0x12A7C) = 0; //disable leader switch
		break;
	case 15:
		*(int*)(pointer + 0x12A80) = 0; //disable awakening
		break;
	case 16:
		*(int*)(pointer + 0x12A84) = 0; //disable supports
		break;
	case 17:
		*(int*)(pointer + 0x12A90) = 0; //disable counter attack
		break;
	}

}
void me_change_control_timed(__int64 char_p, int enemy, int control, float duration_s) {
	__int64 pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer
	if (enemy == 0)
		pointer = char_p;
	else
		pointer = sub_1409B9FB0_f(char_p);


	// Disable the control (set value to 0)
	switch (control) {
	case 0:
		*(int*)(pointer + 0x12A34) = 0; // disable PL_ANM_ATK
		break;
	case 1:
		*(int*)(pointer + 0x12A3C) = 0; // disable Ultimate Jutsu
		break;
	case 2:
		*(int*)(pointer + 0x12A44) = 0; // disable Jutsu 1 and Jutsu 2
		break;
	case 3:
		*(int*)(pointer + 0x12A4C) = 0; // disable PL_ANM_PRJ_LAND and PL_ANM_PRJ_ATK
		break;
	case 4:
		*(int*)(pointer + 0x12A54) = 0; // disable grab
		break;
	case 5:
		*(int*)(pointer + 0x12A58) = 0; // disable substitution jutsu
		break;
	case 6:
		*(int*)(pointer + 0x12A5C) = 0; // disable guard
		break;
	case 7:
		*(int*)(pointer + 0x12A60) = 0; // disable chakra load
		break;
	case 8:
		*(int*)(pointer + 0x12A64) = 0; // disable WASD and chakra
		break;
	case 9:
		*(int*)(pointer + 0x12A68) = 0; // disable jump
		break;
	case 10:
		*(int*)(pointer + 0x12A6C) = 0; // disable ninja movement
		break;
	case 11:
		*(int*)(pointer + 0x12A70) = 0; // disable air dash
		break;
	case 12:
		*(int*)(pointer + 0x12A74) = 0; // disable land dash
		break;
	case 13:
		*(int*)(pointer + 0x12A78) = 0; // disable D-Pad items
		break;
	case 14:
		*(int*)(pointer + 0x12A7C) = 0; // disable leader switch
		break;
	case 15:
		*(int*)(pointer + 0x12A80) = 0; // disable awakening
		break;
	case 16:
		*(int*)(pointer + 0x12A84) = 0; // disable supports
		break;
	case 17:
		*(int*)(pointer + 0x12A90) = 0; // disable counter attack
		break;
	}

	// Start a new thread to restore the control after the duration
	std::thread([=]() {
		// Wait for the specified duration
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(duration_s * 1000)));

		// Restore the control (set value back to 1)
		switch (control) {
		case 0:
			*(int*)(pointer + 0x12A34) = 1; // restore PL_ANM_ATK
			break;
		case 1:
			*(int*)(pointer + 0x12A3C) = 1; // restore Ultimate Jutsu
			break;
		case 2:
			*(int*)(pointer + 0x12A44) = 1; // restore Jutsu 1 and Jutsu 2
			break;
		case 3:
			*(int*)(pointer + 0x12A4C) = 1; // restore PL_ANM_PRJ_LAND and PL_ANM_PRJ_ATK
			break;
		case 4:
			*(int*)(pointer + 0x12A54) = 1; // restore grab
			break;
		case 5:
			*(int*)(pointer + 0x12A58) = 1; // restore substitution jutsu
			break;
		case 6:
			*(int*)(pointer + 0x12A5C) = 1; // restore guard
			break;
		case 7:
			*(int*)(pointer + 0x12A60) = 1; // restore chakra load
			break;
		case 8:
			*(int*)(pointer + 0x12A64) = 1; // restore WASD and chakra
			break;
		case 9:
			*(int*)(pointer + 0x12A68) = 1; // restore jump
			break;
		case 10:
			*(int*)(pointer + 0x12A6C) = 1; // restore ninja movement
			break;
		case 11:
			*(int*)(pointer + 0x12A70) = 1; // restore air dash
			break;
		case 12:
			*(int*)(pointer + 0x12A74) = 1; // restore land dash
			break;
		case 13:
			*(int*)(pointer + 0x12A78) = 1; // restore D-Pad items
			break;
		case 14:
			*(int*)(pointer + 0x12A7C) = 1; // restore leader switch
			break;
		case 15:
			*(int*)(pointer + 0x12A80) = 1; // restore awakening
			break;
		case 16:
			*(int*)(pointer + 0x12A84) = 1; // restore supports
			break;
		case 17:
			*(int*)(pointer + 0x12A90) = 1; // restore counter attack
			break;
		}
		}).detach(); // Detach the thread to run independently
}



void me_character_param(__int64 char_p) {
	cout << (char_p) << endl;
	cout << (char_p + 0xE64) << endl;
}




std::vector<int> crc32_table() {
	std::vector<int> table(256);
	for (int i = 0; i < 256; ++i) {
		int crc = i << 24;
		for (int j = 0; j < 8; ++j) {
			if (crc & 0x80000000) {
				crc = (crc << 1) ^ 0x04C11DB7;
			}
			else {
				crc <<= 1;
			}
		}
		table[i] = crc;
	}
	return table;
}

int crc32(const std::string& input) {
	const auto table = crc32_table();
	int crc = 0xFFFFFFFF;
	for (unsigned char byte : input) {
		int lookup_index = ((crc >> 24) ^ byte) & 0xFF;
		crc = (crc << 8) ^ table[lookup_index];
	}
	crc = ~crc;
	return crc;
}

bool ReplaceBytes(void* address, const uint8_t* newBytes, size_t size)
{
	DWORD oldProtect;
	if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
		return false;

	memcpy(address, newBytes, size);

	DWORD tempProtect;
	VirtualProtect(address, size, oldProtect, &tempProtect);
	FlushInstructionCache(GetCurrentProcess(), address, size);
	return true;
}
