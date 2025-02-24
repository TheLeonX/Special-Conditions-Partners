#include <windows.h>
#include <cstdint>
#include <cstring>
#include "Common.h"
#include "Offsets.h"
#include "PatternScan.h"
#include "Condition.h"
#include "Main.h"
#include <iostream>
#include <cstdio>
#include <Psapi.h>
#include "mem.h"
#include <sstream>
#include <iomanip>
#include "ConditionPrmManager.h"



#include <vector>
#include <string>
#include <fstream>


#include "ConditionPrmManager.h"
#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

// Define the global dynamic vector.
std::vector<ConditionEntry> g_ConditionEntries;

char** __fastcall ConditionPrmManager::GetConditionEntry(int index) {
    if (index < 1 || static_cast<size_t>(index) > g_ConditionEntries.size() - 1)
        return nullptr;


    //std::cout << " ConditionName: " << g_ConditionEntries[index].conditionName << "\n";
    return reinterpret_cast<char**>(&g_ConditionEntries[index]);
}



void ConditionPrmManager::ReadConditionEntries(const std::string& filename)
{
    // (Your existing code for reading the file goes here.
    // This code should fill g_ConditionEntries dynamically.)

    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    // Read entire file into a buffer.
    std::vector<char> buffer((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    size_t fileSize = buffer.size();
    size_t entrySize = 112; // For example, each entry is 112 bytes.
    if (fileSize % entrySize != 0)
    {
        std::cerr << "File size is not a multiple of entry size.\n";
        return;
    }

    size_t entryCount = fileSize / entrySize;
    g_ConditionEntries.clear();
    g_ConditionEntries.reserve(entryCount);
    // Temporary storage for reaction strings if needed…
    std::vector<std::string> reactionStrings;
    reactionStrings.reserve(entryCount);



#pragma pack(push, 1)
    struct ConditionEntryRaw {
        char conditionName[0x30];
        char reactionConditionName[0x30];
        uint32_t stateValue;
        uint32_t iconID;
        uint32_t statusEffectID;
        uint32_t auraEffect;
    };
#pragma pack(pop)

    for (size_t i = 0; i < entryCount; i++)
    {
        ConditionEntryRaw raw;
        memcpy(&raw, buffer.data() + i * entrySize, entrySize);

        ConditionEntry entry;

        char nameBuffer[0x31] = { 0 };
        memcpy(nameBuffer, buffer.data() + i * entrySize, 0x30);
        // Allocate and copy the name string (ensuring null termination)
        size_t nameLen = strnlen(nameBuffer, 0x30);
        char* nameStr = new char[nameLen + 1];
        memcpy(nameStr, nameBuffer, nameLen);
        nameStr[nameLen] = '\0';


        entry.conditionName = nameStr;
        entry.stateValue = raw.stateValue;
        entry.iconID = raw.iconID;
        entry.statusEffectID = raw.statusEffectID;
        entry.auraEffect = raw.auraEffect;
        entry.padding = 0;
        g_ConditionEntries.push_back(entry);

        // Save reaction string for later conversion.
        std::string reactionStr = std::string(raw.reactionConditionName, strnlen(raw.reactionConditionName, 0x30));
        reactionStrings.push_back(reactionStr);
    }

    // Convert reactionConditionName to reactionAfterEnd for each entry.
    for (size_t i = 0; i < g_ConditionEntries.size(); i++)
    {
        const std::string& reactionStr = reactionStrings[i];
        bool found = false;
        for (size_t j = 0; j < g_ConditionEntries.size(); j++)
        {
            if (g_ConditionEntries[j].conditionName == reactionStr)
            {
                g_ConditionEntries[i].reactionAfterEnd = static_cast<uint32_t>(j);
                found = true;
                break;
            }
        }
        if (!found)
            g_ConditionEntries[i].reactionAfterEnd = 0;

        std::cout << "Entry " << i << ":\n";
        std::cout << " ConditionName: " << g_ConditionEntries[i].conditionName << "\n";
        std::cout << " ReactionAfterEnd: " << g_ConditionEntries[i].reactionAfterEnd << "\n";
    }
    uintptr_t newArrayAddr = reinterpret_cast<uintptr_t>(g_ConditionEntries.data());
    std::cout << "g_ConditionEntries address: " << newArrayAddr << "\n";
   

}
