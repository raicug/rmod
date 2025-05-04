#include "memory.h"

#include <Windows.h>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring> 
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

uint8_t* memory::pattern_scanner(const std::string& module_name, const std::string& signature)
{
	auto* const module_handle = GetModuleHandleA(module_name.c_str());

	if (!module_handle)
		return nullptr;

	static auto pattern_to_byte = [](const std::string& pattern)
		{
			auto bytes = std::vector<int>();
			auto* const start = const_cast<char*>(pattern.c_str());
			auto* const end = const_cast<char*>(pattern.c_str()) + std::strlen(pattern.c_str());

			for (auto* current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;

					if (*current == '?')
						++current;

					bytes.push_back(-1);
				}
				else
					bytes.push_back(std::strtoul(current, &current, 16));
			}

			return bytes;
		};

	auto* const dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
	auto* const nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(module_handle) + dos_header->e_lfanew);

	const auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
	auto pattern_bytes = pattern_to_byte(signature);
	auto* const scan_bytes = reinterpret_cast<uint8_t*>(module_handle);

	const auto s = pattern_bytes.size();
	auto* const d = pattern_bytes.data();

	for (auto i = 0ul; i < size_of_image - s; ++i)
	{
		bool found = true;

		for (auto j = 0ul; j < s; ++j)
		{
			if (scan_bytes[i + j] != d[j] && d[j] != -1)
			{
				found = false;
				break;
			}
		}

		if (found)
			return &scan_bytes[i];
	}

	return nullptr;
}

static auto generateBadCharTable(const std::string& pattern) noexcept
{
    std::array<std::size_t, 256> table;
    table.fill(pattern.length());

    for (std::size_t i = 0; i < pattern.length() - 1; ++i)
    {
        table[static_cast<std::uint8_t>(pattern[i])] = pattern.length() - 1 - i;
    }

    return table;
}

// Main findPattern function
const char* memory::findPattern(const char* moduleName, const std::string& pattern, const std::string& patternName) noexcept
{
    HMODULE moduleHandle = GetModuleHandleA(moduleName);
    if (!moduleHandle)
    {
        std::cerr << "Module not found: " << moduleName << std::endl;
        return nullptr;
    }

    // Get the module's base address and size
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo)))
    {
        std::cerr << "Failed to get module info for: " << moduleName << std::endl;
        return nullptr;
    }

    uintptr_t moduleBase = reinterpret_cast<uintptr_t>(moduleInfo.lpBaseOfDll);
    size_t moduleSize = moduleInfo.SizeOfImage;

    // Generate the bad character table
    auto badCharTable = generateBadCharTable(pattern);

    const char* start = reinterpret_cast<const char*>(moduleBase);
    const char* end = start + moduleSize - pattern.length();

    // Boyer-Moore search for the pattern in the module's memory
    while (start <= end)
    {
        int i = pattern.length() - 1;
        while (i >= 0 && (pattern[i] == '?' || start[i] == pattern[i]))
            --i;

        if (i < 0)
        {
            return start;
        }

        start += badCharTable[static_cast<std::uint8_t>(start[pattern.length() - 1])];
    }

    std::cerr << "Failed to find pattern: " << patternName << std::endl;
    return nullptr;
}

char* memory::GetRealFromRelative(char* address, int offset, int instructionSize, bool isRelative) // Address must be an instruction, not a pointer! And offset = the offset to the bytes you want to retrieve.
{
#ifdef _WIN64
    isRelative = true;
#endif
    char* instruction = address + offset;
    if (!isRelative)
    {
        return *(char**)(instruction);
    }

    int relativeAddress = *(int*)(instruction);
    char* realAddress = address + instructionSize + relativeAddress;
    return realAddress;
}