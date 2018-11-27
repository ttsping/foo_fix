// foo_fix.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		break;

	default:
		break;
	}
	return TRUE;
}

namespace {
	DECLARE_COMPONENT_VERSION("Fix Tool","0.1","FIX TOOL\nby ttsping");
	VALIDATE_COMPONENT_FILENAME("foo_fix.dll");
};