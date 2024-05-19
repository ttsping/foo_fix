// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

HMODULE gModule = nullptr;

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID /*reserved*/)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            gModule = mod;
            DisableThreadLibraryCalls(mod);
            break;
        case DLL_PROCESS_DETACH:
            break;
        default:
            break;
    }
    return TRUE;
}

namespace
{
    static const char* spec = R"(FIX TOOL by ohyeah

Third party libraries:
TagLib Audio Meta-Data Library
https://taglib.org/

)";

DECLARE_COMPONENT_VERSION("Fix Tool", "0.2", spec);
}