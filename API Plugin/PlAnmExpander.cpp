#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <Psapi.h>
#include "PatternScan.h"
#include "Condition.h"
#include "mem.h"
#include "PlAnmExpander.h"

// Новый тип записи.
struct PlAnmEntry {
    char* name;      // Адрес строки
};

// Глобальные переменные
int PlAnmExpander::ORIGINAL_COUNT = 1009;                  // Оригинальное число записей.
int g_NewStringCountAllocated = PlAnmExpander::ORIGINAL_COUNT + 1000; // Всего слотов.
int g_NewStringCount = PlAnmExpander::ORIGINAL_COUNT;         // Текущий активный счёт.
PlAnmEntry* g_pNewEntryArray = nullptr;                      // Указатель на новый массив.

//------------------------------------------------------------------------------
// Выделяет память в пределах ±2ГБ от указанного базового адреса.
LPVOID PlAnmExpander::AllocNearModule(LPVOID base, SIZE_T size) {
    const uintptr_t TWO_GB = 0x80000000;
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(base);
    const uintptr_t PAGE_SIZE = 0x1000;

    // Сначала пробуем адреса выше базового.
    for (int i = 0; i < 1000; i++) {
        uintptr_t candidate = baseAddr + i * PAGE_SIZE;
        if (candidate - baseAddr > TWO_GB)
            break;
        LPVOID addr = VirtualAlloc(reinterpret_cast<LPVOID>(candidate), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (addr)
            return addr;
    }
    // Затем адреса ниже базового.
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
// Патчит абсолютные ссылки в заданном диапазоне.
void PlAnmExpander::PatchAbsoluteReferencesInRange(uintptr_t start, uintptr_t end, uintptr_t oldArrayAddr, uintptr_t newArrayAddr) {
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
// Патчит RIP-relative инструкции (LEA или MOV), ссылающиеся на старый массив.
void PlAnmExpander::PatchRelativeInstrInRange(uintptr_t start, uintptr_t end, uintptr_t oldArrayAddr, uintptr_t newArrayAddr, uint8_t opcode) {
    DWORD oldProtect;
    // Инструкции без REX (6 байт)
    for (uintptr_t addr = start; addr <= end - 6; addr++) {
        if (*(uint8_t*)addr == opcode) {
            uint8_t modrm = *(uint8_t*)(addr + 1);
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
    // Инструкции с REX (7 байт)
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
void PlAnmExpander::AutoPatchStringArrayReferences(uintptr_t oldArrayAddr, uintptr_t newArrayAddr) {
    HMODULE hModule = GetModuleHandle(nullptr);
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
        condition::sub_1412528C0("GetModuleInformation failed.\n");
        return;
    }
    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
    PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(hModule) + dosHeader->e_lfanew);
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

//------------------------------------------------------------------------------
// Патчит 8-битовые константы, сравнивающие старый и новый счёт записей.
void PlAnmExpander::PatchCountImmediate32(uint32_t oldCount, uint32_t newCount) {
    HMODULE hModule = GetModuleHandle(nullptr);
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
        condition::sub_1412528C0("GetModuleInformation failed (PatchCountImmediate32).\n");
        return;
    }
    uintptr_t moduleStart = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    uintptr_t moduleEnd = moduleStart + modInfo.SizeOfImage;

    for (uintptr_t addr = moduleStart; addr < moduleEnd - 10; addr++) {
        uint8_t opcode = *(uint8_t*)(addr);

        // Проверяем `cmp eax, imm32` (3D <imm32>)
        if (opcode == 0x3D) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 1);
            if (imm == oldCount) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 1) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 3D cmp eax, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 1));
                }
            }
        }
        // Проверяем `cmp ebx, imm32` (81 FB <imm32>)
        else if (opcode == 0x81 && *(uint8_t*)(addr + 1) == 0xFB) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 2);
            if (imm == oldCount) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 2) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 81 FB cmp ebx, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 2));
                }
            }
        }
        // Проверяем `cmp dword ptr [rbp+offset], imm32` (81 BD ?? ?? ?? ?? <imm32>)
        else if (opcode == 0x81 && *(uint8_t*)(addr + 1) == 0xBD) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 6);
            if (imm == oldCount) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 6), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 6) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 6), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 81 BD cmp [rbp+XX], 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 6));
                }
            }
        }
        // Проверяем MOV dword ptr [esp+offset], imm32 (C7 44 24 ?? <imm32>)
        else if (opcode == 0xC7 && *(uint8_t*)(addr + 1) == 0x44 && *(uint8_t*)(addr + 2) == 0x24) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 4);
            if (imm == oldCount) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 4), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 4) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 4), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched MOV (C7 44 24 ??) 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 4));
                }
            }
        }
        // Проверяем MOV eax, imm32 (B8 <imm32>)
        else if (opcode == 0xB8) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 1);
            if (imm == oldCount) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 1) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched MOV eax, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 1));
                }
            }
        }
        // Проверяем `cmp edi, imm32` (81 FF <imm32>)
        else if (opcode == 0x81 && *(uint8_t*)(addr + 1) == 0xFF) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 2);
            if (imm == oldCount) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 2) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 81 FF cmp edi, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 2));
                }
            }
        }
    }
}



