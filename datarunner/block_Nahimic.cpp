/*
    Block NahimicOSD.dll inject to openGL that cause program crash
*/
#ifdef _WIN32
#include "block_Nahimic.hpp"

#include <Windows.h>
#include <string>
#include "detours/detours.h"

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef void(WINAPI* LdrLoadDllFunc) (
    IN PWCHAR               PathToFile OPTIONAL,
    IN PULONG                Flags OPTIONAL,
    IN PUNICODE_STRING      ModuleFileName,
    OUT HMODULE* ModuleHandle);

static LdrLoadDllFunc RealLdrLoadDll = NULL;
static void WINAPI LdrLoadDllHook(
    IN PWCHAR               PathToFile OPTIONAL,
    IN PULONG               Flags OPTIONAL,
    IN PUNICODE_STRING      ModuleFileName,
    OUT HMODULE* ModuleHandle)
{
    if (ModuleFileName->Buffer != NULL)
    {
        std::wstring fileName(ModuleFileName->Buffer, ModuleFileName->Length / sizeof(WCHAR));
        if (std::wstring::npos != fileName.find(L"NahimicOSD"))
        {
            return;
        }
    }

    if (RealLdrLoadDll != NULL)
    {
        RealLdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);
    }
}

void BlockNahimicOSDInject(void)
{
    LdrLoadDllFunc baseLdrLoadDll = (LdrLoadDllFunc)GetProcAddress(LoadLibraryA("ntdll.dll"), "LdrLoadDll");
    if (baseLdrLoadDll == NULL)
    {
        return;
    }

    DetourTransactionBegin();

    RealLdrLoadDll = baseLdrLoadDll;
    DetourAttach(&(PVOID&)RealLdrLoadDll, LdrLoadDllHook);

    DetourTransactionCommit();
}
#endif