#ifndef PLUGIN_H
#define PLUGIN_H
#pragma once
#include <string>
#include <vector>

static bool enablemessages = 0;

namespace plugin
{
	inline __int64 moduleBase;
	inline __int64 moduleLength;
};

extern "C"
{
	__declspec(dllexport) void __stdcall InitializePlugin(__int64 a, std::vector<__int64> b);
	__declspec(dllexport) void __stdcall InitializeCommands(__int64 a, __int64 addCommandFunctionAddress);
	__declspec(dllexport) void __stdcall InitializeHooks(__int64 a, __int64 hookFunctionAddress);
	__declspec(dllexport) void __stdcall InitializeLuaCommands(__int64 a, __int64 addCommandFunction);
	__declspec(dllexport) void __stdcall GameLoop(__int64 a);
	__declspec(dllexport) bool __stdcall ParseApiFiles(__int64 a, std::string filePath, std::vector<char> fileBytes);
}

#endif