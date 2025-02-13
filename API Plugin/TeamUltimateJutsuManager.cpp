#include "TeamUltimateJutsuManager.h"
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
using namespace std;

//------------------------------------------------------------------------------
// Структура для каждого Team Ultimate Jutsu (TUJ) элемента:
// 8 байт для указателя (имя) + 4 байта для unlockVal + 4 байта зарезервировано.
struct TeamUltimateEntry {
    char* name;      // Адрес строки
    int   unlockVal; // Значение разблокировки
    int   reserved;  // Зарезервировано (обычно 0)
};

int g_NewTUJCount = 0;
//------------------------------------------------------------------------------

uintptr_t OFFSET_TEAM_ULT_ARRAY = offset::OFFSET_TEAM_ULT_ARRAY; // Исходное местоположение статического массива
int ORIGINAL_COUNT = offset::ORIGINAL_COUNT_TEAM_ULT_ARRAY;
//constexpr int NEW_COUNT = 58;

// Глобальный указатель на наш новый (расширенный) TUJ массив.
TeamUltimateEntry* g_pNewTeamUltimateArray = nullptr;

//------------------------------------------------------------------------------
// Функция пытается выделить память размером size в пределах ±2ГБ от baseAddr.
// Если выделение успешно – возвращает указатель на выделенную память, иначе nullptr.
LPVOID AllocNearModule(LPVOID base, SIZE_T size) {
    const uintptr_t TWO_GB = 0x80000000; // 2 ГБ
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(base);
    const uintptr_t PAGE_SIZE = 0x1000;

    // Сначала попробуем адреса выше base.
    for (int i = 0; i < 1000; i++) {
        uintptr_t candidate = baseAddr + i * PAGE_SIZE;
        if (candidate - baseAddr > TWO_GB)
            break;
        LPVOID addr = VirtualAlloc(reinterpret_cast<LPVOID>(candidate), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (addr)
            return addr;
    }
    // Затем попробуем адреса ниже base.
    for (int i = 1; i < 1000; i++) {
        if (baseAddr < i * PAGE_SIZE)
            break;
        uintptr_t candidate = baseAddr - i * PAGE_SIZE;
        if (baseAddr - candidate > TWO_GB)
            break;
        LPVOID addr = VirtualAlloc(reinterpret_cast<LPVOID>(candidate), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (addr)
            return addr;
    }
    return nullptr;
}

//------------------------------------------------------------------------------
// Выделяет новый расширенный TUJ массив, копирует исходные данные точно (указатели и целые значения),
// обнуляет оставшиеся элементы и добавляет тестовую запись на позиции ORIGINAL_COUNT.
//void ExpandTeamUltimateArray_New() {
//    // Исходный массив находится по адресу: plugin::moduleBase + OFFSET_TEAM_ULT_ARRAY.
//    TeamUltimateEntry* pOriginal = reinterpret_cast<TeamUltimateEntry*>(plugin::moduleBase + OFFSET_TEAM_ULT_ARRAY);
//
//    // Вычисляем размер нового массива.
//    size_t newSize = NEW_COUNT * sizeof(TeamUltimateEntry);
//    // Пытаемся выделить память вблизи plugin::moduleBase, чтобы обеспечить корректную работу RIP-relative адресации.
//    g_pNewTeamUltimateArray = reinterpret_cast<TeamUltimateEntry*>(
//        AllocNearModule(reinterpret_cast<LPVOID>(plugin::moduleBase), newSize)
//        );
//    if (!g_pNewTeamUltimateArray) {
//        condition::sub_1412528C0("Failed to allocate new TUJ array.\n");
//        return;
//    }
//
//    // Copy original 55 entries (indices 0..54)
//    memcpy(g_pNewTeamUltimateArray, pOriginal, ORIGINAL_COUNT * sizeof(TeamUltimateEntry));
//
//    // Clear the rest
//    memset(g_pNewTeamUltimateArray + ORIGINAL_COUNT, 0, (NEW_COUNT - ORIGINAL_COUNT) * sizeof(TeamUltimateEntry));
//
//    // Do NOT fill indices 55 and 56 (vanilla load will use count==55)
//    // Instead, only fill your custom entry at index 57:
//    g_pNewTeamUltimateArray[55].name = const_cast<char*>("skip");
//    g_pNewTeamUltimateArray[55].unlockVal = -1;
//    g_pNewTeamUltimateArray[55].reserved = 0;
//    g_pNewTeamUltimateArray[56].name = const_cast<char*>("skip");
//    g_pNewTeamUltimateArray[56].unlockVal = -1;
//    g_pNewTeamUltimateArray[56].reserved = 0;
//    g_pNewTeamUltimateArray[57].name = const_cast<char*>("test");
//    g_pNewTeamUltimateArray[57].unlockVal = -1;
//    g_pNewTeamUltimateArray[57].reserved = 0;
//
//
//    
//
//}

//------------------------------------------------------------------------------
// Helper: сканирует диапазон памяти [start, end) на предмет встреч старого значения и заменяет его на новое.
// Каждая замена выводится через condition::sub_1412528C0.
void PatchAllOccurrencesInRange(uintptr_t start, uintptr_t end, uintptr_t oldValue, uintptr_t newValue) {
    for (uintptr_t addr = start; addr < end - sizeof(uintptr_t); addr++) {
        if (*reinterpret_cast<uintptr_t*>(addr) == oldValue) {
            DWORD oldProtect;
            if (VirtualProtect(reinterpret_cast<LPVOID>(addr), sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *reinterpret_cast<uintptr_t*>(addr) = newValue;
                VirtualProtect(reinterpret_cast<LPVOID>(addr), sizeof(uintptr_t), oldProtect, &oldProtect);
                condition::sub_1412528C0("Patched address: 0x%p\n", (void*)addr);
            }
            else {
                condition::sub_1412528C0("Failed to change protection for patch at 0x%p\n", (void*)addr);
            }
        }
    }
}

//------------------------------------------------------------------------------
// Патчит абсолютные указатели в диапазоне [start, end).
void PatchAbsoluteReferencesInRange(uintptr_t start, uintptr_t end, uintptr_t oldArrayAddr, uintptr_t newArrayAddr) {
    for (uintptr_t addr = start; addr <= end - sizeof(uintptr_t); addr++) {
        if (*reinterpret_cast<uintptr_t*>(addr) == oldArrayAddr) {
            DWORD oldProtect;
            if (VirtualProtect(reinterpret_cast<LPVOID>(addr), sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
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

//------------------------------------------------------------------------------
// Универсальная функция для патчинга RIP-relative инструкций (LEA или MOV), использующих 32-битное смещение.
// Обрабатывает инструкции без REX (6 байт) и с REX (7 байт). 'opcode' должен быть 0x8D (LEA) или 0x8B (MOV).
void PatchRelativeInstrInRange(uintptr_t start, uintptr_t end, uintptr_t oldArrayAddr, uintptr_t newArrayAddr, uint8_t opcode) {
    DWORD oldProtect;
    // --- Случай 1: без REX (длина инструкции = 6 байт) ---
    for (uintptr_t addr = start; addr <= end - 6; addr++) {
        if (*(uint8_t*)addr == opcode) {
            uint8_t modrm = *(uint8_t*)(addr + 1);
            // Для RIP-relative адресации: mod == 00 и r/m == 101.
            if ((modrm & 0xC7) == 0x05) {
                int32_t disp = *reinterpret_cast<int32_t*>(addr + 2);
                uintptr_t instrEnd = addr + 6;
                if (instrEnd + disp == oldArrayAddr) {
                    int32_t newDisp = static_cast<int32_t>(newArrayAddr - instrEnd);
                    if (VirtualProtect((LPVOID)(addr + 2), sizeof(int32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        *reinterpret_cast<int32_t*>(addr + 2) = newDisp;
                        VirtualProtect((LPVOID)(addr + 2), sizeof(int32_t), oldProtect, &oldProtect);
                        condition::sub_1412528C0("Patched RIP-relative (no REX, opcode 0x%X) at: 0x%p\n", opcode, (void*)addr);
                    }
                    else {
                        condition::sub_1412528C0("Failed to change protection at: 0x%p\n", (void*)addr);
                    }
                }
            }
        }
    }
    // --- Случай 2: с REX (длина инструкции = 7 байт) ---
    for (uintptr_t addr = start; addr <= end - 7; addr++) {
        uint8_t rex = *(uint8_t*)addr;
        if (rex >= 0x40 && rex <= 0x4F && *(uint8_t*)(addr + 1) == opcode) {
            uint8_t modrm = *(uint8_t*)(addr + 2);
            if ((modrm & 0xC7) == 0x05) {
                int32_t disp = *reinterpret_cast<int32_t*>(addr + 3);
                uintptr_t instrEnd = addr + 7;
                if (instrEnd + disp == oldArrayAddr) {
                    int32_t newDisp = static_cast<int32_t>(newArrayAddr - instrEnd);
                    if (VirtualProtect((LPVOID)(addr + 3), sizeof(int32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        *reinterpret_cast<int32_t*>(addr + 3) = newDisp;
                        VirtualProtect((LPVOID)(addr + 3), sizeof(int32_t), oldProtect, &oldProtect);
                        condition::sub_1412528C0("Patched RIP-relative (with REX, opcode 0x%X) at: 0x%p\n", opcode, (void*)addr);

                        condition::sub_1412528C0("Old array address: 0x%p, New array address: 0x%p\n",
                            (void*)oldArrayAddr, (void*)newArrayAddr);

                    }
                    else {
                        condition::sub_1412528C0("Failed to change protection at: 0x%p\n", (void*)addr);
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
// Сканирует исполняемые секции основного модуля и патчит все ссылки на старый массив.
void AutoPatchTUJReferences(uintptr_t oldArrayAddr, uintptr_t newArrayAddr) {
    HMODULE hModule = GetModuleHandle(nullptr); // Основной EXE
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
        condition::sub_1412528C0("GetModuleInformation failed.\n");
        return;
    }
    // Итерация по секциям PE.
    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
    PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<uint8_t*>(hModule) + dosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    for (unsigned i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, section++) {
        if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
            uintptr_t secStart = reinterpret_cast<uintptr_t>(hModule) + section->VirtualAddress;
            uintptr_t secEnd = secStart + section->Misc.VirtualSize;
            condition::sub_1412528C0("Scanning section %.8s: 0x%p - 0x%p\n", section->Name, (void*)secStart, (void*)secEnd);
            PatchAbsoluteReferencesInRange(secStart, secEnd, oldArrayAddr, newArrayAddr);
            PatchRelativeInstrInRange(secStart, secEnd, oldArrayAddr, newArrayAddr, 0x8D); // LEA
            PatchRelativeInstrInRange(secStart, secEnd, oldArrayAddr, newArrayAddr, 0x8B); // MOV
        }
    }
}

void Patch8BitImmediateConstant(uint8_t oldVal, uint8_t newVal) {
    HMODULE hModule = GetModuleHandle(nullptr);
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
        condition::sub_1412528C0("GetModuleInformation failed (Patch8BitImmediateConstant).\n");
        return;
    }
    uintptr_t moduleStart = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    uintptr_t moduleEnd = moduleStart + modInfo.SizeOfImage;
    // Scan the entire module for the specific 4-byte sequence.
    for (uintptr_t addr = moduleStart; addr < moduleEnd - 3; addr++) {
        // Check if we have the pattern: 0x41, 0x83, 0xFF, <imm>
        if (*(uint8_t*)(addr) == 0x41 &&
            *(uint8_t*)(addr + 1) == 0x83 &&
            *(uint8_t*)(addr + 2) == 0xFF &&
            *(uint8_t*)(addr + 3) == oldVal)
        {
            DWORD oldProtect;
            if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 3), 1, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                *(uint8_t*)(addr + 3) = newVal;
                VirtualProtect(reinterpret_cast<LPVOID>(addr + 3), 1, oldProtect, &oldProtect);
                condition::sub_1412528C0("Patched 8-bit immediate at: 0x%p: 0x%X -> 0x%X\n", (void*)(addr + 3), oldVal, newVal);
                break;
            }
        }
    }
}

std::string GetAllocationString() {
    // Calculate allocation value, e.g. 55 * 0x48 = 0xF78
    unsigned int allocValue = ORIGINAL_COUNT * 0x48;
    char buf[9] = { 0 };
    // Format each byte as two hex digits in little-endian order.
    sprintf(buf, "%02X%02X%02X%02X",
        allocValue & 0xFF,
        (allocValue >> 8) & 0xFF,
        (allocValue >> 16) & 0xFF,
        (allocValue >> 24) & 0xFF);
    return std::string(buf);
}
std::string DecimalToHex(int value) {
    std::stringstream stream;
    stream << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << value;
    return stream.str();
}

//------------------------------------------------------------------------------
// TUJManager::ExpandTeamUltimateArray:
// 1. Выделяет и инициализирует новый расширенный массив.
// 2. Сканирует и патчит ссылки к старому массиву на новый.
// 3. Выводит отладочную информацию.



void TUJManager::ExpandTeamUltimateArray() {
    //ExpandTeamUltimateArray_New();
    if (!g_pNewTeamUltimateArray) {
        condition::sub_1412528C0("New TUJ array not allocated; aborting patch.\n");
        return;
    }
    uintptr_t oldArrayAddr = plugin::moduleBase + OFFSET_TEAM_ULT_ARRAY;
    uintptr_t newArrayAddr = reinterpret_cast<uintptr_t>(g_pNewTeamUltimateArray);
    AutoPatchTUJReferences(oldArrayAddr, newArrayAddr);
    Patch8BitImmediateConstant(ORIGINAL_COUNT, g_NewTUJCount);

    std::string allocationStr = GetAllocationString();
    __int64 TUJ_Address = PatternScan::Scan("48xxxxxxxx48xxxxxxxx48xxxxxxxx5541xx41xx41xx41xx48xxxxxxxx48xxxxxxxxxxxx48xxxxxxxxxxxx48xxxx48xxxxxx4Cxxxx48xxxxxxxx48xxxxFFxxxx9041xxxxxxxxxx48xxxxxxxxxxxxb9xxxxxxxx");
    __int64 MemoryAlloc_Address_1 = PatternScan::Scan(allocationStr.c_str(), TUJ_Address);
    __int64 MemoryAlloc_Address_2 = PatternScan::Scan(allocationStr.c_str(), MemoryAlloc_Address_1 + 4);

    const std::array<std::uint8_t, 4> NewMemoryAllocationBytes{
        static_cast<std::uint8_t>((g_NewTUJCount * 0x48) & 0xFF),
        static_cast<std::uint8_t>(((g_NewTUJCount * 0x48) >> 8) & 0xFF),
        static_cast<std::uint8_t>(((g_NewTUJCount * 0x48) >> 16) & 0xFF),
        static_cast<std::uint8_t>(((g_NewTUJCount * 0x48) >> 24) & 0xFF)
    };

    util::memory::mem::write_bytes(MemoryAlloc_Address_1, NewMemoryAllocationBytes);
    util::memory::mem::write_bytes(MemoryAlloc_Address_2, NewMemoryAllocationBytes);
    std::string hexTUJCountStr = DecimalToHex(ORIGINAL_COUNT - 1);  
    std::string hexTUJCountStr2 = DecimalToHex(ORIGINAL_COUNT);
    const char* hexTUJCount = hexTUJCountStr.c_str();
    const char* hexTUJCount2 = hexTUJCountStr2.c_str();

    const std::array<std::uint8_t, 1> TUJCountByte{ g_NewTUJCount - 1 };
    const std::array<std::uint8_t, 1> TUJCountByte2{ g_NewTUJCount };

    __int64 TUJ_Address2 = PatternScan::Scan("48xxxxxx4Cxxxxxx48xxxxxx49xxxx74xx39xxxx74xx48xxxxxx49xxxx75xx33xx48xxxxxxC348xxxxxx48xxxxxxxx48xxxxxxxx83xxxx77xx48xxxx48xxxxxxxxxxxx48xxxx8Bxxxxxx8Bxxxxxx83xxxx74xxE8xxxxxxxx3Bxx7Cxx48xxxxxxxxxxxx");
    __int64 TUJ_CountAddress2 = PatternScan::Scan("83xx"+ std::string(hexTUJCount) + "77xx49xxxx", TUJ_Address2) + 2;
    util::memory::mem::write_bytes(TUJ_CountAddress2, TUJCountByte);
    std::cout << "0x" << std::hex << TUJ_CountAddress2 << std::dec << std::endl;

    __int64 TUJ_Address3 = PatternScan::Scan("48xxxxxxxx48xxxxxxxx5748xxxxxx4Cxxxx48xxxx74xx33xx48xxxxxxxxxxxx44xxxx48xxxxxxxxxxxx4Cxxxx0Fxxxx4Dxxxx49xxxx4Dxxxx0Fxxxxxxxxxxxx0Fxxxx");
    __int64 TUJ_CountAddress3 = PatternScan::Scan("41xxxx" + std::string(hexTUJCount) + "77xx49xxxx", TUJ_Address3) + 3;
    util::memory::mem::write_bytes(TUJ_CountAddress3, TUJCountByte);
    std::cout << "0x" << std::hex << TUJ_CountAddress3 << std::dec << std::endl;

    __int64 TUJ_Address4 = PatternScan::Scan("48xxxxxxxx5748xxxxxx83xxxx77xx48xxxxxxxxxxxx48xxxx48xxxx8Bxxxxxx8Bxxxxxx83xxxx74xxE8xxxxxxxx3Bxx7Cxx48xxxxxxxxxxxx8BxxE8xxxxxxxx85xx74xx");
    __int64 TUJ_CountAddress4 = PatternScan::Scan("83xx" + std::string(hexTUJCount) + "77xx48xxxxxxxxxxxx48xxxx", TUJ_Address4) + 2;
    util::memory::mem::write_bytes(TUJ_CountAddress4, TUJCountByte);
    std::cout << "0x" << std::hex << TUJ_CountAddress4 << std::dec << std::endl;

    __int64 TUJ_Address5 = PatternScan::Scan("48xxxxxxxx5748xxxxxx48xxxxxx48xxxxxxxxxxxxxx48xxxxxxxx49xxxx48xxxx72xx48xxxx48xxxxxxxxxxxxC6xxxx48xxxxE8xxxxxxxx49xxxxxxxxxxxx9049xxxx");
    __int64 TUJ_CountAddress5 = PatternScan::Scan("B8" + std::string(hexTUJCount2) + "000000C3", TUJ_Address5) + 1;
    util::memory::mem::write_bytes(TUJ_CountAddress5, TUJCountByte2);
    std::cout << "0x" << std::hex << TUJ_CountAddress5 << std::dec << std::endl;




    condition::sub_1412528C0("AutoPatch: New TUJ array at 0x%p, count = %d\n", g_pNewTeamUltimateArray, g_NewTUJCount);
    condition::sub_1412528C0("AutoPatch: Old TUJ array at 0x%p\n", oldArrayAddr);
}




// Reads pairSpSkillManagerParam.xfbin and builds a new TUJ array.
void TUJManager::ReadPairSpSkillManagerParam(std::string _file)
{
    std::vector<BYTE> fileBytes = condition::ReadAllBytes(_file);
    if (fileBytes.empty())
    {
        condition::sub_1412528C0("Failed to read file: %s\n", _file.c_str());
        return;
    }
    // If file size is off by one, remove the extra byte.
    if (fileBytes.size() % 0x18 != 0)
    {
        if ((fileBytes.size() - 1) % 0x18 == 0)
        {
            fileBytes.pop_back();
        }
        else
        {
            condition::sub_1412528C0("Invalid file size: %d for %s\n", fileBytes.size(), _file.c_str());
            return;
        }
    }

    g_NewTUJCount = static_cast<int>(fileBytes.size() / 0x18);
    size_t newSize = g_NewTUJCount * sizeof(TeamUltimateEntry);
    g_pNewTeamUltimateArray = reinterpret_cast<TeamUltimateEntry*>(
        AllocNearModule(reinterpret_cast<LPVOID>(plugin::moduleBase), newSize)
        );
    if (!g_pNewTeamUltimateArray)
    {
        condition::sub_1412528C0("Failed to allocate new TUJ array from file.\n");
        return;
    }
    for (int i = 0; i < g_NewTUJCount; i++)
    {
        int offset = i * 0x18;
        char nameBuffer[0x11] = { 0 };
        memcpy(nameBuffer, &fileBytes[offset], 0x10);
        // Allocate and copy the name string (ensuring null termination)
        size_t nameLen = strnlen(nameBuffer, 0x10);
        char* nameStr = new char[nameLen + 1];
        memcpy(nameStr, nameBuffer, nameLen);
        nameStr[nameLen] = '\0';
        g_pNewTeamUltimateArray[i].name = nameStr;
        g_pNewTeamUltimateArray[i].unlockVal = *reinterpret_cast<int*>(&fileBytes[offset + 0x10]);
        g_pNewTeamUltimateArray[i].reserved = *reinterpret_cast<int*>(&fileBytes[offset + 0x14]);
        condition::sub_1412528C0("Entry at index %d: { name = %s, unlockVal = %d, reserved = %d }\n",
            i,
            g_pNewTeamUltimateArray[i].name,
            g_pNewTeamUltimateArray[i].unlockVal,
            g_pNewTeamUltimateArray[i].reserved);

    }

    condition::sub_1412528C0("Loaded %d TUJ entries from %s\n", g_NewTUJCount, _file.c_str());
}