#include <windows.h>
#include <psapi.h>
#include "Offsets.h"

// Checks if the byte(s) at addr represent one of the conditional jumps:
// < (JL), <= (JLE), = (JE), >= (JGE), or > (JG).
// This function supports both one-byte (short) and two-byte (near) encodings.
bool HasRelationalConditionalJump(uintptr_t addr, uintptr_t moduleEnd) {
    if (addr >= moduleEnd)
        return false;
    uint8_t op = *(uint8_t*)addr;
    // One-byte conditional jumps: JE (0x74), JL (0x7C), JLE (0x7E), JGE (0x7D), JG (0x7F)
    if (op == 0x74 || op == 0x7C || op == 0x7E || op == 0x7D || op == 0x7F)
        return true;
    // Two-byte conditional jumps: first byte 0x0F, then:
    // JE (0x84), JL (0x8C), JLE (0x8E), JGE (0x8D), JG (0x8F)
    if (op == 0x0F && addr + 1 < moduleEnd) {
        uint8_t op2 = *(uint8_t*)(addr + 1);
        if (op2 == 0x84 || op2 == 0x8C || op2 == 0x8E || op2 == 0x8D || op2 == 0x8F)
            return true;
    }
    return false;
}

void PlAnmExpander::PatchCountConditional(uint32_t oldCount, uint32_t newCount) {
    HMODULE hModule = GetModuleHandle(nullptr);
    MODULEINFO modInfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo))) {
        condition::sub_1412528C0("GetModuleInformation failed (PatchCountConditional).\n");
        return;
    }
    uintptr_t moduleStart = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    uintptr_t moduleEnd = moduleStart + modInfo.SizeOfImage;

    // Scan the module memory
    for (uintptr_t addr = moduleStart; addr < moduleEnd - 10; addr++) {
        uint8_t opcode = *(uint8_t*)(addr);

        // 1. cmp eax, imm32 (3D <imm32>) - length: 1+4=5 bytes.
        if (opcode == 0x3D) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 1);
            if (imm == oldCount && HasRelationalConditionalJump(addr + 5, moduleEnd)) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 1) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 3D cmp eax, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 1));
                }
            }
        }
        // 2. cmp ebx, imm32 (81 FB <imm32>) - length: 2+4=6 bytes.
        else if (opcode == 0x81 && *(uint8_t*)(addr + 1) == 0xFB) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 2);
            if (imm == oldCount && HasRelationalConditionalJump(addr + 6, moduleEnd)) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 2) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 81 FB cmp ebx, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 2));
                }
            }
        }
        // 3. cmp dword ptr [rbp+offset], imm32 (81 BD ?? ?? ?? ?? <imm32>) - length: 2+4+4=10 bytes.
        else if (opcode == 0x81 && *(uint8_t*)(addr + 1) == 0xBD) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 6);
            if (imm == oldCount && HasRelationalConditionalJump(addr + 10, moduleEnd)) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 6), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 6) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 6), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 81 BD cmp [rbp+XX], 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 6));
                }
            }
        }
        // 4. MOV dword ptr [esp+offset], imm32 (C7 44 24 ?? <imm32>) - length: 1+1+1+1+4 = 8 bytes.
        else if (opcode == 0xC7 && *(uint8_t*)(addr + 1) == 0x44 && *(uint8_t*)(addr + 2) == 0x24) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 4);
            if (imm == oldCount && HasRelationalConditionalJump(addr + 8, moduleEnd)) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 4), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 4) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 4), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched MOV (C7 44 24) 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 4));
                }
            }
        }
        // 5. MOV eax, imm32 (B8 <imm32>) - length: 1+4 = 5 bytes.
        else if (opcode == 0xB8) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 1);
            if (imm == oldCount && HasRelationalConditionalJump(addr + 5, moduleEnd)) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 1) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 1), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched MOV eax, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 1));
                }
            }
        }
        // 6. cmp edi, imm32 (81 FF <imm32>) - length: 2+4 = 6 bytes.
        else if (opcode == 0x81 && *(uint8_t*)(addr + 1) == 0xFF) {
            uint32_t imm = *reinterpret_cast<uint32_t*>(addr + 2);
            if (imm == oldCount && HasRelationalConditionalJump(addr + 6, moduleEnd)) {
                DWORD oldProtect;
                if (VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                    *reinterpret_cast<uint32_t*>(addr + 2) = newCount;
                    VirtualProtect(reinterpret_cast<LPVOID>(addr + 2), sizeof(uint32_t), oldProtect, &oldProtect);
                    condition::sub_1412528C0("Patched 81 FF cmp edi, 0x%X -> 0x%X at: 0x%p\n", oldCount, newCount, (void*)(addr + 2));
                }
            }
        }
    }
}






