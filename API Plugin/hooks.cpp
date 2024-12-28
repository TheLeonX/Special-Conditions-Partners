#include <iostream>

#include "hooks.h"

#include "Main.h"

#include "condition.h"
#include "MovesetPlus.h"

#include "mem.h"
#include "PatternScan.h"
#include "Thirdparty/MinHook.h"
using namespace std;

//Use minhook to hook all functions here.
void plugin::hookall() {

	// Partner function
	__int64 partnerFunctionAddress = PatternScan::Scan("33xx48xxxxxxxxxxxx4Cxxxxxxxxxxxx3Bxx74xxFFxx48xxxxxx49xxxx7Cxx33xxC348xxxx48xxxxxxxxxxxx48xxxx48xxxxxx");
	if (partnerFunctionAddress > 1) {
		cout << "NS4Framework :: Creating Partners" << endl;
		plugin::Hook((void*)(partnerFunctionAddress), condition::CreatePartner, 16);
	}
	// Conditions
	__int64 charCondFunctionAddress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx5548xxxx48xxxxxx8Bxx81xxxxxxxxxx0Fxxxxxxxxxx8Dxxxx48xxxxxxxxxxxxE8xxxxxxxx85xx0Fxxxxxxxxxx");
	if (charCondFunctionAddress > 1) {
		cout << "CharacterConditionManager :: Creating Conditions" << endl;
		plugin::hookfunc(charCondFunctionAddress, condition::Create, (LPVOID*)&co);
	}
	// Moveset Plus function
	__int64 movesetPlusFunctionAddress = PatternScan::Scan("48xxxxxx48xxxxFFxxxxxxxxxx48xxxx74xx48xxxx48xxxxFFxxxxxxxxxxB8xxxxxxxx48xxxxxxC3");
	if (movesetPlusFunctionAddress > 1) {

		cout << "MovesetPlus :: Creating Functions" << endl;
		MovesetPlus::BgmOffFunctionAdress = PatternScan::Scan("48xxxxxxxx5748xxxxxx48xxxx48xxxx48xxxxxxxxxxxx8BxxE8xxxxxxxx48xxxxxxxxxxxx44xxxx33xxE8xxxxxxxxC7xxxxxxxxxxxxxxxxxxxx");
		MovesetPlus::BgmOnFunctionAdress = PatternScan::Scan("45xxxx48xxxxxxxxxxxx66xxxxxxxxxx39xx74xx49xxxx48xxxxxx49xxxxxx7Cxx33xxC3B8xxxxxxxxC3");
		MovesetPlus::BgmOff2FunctionAdress = PatternScan::Scan("48xxxxxxxx5748xxxxxx83xxxxxx8Bxx48xxxx75xx48xxxxxxxxxxxx48xxxx74xx48xxxxxxxx49xxxx39xxxxxxxxxxxx74xxF3xxxxxxxx45xxxx");


		MovesetPlus::EffectHandlerAddress = PatternScan::Scan("40xx48xxxxxx48xxxx33xxFFxxxx3Bxxxxxxxxxx74xx48xxxxxxxxxxxx4Cxxxxxx49xxxx49xxxxxx38xxxx75xx0Fxxxx");
		MovesetPlus::ResolveEffectAddress = PatternScan::Scan("48xxxxxxxx5748xxxxxx48xxxx33xx908BxxE8xxxxxxxx48xxxx74xx48xxxx48xxxx74xx4Cxxxx4Cxxxx66xxxxxxxxxx");
		MovesetPlus::ExecuteEffectAddress = PatternScan::Scan("4Cxxxx535741xx48xxxxxxxxxxxx0Fxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxx0Fxxxx41xxxx44xxxx");

		MovesetPlus::OriginalStageAddress = PatternScan::Scan("48xxxxxxxx5748xxxxxx8Bxxxxxxxxxx48xxxx65xxxxxxxxxxxxxxxx48xxxxB9xxxxxxxx4Dxxxxxx42xxxxxx39xxxxxxxxxx7Fxx");
		MovesetPlus::HandleStageChangeAddress = PatternScan::Scan("40xx48xxxxxx8Bxx48xxxxxxxxxxxxE8xxxxxxxx48xxxxxxxxxxxx8BxxE8xxxxxxxx48xxxxxxxxxxxxC7xxxxxxxxxxxxC7xxxxxxxxxxxx");
		MovesetPlus::DefaultStageHandlerAddress = PatternScan::Scan("40xx48xxxxxx83xxxxxx48xxxx74xx8BxxxxE8xxxxxxxxC7xxxxxxxxxxxx48xxxxxx5BC3");
		MovesetPlus::SpecificStageHandlerAddress = PatternScan::Scan("40xx48xxxxxx48xxxxxxxxxxxx48xxxx39xxxx74xxE8xxxxxxxxC7xxxxxxxxxxxx48xxxxxx5BC3");

		MovesetPlus::OriginalStageAddress2 = PatternScan::Scan("48xxxxxxxx5748xxxxxx8Bxxxxxxxxxx48xxxx65xxxxxxxxxxxxxxxx48xxxxB9xxxxxxxx4Dxxxxxx42xxxxxx39xxxxxxxxxx7Fxx");

		plugin::Hook((void*)(movesetPlusFunctionAddress), MovesetPlus::meTest, 16);

	}

	//Wall Run
	__int64 wallFunctionAdress = PatternScan::Scan("40xx48xxxxxx48xxxxxxxxxxxx33xx8Bxxxx81xxxxxxxxxx74xx81xxxxxxxxxx74xx81xxxxxxxxxx74xx8Bxx48xxxxxx");
	if (wallFunctionAdress > 1) {
		cout << "WallRunManager :: Hooking Wall Run" << endl;
		plugin::Hook((void*)(wallFunctionAdress), condition::WallRun, 15);
	}

	//No Clip Cursor
	__int64 noClipCursorFunctionAdress = PatternScan::Scan("40xx48xxxxxx48xxxx48xxxxxxxxxxxxE8xxxxxxxx48xxxxxx48xxxxFFxxxxxxxxxxxxxxxxFFxxxxxxxxxx48xxxx74xx41xxxxxxxxxx");
	if (noClipCursorFunctionAdress > 1) {
		cout << "No Clip Cursor Enabled" << endl;
		plugin::Hook((void*)(noClipCursorFunctionAdress), condition::NoClipCursor, 16);
	}
	//Disable Upgrade Cancellation
	__int64 upgradeCancelFunctionAdress = PatternScan::Scan("40xx48xxxxxx8Bxxxxxxxxxx48xxxx83xxxx83xxxx0Fxxxxxxxxxx8BxxxxxxxxxxE8xxxxxxxx83xxxx74xx8Bxxxxxxxxxx");
	if (upgradeCancelFunctionAdress > 1) {
		cout << "MovesetPlus :: Upgrade Cancel Disabled" << endl;
		plugin::Hook((void*)(upgradeCancelFunctionAdress), condition::EnableUpgradeCancel, 15);
		__int64 CancelFunctionAdress = PatternScan::Scan("48xxxxxxE8xxxxxxxxB8xxxxxxxx48xxxxxxC3");
		if (CancelFunctionAdress > 1) {
			condition::sub_140978B00Adress = PatternScan::Scan("48xxxxxxxx5748xxxxxx48xxxxxxxxxxxx48xxxxBAxxxxxxxx33xx48xxxxxxE8xxxxxxxx48xxxx74xx48xxxx");

			plugin::Hook((void*)(CancelFunctionAdress), condition::Motion_cancel_function, 14);
			cout << "MovesetPlus :: Update Upgrade Cancel Function" << endl;
		}
	}

	//Update Cancel Action Function
	__int64 ActionCancelFunctionAdress = PatternScan::Scan("48xxxxxxxx83xxxx0Fxxxxxxxxxx4Cxxxxxxxxxxxx41xxxxxxxxxxxxxx49xxxxFFxxC7xxxxxxxxxx01xxxxxxB8xxxxxxxxC3C7xxxxxxxxxx02xxxxxxB8xxxxxxxx");
	if (ActionCancelFunctionAdress > 1) {
		cout << "MovesetPlus :: Action Cancel Updated" << endl;
		plugin::Hook((void*)(ActionCancelFunctionAdress), MovesetPlus::CancelActionFunction, 14);
	}

	//Update Condition Add Param Function
	__int64 ConditionAddParamAdress = PatternScan::Scan("48xxxxxxxx5648xxxxxx48xxxxE8xxxxxxxx48xxxx48xxxx74xx48xxxxxxxx48xxxx48xxxxxxxxE8xxxxxxxx48xxxxE8xxxxxxxx0Fxxxxxx48xxxx");
	if (ConditionAddParamAdress > 1) {
		cout << "MovesetPlus :: Condition Add Function Updated" << endl;
		plugin::Hook((void*)(ConditionAddParamAdress), MovesetPlus::ApplyStatusEffect, 18);
	}


	//Expend Upgrade Function
	__int64 upgradeFunctionAdress = PatternScan::Scan("48xxxxxxxx5748xxxxxx8Bxxxxxxxxxx48xxxx89xxxxxxxxxxE8xxxxxxxx48xxxx74xx4Cxxxxxxxxxxxx44xxxxxx48xxxx48xxxxxxxxxxxx");
	if (upgradeFunctionAdress > 1) {
		cout << "MovesetPlus :: Upgrade Function Expanded" << endl;

		condition::sub_14097F1E0Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx5748xxxxxx48xxxx48xxxx48xxxxxx48xxxxxxxxxxxx48xxxx75xx48xxxxxxxxxxxx");
		condition::sub_1405C3970Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx55565741xx41xx41xx41xx48xxxxxx4Dxxxx49xxxx48xxxx4Cxxxx48xxxxxxxx");
		condition::sub_1405C7EC0Adress = PatternScan::Scan("45xxxx4Cxxxx45xxxx44xxxxxxxxxxxx7Exx45xxxx66xxxxxxxxxxxxxxxxxxxx48xxxxxxxxxxxx49xxxxxx");
		condition::sub_1405C8F00Adress = PatternScan::Scan("45xxxx4Cxxxx44xxxxxxxxxxxx7Exx45xxxx0Fxxxxxx66xxxxxxxxxxxxxxxxxx49xxxxxxxxxxxx49xxxxxx48xxxx");

		condition::cancelUpgradeFunctionState = 0;


		cout << "MotionBlurManager :: Fix Upgrade Function" << endl;
		__int64 fixUpgradeAddress1 = PatternScan::Scan("8B879C0E0000");
		__int64 fixUpgradeAddress2 = PatternScan::Scan("8B879C0E0000", fixUpgradeAddress1 + 6);
		__int64 fixUpgradeAddress3 = PatternScan::Scan("8B879C0E0000", fixUpgradeAddress2 + 6);
		const std::array<std::uint8_t, 2> fixUpgradeBytes1{ 0x04, 0x00 };
		util::memory::mem::write_bytes(fixUpgradeAddress3 + 2, fixUpgradeBytes1);


		plugin::Hook((void*)(upgradeFunctionAdress), condition::UpgradeVerUnlocker, 16);
	}

	//Increase costume limit
	__int64 costumeLimitAdress = PatternScan::Scan("48xxxxxxxx55565741xx41xx41xx41xx48xxxxxxxxxxxxxx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxx");
	if (costumeLimitAdress > 1) {
		cout << "Increase costume limit" << endl;
		__int64 costumeLimitAdress1 = PatternScan::Scan("488B7DA083F8");
		const std::array<std::uint8_t, 1> costumeLimitBytes1{ 0x64 };
		util::memory::mem::write_bytes(costumeLimitAdress1 + 6, costumeLimitBytes1);
	}

	//Motion blur
	__int64 MotionBlurAdress = PatternScan::Scan("B9xxxxxxxx48xxxxxx80xxxxxx75xxE8xxxxxxxx65xxxxxxxxxxxxxxxxB9xxxxxxxx48xxxxxx48xxxxxx83xxxxxxxxxxxx75xx48xxxxxxxxxxxxB9xxxxxxxxE8xxxxxxxx");
	if (MotionBlurAdress > 1) {
		cout << "MotionBlurManager :: Motion blur Hooked" << endl;
		__int64 MotionBlur1Adress = PatternScan::Scan("B92C34xxxx", MotionBlurAdress);
		const std::array<std::uint8_t, 1> motionBytes1 { 0x34 };
		util::memory::mem::write_bytes(MotionBlur1Adress+1, motionBytes1);

		__int64 MotionBlur2Adress = PatternScan::Scan("B90034xxxx", MotionBlurAdress);
		const std::array<std::uint8_t, 1> motionBytes2{ 0x08 };
		util::memory::mem::write_bytes(MotionBlur2Adress + 1, motionBytes2);


		__int64 MotionBlur3Adress = PatternScan::Scan("83xx4002xxxx", MotionBlurAdress);
		const std::array<std::uint8_t, 1> motionBytes3{ 0x48 };
		util::memory::mem::write_bytes(MotionBlur3Adress + 2, motionBytes3);
	}

	//Character Expander
	const std::array<std::uint8_t, 2> charBytes6{ 0x2C, 0x03 };
	__int64 characterExpend1Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx48xxxxxxxx5541xx41xx41xx41xx48xxxxxxxx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxx4CxxxxC7xxxxxxxxxxxx");
	if (characterExpend1Adress > 1){
		cout << "CharacterExpanderManager :: Character Code Hooked" << endl;
		__int64 char1Adress = PatternScan::Scan("C7xxxxxxxxxxxx41xxxxxxxxxx48xxxxxxxxxxxx", characterExpend1Adress);
		//cout << char1Adress << endl;
		util::memory::mem::write_bytes(char1Adress + 3, charBytes6);

		__int64 char2Adress = PatternScan::Scan("B9xxxxxxxxE8xxxxxxxx49xxxx45xxxxxx", characterExpend1Adress);
		//cout << char2Adress << endl;
		const std::array<std::uint8_t, 4> charBytes2{ 0x64, 0x78, 0x01, 0x00 };
		util::memory::mem::write_bytes(char2Adress + 1, charBytes2);


		__int64 char3Adress = PatternScan::Scan("41xxxxxxxxxxxx0Fxxxxxxxxxx48xxxxxx48xxxxE8xxxxxxxx4Cxxxxxxxxxxxxxx49xxxxxx", characterExpend1Adress);
		//cout << char3Adress << endl;
		util::memory::mem::write_bytes(char3Adress + 3, charBytes6);
	}
	//__int64 characterExpend2Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx48xxxxxxxx5541xx41xx41xx41xx48xxxx48xxxxxx48xxxxxxxxxxxx");
	//if (characterExpend2Adress > 1) {
	//	//cout << "CharacterExpanderManager :: Character Sounds Hooked" << endl;
	//	__int64 char4Adress = PatternScan::Scan("81xxxxxxxxxx0Fxxxxxxxxxx48xxxxxxxxxxxx4Cxxxxxxxxxxxx", characterExpend2Adress);
	//	cout << char4Adress << endl;
	//	util::memory::mem::write_bytes(char4Adress + 2, charBytes6);
	//}
	__int64 characterExpend6Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx48xxxxxxxx41xx48xxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxx33xx66xxxxxxxxxxxxxxxxxx");
	if (characterExpend6Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 1 Hooked" << endl;
		__int64 char8Adress = PatternScan::Scan("81xxxxxxxxxx77xxE8xxxxxxxx", characterExpend6Adress);
		//cout << char8Adress << endl;
		util::memory::mem::write_bytes(char8Adress + 2, charBytes6);
		//cout << "CharacterExpanderManager :: Character 2 Hooked" << endl;
		__int64 char9Adress = PatternScan::Scan("3Dxxxxxxxx77xx8Bxx", char8Adress);
		//cout << char9Adress << endl;
		util::memory::mem::write_bytes(char9Adress + 1, charBytes6);
		//cout << "CharacterExpanderManager :: Character 3 Hooked" << endl;
		__int64 char11Adress = PatternScan::Scan("3Dxxxxxxxx77xx8Bxx", char9Adress + 1);
		//cout << char11Adress << endl;
		util::memory::mem::write_bytes(char11Adress + 1, charBytes6);
	}
	__int64 characterExpend4Adress = PatternScan::Scan("48xxxxxx81xxxxxxxxxx77xxE8xxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxE9xxxxxxxx33xx48xxxxxxC3");
	if (characterExpend4Adress > 1) {
		//cout << "CharacterExpanderManager :: Character Sound Entries Hooked" << endl;
		__int64 char6Adress = PatternScan::Scan("81xxxxxxxxxx77xxE8xxxxxxxx", characterExpend4Adress);
		//cout << char6Adress << endl;
		util::memory::mem::write_bytes(char6Adress + 2, charBytes6);
	}
	__int64 characterExpend5Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx5748xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxxxx48xxxxxxxxxxxx48xxxxxxxxxxxx49xxxx");
	if (characterExpend5Adress > 1) {
		//cout << "CharacterExpanderManager :: Character Sound ACBs Hooked" << endl;
		__int64 char7Adress = PatternScan::Scan("41xxxxxxxxxxxx0Fxxxxxxxxxx83xxxx0Fxxxxxxxxxx41xxxxxx", characterExpend5Adress);
		//cout << char7Adress << endl;
		util::memory::mem::write_bytes(char7Adress + 3, charBytes6);
	}
	__int64 characterExpend3Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx48xxxxxxxx5541xx41xx41xx41xx48xxxx48xxxxxx48xxxxxxxxxxxx48xxxx48xxxxxx4Cxxxx");
	if (characterExpend3Adress > 1) {
		//cout << "CharacterExpanderManager :: Character PRM_Load Hooked" << endl;
		__int64 char5Adress = PatternScan::Scan("81xxxxxxxxxx0Fxxxxxxxxxx48xxxxxxxxxxxx4Cxxxxxxxxxxxx45xxxx44xxxx", characterExpend3Adress);
		//cout << char5Adress << endl;
		util::memory::mem::write_bytes(char5Adress + 2, charBytes6);
	}
	__int64 characterExpend7Adress = PatternScan::Scan("4Cxxxx41xx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxxxx4Cxxxxxxxxxxxx45xxxx44xxxxxxxx44xxxx89xxxxxx4Dxxxxxx");
	if (characterExpend7Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 4 Hooked" << endl;
		__int64 char12Adress = PatternScan::Scan("81xxxxxxxxxx77xx8BxxE8xxxxxxxx", characterExpend7Adress);
		//cout << char12Adress << endl;
		util::memory::mem::write_bytes(char12Adress + 2, charBytes6);
	}
	__int64 characterExpend8Adress = PatternScan::Scan("48xxxxxxxx5748xxxxxx41xxxx8Bxx81xxxxxxxxxx0FxxxxxxxxxxE8xxxxxxxx48xxxxxxxxxxxx48xxxxE8xxxxxxxx");
	if (characterExpend8Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 5 Hooked" << endl;
		__int64 char13Adress = PatternScan::Scan("81xxxxxxxxxx0FxxxxxxxxxxE8xxxxxxxx48xxxxxxxxxxxx48xxxxE8xxxxxxxx", characterExpend8Adress);
		//cout << char13Adress << endl;
		util::memory::mem::write_bytes(char13Adress + 2, charBytes6);
	}
	__int64 characterExpend9Adress = PatternScan::Scan("41xx41xx41xx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxxxx45xxxx44xxxxxxxx44xxxx89xxxxxxxx");
	if (characterExpend9Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 6 Hooked" << endl;
		__int64 char14Adress = PatternScan::Scan("41xxxxxxxxxxxx0Fxxxxxxxxxx41xxxx", characterExpend9Adress);
		//cout << char14Adress << endl;
		util::memory::mem::write_bytes(char14Adress + 3, charBytes6);
	}
	__int64 characterExpend10Adress = PatternScan::Scan("40xx555641xx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxxxx4Cxxxx41xxxx48xxxxxxxxxxxx8Bxx");
	if (characterExpend10Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 7 Hooked" << endl;
		__int64 char15Adress = PatternScan::Scan("81xxxxxxxxxx77xx8BxxE8xxxxxxxx", characterExpend10Adress);
		//cout << char15Adress << endl;
		util::memory::mem::write_bytes(char15Adress + 2, charBytes6);
	}
	__int64 characterExpend11Adress = PatternScan::Scan("48xxxxxx85xx74xx81xxxxxxxxxx77xx48xxxxxxxxE8xxxxxxxx48xxxxxxxxxxxx48xxxx");
	if (characterExpend11Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 8 Hooked" << endl;
		__int64 char16Adress = PatternScan::Scan("81xxxxxxxxxx77xx48xxxxxxxx", characterExpend11Adress);
		//cout << char16Adress << endl;
		util::memory::mem::write_bytes(char16Adress + 2, charBytes6);
	}
	__int64 characterExpend12Adress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx565741xx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxx49xxxx");
	if (characterExpend12Adress > 1) {
		//cout << "CharacterExpanderManager :: Character 9 Hooked" << endl;
		__int64 char17Adress = PatternScan::Scan("81xxxxxxxxxx0Fxxxxxxxxxx85xx78xx8BxxE8xxxxxxxx", characterExpend12Adress);
		//cout << char17Adress << endl;
		util::memory::mem::write_bytes(char17Adress + 2, charBytes6);
	}
}


