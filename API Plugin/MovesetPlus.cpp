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

#include <thread>
#include <chrono>
using namespace std;


__int64 MovesetPlus::CharacterCondition_funcAddress = 0;

__int64 stageOffset = 0x14218C5E0; //get that id from kaguya's screen stage switch effect

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

#include <cstdint>

uintptr_t me_FindHealthPointer(int characterId, int playerSide)
{
	uintptr_t* p1 = nullptr;
	uintptr_t* p2 = nullptr;
	uintptr_t* p3 = nullptr;
	uintptr_t* p4 = nullptr;
	uintptr_t* p5 = nullptr;
	uintptr_t* p6 = nullptr;

	// Base address for the module
	p1 = (uintptr_t*)(plugin::moduleBase + 0x0219A628);
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


#include <cmath> // For fabs (absolute value for floats)

void me_SetPlayerParam(__int64 s, int param2, float value)
{
	const float EPSILON = 0.0001f; // Small value to account for floating-point precision errors
	float new_val = 0.0f;

	switch (param2)
	{
	case 0: // health
		if (*(float*)(s + 0x00) + value > *(float*)(s + 0x04) + EPSILON)
		{
			new_val = (*(float*)(s + 0x00) + value) - *(float*)(s + 0x04);
		}
		*(float*)(s + 0x00) = value - new_val;
		break;

	case 1: // maxhealth
		*(float*)(s + 0x04) = value;
		*(float*)(s + 0x00) = min(*(float*)(s + 0x00), value); // Ensure current health <= max health
		break;

	case 2: // chakra
		if (*(float*)(s + 0x08) + value > *(float*)(s + 0x0C) + EPSILON)
		{
			new_val = (*(float*)(s + 0x08) + value) - *(float*)(s + 0x0C);
		}
		*(float*)(s + 0x08) = value - new_val;
		break;

	case 3: // maxchakra
		*(float*)(s + 0x0C) = value;
		*(float*)(s + 0x08) = min(*(float*)(s + 0x08), value);
		break;

	case 4: // sub
		if (*(float*)(s + 0x10) + value > *(float*)(s + 0x14) + EPSILON)
		{
			new_val = (*(float*)(s + 0x10) + value) - *(float*)(s + 0x14);
		}
		*(float*)(s + 0x10) = value - new_val;
		break;

	case 5: // maxsub
		*(float*)(s + 0x14) = value;
		*(float*)(s + 0x10) = min(*(float*)(s + 0x10), value);
		break;
	}
}

void me_ChangePlayerParam(__int64 s, int param2, float value)
{
	const float EPSILON = 0.0001f; // Small value to account for floating-point precision errors
	float new_val = 0.0f;

	switch (param2)
	{
	case 0: // health
		if (*(float*)(s + 0x00) + value > *(float*)(s + 0x04) + EPSILON)
		{
			new_val = (*(float*)(s + 0x00) + value) - *(float*)(s + 0x04);
		}
		*(float*)(s + 0x00) += value - new_val;
		break;

	case 1: // maxhealth
		*(float*)(s + 0x04) += value;
		*(float*)(s + 0x00) = min(*(float*)(s + 0x00), *(float*)(s + 0x04)); // Ensure current health <= max health
		break;

	case 2: // chakra
		if (*(float*)(s + 0x08) + value > *(float*)(s + 0x0C) + EPSILON)
		{
			new_val = (*(float*)(s + 0x08) + value) - *(float*)(s + 0x0C);
		}
		*(float*)(s + 0x08) += value - new_val;
		break;

	case 3: // maxchakra
		*(float*)(s + 0x0C) += value;
		*(float*)(s + 0x08) = min(*(float*)(s + 0x08), *(float*)(s + 0x0C));
		break;

	case 4: // sub
		if (*(float*)(s + 0x10) + value > *(float*)(s + 0x14) + EPSILON)
		{
			new_val = (*(float*)(s + 0x10) + value) - *(float*)(s + 0x14);
		}
		*(float*)(s + 0x10) += value - new_val;
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

			*(int*)(char_p + 0xE74) = index; //Jutsu 1
		}
		else if (jutsu == 1) {

			*(int*)(char_p + 0xE78) = index; //Jutsu 2
		}
		else if (jutsu == 2) {

			*(int*)(char_p + 0xE7C) = index; // Ultimate Jutsu
		}
	}
	
}
void me_play_blur(__int64 p) {
	*(int*)(p + 72072) = 1;

}
void me_change_fov(__int64 p, float FOV) {
	if (FOV > 0)
		*(float*)(p + 72080) = FOV;
	else
		*(float*)(p + 72080) = -1;

}

