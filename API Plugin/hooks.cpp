#include <iostream>

#include "hooks.h"

#include "Main.h"

#include "condition.h"
#include "MovesetPlus.h"

#include "mem.h"
#include "PatternScan.h"
#include "Thirdparty/MinHook.h"
#include "SpecialInteractingManager.h"
#include "ConditionPrmManager.h"
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
		MovesetPlus::FixCharPositionAddress = PatternScan::Scan("48xxxx48xxxxxx48xxxxxx48xxxxxx5541xx41xx41xx41xx48xxxxxxxxxxxx48xxxxxxxxxxxx0Fxxxxxx0Fxxxxxx44xxxxxxxx44xxxxxxxx44xxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxx4Cxxxx33xx89xxxxxx48xxxxxxxxxxxx45xxxxxx");

		MovesetPlus::VTableAddress = PatternScan::Scan("48xxxxxxxx5748xxxxxx8Bxxxxxxxxxx48xxxx65xxxxxxxxxxxxxxxx48xxxxB9xxxxxxxx4Dxxxxxx42xxxxxx39xxxxxxxxxx7Fxx48xxxxE8xxxxxxxx48xxxxxx4Cxxxx");
		
		
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


		cout << "MovesetPlus :: Fix Upgrade Function" << endl; //find sub_1409B94C0 in 1.50 exe and find 112 characode (5mdr). its some switch value
		__int64 fixUpgradeAddress1 = PatternScan::Scan("8B87A00E0000");
		__int64 fixUpgradeAddress2 = PatternScan::Scan("8B87A00E0000", fixUpgradeAddress1 + 6);
		__int64 fixUpgradeAddress3 = PatternScan::Scan("8B87A00E0000", fixUpgradeAddress2 + 6);
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

	//Increase Memory Limits sub_140605050 (1.50)
	__int64 MemoryLimitAddress = PatternScan::Scan("48xxxxxxxx5748xxxxxxE8xxxxxxxxC7xxxxxxxxxxxxE8xxxxxxxx48xxxxBAxxxxxxxxE8xxxxxxxxE8xxxxxxxx");
	if (MotionBlurAdress > 1) {
		cout << "MemoryLimitExpand :: Memory Increase Start" << endl;
		__int64 MemoryLimit1Address = PatternScan::Scan("48xxxxxxxxxxxx0000A000", MemoryLimitAddress);
		const std::array<std::uint8_t, 4> MemoryLimitBytes1{ 0x00, 0x00, 0x40, 0x01 };
		util::memory::mem::write_bytes(MemoryLimit1Address + 7, MemoryLimitBytes1);
		cout << "MemoryLimitExpand :: Memory Increase 1 " << MemoryLimit1Address << endl;


		__int64 MemoryLimit2Address = PatternScan::Scan("41xx0000A000", MemoryLimit1Address);
		const std::array<std::uint8_t, 4> MemoryLimitBytes2{ 0x00, 0x00, 0x40, 0x01 };
		util::memory::mem::write_bytes(MemoryLimit2Address + 2, MemoryLimitBytes2);
		cout << "MemoryLimitExpand :: Memory Increase 2 " << MemoryLimit2Address << endl;


		__int64 MemoryLimit3Address = PatternScan::Scan("41xx00000019", MemoryLimit2Address); //App 400MB -> 800MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes3{ 0x00, 0x00, 0x00, 0x32 };
		util::memory::mem::write_bytes(MemoryLimit3Address + 2, MemoryLimitBytes3);
		cout << "MemoryLimitExpand :: Memory Increase 3 " << MemoryLimit3Address << endl;


		__int64 MemoryLimit4Address = PatternScan::Scan("41xx0000E015", MemoryLimit3Address); //UI 350MB -> 700MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes4{ 0x00, 0x00, 0xC0, 0x2B };
		util::memory::mem::write_bytes(MemoryLimit4Address + 2, MemoryLimitBytes4);
		cout << "MemoryLimitExpand :: Memory Increase 4 " << MemoryLimit4Address << endl;


		__int64 MemoryLimit5Address = PatternScan::Scan("48xxxxxxxx0000A000", MemoryLimit4Address); //Param 10MB -> 20MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes5{ 0x00, 0x00, 0x40, 0x01 };
		util::memory::mem::write_bytes(MemoryLimit5Address + 5, MemoryLimitBytes5);
		cout << "MemoryLimitExpand :: Memory Increase 5 " << MemoryLimit5Address << endl;


		__int64 MemoryLimit6Address = PatternScan::Scan("48xxxxxxxx0000A000", MemoryLimit5Address); //Skill 10MB -> 20MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes6{ 0x00, 0x00, 0x40, 0x01 };
		util::memory::mem::write_bytes(MemoryLimit6Address + 5, MemoryLimitBytes6);
		cout << "MemoryLimitExpand :: Memory Increase 6 " << MemoryLimit6Address << endl;


		__int64 MemoryLimit7Address = PatternScan::Scan("41xx00002003", MemoryLimit6Address); //Texture 50MB -> 300MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes7{ 0x00, 0x00, 0xC0, 0x12 };
		util::memory::mem::write_bytes(MemoryLimit7Address + 2, MemoryLimitBytes7);
		cout << "MemoryLimitExpand :: Memory Increase 7 " << MemoryLimit7Address << endl;


		__int64 MemoryLimit8Address = PatternScan::Scan("41xx0000E001", MemoryLimit7Address); //Sound 30MB -> 60MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes8{ 0x00, 0x00, 0xC0, 0x03 };
		util::memory::mem::write_bytes(MemoryLimit8Address + 2, MemoryLimitBytes8);
		cout << "MemoryLimitExpand :: Memory Increase 8 " << MemoryLimit8Address << endl;


		__int64 MemoryLimit9Address = PatternScan::Scan("41xx00009001", MemoryLimit8Address); //S_Texture 25MB -> 50MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes9{ 0x00, 0x00, 0x20, 0x03 };
		util::memory::mem::write_bytes(MemoryLimit9Address + 2, MemoryLimitBytes9);
		cout << "MemoryLimitExpand :: Memory Increase 9 " << MemoryLimit9Address << endl;


		__int64 MemoryLimit10Address = PatternScan::Scan("41xx00002003", MemoryLimit9Address); //Stage 50MB -> 200MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes10{ 0x00, 0x00, 0x80, 0x0C };
		util::memory::mem::write_bytes(MemoryLimit10Address + 2, MemoryLimitBytes10);
		cout << "MemoryLimitExpand :: Memory Increase 10 " << MemoryLimit10Address << endl;



		__int64 MemoryLimit11Address = PatternScan::Scan("41xx00004001", MemoryLimit10Address); //StageSub 20MB -> 40MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes11{ 0x00, 0x00, 0x80, 0x02 };
		util::memory::mem::write_bytes(MemoryLimit11Address + 2, MemoryLimitBytes11);
		cout << "MemoryLimitExpand :: Memory Increase 11 " << MemoryLimit11Address << endl;


		__int64 MemoryLimit12Address = PatternScan::Scan("41xx0000E001", MemoryLimit11Address); //Player 30MB -> 60MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes12{ 0x00, 0x00, 0xC0, 0x03 };
		util::memory::mem::write_bytes(MemoryLimit12Address + 2, MemoryLimitBytes12);
		cout << "MemoryLimitExpand :: Memory Increase 12 " << MemoryLimit12Address << endl;

		__int64 MemoryLimit13Address = PatternScan::Scan("41xx0000E015", MemoryLimit12Address); //Resident 350MB -> 700MB
		const std::array<std::uint8_t, 4> MemoryLimitBytes13{ 0x00, 0x00, 0xC0, 0x2B };
		util::memory::mem::write_bytes(MemoryLimit13Address + 2, MemoryLimitBytes13);
		cout << "MemoryLimitExpand :: Memory Increase 13 " << MemoryLimit13Address << endl;
		cout << "MemoryLimitExpand :: Memory Increase Complete!" << endl;

	}

	char ApiPath[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, ApiPath);
	int ActualLength = strlen(ApiPath);

	strcat(ApiPath, "\\moddingapi\\");

	char ConfigPath[_MAX_PATH];
	strcpy(ConfigPath, ApiPath);
	ActualLength = strlen(ConfigPath);
	strcat(ConfigPath, "config.ini");

	__int64 debug_str_address = PatternScan::Scan("48xxxxxxxx4Cxxxxxxxx4CxxxxxxxxC30Fxxxx33xx4Cxxxx44xxxx85xx74xx908Bxx4Dxxxxxx83xxxx83xxxxD3xx41xxxx41xxxxxxxxxxxx33xx41xxxxxx85xx75xxC3");
	if (GetPrivateProfileInt("General", "enable_debug", 1, ConfigPath) == 1 && debug_str_address > 1)
	{
		plugin::Hook((void*)(debug_str_address), condition::sub_1412528C0, 15);
	}




	//plugin::Hook((void*)(plugin::moduleBase + 0xAD1B50), condition::SkipIntroFunction, 15); //1.60

	//SpecialInteraction UJ
	__int64 specialInteractionChangerAddress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx5741xx41xx48xxxxxx8Bxxxxxxxxxx33xx41xxxx8Bxx4Cxxxx44xxxx83xxxx74xx3Dxxxxxxxx75xx81xxxxxxxxxx75xx48xxxxxxxxxxxx48xxxx74xx8BxxE8xxxxxxxx4Cxxxx48xxxx0Fxxxxxxxxxx85xx");
	if (specialInteractionChangerAddress > 1) {
		SpecialInteractionManager::sub_1409BB1B0Adress = PatternScan::Scan("40xx48xxxxxx48xxxx33xxFFxxxx3Bxxxxxxxxxx74xx48xxxxxxxxxxxx4Cxxxxxx49xxxx49xxxxxx38xxxx75xx0Fxxxx39xxxx73xx48xxxxxxEBxx48xxxx48xxxx38xxxx74xx38xxxx");
		cout << "SpecialInteractionManager :: Initializing" << endl;
		plugin::Hook((void*)(specialInteractionChangerAddress), SpecialInteractionManager::SpecialInteractionUJChanger, 15);
		//cout << "SpecialInteractionManager :: Initializing Sounds" << endl;
		__int64 specialInteractionSoundAddress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx48xxxxxxxx48xxxxxxxx41xx48xxxxxx8Bxxxxxxxxxx48xxxx33xxE8xxxxxxxx8Bxx83xxxx74xx83xxxx75xx48xxxx44xxxxE8xxxxxxxx48xxxx74xx44xxxxxxxxxxxx83xxxx75xx41xxxx48xxxxE8xxxxxxxx85xx74xx8Dxxxx");
		plugin::Hook((void*)(specialInteractionSoundAddress), SpecialInteractionManager::SpecialInteractionSoundFunction, 15);
		
		cout << "SpecialInteractionManager :: Initializing Sounds" << endl;
		__int64 specialInteractionSound2Address = PatternScan::Scan("48xxxxxxxx5748xxxxxx48xxxx33xxE8xxxxxxxx48xxxx0Fxxxxxxxxxx8Bxxxxxxxxxx8Bxxxxxxxxxx83xxxx74xx83xxxx74xx83xxxx74xx83xxxx75xx48xxxxE8xxxxxxxx85xx74xxBBxxxxxxxx8Bxx48xxxxxxxx48xxxxxx5FC348xxxx");
		plugin::Hook((void*)(specialInteractionSound2Address), SpecialInteractionManager::SpecialInteractionFunction2, 15);

		cout << "SpecialInteractionManager :: Initializing PRM Function" << endl;
		__int64 specialInteractionPRMFunctionAddress = PatternScan::Scan("48xxxxxxxx48xxxxxxxx41xx48xxxxxx8Dxxxxxxxxxx8Bxx4Cxxxx8Bxx3Dxxxxxxxx0Fxxxxxxxxxx48xxxxxxxxE8xxxxxxxx83xxxx75xx8Dxxxx49xxxxE8xxxxxxxx83xxxx75xx8Dxxxx49xxxxE8xxxxxxxx83xxxx75xx8Dxxxxxxxxxx41xxxxxxxxxxxx");
		plugin::Hook((void*)(specialInteractionPRMFunctionAddress), SpecialInteractionManager::AdjustSpecialInteractionValue, 16);


		SpecialInteractionManager::sub_140AC49A0Adress = PatternScan::Scan("83xxxx77xx83xxxx77xx4Cxxxxxxxxxxxx4Dxxxx74xx48xxxx49xxxxxx48xxxxxx48xxxxxx49xxxxE9xxxxxxxx33xxC383xxxx77xx83xxxx77xx4Cxxxxxxxxxxxx4Dxxxx74xx48xxxx49xxxxxxFFxx48xxxxxx48xxxxxx49xxxxE9xxxxxxxx33xxC3");

	}

	//conditionprm
	__int64 conditionprmAddress = PatternScan::Scan("8Dxxxx3DFB01000077xx48xxxx48xxxxxxxxxxxx48xxxxxx48xxxxC333xxC3");
	if (conditionprmAddress > 1) {

		

		plugin::Hook((void*)(conditionprmAddress), ConditionPrmManager::GetConditionEntry, 20);

		__int64 conditionprmAddress1 = PatternScan::Scan("4Cxxxxxx4Dxxxx74xx81xxFC01000077xx48xxxx48xxxxxxxxxxxx49xxxxC333xxC3");
		__int64 conditionprmAddress1_2 = PatternScan::Scan("FC010000", conditionprmAddress1);
		const std::array<std::uint8_t, 4> conditionBytes1{
		   static_cast<std::uint8_t>((g_ConditionEntries.size() - 1) & 0xFF),
		   static_cast<std::uint8_t>(((g_ConditionEntries.size() - 1) >> 8) & 0xFF),
		   static_cast<std::uint8_t>(((g_ConditionEntries.size() - 1) >> 16) & 0xFF),
		   static_cast<std::uint8_t>(((g_ConditionEntries.size() - 1) >> 24) & 0xFF)
		};
		const std::array<std::uint8_t, 4> conditionBytes2{
		   static_cast<std::uint8_t>((g_ConditionEntries.size()) & 0xFF),
		   static_cast<std::uint8_t>(((g_ConditionEntries.size()) >> 8) & 0xFF),
		   static_cast<std::uint8_t>(((g_ConditionEntries.size()) >> 16) & 0xFF),
		   static_cast<std::uint8_t>(((g_ConditionEntries.size()) >> 24) & 0xFF)
		};
		util::memory::mem::write_bytes(conditionprmAddress1_2, conditionBytes1);


		__int64 conditionprmAddress2 = PatternScan::Scan("48xxxxxxxx5748xxxxxx8Bxx33xx66xx8BxxE8xxxxxxxx48xxxx74xx48xxxx48xxxx74xxE8xxxxxxxx3Bxx74xxFFxx81xxFD0100007Cxx33xx48xxxxxxxx48xxxxxx5FC38Bxx48xxxxxxxx48xxxxxx5FC3");
		__int64 conditionprmAddress2_2 = PatternScan::Scan("FD010000", conditionprmAddress2);
		__int64 conditionprmAddress2_3 = PatternScan::Scan("FD010000", conditionprmAddress2_2 + 4);
		__int64 conditionprmAddress2_4 = PatternScan::Scan("FD010000", conditionprmAddress2_3 + 4);

		cout << conditionprmAddress2 << endl;
		util::memory::mem::write_bytes(conditionprmAddress2_2, conditionBytes2);
		util::memory::mem::write_bytes(conditionprmAddress2_3, conditionBytes2);
		util::memory::mem::write_bytes(conditionprmAddress2_4, conditionBytes2);
	}
	// Hide HUD
	__int64 hudon_address1 = PatternScan::Scan("48xxxxxxxx48xxxxxxxx41xx48xxxxxx48xxxx49xxxx0Fxxxxxx4Cxxxx0Fxxxx24xx3Cxx74xxF6xxxx75xx33xx49xxxxxx49xxxx41xxxxxx48xxxxxxxx48xxxxxxxx48xxxxxx41xxC38Bxxxxxxxxxx");
	condition::hudon_address = PatternScan::Scan("3C01", hudon_address1) + 1;


	// Enable Monitor on stages
	__int64 monitor_address = PatternScan::Scan("48xxxx48xxxxxx48xxxxxx48xxxxxx5541xx41xx41xx41xx48xxxxxxxxxxxx48xxxxxxxxxxxx0Fxxxxxx0Fxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxxxxxxxx4Cxxxx48xxxxxxxx48xxxxxxxx48xxxxFFxxxx904Cxxxxxxxxxxxx");
	__int64 monitor_address2 = PatternScan::Scan("C7xxxxxxxxxx00000000", monitor_address) + 6;
	const std::array<std::uint8_t, 4> monitor_bytes{ 0x01, 0x00, 0x00, 0x00 };
	util::memory::mem::write_bytes(monitor_address2, monitor_bytes);



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
