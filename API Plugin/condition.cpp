#include "condition.h"

#include "Main.h"

#include <iostream>
#include <Windows.h>
#include "PatternScan.h"
#include "Common.h"
using namespace std;

int condition::cancelUpgradeFunctionState = 0;
__int64 condition::sub_14097F1E0Adress = 0;
__int64 condition::sub_1405C3970Adress = 0;
__int64 condition::sub_1405C7EC0Adress = 0;
__int64 condition::sub_1405C8F00Adress = 0;
__int64 condition::sub_140978B00Adress = 0;

std::vector<BYTE> condition::ReadAllBytes(std::string _file)
{
	std::ifstream f;
	f.open(_file);

	int FileSize = 0;
	while (!f.eof())
	{
		f.get();
		FileSize++;
	}

	f.close();
	f.open(_file, std::ios::binary);

	std::vector<BYTE> result(FileSize);

	f.seekg(0, std::ios::beg);

	for (int x = 0; x < FileSize; x++)
	{
		BYTE a = f.get();
		memcpy((void*)&result[0 + x], &a, 1);
	}

	f.close();

	return result;
}

std::vector<PartnerFunction*> condition::partnerFunctionList;
std::vector<PartnerFramework*> condition::frameworkPartnerList;

void condition::InitialScan()
{
	__int64 partnerBase = PatternScan::Scan("327474693030743020722068616E64003268686130307430206C2068616E6400327474693030743020722066696E676572300000000000003268686130307430206C2066696E67657230000000000000", 0, false);
	if (partnerBase > 0) partnerBase = PatternScan::Scan("2B00000000000000", partnerBase, true) - 8;

	if (partnerBase < 1)
	{
		std::cout << "PartnerManager :: Partner search failed" << std::endl;
		return;
	}
	else
	{
		std::cout << "PartnerManager :: Found Instance" << std::endl;
	}

	int partnerCount = 0;
	__int64 actualPartner = partnerBase;

	int* checkIntA = (int*)(actualPartner + 0xA);
	short* checkIntB = (short*)(actualPartner + 0xE);
	while (*checkIntA == 0 && *checkIntB == 0)
	{
		partnerCount++;

		PartnerFunction* partner = new PartnerFunction();
		partner->address = actualPartner;

		int characode = *(int*)(actualPartner + 8);
		switch (characode)
		{
		default:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_UNK";
			std::cout << "PartnerSystem :: Unknown partner type found" << std::endl;
			break;
		}
		case 0x2B:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_2PAR";
			break;
		}
		case 0x2C:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_2KKG";
			break;
		}
		case 0x29:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_2AKM";
			break;
		}
		case 0x2A:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_2KRS";
			break;
		}
		case 0x66:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_1AKM";
			break;
		}
		case 0x6A:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_1KRS";
			break;
		}
		case 0x45:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_2SCX";
			break;
		}
		case 0xE6:
		{
			partner->PARTNER_TYPE = "PARTNER_TYPE_8AEM";
			break;
		}
		}

		condition::partnerFunctionList.push_back(partner);

		actualPartner += 0x10;
		checkIntA = (int*)(actualPartner + 0xA);
		checkIntB = (short*)(actualPartner + 0xE);
	}

}

void condition::ReadPartnerSlotParam(std::string _file)
{
	std::vector<BYTE> fileBytes = condition::ReadAllBytes(_file);

	int slotCount = fileBytes.size() / 0x20;

	for (int x = 0; x < slotCount; x++)
	{
		std::string slotType = "";
		int slotCharacter = 0;
		int actual = (0x20 * x);

		while (actual < (0x20 * x) + 0x17)
		{
			if (fileBytes[actual] != 0x0)
			{
				slotType = slotType + (char)fileBytes[actual];
				actual++;
			}
			else
			{
				actual = (0x20 * x) + 0x17;
			}
		}

		slotCharacter = (fileBytes[actual] * 0x1) + (fileBytes[actual + 1] * 0x100);

		uintptr_t slotFunct = 0x0;
		int slotIndex = -1;
		int gamePartnerCount = condition::partnerFunctionList.size();

		for (int a = 0; a < gamePartnerCount; a++)
		{
			std::string thisSlotType = condition::partnerFunctionList[a]->PARTNER_TYPE;

			if (thisSlotType == slotType)
			{
				slotIndex = a;
				a = gamePartnerCount;
			}
		}

		if (slotIndex == -1)
		{
			std::cout << "PartnerManager :: Error loading character " << std::hex << slotCharacter << " - Attempted using an unknown type of partner (" << slotType << ")" << std::endl;
		}
		else
		{
			PartnerFramework* newPartner = new PartnerFramework();
			newPartner->address = condition::partnerFunctionList[slotIndex]->address;
			newPartner->characode = slotCharacter;
			condition::frameworkPartnerList.push_back(newPartner);
		}
	}
}

