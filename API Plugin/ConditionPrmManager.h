// Our dynamic condition entry class.
#pragma pack(push, 1)
class ConditionEntry {
public:
    char* conditionName;       // offset 0 (8 bytes)
    uint32_t reactionAfterEnd; // offset 8 (4 bytes) – (ID of condition that plays after effect)
    uint32_t stateValue;       // offset 12 (4 bytes)
    uint32_t iconID;           // offset 16 (4 bytes)
    uint32_t statusEffectID;   // offset 20 (4 bytes)
    uint32_t auraEffect;       // offset 24 (4 bytes)
    uint32_t padding;          // offset 28 (4 bytes)
};
#pragma pack(pop)

#pragma once
#include <vector>
#include <string>
#include "ConditionPrmManager.h" // Assumed to define class ConditionEntry

class ConditionPrmManager {
public:
    // Returns a pointer to the conditionName pointer of the (1-based) entry.
    static char** __fastcall GetConditionEntry(int index);
    static void ReadConditionEntries(const std::string& filename);
};


// Declare the global dynamic vector.
extern std::vector<ConditionEntry> g_ConditionEntries;
