#include "Main.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "hooks.h"
#include "condition.h"
#include "TeamUltimateJutsuManager.h"
#include <Psapi.h>
#include "PatternScan.h"
#include "SpecialInteractingManager.h"
#include "ConditionPrmManager.h"
#include "PlAnmExpander.h"
#include "mem.h"
#include "BGMManager.cpp"
using namespace std;
std::string GetExeFileName()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}

// This function is called when booting the game. In the modding api, 0xC00 is added to the module base by default. In my modified code, I am removing it.
void __stdcall InitializePlugin(__int64 a, std::vector<__int64> b)
{
    std::string f = GetExeFileName();
    std::string e = f.substr(0, f.length() - 10) + "moddingapi\\mods\\base_game\\";
    plugin::moduleBase = a - 0xC00;
    MODULEINFO mInfo;
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &mInfo, sizeof(MODULEINFO));
    plugin::moduleLength = mInfo.SizeOfImage;

    condition::InitialScan();
    std::ifstream af(e + "specialCondParam.xfbin");
    af.is_open();
    bool test = af.good();
    if (test)
    {
        condition::ReadSpecialConditionParam(e + "specialCondParam.xfbin");
    }
    std::ifstream af_partner(e + "partnerSlotParam.xfbin");
    af_partner.is_open();
    bool test_partner = af_partner.good();
    if (test_partner)
    {
        condition::ReadPartnerSlotParam(e + "partnerSlotParam.xfbin");
    }
    std::ifstream af_pair(e + "pairSpSkillManagerParam.xfbin");
    af_pair.is_open();
    bool test_pair = af_pair.good();
    if (test_pair)
    {
        TUJManager::ReadPairSpSkillManagerParam(e + "pairSpSkillManagerParam.xfbin");
    }
    PlAnmExpander::ExpandStringArray();
    std::ifstream af_planm(e + "plAnmExpanderParam.xfbin");
    af_planm.is_open();
    bool test_planm = af_planm.good();
    if (test_planm)
    {
        PlAnmExpander::ReadPlAnmExpander(e + "plAnmExpanderParam.xfbin");
    }

    TUJManager::ExpandTeamUltimateArray();

    std::ifstream af_specialInteraction(e + "specialInteractionManager.xfbin");
    af_specialInteraction.is_open();
    bool test_specialInteraction = af_specialInteraction.good();
    if (test_specialInteraction)
    {
        SpecialInteractionManager::ReadSpecialInteractionParam(e + "specialInteractionManager.xfbin");
    }

    std::ifstream af_teamJutsu(e + "teamJutsuParam.xfbin");
    af_teamJutsu.is_open();
    bool test_teamJutsu = af_teamJutsu.good();
    if (test_teamJutsu)
    {
        SpecialInteractionManager::ReadTeamJutsuParam(e + "teamJutsuParam.xfbin");
    }


    std::ifstream af_conditionprm(e + "conditionprmManager.xfbin");
    af_conditionprm.is_open();
    bool test_conditionprm = af_conditionprm.good();
    if (test_conditionprm)
    {
        ConditionPrmManager::ReadConditionEntries(e + "conditionprmManager.xfbin");
    }


    //BGMs List

    BGMExpander::ExpandBGMList();
    std::ifstream af_bgmManagerParam(e + "bgmManagerParam.xfbin");
    af_bgmManagerParam.is_open();
    bool test_bgmManagerParam = af_bgmManagerParam.good();
    if (test_bgmManagerParam)
    {
        BGMExpander::ReadBGMFile(e + "bgmManagerParam.xfbin");
    }
}


// This function adds commands to the API's console.
void __stdcall InitializeCommands(__int64 a, __int64 addCommandFunctionAddress)
{
	typedef void(__stdcall *AddCmd)(std::string command, __int64 function, int paramcount);
	AddCmd AddCommand = (AddCmd)addCommandFunctionAddress;
	//AddCommand("PluginTest", (__int64)MessageCommand, 0);
}

// Use this function to hook any of the game's original functions. I have modified this to initialize and then use minhook for hooks.
void __stdcall InitializeHooks(__int64 a, __int64 hookFunctionAddress)
{
	plugin::init();
	plugin::hookall();
	plugin::enableall();
}

// Use this function to add any lua commands to the game.
void __stdcall InitializeLuaCommands(__int64 a, __int64 addCommandFunction)
{
	typedef void(__stdcall *LuaAddFc)(std::string command, __int64 function);
	LuaAddFc LuaAddFunct = (LuaAddFc)addCommandFunction;

	// Example: This line adds a command "LuaTest", which calls the function LuaTest in this code.
	// LuaAddFunct("LuaTest", (__int64)LuaTest);
}

// This function will be called all the time while you're playing after the plugin has been initialized.
void __stdcall GameLoop(__int64 a)
{
    //This if statement enables and disables the hud.
    if ((GetAsyncKeyState(VK_F1) & 0x01)) {
        if (render::hudon == 2) {
            std::cout << "No hud enabled!" << std::endl;
            render::hudon = 1;
        }
        else {
            std::cout << "No hud disabled!" << std::endl;
            render::hudon = 2;
        }
        std::array<std::uint8_t, 1> hudon_bytes{ render::hudon };
        util::memory::mem::write_bytes(condition::hudon_address, hudon_bytes);

    }
}

// This function is called when the API is loading a mod's files. Return true if the file was read by this plugin, otherwise return false for the API to manage the file.
bool __stdcall ParseApiFiles(__int64 a, std::string filePath, std::vector<char> fileBytes)
{
	
	return false;
}

__int64 plugin::api::RecalculateAddress(__int64 a)
{
	uintptr_t recalc = plugin::moduleBase + a;

	if (a > 0x13A38AD) recalc += 0x400;
	else if (a > 0xEA7420) recalc += 0x400;

	return recalc;
}


std::vector<int> plugin::api::crc32_table() {
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

int plugin::api::crc32(const std::string& input) {
    const auto table = plugin::api::crc32_table();
    int crc = 0xFFFFFFFF;
    for (unsigned char byte : input) {
        int lookup_index = ((crc >> 24) ^ byte) & 0xFF;
        crc = (crc << 8) ^ table[lookup_index];
    }
    crc = ~crc;
    return crc;
}