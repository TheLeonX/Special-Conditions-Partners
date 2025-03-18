#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <Psapi.h>
#include <fstream>
#include "condition.h"
#include "Main.h"
#include "Offsets.h"
#include <iostream>
using namespace std;

// Structure for a BGM list entry.
struct BGMEntry {
    uint32_t crc32;        // CRC32 of stage name (4 bytes)
    int32_t bgmId;         // BGM ID (4 bytes)
    int32_t unknown;       // Unknown (usually -1) (4 bytes)
    uint32_t padding;      // Padding (4 bytes)
    uint64_t bgmFunc;      // Address to BGM function (8 bytes)
    uint64_t stageSoundStr;// Address to stage sound string (8 bytes)
};

class BGMExpander {
public:
    // ORIGINAL_COUNT is determined by scanning the vanilla array.
    static inline int ORIGINAL_COUNT;
    static inline BGMEntry* g_pNewBGMArray;
    static inline int g_NewBGMCountAllocated; // Total allocated slots
    static inline int BGM_SLOT_COUNT = 500;
    // Allocates memory within ±2GB of the given base address.
    static LPVOID AllocNearModule(LPVOID base, SIZE_T size) {
        const uintptr_t TWO_GB = 0x80000000;
        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(base);
        const uintptr_t PAGE_SIZE = 0x1000;
        // Try addresses above base.
        for (int i = 0; i < 1000; i++) {
            uintptr_t candidate = baseAddr + i * PAGE_SIZE;
            if (candidate - baseAddr > TWO_GB)
                break;
            LPVOID addr = VirtualAlloc(reinterpret_cast<LPVOID>(candidate), size,
                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (addr)
                return addr;
        }
        // Then try addresses below base.
        for (int i = 1; i < 1000; i++) {
            if (baseAddr < i * PAGE_SIZE)
                break;
            uintptr_t candidate = baseAddr - i * PAGE_SIZE;
            if (baseAddr - candidate > TWO_GB)
                break;
            LPVOID addr = VirtualAlloc(reinterpret_cast<LPVOID>(candidate), size,
                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (addr)
                return addr;
        }
        return nullptr;
    }

    // Patches absolute references in the specified range.
    static void PatchAbsoluteReferencesInRange(uintptr_t start, uintptr_t end,
        uintptr_t oldArrayAddr, uintptr_t newArrayAddr) {
        for (uintptr_t addr = start; addr <= end - sizeof(uintptr_t); addr++) {
            if (*reinterpret_cast<uintptr_t*>(addr) == oldArrayAddr) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr), sizeof(uintptr_t),
                    PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uintptr_t*>(addr) = newArrayAddr;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr), sizeof(uintptr_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched absolute ref at: 0x%p\n", (void*)addr);
                }
                else {
                    condition::sub_1412528C0("Failed to change protection at: 0x%p\n", (void*)addr);
                }
            }
        }
    }

    // Patches RIP-relative instructions (LEA/MOV) in the specified range.
    static void PatchRelativeInstrInRange(uintptr_t start, uintptr_t end,
        uintptr_t oldArrayAddr, uintptr_t newArrayAddr, uint8_t opcode) {
        DWORD oldProtect;
        // Without REX (6-byte instructions)
        for (uintptr_t addr = start; addr <= end - 6; addr++) {
            if (*(uint8_t*)addr == opcode) {
                uint8_t modrm = *(uint8_t*)(addr + 1);
                if ((modrm & 0xC7) == 0x05) {
                    int32_t disp = *reinterpret_cast<int32_t*>(addr + 2);
                    uintptr_t instrEnd = addr + 6;
                    if (instrEnd + disp == oldArrayAddr) {
                        int32_t newDisp = static_cast<int32_t>(newArrayAddr - instrEnd);
                        if (VirtualProtect((LPVOID)(addr + 2), sizeof(int32_t),
                            PAGE_EXECUTE_READWRITE, &oldProtect)) {
                            *reinterpret_cast<int32_t*>(addr + 2) = newDisp;
                            VirtualProtect((LPVOID)(addr + 2), sizeof(int32_t), oldProtect, &oldProtect);
                            condition::sub_1412528C0("Patched RIP-relative (no REX, opcode 0x%X) at: 0x%p\n",
                                opcode, (void*)addr);
                        }
                        else {
                            condition::sub_1412528C0("Failed to change protection at: 0x%p\n", (void*)addr);
                        }
                    }
                }
            }
        }
        // With REX (7-byte instructions)
        for (uintptr_t addr = start; addr <= end - 7; addr++) {
            uint8_t rex = *(uint8_t*)addr;
            if (rex >= 0x40 && rex <= 0x4F && *(uint8_t*)(addr + 1) == opcode) {
                uint8_t modrm = *(uint8_t*)(addr + 2);
                if ((modrm & 0xC7) == 0x05) {
                    int32_t disp = *reinterpret_cast<int32_t*>(addr + 3);
                    uintptr_t instrEnd = addr + 7;
                    if (instrEnd + disp == oldArrayAddr) {
                        int32_t newDisp = static_cast<int32_t>(newArrayAddr - instrEnd);
                        if (VirtualProtect((LPVOID)(addr + 3), sizeof(int32_t),
                            PAGE_EXECUTE_READWRITE, &oldProtect)) {
                            *reinterpret_cast<int32_t*>(addr + 3) = newDisp;
                            VirtualProtect((LPVOID)(addr + 3), sizeof(int32_t), oldProtect, &oldProtect);
                            condition::sub_1412528C0("Patched RIP-relative (with REX, opcode 0x%X) at: 0x%p\n",
                                opcode, (void*)addr);
                        }
                        else {
                            condition::sub_1412528C0("Failed to change protection at: 0x%p\n", (void*)addr);
                        }
                    }
                }
            }
        }
    }

    // Scans executable sections to patch all references to the old BGM list address.
    static void AutoPatchBGMListReferences(uintptr_t oldArrayAddr, uintptr_t newArrayAddr) {
        HMODULE hModule = GetModuleHandle(nullptr);
        MODULEINFO modInfo = { 0 };
        if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
            condition::sub_1412528C0("GetModuleInformation failed in AutoPatchBGMListReferences.\n");
            return;
        }
        PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
        PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<uint8_t*>(hModule) + dosHeader->e_lfanew);
        PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
        for (unsigned i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, section++) {
            if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                uintptr_t secStart = reinterpret_cast<uintptr_t>(hModule) + section->VirtualAddress;
                uintptr_t secEnd = secStart + section->Misc.VirtualSize;
                condition::sub_1412528C0("Scanning section %.8s: 0x%p - 0x%p\n",
                    section->Name, (void*)secStart, (void*)secEnd);
                PatchAbsoluteReferencesInRange(secStart, secEnd, oldArrayAddr, newArrayAddr);
                PatchRelativeInstrInRange(secStart, secEnd, oldArrayAddr, newArrayAddr, 0x8D); // LEA
                PatchRelativeInstrInRange(secStart, secEnd, oldArrayAddr, newArrayAddr, 0x8B); // MOV
            }
        }
    }

    // Returns the current number of entries by scanning until a termination marker (0xFFFFFFFF) is found.
    static int GetCurrentBGMCount() {
        int count = 0;
        while (count < BGMExpander::g_NewBGMCountAllocated) {
            if (BGMExpander::g_pNewBGMArray[count].crc32 == 0xFFFFFFFF)
                break;
            count++;
        }
        return count;
    }

    // Expands the vanilla BGM list by copying all entries until the termination marker,
    // then allocating a new array and patching code references.
    static void ExpandBGMList() {
        HMODULE hModule = GetModuleHandle(nullptr);
        uintptr_t moduleBase = reinterpret_cast<uintptr_t>(hModule);
        BGMEntry* pOriginal = reinterpret_cast<BGMEntry*>(moduleBase + 0x200A5E0);
        int originalCount = 0;
        while (true) {
            if (pOriginal[originalCount].crc32 == 0xFFFFFFFF)
                break;
            originalCount++;
        }
        BGMExpander::ORIGINAL_COUNT = originalCount;
        BGMExpander::g_NewBGMCountAllocated = originalCount + BGM_SLOT_COUNT;
        size_t newSize = BGMExpander::g_NewBGMCountAllocated * sizeof(BGMEntry);
        BGMExpander::g_pNewBGMArray = reinterpret_cast<BGMEntry*>(AllocNearModule(reinterpret_cast<LPVOID>(moduleBase), newSize));
        if (!BGMExpander::g_pNewBGMArray) {
            condition::sub_1412528C0("Failed to allocate new BGM list array.\n");
            return;
        }
        for (int i = 0; i < originalCount; i++) {
            BGMExpander::g_pNewBGMArray[i] = pOriginal[i];
            condition::sub_1412528C0("Copied BGM entry %d: CRC32=0x%X, BGM ID=%d\n", i, pOriginal[i].crc32, pOriginal[i].bgmId);
        }
        // Append termination entry.
        g_pNewBGMArray[originalCount].crc32 = 0xFFFFFFFF;
        g_pNewBGMArray[originalCount].bgmId = 0;
        g_pNewBGMArray[originalCount].unknown = 0;
        g_pNewBGMArray[originalCount].padding = 0;
        g_pNewBGMArray[originalCount].bgmFunc = 0;
        g_pNewBGMArray[originalCount].stageSoundStr = 0;
        AutoPatchBGMListReferences(moduleBase + offset::OFF_BGMARRAY, reinterpret_cast<uintptr_t>(BGMExpander::g_pNewBGMArray));
        condition::sub_1412528C0("Expanded BGM list: new array at 0x%p, vanilla count = %d (allocated %d slots)\n",
            g_pNewBGMArray, originalCount, BGMExpander::g_NewBGMCountAllocated);
    }

    // Reads custom BGM entries from a file and appends them.
    // Each record is 0x68 bytes:
    // • 0x30 bytes: stage name string (max)
    // • 0x30 bytes: stage sound string (max)
    // • 4 bytes: BGM ID
    // • 4 bytes: unknown value
    // New entries use:
    // • CRC32: computed from the stage name.
    // • BGM function: copied from the last vanilla entry.
    // • Stage sound string: allocated new.
    static void ReadBGMFile(const std::string& filename) {
        std::vector<BYTE> fileBytes = condition::ReadAllBytes(filename);
        if (fileBytes.empty()) {
            condition::sub_1412528C0("Failed to read file: %s\n", filename.c_str());
            return;
        }
        size_t recordSize = 0x68;
        if (fileBytes.size() < recordSize) {
            condition::sub_1412528C0("Invalid file size: 0x%X, expected at least 0x%X bytes for %s\n",
                static_cast<unsigned int>(fileBytes.size()), recordSize, filename.c_str());
            return;
        }
        int numEntries = static_cast<int>(fileBytes.size() / recordSize);
        if (!BGMExpander::g_pNewBGMArray) {
            condition::sub_1412528C0("Custom BGM list array not allocated.\n");
            return;
        }
        // Use the BGM function address from the last vanilla entry.
        uint64_t bgmFuncAddress = 0;
        if (BGMExpander::ORIGINAL_COUNT > 0)
            bgmFuncAddress = BGMExpander::g_pNewBGMArray[BGMExpander::ORIGINAL_COUNT - 1].bgmFunc;

        for (int i = 0; i < numEntries; i++) {
            int offset = i * recordSize;
            char stageNameBuffer[0x31] = { 0 };
            memcpy(stageNameBuffer, &fileBytes[offset], 0x30);
            stageNameBuffer[0x30] = '\0';
            char stageSoundBuffer[0x31] = { 0 };
            memcpy(stageSoundBuffer, &fileBytes[offset + 0x30], 0x30);
            stageSoundBuffer[0x30] = '\0';
            int bgmId = *reinterpret_cast<int*>(&fileBytes[offset + 0x60]);
            int unknown = *reinterpret_cast<int*>(&fileBytes[offset + 0x64]);

            // Use the stage name as-is.
            std::string stageName(stageNameBuffer);
            int crc = plugin::api::crc32(stageName);

            size_t soundLen = strnlen(stageSoundBuffer, 0x30);
            char* newStageSoundStr = new char[soundLen + 1];
            memcpy(newStageSoundStr, stageSoundBuffer, soundLen);
            newStageSoundStr[soundLen] = '\0';

            BGMEntry newEntry;
            newEntry.crc32 = static_cast<uint32_t>(crc);
            newEntry.bgmId = bgmId;
            newEntry.unknown = unknown;
            newEntry.padding = 0;
            newEntry.bgmFunc = bgmFuncAddress;
            newEntry.stageSoundStr = reinterpret_cast<uint64_t>(newStageSoundStr);

            bool replaced = false;
            // Get the current count dynamically.
            int currentCount = BGMExpander::GetCurrentBGMCount();
            for (int j = 0; j < currentCount; j++) {
                if (BGMExpander::g_pNewBGMArray[j].crc32 == newEntry.crc32) {
                    BGMExpander::g_pNewBGMArray[j] = newEntry;
                    condition::sub_1412528C0("Replaced existing entry at index %d: CRC32=0x%X, BGM ID=%d\n",
                        j, newEntry.crc32, newEntry.bgmId);
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                currentCount = BGMExpander::GetCurrentBGMCount();
                if (currentCount >= BGMExpander::g_NewBGMCountAllocated - 1) {
                    condition::sub_1412528C0("Not enough space for custom BGM entry at index %d\n", currentCount);
                    break;
                }
                BGMExpander::g_pNewBGMArray[currentCount] = newEntry;
                condition::sub_1412528C0("Added custom BGM entry %d: CRC32=0x%X, BGM ID=%d\n",
                    currentCount, newEntry.crc32, newEntry.bgmId);
            }
        }
        // Append termination marker.
        int finalCount = BGMExpander::GetCurrentBGMCount();
        BGMExpander::g_pNewBGMArray[finalCount].crc32 = 0xFFFFFFFF;
        BGMExpander::g_pNewBGMArray[finalCount].bgmId = 0;
        BGMExpander::g_pNewBGMArray[finalCount].unknown = 0;
        BGMExpander::g_pNewBGMArray[finalCount].padding = 0;
        BGMExpander::g_pNewBGMArray[finalCount].bgmFunc = 0;
        BGMExpander::g_pNewBGMArray[finalCount].stageSoundStr = 0;
        condition::sub_1412528C0("Custom BGM entries appended. New total entries: %d\n", finalCount);
    }




};
