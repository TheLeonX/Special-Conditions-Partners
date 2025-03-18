namespace offset
{
	static __int64 stageOffset = 0x14218F7C0; //get that id from kaguya's screen stage switch effect
	static constexpr uintptr_t VTableXmmwordOffset = 0x22FB990; //get that from kaguya's screen stage switch effect v7 = (__int64)sub_141086E40(*((__int64 *)&xmmword_1422FB990 + 1), (__int64)"StageMove");
	static __int64 bgmOffset = 0x14218F000; //find ccAdvPlayStageBgm 
	static uintptr_t OFFSET_TEAM_ULT_ARRAY = 0x18B2470; // can be found in any pairSpSkillCombinationParam function, its array of TUJs. Look for "2nrt3ssk" string to find it
	static int ORIGINAL_COUNT_TEAM_ULT_ARRAY = 55; // count of entries from offset which contain TUJ names
	static uintptr_t OFF_STRINGS_ARRAY = 0x201E930;  //Address for PL_ANM entries, list off_14201E930 1.60  dq offset aPlAnmDwnlp
	static uintptr_t OFF_BGMARRAY = 0x200A5E0; //Address for BGM entries, unk_14200A5E0 1.60 (literally start of array)
};