void me_change_walk_speed(__int64 p, float speed) {
	*(float*)(p + 68916) = speed;
}
void me_change_jump_height(__int64 p, float jump_h) {
	*(float*)(p + 68908) = jump_h;
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

__int64 MovesetPlus::OriginalStageAddress2 = 0;

int MovesetPlus::STAGE_ID;
void me_test_switch_stage(__int64 function_param, __int64 param2, char* string_param, __int64 character_pointer )
{
	__int64 enemy_pointer;
	typedef signed  __int64(__fastcall* sub_1409B9FB0)(__int64 a1);
	sub_1409B9FB0 sub_1409B9FB0_f = (sub_1409B9FB0)(MovesetPlus::EffectHandlerAddress); //Get Enemy Pointer instead of player pointer

	typedef signed __int64(__fastcall* Sub_1408EAE00)(unsigned int);
	Sub_1408EAE00 sub_1408EAE00_f = (Sub_1408EAE00)(MovesetPlus::HandleStageChangeAddress);

	typedef signed __int64(__fastcall* Sub_1406F6800)(__int64);
	Sub_1406F6800 sub_1406F6800_f = (Sub_1406F6800)(MovesetPlus::DefaultStageHandlerAddress);

	typedef void(__fastcall* Sub_1406F6830)(__int64, unsigned int);
	Sub_1406F6830 sub_1406F6830_f = (Sub_1406F6830)(MovesetPlus::SpecificStageHandlerAddress);


	



	enemy_pointer = sub_1409B9FB0_f(character_pointer);
	if (*(int*)(character_pointer + 0xEF4) == 0) {

		*(int*)(character_pointer + 0xEF4) = *(int*)(Common::GetQword(stageOffset) + 8);
		*(int*)(enemy_pointer + 0xEF4) = *(int*)(Common::GetQword(stageOffset) + 8);
	}
	if (param2 == 0) {
		sub_1406F6830_f(Common::GetQword(stageOffset) + 8, crc32((std::string)string_param)); // Specific stage handler << endl;
	}
	else {
		sub_1406F6830_f(Common::GetQword(stageOffset) + 8, *(int*)(character_pointer + 0xEF4)); // Specific stage handler
	}


	cout << "Original Stage1: " << *(int*)(character_pointer + 0xEF4) << endl;
	cout << "Original Stage2: " << *(int*)(enemy_pointer + 0xEF4) << endl;
	cout << "Hash Stage: " << *(int*)(Common::GetQword(stageOffset) + 8) << endl;

	sub_1408EAE00_f(*(int*)(Common::GetQword(stageOffset) + 8));
}



__int64 MovesetPlus::BgmOffFunctionAdress = 0;
__int64 MovesetPlus::BgmOnFunctionAdress = 0;
__int64 MovesetPlus::BgmOff2FunctionAdress = 0;

int MovesetPlus::BGM_ID;
void me_PlayBGM(int snd)
{
	//find ccAdvPlayStageBgm or check offset 0xB2B10 in 1.01 version of file
	__int64 offset = 0x14218BE20;
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
	if (*(__int64*)(a1 + 71496))
		return *(__int64*)(a1 + 71496);
	return result;
}


__int64 __fastcall MovesetPlus::CancelActionFunction(__int64 a1, __int64 a2)
{
	__int64 result; // rax 


	// change 73712 value with update
	switch (*(int*)(a2 + 36))
	{
	case 0:
		*(int*)(a1 + 73712) = 1;
		result = 1i64;
		break;
	case 1:
		*(int*)(a1 + 73712) = 2; //JUMP CANCEL
		result = 1i64;
		break;
	case 2:
		*(int*)(a1 + 73712) = 4;
		result = 1i64;
		break;
	case 3:
		*(int*)(a1 + 73712) = 8; //DASH CANCEL
		result = 1i64;
		break;
	case 4:
		*(int*)(a1 + 73712) = 16; // COMBO CANCEL
		result = 1i64;
		break;
	case 5:
		*(int*)(a1 + 73712) = 32; // GRAB CANCEL
		result = 1i64;
		break;
	case 6:
		*(int*)(a1 + 73712) = 64; //JUTSU CANCEL
		result = 1i64;
		break;
	case 7:
		*(int*)(a1 + 73712) = 128;
		result = 1i64;
		break;
	case 8:
		*(int*)(a1 + 73712) = 256; //ULTIMATE JUTSU CANCEL
		result = 1i64;
		break;
	case 9:
		*(int*)(a1 + 73712) = 15; //WALK
		result = 1i64;
		break;
	case 0xA:
		*(int*)(a1 + 73712) = 208; //JUTSU, COMBO CANCEL
		result = 1i64;
		break;
	case 0xB:
		*(int*)(a1 + 73712) = 192;
		result = 1i64;
		break;
	case 0xC:
		*(int*)(a1 + 73712) = -1; //ALL CANCEL
		result = 1i64;
		break;
	case 0xD:
		*(int*)(a1 + 73712) = 512;
		result = 1i64;
		break;
	case 0xE:
		*(int*)(a1 + 73712) = 1024; //SHURIKEN CANCEL
		result = 1i64;
		break;
	case 0xF:
		*(int*)(a1 + 73712) = -1030; //COMBO, JUTSU, ULTIMATE, JUMP, GRAB, DASH
		result = 1i64;
		break;
	case 0x10:
		*(int*)(a1 + 73712) = -22; // SHURIKEN, JUTSU, ULTIMATE, JUMP, GRAB, DASH
		result = 1i64;
		break;
	case 0x11:
		*(int*)(a1 + 73712) = 1120; // GRAB, SHURIKEN, JUTSU
		result = 1i64;
		break;
	case 0x12:
		*(int*)(a1 + 73712) = -6; //COMBO, JUTSU, ULTIMATE, JUMP, SHURIKEN, GRAB, DASH
		result = 1i64;
		break;
	default:
		*(int*)(a1 + 73712) = 0;
		result = 1i64;
		break;
	}
	return result;
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