void condition::ReadSpecialConditionParam(std::string _file)
{
	std::vector<BYTE> fileBytes = condition::ReadAllBytes(_file);

	int slotCount = fileBytes.size() / 0x20;

	for (int x = 0; x < slotCount; x++)
	{
		std::string slotType = "";
		int condCharacter = 0;
		int actual = (0x20 * x);

		while (actual < (0x20 * x) + 0x17)
		{
			if (fileBytes[actual] != 0x0)
			{
				slotType = slotType + (char)fileBytes[actual];
				actual++;
			}
			else
			{
				actual = (0x20 * x) + 0x17;
			}
		}

		condCharacter = (fileBytes[actual] * 0x1) + (fileBytes[actual + 1] * 0x100);

		int condFunct = 0x0;
		int actualConditionIndex = -1;
		bool found = false;

		if (slotType == "COND_1CMN") condFunct = 0x0;
		else if (slotType == "COND_2SIK") condFunct = 0xA;
		else if (slotType == "COND_2HDN") condFunct = 0x20;
		else if (slotType == "COND_2KKS") condFunct = 0x12;
		else if (slotType == "COND_2CYB") condFunct = 0x1B;
		else if (slotType == "COND_2SCO") condFunct = 0x1E;
		else if (slotType == "COND_2DDR") condFunct = 0x1F;
		else if (slotType == "COND_3HNZ") condFunct = 0x51;
		else if (slotType == "COND_3TOB") condFunct = 0x49;
		else if (slotType == "COND_3TYO") condFunct = 0x53;
		else if (slotType == "COND_3MDR_2") condFunct = 0x6F;
		else if (slotType == "COND_3KBT") condFunct = 0x5E;
		else if (slotType == "COND_2KNK") condFunct = 0x10;
		else if (slotType == "COND_2JRY") condFunct = 0x15;
		else if (slotType == "COND_3GAR") condFunct = 0x56; // same as below
		else if (slotType == "COND_2GAV") condFunct = 0x70; // same as above
		else if (slotType == "COND_2ORC") condFunct = 0x17;
		else if (slotType == "COND_5KGY") condFunct = 0x7C;
		else if (slotType == "COND_2FOU") condFunct = 0x2D;
		else if (slotType == "COND_2DNZ") condFunct = 0x3A;
		else if (slotType == "COND_3GUY") condFunct = 0xC4;
		else if (slotType == "COND_2KBT") condFunct = 0x18;
		else if (slotType == "COND_2MDR") condFunct = 0x39;
		else if (slotType == "COND_2KIB") condFunct = 0x0C;
		else if (slotType == "COND_2KNN") condFunct = 0x26;
		else if (slotType == "COND_2SGT") condFunct = 0x22;
		else if (slotType == "COND_4MKG") condFunct = 0x57;
		else if (slotType == "COND_2NRT") condFunct = 0x01; // same as below
		else if (slotType == "COND_2NRG") condFunct = 0x37; // same as below
		else if (slotType == "COND_3NRT") condFunct = 0x54; // same as above
		else if (slotType == "COND_2NRX") condFunct = 0x02;
		else if (slotType == "COND_3SSK") condFunct = 0x55;
		else if (slotType == "COND_5TYY") condFunct = 0x76;
		else if (slotType == "COND_5MDR") condFunct = 0x72;
		else if (slotType == "COND_5KDM") condFunct = 0x74;
		else if (slotType == "COND_BMDR") condFunct = 0x78;
		else if (slotType == "COND_1KNK") condFunct = 0x69;
		else if (slotType == "COND_2YMT") condFunct = 0x1D;
		else if (slotType == "COND_3MDR") condFunct = 0x46;
		else if (slotType == "COND_5SKN") condFunct = 0x75;
		else if (slotType == "COND_5JRB") condFunct = 0x73;
		else if (slotType == "COND_7BRX") condFunct = 0xCE;
		else if (slotType == "COND_7SKD") condFunct = 0xD0;
		else if (slotType == "COND_7YRI") condFunct = 0xD3;
		else if (slotType == "COND_7NRN") condFunct = 0xC8;
		else if (slotType == "COND_7SSX") condFunct = 0xC9;
		else if (slotType == "COND_7MMS") condFunct = 0xD1;
		else if (slotType == "COND_7KIN") condFunct = 0xD2;
		else if (slotType == "COND_7GAR") condFunct = 0xCB;
		else if (slotType == "COND_7MTK") condFunct = 0xD8;
		else if (slotType == "COND_8MMS") condFunct = 0xDA;
		else if (slotType == "COND_8KIN") condFunct = 0xDB;
		else if (slotType == "COND_8KNK") condFunct = 0xE3;
		else if (slotType == "COND_8SIK") condFunct = 0xDD;
		else if (slotType == "COND_8TYO") condFunct = 0xDE;
		else if (slotType == "COND_8SSF") condFunct = 0xF1;
		else if (slotType == "COND_9BOR") condFunct = 0xEF;
		else if (slotType == "COND_8MTA") condFunct = 0xF0;
		else if (slotType == "COND_9DLT") condFunct = 0xEE;
		else if (slotType == "COND_9KKJ") condFunct = 0xED;
		else if (slotType == "COND_9KWK") condFunct = 0xEB;
		else if (slotType == "COND_9JGN") condFunct = 0xEC;
		else if (slotType == "COND_9NRT") condFunct = 0xE9;
		else if (slotType == "COND_8BRA") condFunct = 0xEA;
		else if (slotType == "COND_9ASR") condFunct = 0xE8;
		else if (slotType == "COND_BKRL") condFunct = 0x8C;
		else if (slotType == "COND_9ISH") condFunct = 0x114;
		else if (slotType == "COND_9KRN") condFunct = 0x115;
		else if (slotType == "COND_9BRM") condFunct = 0x117;
		else if (slotType == "COND_BSSN") condFunct = 0x8D;
		else if (slotType == "COND_BOBT") condFunct = 0x8E;
		else if (slotType == "COND_BKRS") condFunct = 0x9B;
		else if (slotType == "COND_BNRX") condFunct = 0xB2;
		else if (slotType == "COND_BSSX") condFunct = 0xB3;
		else if (slotType == "COND_BSKK") condFunct = 0xFB;
		else if (slotType == "COND_BNRY") condFunct = 0xFC;
		else if (slotType == "COND_BNRK") condFunct = 0x105;
		else if (slotType == "COND_BNRM") condFunct = 0x106;
		else if (slotType == "COND_BNNS") condFunct = 0xF2;
		else if (slotType == "COND_BMRL") condFunct = 0xF7;

		charfunction.push_back(condFunct);
		charcodes.push_back(condCharacter);

	}
}


