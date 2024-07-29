#pragma once

#include <cstdint>
#include <windows.h>
#include <array>
#include <string>
#include <iostream>
#include <vector>
//Modifying Bytes in Memory, reading and writing, mainly used for nopping/modifying exe code

namespace util::memory {
	class mem {
	public:
		template <std::size_t size>
		static auto write_bytes(const std::uintptr_t start, const std::array<std::uint8_t, size>& bytes) -> bool {
			DWORD old_prot;

			if (!VirtualProtect(reinterpret_cast<void*>(start), bytes.size(), PAGE_EXECUTE_READWRITE, &old_prot))
				return false;

			for (auto i = 0u; i < bytes.size(); ++i)
				*reinterpret_cast<std::uint8_t*>(start + i) = bytes.at(i);

			VirtualProtect(reinterpret_cast<void*>(start), bytes.size(), old_prot, &old_prot);
			return true;
		}

	private:

	};
}
