#include <vector>
#include <stdio.h>
#include <iostream>
#include <Windows.h>


void anti_attach()
{
	FatalAppExitA(MB_ICONWARNING, "Don't try to attach MOTHERFUCKER");
	ExitThread(0);
}

bool EnableSeDebugPrivilege()
{
	HANDLE process = GetCurrentProcess();
	HANDLE processToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &processToken))
	{
		printf("Error OpenThreadToken: %d\n", GetLastError());
		return false;
	}
		

	LUID privilege_value;
	
	if (LookupPrivilegeValue(
		NULL,
		SE_DEBUG_NAME,
		&privilege_value
	))
	{
		TOKEN_PRIVILEGES new_state;
		new_state.PrivilegeCount = 1;
		new_state.Privileges[0].Luid = privilege_value;
		new_state.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if(AdjustTokenPrivileges(
			processToken,
			0,
			&new_state,
			sizeof(TOKEN_PRIVILEGES),
			0,
			0
		))
		{ 
			printf("Obtained SeDebugPrivilege\n");
			return true;
		}
		printf("AdjustTokenPrivileges, last error: %d\n", GetLastError());
		return false;
	}
	printf("LookupPrivilegeValue, last error: %d\n", GetLastError());
	return false;

	
}

int main()
{
	HMODULE ntdll = LoadLibraryA("ntdll.dll");
	FARPROC DbgBreakPoint = GetProcAddress(ntdll, "DbgBreakPoint");
	std::uint32_t oldProtect;

	EnableSeDebugPrivilege();

	printf("Avoiding debugger from attach to process...\n");

	VirtualProtect(reinterpret_cast<LPVOID>(DbgBreakPoint), 100, PAGE_EXECUTE_READWRITE, reinterpret_cast<PDWORD>(&oldProtect));

	printf("Injecting JMP to DbgBreakPoint: 0x%08X\n", DbgBreakPoint);
	printf("Address where to JMP: 0x%08X\n", anti_attach);
	std::uint8_t PUSH = 0x68;
	memcpy(reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(DbgBreakPoint)), &PUSH, 1);

	std::uintptr_t address_to_jmp = 
		(reinterpret_cast<std::uintptr_t>(anti_attach));
	memcpy(reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(DbgBreakPoint)+1), &address_to_jmp, sizeof(std::uintptr_t));
	
	std::uint8_t RET = 0xC3;
	memcpy(reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(DbgBreakPoint) + 5), &RET, 1);

	printf("To finish execution, press CTRL+C");
	Sleep(INFINITE);



    return 0;
}

