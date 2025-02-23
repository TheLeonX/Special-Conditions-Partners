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
#include "SpecialInteractingManager.h"
using namespace std;



__int64 SpecialInteractionManager::sub_1409BB1B0Adress = 0;
// Assume this structure and global list exist and are filled by ReadSpecialInteractionParam:
struct SpecialInteractionFramework {
    int mainCharacter;
    std::vector<int> triggerList;  // Используем std::vector<int> вместо массива int[30]
};


namespace specialInteraction {
    std::vector<SpecialInteractionFramework*> specialInteractionList;
}

//void SpecialInteractionManager::ReadSpecialInteractionParam(std::string _file)
//{
//    std::vector<BYTE> fileBytes = condition::ReadAllBytes(_file);
//
//    // Each entry is 0x54 bytes.
//    int entryCount = fileBytes.size() / 0x7C;
//
//    for (int x = 0; x < entryCount; x++)
//    {
//        int offset = 0x7C * x;
//
//        // Read the main character ID (first 4 bytes)
//        int mainCharacter = *reinterpret_cast<int*>(&fileBytes[offset]);
//
//        // Read the trigger character IDs (20 entries from offset+4 to offset+0x54)
//        int triggerIDs[30] = { 0 };
//        for (int i = 0; i < 30; i++)
//        {
//            triggerIDs[i] = *reinterpret_cast<int*>(&fileBytes[offset + 4 + (i * 4)]);
//        }
//
//        // Check for an invalid main character (0x00 means no character)
//        if (mainCharacter == 0)
//        {
//            std::cout << "SpecialInteractionManager :: Error loading entry " << std::hex << x
//                << " - main character ID is 0." << std::endl;
//            continue;
//        }
//
//        // Here you would create and store your special interaction entry.
//        // For example, if you have a SpecialInteractionFramework class:
//        SpecialInteractionFramework* newInteraction = new SpecialInteractionFramework();
//        newInteraction->mainCharacter = mainCharacter;
//        // Copy the trigger IDs into the framework's trigger list.
//        for (int i = 0; i < 30; i++)
//        {
//            newInteraction->triggerList[i] = triggerIDs[i];
//        }
//        // Store the new interaction (assume specialInteractionList is a global vector).
//        specialInteraction::specialInteractionList.push_back(newInteraction);
//
//        // Log the loaded entry.
//        condition::sub_1412528C0("SpecialInteraction Entry at index %d: { mainCharacter = 0x%X, triggers = ",
//            x, mainCharacter);
//        for (int i = 0; i < 20; i++)
//        {
//            condition::sub_1412528C0("0x%X ", triggerIDs[i]);
//        }
//        condition::sub_1412528C0("}\n");
//    }
//}

void SpecialInteractionManager::ReadSpecialInteractionParam(std::string _file)
{
    std::vector<BYTE> fileBytes = condition::ReadAllBytes(_file);
    size_t pos = 0;
    size_t fileSize = fileBytes.size();

    // Verify that file contains at least 4 bytes for the total entry count.
    if (fileSize < 4)
    {
        condition::sub_1412528C0("SpecialInteractionManager :: Error: File too small: %s\n", _file.c_str());
        return;
    }

    // Read total entry count.
    int totalEntryCount = *reinterpret_cast<int*>(&fileBytes[pos]);
    pos += 4;
    condition::sub_1412528C0("SpecialInteractionManager :: Total entries: %d\n", totalEntryCount);

    // Process each entry.
    for (int entryIndex = 0; entryIndex < totalEntryCount; entryIndex++)
    {
        // Ensure there are at least 8 bytes for mainCharacter and triggerCount.
        if (pos + 8 > fileSize)
        {
            condition::sub_1412528C0("SpecialInteractionManager :: Error: Not enough data for entry %d in file %s\n", entryIndex, _file.c_str());
            break;
        }

        // Read main character ID.
        int mainCharacter = *reinterpret_cast<int*>(&fileBytes[pos]);
        pos += 4;

        // Read trigger count.
        int triggerCount = *reinterpret_cast<int*>(&fileBytes[pos]);
        pos += 4;

        if (triggerCount < 0 || pos + triggerCount * 4 > fileSize)
        {
            condition::sub_1412528C0("SpecialInteractionManager :: Error: Invalid trigger count (%d) for entry %d at pos %zu in file %s\n",
                triggerCount, entryIndex, pos, _file.c_str());
            break;
        }

        if (mainCharacter == 0)
        {
            condition::sub_1412528C0("SpecialInteractionManager :: Error: Main character ID is 0 for entry %d at pos %zu.\n", entryIndex, pos);
            pos += triggerCount * 4;
            continue;
        }

        // Create a new entry.
        SpecialInteractionFramework* newEntry = new SpecialInteractionFramework();
        newEntry->mainCharacter = mainCharacter;
        newEntry->triggerList.reserve(triggerCount);
        for (int i = 0; i < triggerCount; i++)
        {
            int triggerID = *reinterpret_cast<int*>(&fileBytes[pos + i * 4]);
            if (triggerID != 0)
                newEntry->triggerList.push_back(triggerID);
        }
        pos += triggerCount * 4;

        // Store the new entry.
        specialInteraction::specialInteractionList.push_back(newEntry);

        // Log the loaded entry.
        condition::sub_1412528C0("SpecialInteraction Entry %d loaded: { mainCharacter = 0x%X, count = %d, triggers = ",
            entryIndex, mainCharacter, triggerCount);
        for (int trigger : newEntry->triggerList)
        {
            condition::sub_1412528C0("0x%X ", trigger);
        }
        condition::sub_1412528C0("}\n");
    }
}