bool plugin::Hook(void* toHook, void* ourFunct, int len, bool isPlugin)
{
	DWORD MinLen = 14;

	if (len < MinLen)
	{
		return false;
	}

	BYTE stub[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, // jmp qword ptr [$+6]
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // ptr
	};

	void* pTrampoline = VirtualAlloc(0, len + sizeof(stub), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	DWORD dwOld = 0;

	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &dwOld);

	DWORD64 retto = (DWORD64)toHook + len;

	// trampoline
	memcpy(stub + 6, &retto, 8);
	memcpy((void*)((DWORD_PTR)pTrampoline), toHook, len);
	memcpy((void*)((DWORD_PTR)pTrampoline + len), stub, sizeof(stub));

	// orig
	memcpy(stub + 6, &ourFunct, 8);

	memcpy(toHook, stub, sizeof(stub));

	for (int i = MinLen; i < len; i++)
	{
		*(BYTE*)((DWORD_PTR)toHook + i) = 0x90;
	}
	VirtualProtect(toHook, len, dwOld, &dwOld);

	return true;
}
//Hooks a singular function.
bool plugin::hookfunc(__int64 funcaddr, LPVOID detourfunc, LPVOID* originalfunc = NULL) {
	bool status = MH_CreateHook((LPVOID)funcaddr, detourfunc, originalfunc);
	if (status != MH_OK)
	{
		if (enablemessages)
			std::cout << "Hook at " << funcaddr << " could not be created." << std::endl;
		return 0;
	}
	if (enablemessages)
		std::cout << "Hook at " << funcaddr << " was created." << std::endl;
	plugin::funclist.push_back(funcaddr);
	return 1;
}