__int64 condition::CreatePartner(__int64 player, int characode)
{
	//std::cout << "Attempted to create partner with characode " << std::hex << std::uppercase << characode << std::endl;

	int partnerCount = condition::frameworkPartnerList.size();
	__int64 address = 0;

	for (int x = 0; x < partnerCount; x++)
	{
		if (condition::frameworkPartnerList[x]->characode == characode)
		{
			address = condition::frameworkPartnerList[x]->address;
			x = partnerCount;
		}
	}

	//std::cout << "Address: " << std::hex << std::uppercase << address << std::endl;
	address = *(__int64*)address;

	typedef __int64(__fastcall* funct)();
	funct fc = (funct)(address);
	__int64 result = 0;
	if (fc) result = fc();

	//std::cout << "Result: " << std::hex << std::uppercase << result << std::endl;

	return result;
}

int __fastcall condition::NoClipCursor(__int64 a1) //ClipCursor
{
	return 0;
}

__int64 __fastcall condition::Create(int characterNum, int a2) {
	for (int x = 0; x < charcodes.size(); x++) {
		if (characterNum == charcodes.at(x))
			return co(charfunction.at(x), a2);
	}
	return co(characterNum, a2);

}
bool condition::WallRun()
{
	return true;


}
int __fastcall condition::EnableUpgradeCancel(int* a1)
{
	return condition::cancelUpgradeFunctionState;

}

