#include "stdafx.h"
// C++ standard functions
#include <iostream>
// WinApi functions
#include <Windows.h>

int main(int argc, char *argv[])
{
	std::string path_to_dll;
	std::uint32_t process_pid;
	HMODULE kernel32_address;
	FARPROC loadLibary_address;
	HANDLE procHandle;
	void *baseAddress;
	HANDLE threadHandle;
	std::uint32_t bytes_written;

	if (argc != 3)
	{
		fprintf(stderr, 
			"[-] USAGE: %s <pid> <path_to_dll>\n\n", 
			argv[0]);
		return -1;
	}
	
	process_pid = std::atoi(argv[1]);
	path_to_dll = argv[2];
	// First start opening remote process
	procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_pid);
	if (procHandle == NULL)
		throw std::exception("[ERROR - OpenProcess]",GetLastError());

	// Allocate virtual memory in remote process
	baseAddress = VirtualAllocEx(procHandle, 
		NULL, 
		path_to_dll.size(), 
		MEM_COMMIT | MEM_RESERVE, 
		PAGE_READWRITE);
	if (baseAddress == NULL)
		throw std::exception("[ERROR - VirtualAllocEx]",GetLastError());

	if (!WriteProcessMemory(procHandle, 
		baseAddress, 
		path_to_dll.c_str(), 
		path_to_dll.size(), 
		reinterpret_cast<SIZE_T*>(&bytes_written)))
	{
		throw std::exception("Error writing in remote process");
	}

	kernel32_address = GetModuleHandleA("Kernel32");
	loadLibary_address = GetProcAddress(kernel32_address, "LoadLibraryA");

	threadHandle = CreateRemoteThread(procHandle, 
		NULL, 
		0, 
		reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibary_address), 
		baseAddress, 
		0, 
		NULL);
	if (threadHandle == NULL)
		throw std::exception("Error creating remote thread");

    return 0;
}