//Initializes Minhook
bool plugin::init() {
	if (MH_Initialize() == MH_OK) {
		if (enablemessages)
			std::cout << "Minhook initialized" << std::endl;
		return 1;
	}
	if (enablemessages)
		std::cout << "Minhook not initialized" << std::endl;
	return 0;
}

//Enables a single hook.
bool plugin::enablehook(__int64 funcaddr) {
	bool status = MH_EnableHook((LPVOID)funcaddr);
	if (status != MH_OK)
	{
		if (enablemessages)
			std::cout << "Hook at " << funcaddr << " could not be enabled." << std::endl;
		return 0;
	}
	if (enablemessages)
		std::cout << "Hook at " << funcaddr << " was enabled." << std::endl;
	return 1;
}

//Disables a single hook.
bool plugin::disablehook(__int64 funcaddr) {
	bool status = MH_DisableHook((LPVOID)funcaddr);
	if (status != MH_OK)
	{
		if (enablemessages)
			std::cout << "Hook at " << funcaddr << " could not be disabled." << std::endl;
		return 0;
	}
	if (enablemessages)
		std::cout << "Hook at " << funcaddr << " was disabled." << std::endl;
	return 1;
}

//Enables every hook initialized in hookall.
void plugin::enableall() {
	for (__int64 i : plugin::funclist) {
		plugin::enablehook(i);
	}
}