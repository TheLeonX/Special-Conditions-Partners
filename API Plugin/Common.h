#pragma once
extern __int64 st_hModule;
extern __int64 datasection;

extern int PluginState;
#include "Main.h"
#include <widemath.h>

class Common
{
public:
	static constexpr unsigned int str2int(const char* str, int h)
	{
		return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
	}

	static __int64 GetQword(__int64 qw)
	{
		__int64 qwr = qw - 0x140000000;
		return *(__int64*)(plugin::moduleBase + qwr);
	}

	struct Xmmword {
		__int64 low;  // Нижние 64 бита
		__int64 high; // Верхние 64 бита
	};

	static __int64* GetXmmword(__int64 addr) {
		__int64 offset = addr - 0x140000000;
		return *(__int64**)(plugin::moduleBase + offset);
	}

	// Get a DWORD pointer from the game
	static int GetDword(__int64 dw)
	{
		__int64 dwr = dw - 0x140000000;
		return *(int*)(plugin::moduleBase + dwr);
	}

	// Get an OFFSET pointer from the game
	static __int64 GetOffset(__int64 of)
	{
		__int64 ofr = of + 0x1000;
		return (plugin::moduleBase + ofr);
	}
};