bool __fastcall SpecialInteractionManager::SpecialInteractionSoundFunction(__int64 a1) // sub_140A15A00
{
    typedef signed __int64(__fastcall* sub_1409BB1B0)(__int64 a1);
    sub_1409BB1B0 sub_1409BB1B0_f = (sub_1409BB1B0)(SpecialInteractionManager::sub_1409BB1B0Adress);

    int mainCharId = *(int*)(a1 + 0xE64);
    bool triggered = false;
    int mode = *(unsigned int*)(a1 + 0xE80);

    // Look up a special interaction entry for the current main character.
    SpecialInteractionFramework* entry = nullptr;
    for (auto* e : specialInteraction::specialInteractionList)
    {
        if (e->mainCharacter == mainCharId)
        {
            entry = e;
            break;
        }
    }

    if (entry)
    {
        // Get enemy character ID from sub_1409BB1B0.
        int enemyId = 0;
        __int64 enemyPtr = sub_1409BB1B0_f(a1);
        if (enemyPtr)
            enemyId = *(int*)(enemyPtr + 0xE64);

        // Check if enemyId is in the trigger list for this entry (using dynamic iteration).
        for (int trigger : entry->triggerList)
        {
            if (trigger != 0 && trigger == enemyId)
            {
                triggered = true;
                break;
            }
        }
    }
    else
    {
        // Fallback to original behavior if no special interaction entry is found.
        switch (mainCharId)
        {
        case 36:
        case 51:
        {
            int enemyId = 0;
            __int64 enemyPtr = sub_1409BB1B0_f(a1);
            if (enemyPtr)
                enemyId = *(int*)(enemyPtr + 0xE64);
            if (mainCharId == 36)
            {
                if ((unsigned int)SpecialInteractionManager::SpecialInteractionKarin(a1, enemyId))
                    triggered = true;
            }
            else if (SpecialInteractionManager::SpecialInteractionHinata(a1, enemyId))
            {
                triggered = true;
            }
        }
        break;
        case 1:
        case 3:
            if (mode == 1)
                triggered = true;
            break;
        case 56:
            triggered = (mode == 1);
            break;
        default:
            break;
        }
    }
    return triggered;
}


__int64 __fastcall SpecialInteractionManager::SpecialInteractionKarin(__int64 a1, int a2) // Karin and Sasuke
{
    unsigned int v2; // r8d

    v2 = 0;
    switch (a2)
    {
    case 3:
    case 4:
    case 56:
    case 85:
    case 126:
    case 130:
    case 191:
    case 193:
    case 201:
    case 241:
        v2 = 1;
        break;
    default:
        return v2;
    }
    return v2;
}

bool __fastcall SpecialInteractionManager::SpecialInteractionHinata(__int64 a1, int a2) // Hinata and Naruto
{
    return a2 == 46 || a2 == 194;
}