__int64 __fastcall condition::UpgradeVerUnlocker(__int64 a1, __int64 a2) //977DDA
{
	int v2; // ebx
	__int64 result; // rax
	int i; // ebx
	int v6; // edx
	
	typedef signed __int64(__fastcall* sub_14097F1E0)(unsigned int* a1, unsigned int a2);
	sub_14097F1E0 sub_14097F1E0_f = (sub_14097F1E0)(condition::sub_14097F1E0Adress);
	typedef signed __int64(__fastcall* sub_1405C3970)(__int64* a1, __int64* a2, int a3, __int64 a4);
	sub_1405C3970 sub_1405C3970_f = (sub_1405C3970)(condition::sub_1405C3970Adress);
	typedef signed __int64(__fastcall* sub_1405C7EC0)(__int64 a1, int a2);
	sub_1405C7EC0 sub_1405C7EC0_f = (sub_1405C7EC0)(condition::sub_1405C7EC0Adress);
	typedef signed __int64(__fastcall* sub_1405C8F00)(__int64 a1, int a2);
	sub_1405C8F00 sub_1405C8F00_f = (sub_1405C8F00)(condition::sub_1405C8F00Adress);


	v2 = *(int*)(a1 + 3744);
	*(int*)(a1 + 3744) = a2;
	result = sub_14097F1E0_f((unsigned int*)a1, a2);
	if (result)
		result = sub_1405C3970_f(*(__int64**)(a1 + 520), *(__int64**)result, *(int*)(result + 8), *(__int64*)(a1 + 3672));
	else
		*(int*)(a1 + 3744) = v2;
	if (*(int*)(a1 + 520))
	{
		for (i = 0; i < 3; ++i)
			result = sub_1405C7EC0_f(*(__int64*)(a1 + 520), (unsigned int)i);
		switch (*(int*)(a1 + 3744))
		{
		case 2:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 0;
			break;
		case 3:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 1;
			break;
		case 4:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 2;
			break;
		case 5:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 3;
			break;
		case 6:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 4;
			break;
		case 7:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 5;
			break;
		case 8:
			condition::cancelUpgradeFunctionState = 0;
			v6 = 6;
			break;
		default:
			return result;
		}
		return sub_1405C8F00_f(*(__int64*)(a1 + 520), v6);
	}
	return result;
}

__int64 condition::Motion_cancel_function(int* a1)
{
	typedef signed __int64(__fastcall* sub_140978B00)(int* a1);
	sub_140978B00 sub_140978B00_f = (sub_140978B00)(condition::sub_140978B00Adress);
	sub_140978B00_f(a1);
	condition::cancelUpgradeFunctionState = 1;
	return 1i64;
}

bool __fastcall condition::sub_14084FB90(unsigned int* a1)
{

	return 1;
}

bool __fastcall condition::CustomConditionFunction(__int64 a1) {

	int character = *((int*)(a1 + 0xE64));
	int condition = *((int*)(a1 + 0xF2C));


	if (character == 124 || condition == 1) {
		return true;
	}
	return false;
}
void __fastcall condition::sub_1412528C0(const char* Format, ...) {
	int v2;
	char Buffer[1024] = { 0 };
	wchar_t WideCharStr[1036] = { 0 };
	va_list va;

	va_start(va, Format);
	v2 = vsnprintf(Buffer, sizeof(Buffer), Format, va);
	va_end(va);

	if (MultiByteToWideChar(0xFDE9, 0, Buffer, -1, WideCharStr, 1024)) {
		fputws(WideCharStr, stdout);  // Print to stdout
		fflush(stdout);  // Ensure immediate output
		OutputDebugStringW(WideCharStr);
	}
}


void __fastcall condition::_DeleteExceptionPtr(struct __ExceptionPtr* const a1)
{
	typedef signed __int64(__fastcall* sub_1406619A0)(__int64);
	sub_1406619A0 sub_1406619A0_f = (sub_1406619A0)(plugin::moduleBase + 0x6619A0);
	typedef signed __int64(__fastcall* sub_140661780)(__int64* a1);
	sub_140661780 sub_140661780_f = (sub_140661780)(plugin::moduleBase + 0x661780);

	std::cout << "Address: " << std::hex << a1 << std::endl;

	sub_1406619A0_f((__int64)a1);
	sub_140661780_f((__int64*)a1);
}


__int64 __fastcall condition::SkipIntroFunction(__int64 a1, int a2)
{
	return 1i64;
}
