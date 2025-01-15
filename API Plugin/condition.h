#pragma once
#include <vector>
#include <fstream>
#include <string>
inline std::vector<int> partnerfunction;
inline std::vector<int> partnercode;
inline std::vector<int> charfunction;
inline std::vector<int> charcodes;
inline int cancelUpgradeFunctionState;
inline __int64 sub_14097F1E0Adress;
inline __int64 sub_1405C3970Adress;
inline __int64 sub_1405C7EC0Adress;
inline __int64 sub_1405C8F00Adress;
inline __int64 sub_140978B00Adress;

typedef __int64(__fastcall* gpi)(__int64 player, int charcode);
inline gpi gpio;
typedef __int64(__fastcall* c)(int characterNum, int a2);
inline c co;
typedef unsigned char BYTE;

struct PartnerFunction
{
	std::string PARTNER_TYPE = "PARTNER_TYPE";
	__int64 address = 0;
};

struct PartnerFramework
{
	__int64 address = 0;
	int characode = 0;
};


class condition {
public:
	static void ReadSpecialConditionParam(std::string _file);
	static __int64 __fastcall Create(int characterNum, int a2);
	static std::vector<PartnerFunction*> partnerFunctionList;
	static std::vector<PartnerFramework*> frameworkPartnerList;
	static int cancelUpgradeFunctionState;
	static __int64 sub_14097F1E0Adress;
	static __int64 sub_1405C3970Adress;
	static __int64 sub_1405C7EC0Adress;
	static __int64 sub_1405C8F00Adress;
	static __int64 sub_140978B00Adress;
	static void InitialScan();
	static void ReadPartnerSlotParam(std::string file);
	static __int64 CreatePartner(__int64 player, int characode);
	static bool WallRun();
	static int __fastcall NoClipCursor(__int64 a1);
	static int __fastcall EnableUpgradeCancel(int* a1);
	static __int64 __fastcall UpgradeVerUnlocker(__int64 a1, __int64 a2);
	static __int64 Motion_cancel_function(int* a1);
	static bool __fastcall sub_14084FB90(unsigned int* a1);
	static bool __fastcall CustomConditionFunction(__int64 a1);
};