//------------------------------------------------------------------------------
// Пересчитывает текущее число записей по непустым элементам нового массива.
void UpdateNewStringCount() {
    int count = PlAnmExpander::ORIGINAL_COUNT;
    for (int i = PlAnmExpander::ORIGINAL_COUNT; i < g_NewStringCountAllocated; i++) {
        if (g_pNewEntryArray[i].name != nullptr)
            count = i + 1;
        else
            break;
    }
    g_NewStringCount = count;
    condition::sub_1412528C0("Updated new string count: %d\n", g_NewStringCount);
    PlAnmExpander::PatchCountConditional(static_cast<uint32_t>(PlAnmExpander::ORIGINAL_COUNT), static_cast<uint32_t>(g_NewStringCount));
    PlAnmExpander::PatchCountConditional(static_cast<uint32_t>(PlAnmExpander::ORIGINAL_COUNT - 1), static_cast<uint32_t>(g_NewStringCount - 1));
}

//------------------------------------------------------------------------------
// Расширяет массив записей:
// 1. Выделяет новый массив структур PlAnmEntry с дополнительными слотами.
// 2. Считывает ванильные записи из EXE по адресу moduleBase + OFF_STRINGS_ARRAY.
// 3. Обнуляет оставшиеся слоты и патчит ссылки на новый массив.
void PlAnmExpander::ExpandStringArray() {
    // Получаем базовый адрес модуля.
    uintptr_t moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
    // Получаем оригинальный массив строк из EXE.
    char** pOriginal = reinterpret_cast<char**>(moduleBase + offset::OFF_STRINGS_ARRAY);
    size_t newSize = g_NewStringCountAllocated * sizeof(PlAnmEntry);

    // Выделяем новый массив рядом с модулем, используя moduleBase в качестве базового адреса.
    g_pNewEntryArray = reinterpret_cast<PlAnmEntry*>(PlAnmExpander::AllocNearModule(reinterpret_cast<LPVOID>(moduleBase), newSize));
    if (!g_pNewEntryArray) {
        condition::sub_1412528C0("Failed to allocate new string array.\n");
        return;
    }

    // Копируем ванильные записи.
    for (int i = 0; i < PlAnmExpander::ORIGINAL_COUNT; i++) {
        g_pNewEntryArray[i].name = pOriginal[i];
        condition::sub_1412528C0("Copied entry %d: %s\n", i, pOriginal[i] ? pOriginal[i] : "(null)");
    }
    // Обнуляем оставшиеся слоты.
    for (int i = PlAnmExpander::ORIGINAL_COUNT; i < g_NewStringCountAllocated; i++) {
        g_pNewEntryArray[i].name = nullptr;
    }

    // Патчим ссылки в коде, заменяя адрес оригинального массива на адрес нового.
    PlAnmExpander::AutoPatchStringArrayReferences(moduleBase + offset::OFF_STRINGS_ARRAY, reinterpret_cast<uintptr_t>(g_pNewEntryArray));

    condition::sub_1412528C0("Expanded string array: new array at 0x%p, count = %d (allocated %d slots)\n",
        g_pNewEntryArray, g_NewStringCount, g_NewStringCountAllocated);
}

