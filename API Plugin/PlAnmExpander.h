
#include <string>
class PlAnmExpander {
public:
	static void ExpandStringArray();
	static LPVOID AllocNearModule(LPVOID base, SIZE_T size);
	static void PatchAbsoluteReferencesInRange(uintptr_t start, uintptr_t end, uintptr_t oldArrayAddr, uintptr_t newArrayAddr);
	static void PatchRelativeInstrInRange(uintptr_t start, uintptr_t end, uintptr_t oldArrayAddr, uintptr_t newArrayAddr, uint8_t opcode);
	static void AutoPatchStringArrayReferences(uintptr_t oldArrayAddr, uintptr_t newArrayAddr);
	static void PatchCountImmediate32(uint32_t oldCount, uint32_t newCount);
	static void ReadPlAnmExpander(std::string _file);
	static void PatchCountConditional(uint32_t oldCount, uint32_t newCount);
	static int ORIGINAL_COUNT;
};