__int64 __fastcall SpecialInteractionManager::SpecialInteractionUJChanger(__int64 a1, unsigned int a2, __int64 a3)
{
    //cout << "SpecialInteractionManager :: Entries Hooked!" << endl;
    int mainChar = *(int*)(a1 + 3684);
    int enemyChar = 0;
    int flag = (int)a3;
    unsigned int ultID = a2;
    __int64 fallbackResult = 0;

    typedef signed __int64(__fastcall* sub_1409BB1B0)(__int64 a1);
    sub_1409BB1B0 sub_1409BB1B0_f = (sub_1409BB1B0)(SpecialInteractionManager::sub_1409BB1B0Adress);


    // Initial conditions (recursive call, etc.)
    if ((mainChar != 16 && mainChar != 227) || a2 != 224 ||
        *(__int64*)(a1 + 4680) == 0 ||
        (fallbackResult = SpecialInteractionManager::SpecialInteractionUJChanger(*(__int64*)(a1 + 4680), a2, a3)) == 0)
    {
        // If flag is zero, leave ultID unchanged.
        if (!flag)
        {
            __int64 handler = *(__int64*)(a1 + 520);
            return (handler) ? SpecialInteractionManager::sub_1405F0060(handler, ultID) : fallbackResult;
        }

        unsigned int newID = ultID;
        if (ultID - 700 <= 0xD0)
        {
            int mode = *(unsigned int*)(a1 + 0xE80);
            if (mode == 1)
                newID = ultID + 84;
            else if (mode == 2)
                newID = ultID + 126;
            else if (mode == 3)
                newID = ultID + 168;

            // --- Special Interaction File Override ---
            int currentMain = *(int*)(a1 + 3684);
            SpecialInteractionFramework* entry = nullptr;
            // Look up a file entry for the current main character.
            for (auto* e : specialInteraction::specialInteractionList)
            {
                if (e->mainCharacter == currentMain)
                {
                    entry = e;
                    break;
                }
            }
            if (entry)
            {
                // --- Vanilla logic fallback (if no file entry exists) ---
                int v12 = *(int*)(a1 + 3684);
                unsigned int diff = v12 - 36;
                const __int64 mask = 0x100000000008001i64;
                if ((diff <= 0x38 && _bittest64(&mask, diff)) || v12 == 108)
                {
                    __int64 enemyPtr = sub_1409BB1B0_f(a1);
                    if (enemyPtr)
                        enemyChar = *(int*)(enemyPtr + 3684);
                    bool fileTriggered = false;
                    for (int trigger : entry->triggerList)
                    {
                        if (trigger != 0 && trigger == enemyChar)
                        {
                            fileTriggered = true;
                            break;
                        }
                    }
                    if (fileTriggered)
                        newID = ultID + 42;
                    // If no trigger from file, leave newID as adjusted from mode.
                }
            }
            else
            {
                // --- Vanilla logic fallback (if no file entry exists) ---
                int v12 = *(int*)(a1 + 3684);
                unsigned int diff = v12 - 36;
                const __int64 mask = 0x100000000008001i64;
                if ((diff <= 0x38 && _bittest64(&mask, diff)) || v12 == 108)
                {
                    __int64 enemyPtr2 = sub_1409BB1B0_f(a1);
                    if (enemyPtr2)
                        enemyChar = *(int*)(enemyPtr2 + 3684);
                    switch (v12)
                    {
                    case 36:
                        switch (enemyChar)
                        {
                        case 3:
                        case 4:
                        case 56:
                        case 85:
                        case 126:
                        case 130:
                        case 191:
                        case 193:
                        case 201:
                        case 241:
                            newID = ultID + 42;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 51:
                        if (enemyChar == 46)
                            newID = ultID + 42;
                        break;
                    case 92:
                        if (enemyChar <= 268)
                        {
                            if (enemyChar != 268)
                            {
                                switch (enemyChar)
                                {
                                case 6:
                                case 7:
                                case 11:
                                case 12:
                                case 16:
                                case 18:
                                case 19:
                                case 20:
                                case 21:
                                case 29:
                                case 40:
                                case 53:
                                case 60:
                                case 67:
                                case 80:
                                case 83:
                                case 87:
                                case 91:
                                case 92:
                                case 93:
                                case 196:
                                case 198:
                                case 222:
                                case 223:
                                case 225:
                                case 227:
                                    newID = ultID + 42;
                                    break;
                                default:
                                    break;
                                }
                            }
                            else
                            {
                                newID = ultID + 42;
                            }
                        }
                        break;
                    case 108:
                        if (((enemyChar - 45) & 0xFFFFFFBF) == 0)
                            newID = ultID + 42;
                        break;
                    default:
                        break;
                    }
                }
            }
            ultID = newID;
        }
        __int64 handler = *(__int64*)(a1 + 520);
        return (handler) ? SpecialInteractionManager::sub_1405F0060(handler, ultID) : fallbackResult;
    }
    return fallbackResult;
}


__int64 __fastcall SpecialInteractionManager::sub_1405F0060(__int64 a1, unsigned int a2)
{
    __int64 v2; // r8

    v2 = *(__int64*)(a1 + 12808);
    if (v2 && a2 < 0x3F1)
        return *(__int64*)(v2 + 8i64 * a2);
    else
        return 0i64;
}


__int64 __fastcall SpecialInteractionManager::SpecialInteractionFunction2(__int64 a1)
{


    typedef signed __int64(__fastcall* sub_1409BB1B0)(__int64 a1);
    sub_1409BB1B0 sub_1409BB1B0_f = (sub_1409BB1B0)(SpecialInteractionManager::sub_1409BB1B0Adress);
    bool triggered = false;
    __int64 enemyPtr = sub_1409BB1B0_f(a1);

    if (enemyPtr)
    {
        int enemyId = *(int*)(enemyPtr + 0xE64);
        int mainCharId = *(int*)(a1 + 0xE64);

        // Look up a special interaction entry for the current main character.
        SpecialInteractionFramework* entry = nullptr;
        for (auto* e : specialInteraction::specialInteractionList)
        {
            if (e->mainCharacter == mainCharId)
            {
                entry = e;
                break;
            }
        }

        // If a special interaction entry exists, check the trigger list
        if (entry)
        {
            for (int trigger : entry->triggerList)
            {
                if (trigger != 0 && trigger == enemyId)
                {
                    triggered = true;
                    break;
                }
            }
        }
        else
        {
            // Fallback logic if no special interaction entry is found
            switch (mainCharId)
            {
            case 36:
                triggered = SpecialInteractionManager::SpecialInteractionKarin(a1, enemyId);
                break;
            case 51:
                if (SpecialInteractionManager::SpecialInteractionHinata(a1, enemyId))
                    return 1;
                break;
            case 92:
                if (SpecialInteractionManager::SpecialInteractionKonohamaru(a1, enemyId))
                    return 1;
                break;
            case 108:
                if (SpecialInteractionManager::SpecialInteractionKushina(a1, enemyId))
                    return 1;
                break;
            default:
                break;
            }
        }
    }
    return triggered;
}

__int64 __fastcall SpecialInteractionManager::SpecialInteractionKonohamaru(__int64 a1, int a2)
{
    unsigned int v2; // r8d

    v2 = 0;
    if (a2 > 268)
    {
        if (a2 != 272)
            return v2;
        return 1;
    }
    if (a2 == 268)
        return 1;
    switch (a2)
    {
    case 6:
    case 7:
    case 11:
    case 12:
    case 16:
    case 18:
    case 19:
    case 20:
    case 21:
    case 29:
    case 40:
    case 53:
    case 60:
    case 67:
    case 80:
    case 83:
    case 87:
    case 91:
    case 92:
    case 93:
    case 196:
    case 198:
    case 222:
    case 223:
    case 225:
    case 227:
        return 1;
    default:
        return v2;
    }
    return v2;
}

bool __fastcall SpecialInteractionManager::SpecialInteractionKushina(__int64 a1, int a2)
{
    return ((a2 - 45) & 0xFFFFFFBF) == 0;
}

__int64 __fastcall SpecialInteractionManager::AdjustSpecialInteractionValue(__int64 a1, unsigned int a2)
{
    unsigned int adjustedValue = a2;

    typedef signed __int64(__fastcall* sub_1409BB1B0)(__int64 a1);
    sub_1409BB1B0 sub_1409BB1B0_f = (sub_1409BB1B0)(SpecialInteractionManager::sub_1409BB1B0Adress);
    // Adjust a2 based on mode from sub_140981F60

    if (a2 - 700 <= 0xD0) {
        int mode = *(unsigned int*)(a1 + 0xE80);
        if (mode == 1) adjustedValue += 84;
        if (mode == 2) adjustedValue += 126;
        if (mode == 3) adjustedValue += 168;

        int mainCharId = *(int*)(a1 + 0xE64);

        // Find special interaction entry
        SpecialInteractionFramework* entry = nullptr;
        for (auto* e : specialInteraction::specialInteractionList)
        {
            if (e->mainCharacter == mainCharId)
            {
                entry = e;
                break;
            }
        }

        __int64 enemyPtr = sub_1409BB1B0_f(a1);
        int enemyId = enemyPtr ? *(int*)(enemyPtr + 0xE64) : 0;

        if (entry)
        {
            if (a2 >= 700 && a2 <= 916) {
                // If an entry exists, check the trigger list for adjustments
                for (int trigger : entry->triggerList)
                {
                    if (trigger == enemyId)
                        return a2 + 42;
                }

            }

        }
        else
        {
            // Fallback to vanilla logic
            if (a2 >= 700 && a2 <= 916)
            {
                if (mainCharId == 36)
                {
                    switch (enemyId)
                    {
                    case 3: case 4: case 56: case 85: case 126: case 130:
                    case 191: case 193: case 201: case 241:
                        return a2 + 42;
                    }
                }
                else if (mainCharId == 51)
                {
                    if (enemyId == 46 || enemyId == 194)
                        return a2 + 42;
                }
                else if (mainCharId == 92)
                {
                    if (enemyId == 268 || enemyId == 272)
                        return a2 + 42;

                    switch (enemyId)
                    {
                    case 6: case 7: case 11: case 12: case 16: case 18: case 19: case 20: case 21:
                    case 29: case 40: case 53: case 60: case 67: case 80: case 83: case 87: case 91:
                    case 92: case 93: case 196: case 198: case 222: case 223: case 225: case 227:
                        return a2 + 42;
                    }
                }
                else if (mainCharId == 108)
                {
                    if (enemyId == 45 || enemyId == 107)
                        return a2 + 42;
                }
            }
        }
    }
    

    return adjustedValue;
}
