#ifndef MovesetPlus_H 
#define MovesetPlus_H
#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "Main.h"
#include "PatternScan.h"

inline __int64 BgmOffFunctionAdress;
inline __int64 BgmOnFunctionAdress;
inline __int64 BgmOff2FunctionAdress;
inline __int64 SlowTimeAddress;
inline __int64 SlowTimeAddress1;
inline __int64 SlowTimeAddress2;


inline __int64 EffectHandlerAddress;
inline __int64 ResolveEffectAddress;
inline __int64 ExecuteEffectAddress;
class MovesetPlus
{
public:
	static __int64 BgmOffFunctionAdress;
	static __int64 BgmOnFunctionAdress;
	static __int64 BgmOff2FunctionAdress;

	static __int64 EffectHandlerAddress;
	static __int64 InitializeHandlerAddress;
	static __int64 ResolveEffectAddress;
	static __int64 ExecuteEffectAddress;

	static __int64 CameraEffectAddress;
	static __int64 OriginalStageAddress;
	static __int64 HandleStageChangeAddress;
	static __int64 DefaultStageHandlerAddress;
	static __int64 SpecificStageHandlerAddress;
	static __int64 CharacterCondition_funcAddress;
	static __int64 FixCharacterPositionAddress;

	static __int64 OriginalStageAddress2;

	static __int64 __fastcall meTest(__int64 a1, __int64 a2);
	static __int64 __fastcall InitializeEffectHandler_f(__int64 a1);
	static __int64 __fastcall CancelActionFunction(__int64 a1, __int64 a2);
	static __int64 __fastcall ApplyStatusEffect(__int64 a1, __int64 a2);
	static void* __fastcall CharacterCondition_func(size_t a1, __int64 a2, int a3);
	static void __fastcall ccSndBgmCtrl(__int64 a1, int snd)
	{
		__int64 BgmOnFunctionAdressHeader = PatternScan::Scan("45xxxx48xxxxxxxxxxxx66xxxxxxxxxx39xx74xx49xxxx48xxxxxx49xxxxxx7Cxx33xxC3B8xxxxxxxxC3");
		typedef void(__fastcall* funct)(__int64 a1, int snd);
		funct fc = (funct)(BgmOnFunctionAdressHeader);
		return fc(a1, snd);
	}
	static int BGM_ID;
	static int STAGE_ID;
	static int stage_switcher;
};

#endif