void PlAnmExpander::ReadPlAnmExpander(std::string _file)
{
    // Читаем все байты из файла.
    std::vector<BYTE> fileBytes = condition::ReadAllBytes(_file);
    if (fileBytes.empty())
    {
        condition::sub_1412528C0("Failed to read file: %s\n", _file.c_str());
        return;
    }
    // Файл должен быть минимум 0x20 байт.
    if (fileBytes.size() < 0x20)
    {
        condition::sub_1412528C0("File too small: 0x%X bytes for %s\n", fileBytes.size(), _file.c_str());
        return;
    }
    // Если размер файла не кратен 0x20, но на один байт больше, удаляем лишний байт.
    if (fileBytes.size() % 0x20 != 0)
    {
        if ((fileBytes.size() - 1) % 0x20 == 0)
        {
            fileBytes.pop_back();
        }
        else
        {
            condition::sub_1412528C0("Invalid file size: 0x%X for %s\n", fileBytes.size(), _file.c_str());
            return;
        }
    }
    int numEntries = fileBytes.size() / 0x20;

    // Убедимся, что новый массив уже выделен.
    if (!g_pNewEntryArray)
    {
        condition::sub_1412528C0("Custom string array not allocated.\n");
        return;
    }

    // Обрабатываем каждую 0x20-битовую запись.
    // При этом строка берётся из первых 0x1F байт, последний байт не используется.
    for (int i = 0; i < numEntries; i++)
    {
        int offset = i * 0x20;
        char entryBuffer[0x20] = { 0 }; // Буфер размером 0x20 байт.
        memcpy(entryBuffer, &fileBytes[offset], 0x1F); // Читаем первые 0x1F байт.
        entryBuffer[0x1F] = '\0'; // Гарантируем нулевое завершение.

        // Индекс для новой записи начинается с g_NewStringCount (изначально ORIGINAL_COUNT).
        int index = g_NewStringCount;
        if (index >= g_NewStringCountAllocated)
        {
            condition::sub_1412528C0("Not enough space for custom entry at index %d\n", index);
            break;
        }
        size_t strLen = strnlen(entryBuffer, 0x1F);
        char* newStr = new char[strLen + 1];
        memcpy(newStr, entryBuffer, strLen);
        newStr[strLen] = '\0';
        g_pNewEntryArray[index].name = newStr;
        condition::sub_1412528C0("Added custom entry %d: %s\n", index, newStr);
        g_NewStringCount++;
    }
   /* // Патчим значения счёта в EXE.
    PlAnmExpander::PatchCountImmediate32(
        static_cast<uint32_t>(PlAnmExpander::ORIGINAL_COUNT),
        static_cast<uint32_t>(g_NewStringCount)
    );
    PlAnmExpander::PatchCountImmediate32(
        static_cast<uint32_t>(PlAnmExpander::ORIGINAL_COUNT - 1),
        static_cast<uint32_t>(g_NewStringCount - 1)
    );*/
    condition::sub_1412528C0("Total string count updated: %d\n", g_NewStringCount);
}


