#include "memoryTools.h"

void SetBytes(void* dst, BYTE* bytes, unsigned int size)
{
	DWORD old;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &old); // get permission

	memcpy(dst, bytes, size);

	VirtualProtect(dst, size, old, &old);
}

void SetByte(void* dst, BYTE byte, unsigned int count)
{
	DWORD old;
	VirtualProtect(dst, count, PAGE_EXECUTE_READWRITE, &old); // get permission

	memset(dst, byte, count);

	VirtualProtect(dst, count, old, &old);
}

bool SetRelativeJmp32(void* src, void* dst, unsigned int len)
{
	if (len < 5) { return false; }
	
	DWORD old;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &old); // Get permission

	memset(src, 0x90, len); // Incase len is larger than the length of the jmp instruction the left over bytes will be replaced with NOP

	*(BYTE*)src = 0xE9; // 0xE9 = jump near, relative, displacement relative to next instruction
	DWORD relativeAddress = (DWORD)(((uintptr_t)dst - (uintptr_t)src) - 5);
	*(DWORD*)((uintptr_t)src + 1) = relativeAddress; // Set the address to jump to. + 1 so 0xe9 is not overwritten

	VirtualProtect(src, len, old, &old);

	return true;
}

bool SetAbsoluteJmp64(void* src, void* dst, unsigned int len) // in 64 bit, there is no relative jmp so an absolute jmp must be used. https://kylehalladay.com/blog/2020/11/13/Hooking-By-Example.html
{
	void* relayFuncMemory = AllocatePageNearAddress(src);

	if (relayFuncMemory == nullptr) { return false; }

	// mov r10, dst
	*(BYTE*)relayFuncMemory = 0x49;
	*(BYTE*)((uintptr_t)relayFuncMemory + 1) = 0xBA;
	*(uintptr_t*)((uintptr_t)relayFuncMemory + 2) = (uintptr_t)dst;

	// jmp r10
	*(BYTE*)((uintptr_t)relayFuncMemory + 10) = 0x41;
	*(BYTE*)((uintptr_t)relayFuncMemory + 11) = 0xFF;
	*(BYTE*)((uintptr_t)relayFuncMemory + 12) = 0xE2;

	return SetRelativeJmp32(src, relayFuncMemory, len);
}

void* TrampolineHook(void* src, void* dst, unsigned int len, bool x64) // Wrapper for SetJmp that also creates and returns a gateway which has the overwritten bytes and a jmp back to the original function.
{
	if (len < 5) { return nullptr; }
	
	// the gateway will execute the overwritten bytes and jump back to the original function
	void* gateway = VirtualAlloc(0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (gateway == nullptr) { return nullptr; }
	
	// set the overwritten bytes
	memcpy_s(gateway, len, src, len); 

	if (x64)
	{
		// set jmp back to original function in the gateway
		SetAbsoluteJmp64((void*)((uintptr_t)gateway + len), (void*)((uintptr_t)src + len), len);

		SetAbsoluteJmp64(src, dst, len);
	}
	else
	{
		// set jmp back to original function in the gateway
		uintptr_t gatewayAddress = (uintptr_t)gateway; // so it doesn't have to be casted over and over again
		*(BYTE*)(gatewayAddress + len) = 0xE9; // jmp
		*(uintptr_t*)(gatewayAddress + len + 1) = (uintptr_t)src - gatewayAddress - 5; // jmp back address
		
		SetRelativeJmp32(src, dst, len);
	}

	return gateway;
}

uintptr_t FindArrayOfBytes(uintptr_t baseAddress, BYTE* bytes, int numBytes, BYTE wildcard) // pattern scan
{
	int currentByte = 0;

	_MEMORY_BASIC_INFORMATION mbi;
	while (baseAddress < 0x7fffffffffff && VirtualQuery((uintptr_t*)baseAddress, &mbi, sizeof(mbi)))
	{
		if (mbi.State == MEM_COMMIT)
		{
			BYTE* buffer = (BYTE*)mbi.BaseAddress;

			for (int i = 0; i < mbi.RegionSize; i++)
			{
				if (buffer[i] == bytes[currentByte] || bytes[currentByte] == wildcard)
				{
					if (currentByte == numBytes - 1)
					{
						return ((uintptr_t)(mbi.BaseAddress) + i) - numBytes + 1; // returns the address at the start of the array
					}
				}
				else
				{
					currentByte = -1;
				}

				currentByte++;
			}
		}

		baseAddress += mbi.RegionSize;
	}

	return 0;
}

uintptr_t ResolvePtrChain(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	for (int i = 0; i < offsets.size(); i++)
	{
		(*(uintptr_t*)ptr) += offsets[i]; // dereference, add offset
	}

	return ptr;
}

// https://kylehalladay.com/blog/2020/11/13/Hooking-By-Example.html
// allocates memory close enough to the provided targetAddr argument to be reachable
// from the targetAddr by a 32 bit jump instruction
void* AllocatePageNearAddress(void* targetAddr)
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	const uint64_t PAGE_SIZE = sysInfo.dwPageSize;

	uint64_t startAddr = (uint64_t(targetAddr) & ~(PAGE_SIZE - 1)); //round down to nearest page boundary
	uint64_t minAddr = min(startAddr - 0x7FFFFF00, (uint64_t)sysInfo.lpMinimumApplicationAddress);
	uint64_t maxAddr = max(startAddr + 0x7FFFFF00, (uint64_t)sysInfo.lpMaximumApplicationAddress);

	uint64_t startPage = (startAddr - (startAddr % PAGE_SIZE));

	uint64_t pageOffset = 1;

	bool needsExit = false;
	while (!needsExit)
	{
		uint64_t byteOffset = pageOffset * PAGE_SIZE;
		uint64_t highAddr = startPage + byteOffset;
		uint64_t lowAddr = (startPage > byteOffset) ? startPage - byteOffset : 0;

		needsExit = highAddr > maxAddr && lowAddr < minAddr;

		if (highAddr < maxAddr)
		{
			void* outAddr = VirtualAlloc((void*)highAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr)
				return outAddr;
		}

		if (lowAddr > minAddr)
		{
			void* outAddr = VirtualAlloc((void*)lowAddr, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (outAddr != nullptr)
				return outAddr;
		}

		pageOffset++;
	}

	return nullptr;
}