#pragma once
#include <iostream>
#include <fstream>
#include <windows.h>
#include <TlHelp32.h>

using _LoadLibraryA = HINSTANCE(WINAPI*)(const char* dllPath);
using _GetProcAddress = FARPROC(WINAPI*)(HINSTANCE moduleHandle, const char* procName);
using _DLL_ENTRY_POINT = BOOL(WINAPI*)(void* dllHandle, DWORD reason, void* reserved);

HANDLE GetProcessHandle(const wchar_t* procName);

std::string GetDLLPath(const char* dllName);

bool InjectByManuallyMapping(HANDLE procHandle, const char* dllPath);

struct InternalManualMapParameter
{
	char* dllBaseAddress;
	_LoadLibraryA loadLibA;
	_GetProcAddress getProcAddr;
	bool succeeded;
};

void __stdcall InternalManualMapCode(InternalManualMapParameter* param);