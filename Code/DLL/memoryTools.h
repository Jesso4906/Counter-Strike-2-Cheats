#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

void SetBytes(void* dst, BYTE* bytes, unsigned int size);

void SetByte(void* dst, BYTE byte, unsigned int count);

bool SetRelativeJmp32(void* src, void* dst, unsigned int len);

bool SetAbsoluteJmp64(void* src, void* dst, unsigned int len);

void* TrampolineHook(void* src, void* dst, unsigned int len, bool x64);

uintptr_t FindArrayOfBytes(uintptr_t baseAddress, BYTE* bytes, int numBytes, BYTE wildcard);

uintptr_t ResolvePtrChain(uintptr_t ptr, std::vector<unsigned int> offsets);

void* AllocatePageNearAddress(void* targetAddr);