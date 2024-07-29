#pragma once
#include <Windows.h>
#include <vector>
namespace plugin {
	static bool Hook(void*, void*, int, bool isPlugin = false);
	bool init();
	void hookall();
	bool hookfunc(__int64 funcaddr, LPVOID detourfunc, LPVOID* originalfunc);
	void enableall();
	bool enablehook(__int64 funcaddr);
	bool disablehook(__int64 funcaddr);
	inline std::vector<__int64> funclist;
}

