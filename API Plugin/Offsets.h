namespace offset
{
	static __int64 stageOffset = 0x14218F7C0; //get that id from kaguya's screen stage switch effect
	static constexpr uintptr_t VTableXmmwordOffset = 0x22FB990; //get that from kaguya's screen stage switch effect v7 = (__int64)sub_141086E40(*((__int64 *)&xmmword_1422FB990 + 1), (__int64)"StageMove");
	static __int64 bgmOffset = 0x14218F000; //find ccAdvPlayStageBgm 
};