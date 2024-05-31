#include "Injector.h"

int main()
{
    std::string dllPath = GetDLLPath("CS2Cheats.dll");

    HANDLE processHandle = GetProcessHandle(L"cs2.exe");

    bool succeeded = false;

    if (processHandle && processHandle != INVALID_HANDLE_VALUE)
    {
        succeeded = InjectByManuallyMapping(processHandle, dllPath.c_str());
    }

    if (processHandle) { CloseHandle(processHandle); }

    if(!succeeded)
    {
        std::cout << "Failed to inject DLL\n";

        std::getchar();
        std::getchar();
    }

    return 0;
}

HANDLE GetProcessHandle(const wchar_t* procName)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                if (!_wcsicmp(procEntry.szExeFile, procName))
                {
                    CloseHandle(hSnap);
                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry.th32ProcessID);
                }
            } while (Process32Next(hSnap, &procEntry));
        }
    }

    CloseHandle(hSnap);
    return INVALID_HANDLE_VALUE;
}

std::string GetDLLPath(const char* dllName)
{
    char path[MAX_PATH];

    GetModuleFileNameA(NULL, path, MAX_PATH);

    std::string str(path);

    str = str.substr(0, str.find_last_of("\\") + 1);
    str += std::string(dllName);

    return str;
}

bool InjectByManuallyMapping(HANDLE procHandle, const char* dllPath)
{
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "DLL file not found";
        return false;
    }

    // Read the dll file

    std::ifstream file(dllPath, std::ios::binary | std::ios::ate);

    if (file.fail())
    {
        std::cout << "Failed to open file";
        file.close();
        return false;
    }

    DWORD fileSize = file.tellg(); // get pointer will be at the end of the file
    if (fileSize < 0x1000)
    {
        std::cout << "File size is invalid";
        file.close();
        return false;
    }

    char* dllFileData = new char[fileSize];

    file.seekg(0, std::ios::beg); // set the get pointer to the beginning
    file.read(dllFileData, fileSize);
    file.close();

    IMAGE_DOS_HEADER* imageDosHeader = (IMAGE_DOS_HEADER*)dllFileData; // image dos header is at the very beginning of PE files

    if (imageDosHeader->e_magic != 0x5A4D) // 0x5A4D = "MZ"; this is a magic number to check the file is a valid PE file
    {
        std::cout << "Invalid file type";
        delete[] dllFileData;
        return false;
    }

    // Allocating memory in the proccess for the dll file

    IMAGE_NT_HEADERS* imageNtHeaders = (IMAGE_NT_HEADERS*)(dllFileData + imageDosHeader->e_lfanew); // e_lfanew is a file offset to the IMAGE_NT_HEADERS struct
    IMAGE_OPTIONAL_HEADER* imageOptHeader = &imageNtHeaders->OptionalHeader;
    IMAGE_FILE_HEADER* imageFileHeader = &imageNtHeaders->FileHeader;

    // ImageBase is the address where the PE file would prefer to be loaded
    char* dllBaseAddress = (char*)VirtualAllocEx(procHandle, (void*)imageOptHeader->ImageBase, imageOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!dllBaseAddress)
    {
        // If it can't be loaded there, let VirtualAllocEx put it where it wants. Relocation data will be used later to adjust for this
        dllBaseAddress = (char*)VirtualAllocEx(procHandle, nullptr, imageOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!dllBaseAddress)
        {
            std::cout << "Failed to allocate memory in proccess for file";
            delete[] dllFileData;
            return false;
        }
    }

    // Writing section data into the process

    IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(imageNtHeaders);

    for (int i = 0; i < imageFileHeader->NumberOfSections; i++)
    {
        if (section->SizeOfRawData != 0) // Only write it if it has data
        {
            if (!WriteProcessMemory(procHandle, dllBaseAddress + section->VirtualAddress, dllFileData + section->PointerToRawData, section->SizeOfRawData, nullptr))
            {
                std::cout << "Failed to map section data";
                delete[] dllFileData;
                VirtualFreeEx(procHandle, dllBaseAddress, 0, MEM_RELEASE);
                return false;
            }
        }
        section++;
    }

    WriteProcessMemory(procHandle, dllBaseAddress, dllFileData, 0x1000, nullptr);

    delete[] dllFileData;

    void* internalCodeLocation = VirtualAllocEx(procHandle, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    void* internalCodeParamLocation = VirtualAllocEx(procHandle, 0, sizeof(InternalManualMapParameter), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!internalCodeLocation || !internalCodeParamLocation)
    {
        std::cout << "Failed to map allocate memory for shell code";
        VirtualFreeEx(procHandle, dllBaseAddress, 0, MEM_RELEASE);
        return false;
    }

    WriteProcessMemory(procHandle, internalCodeLocation, InternalManualMapCode, 0x1000, nullptr);

    InternalManualMapParameter internalParam = {};
    internalParam.dllBaseAddress = dllBaseAddress;
    internalParam.loadLibA = LoadLibraryA; // the internal code will not have access to these functions so they must be passed as a parameter
    internalParam.getProcAddr = GetProcAddress;

    WriteProcessMemory(procHandle, internalCodeParamLocation, &internalParam, sizeof(InternalManualMapParameter), nullptr);

    HANDLE threadHandle = CreateRemoteThread(procHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)internalCodeLocation, internalCodeParamLocation, 0, nullptr);

    if (!threadHandle)
    {
        std::cout << "Failed to create remote thread in process to run shell code";
        VirtualFreeEx(procHandle, dllBaseAddress, 0, MEM_RELEASE);
        VirtualFreeEx(procHandle, internalCodeLocation, 0, MEM_RELEASE);
        VirtualFreeEx(procHandle, internalCodeParamLocation, 0, MEM_RELEASE);
        return false;
    }

    CloseHandle(threadHandle);

    bool injected = false;
    while (!injected)
    {
        InternalManualMapParameter readParam = {};
        ReadProcessMemory(procHandle, internalCodeParamLocation, &readParam, sizeof(InternalManualMapParameter), nullptr);
        injected = readParam.succeeded;
        Sleep(100);
    }

    VirtualFreeEx(procHandle, internalCodeLocation, 0, MEM_RELEASE);
    VirtualFreeEx(procHandle, internalCodeParamLocation, 0, MEM_RELEASE);

    return true;
}

