#include<Windows.h>
#include<TlHelp32.h>
#include<string>
#include<iostream>

#define status_success 1
#define status_error 0

class Offsets
{
public:
	unsigned long local_entity = 0x0; // put offsets here
	unsigned long cl_entitylist = 0x0;
	unsigned long glow_enable = 0x0;
};

class Process
{
public:
	__inline unsigned long get_process_id(std::string process_image)
	{
		PROCESSENTRY32 ProcEntry = {};
		ProcEntry.dwSize = sizeof(PROCESSENTRY32);
		void* ProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (ProcSnap == INVALID_HANDLE_VALUE)
			return -1;
		if (Process32First(ProcSnap, &ProcEntry) == FALSE)
			return -1;
		while (Process32Next(ProcSnap, &ProcEntry))
		{
			if (!strcmp(ProcEntry.szExeFile, process_image.c_str()))
			{
				return ProcEntry.th32ProcessID;
				CloseHandle(ProcSnap);
			}
		}
		CloseHandle(ProcSnap);
		return 0;
	}
};

class Modules
{
public:
	unsigned long get_module_size(unsigned long module_image)
	{
		IMAGE_DOS_HEADER dos_header = *(IMAGE_DOS_HEADER*)module_image;
		IMAGE_NT_HEADERS nt_headers = *(IMAGE_NT_HEADERS*)(module_image + dos_header.e_lfanew);
		return nt_headers.OptionalHeader.SizeOfImage;
	}
public:
	HINSTANCE get_module_handle(const char* module_image)
	{
		auto result = GetModuleHandleA(module_image);
		return result;
	}
};

int main()
{
	SetConsoleTitleA("Simple Read / Write External");

	printf("[ - LOGS - ] -> Enter Process: ");
	std::string process_image;
	std::cin >> process_image;
	
	Process process;
	Modules modules;

	unsigned long result = process.get_process_id(process_image);
	if (!result) {
		fprintf(stderr, "[ - DEBUG - ] -> Failed To Locate Process\n");
		Sleep(3000);
		exit(0);
	}

	system("cls");
	printf("[ - LOGS - ] -> Found Process Opening A Handle\n");

	void* OpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, result);
	if (OpenProc == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "[ - DEBUG - ] -> Failed To Open A Handle\n");
		Sleep(3000);
		exit(0);
	}

	system("cls");
	printf("[ - LOGS - ] -> Enter Module Image: ");
	std::string module_image;
	std::cin >> module_image;

	HMODULE module_result = modules.get_module_handle(module_image.c_str());
	if (!module_result) {
		fprintf(stderr, "[ - DEBUG - ] -> Failed To Get Module\n");
		Sleep(3000);
		exit(0);
	}

	system("cls");
	printf("[ - LOGS - ] -> Found Module Image\n");
	Sleep(3000);

	Offsets offsets;
	while (true)
	{
		unsigned long module_base = (unsigned long)module_result;
		unsigned long local_entity = *(unsigned long*)(module_base + offsets.local_entity);
		
		if (!local_entity)
			continue;

		for (int index = 0; index < 100; index++)
		{
			unsigned long entity = *(unsigned long*)(module_base + offsets.cl_entitylist + index * 32);
			
			if (!entity)
				continue;

			*(bool*)(entity + offsets.glow_enable) = true;
		}
	}
}