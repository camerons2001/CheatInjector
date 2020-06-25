/*


                                    ==============================================
                                    ||                                          ||
                                    ||                                          ||
                                    ||                                          ||
                                    ||            AUTHOR: @Ox000b               ||
                                    ||            CREDITS TO: @HoShiMin         ||
                                    ||                                          ||
                                    ||                                          ||
                                    ||                                          ||
                                    ==============================================


*/






#include <iostream>
#include <windows.h>
#include <tlhelp32.h>




DWORD get_process_id_by_process_name(const wchar_t* process_name)
{
    PROCESSENTRY32 process_entry = { sizeof(PROCESSENTRY32) };
    HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // loop through all process to find one that matches the process_name
    if (Process32First(processes_snapshot, &process_entry))
    {
        do
        {
            if (lstrcmp(process_entry.szExeFile, (LPCWSTR)process_name) == 0)
            {
                CloseHandle(processes_snapshot);
                return process_entry.th32ProcessID;
            }
        } while (Process32Next(processes_snapshot, &process_entry));
    }

    CloseHandle(processes_snapshot);
    return NULL;
}



bool inject_dll_file(DWORD process_id, const wchar_t* dll_file)
{
    // get the full path of the dll file
    TCHAR full_dll_path[MAX_PATH];
    GetFullPathName((LPWSTR)dll_file, MAX_PATH, (LPWSTR)full_dll_path, NULL);

    // get the function LoadLibraryA
    LPVOID load_library = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (load_library == NULL)
    {
        return false;
    }

    // open the process
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
    if (process_handle == NULL)
    {
        std::cout << "[!]Failed to open target process!" << std::endl;
        return false;
    }
    std::cout << "[+]Opening Target Process..." << std::endl;

    // allocate space to write the dll location
    std::cout << "[+]Allocating memory in Targer Process." << std::endl;
    LPVOID dll_parameter_address = VirtualAllocEx(process_handle, 0, lstrlen(full_dll_path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (dll_parameter_address == NULL)
    {
        CloseHandle(process_handle);
        std::cout << "[!]Failed to allocate memory in Target Process." << std::endl;
        return false;
    }

    // write the dll location to the space we previously allocated
    BOOL wrote_memory = WriteProcessMemory(process_handle, dll_parameter_address, full_dll_path, wcslen(full_dll_path)*2, NULL);
    if (wrote_memory == false) 
    {
        CloseHandle(process_handle);
        return false;
    }
    std::cout << "[+]Creating Remote Thread in Target Process" << std::endl;

    // launch the dll using LoadLibraryA
    HANDLE dll_thread_handle = CreateRemoteThread(process_handle, 0, 0, (LPTHREAD_START_ROUTINE)load_library, dll_parameter_address, 0, 0);
    if (dll_thread_handle == NULL)
    {
        CloseHandle(process_handle);
        std::cout << "[!]Failed to create Remote Thread" << std::endl;
        return false;
    }
    std::cout << "[+]DLL Successfully Injected :)" << std::endl;

    CloseHandle(dll_thread_handle);
    CloseHandle(process_handle);
    return true;
}




int main()
{
    HANDLE hConsole;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int color = 2;

    std::cout << "SOURCE:" << std::endl;
    std::cout << "www.github.com/camerons2001/CheatInjector" << std::endl;

    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, color);

    std::cout << "\n";

    // variable to store process id
    DWORD process_id = get_process_id_by_process_name(L"csgo.exe");

    // inject dll file
    inject_dll_file(process_id, L"D:\\Cheats\\OTC.dll");

    // freeze for 6 seconds while the first dll being injected 
    Sleep(6000);

    // inject another dll
    inject_dll_file(process_id, L"D:\\Cheats\\features 2.1.1 free.dll");

    std::cout << "\n\n\n";
    puts("Press any key to exit!");
    getchar();

    SetConsoleTextAttribute(hConsole, 2);
    return 0;
}