void __stdcall InternalManualMapCode(InternalManualMapParameter* param)
{
    if (!param) { return; }

    char* dllBaseAddress = param->dllBaseAddress;

    IMAGE_DOS_HEADER* imageDosHeader = (IMAGE_DOS_HEADER*)dllBaseAddress;
    IMAGE_NT_HEADERS* imageNtHeaders = (IMAGE_NT_HEADERS*)(dllBaseAddress + imageDosHeader->e_lfanew);
    IMAGE_OPTIONAL_HEADER* imageOptHeader = &imageNtHeaders->OptionalHeader;

    uintptr_t locationOffset = (uintptr_t)dllBaseAddress - imageOptHeader->ImageBase;
    if (locationOffset != 0) // The dll is not loaded where the dll assumed it would be, so relocation data needs to be used to adjust
    {
        if (imageOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0) { return; }

        IMAGE_BASE_RELOCATION* relocation = (IMAGE_BASE_RELOCATION*)(dllBaseAddress + imageOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

        while (relocation->VirtualAddress != 0)
        {
            WORD* relativeInfo = (WORD*)(relocation + 1);

            int numberOfEntries = (relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            for (int i = 0; i < numberOfEntries; i++)
            {
                if (((*relativeInfo) >> 0x0C) == IMAGE_REL_BASED_HIGHLOW || ((*relativeInfo) >> 0x0C) == IMAGE_REL_BASED_DIR64) // checking flags to see if this relocation is relevant (32 or 64 bit)
                {
                    uintptr_t* patch = (uintptr_t*)(dllBaseAddress + relocation->VirtualAddress + ((*relativeInfo) & 0xFFF));
                    *patch += locationOffset;
                }

                relativeInfo++;
            }

            relocation = (IMAGE_BASE_RELOCATION*)((char*)relocation + relocation->SizeOfBlock);
        }
    }

    //Load imports

    if (imageOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)
    {
        IMAGE_IMPORT_DESCRIPTOR* imageImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)(dllBaseAddress + imageOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        while (imageImportDesc->Name != 0)
        {
            char* moduleName = (char*)(dllBaseAddress + imageImportDesc->Name);
            HINSTANCE dllHandle = param->loadLibA(moduleName);

            // Load functions

            UINT_PTR* thunk = (UINT_PTR*)(dllBaseAddress + imageImportDesc->OriginalFirstThunk);
            UINT_PTR* func = (UINT_PTR*)(dllBaseAddress + imageImportDesc->FirstThunk);

            if (!thunk) { thunk = func; }

            while (*thunk)
            {
                if (IMAGE_SNAP_BY_ORDINAL(*thunk))
                {
                    *func = (UINT_PTR)param->getProcAddr(dllHandle, (char*)(*thunk & 0xFFFF));
                }
                else
                {
                    IMAGE_IMPORT_BY_NAME* imageImport = (IMAGE_IMPORT_BY_NAME*)(dllBaseAddress + (*thunk));
                    *func = (UINT_PTR)param->getProcAddr(dllHandle, imageImport->Name);
                }

                thunk++;
                func++;
            }

            imageImportDesc++;
        }
    }

    // TLS (Thread local storage) callbacks, these are functions that are set to call when a thread is created

    if (imageOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size != 0)
    {
        IMAGE_TLS_DIRECTORY* imageTlsDir = (IMAGE_TLS_DIRECTORY*)(dllBaseAddress + imageOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        PIMAGE_TLS_CALLBACK* tlsCallback = (PIMAGE_TLS_CALLBACK*)(imageTlsDir->AddressOfCallBacks);

        while (tlsCallback && *tlsCallback)
        {
            (*tlsCallback)(dllBaseAddress, DLL_PROCESS_ATTACH, nullptr);
            tlsCallback++;
        }
    }

    // call dll main

    _DLL_ENTRY_POINT dllMain = (_DLL_ENTRY_POINT)(dllBaseAddress + imageOptHeader->AddressOfEntryPoint);
    dllMain(dllBaseAddress, DLL_PROCESS_ATTACH, nullptr);

    param->succeeded = true;
}