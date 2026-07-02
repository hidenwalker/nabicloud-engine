// standalone_load_probe.cpp -- Load NabiCloud.dll from a libhangul-free directory.
// The test proves the TSF DLL starts under the Windows loader without a sibling
// libhangul.dll. Runtime feature locks are exercised by NabiCloud.dll guards.

#include <windows.h>
#include <stdio.h>

typedef HRESULT (STDAPICALLTYPE *DllCanUnloadNowFn)(void);

int wmain(int argc, wchar_t** argv)
{
    if (argc != 2 || argv[1] == nullptr || argv[1][0] == L'\0')
    {
        fwprintf(stderr, L"usage: standalone_load_probe <NabiCloud.dll>\n");
        return 2;
    }

    HMODULE mod = LoadLibraryExW(
        argv[1],
        nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (mod == nullptr)
    {
        fwprintf(stderr, L"STANDALONE_LOAD_FAIL LoadLibraryExW error=%lu path=%ls\n",
                 GetLastError(), argv[1]);
        return 1;
    }

    FARPROC proc = GetProcAddress(mod, "DllCanUnloadNow");
    if (proc == nullptr)
    {
        fprintf(stderr, "STANDALONE_LOAD_FAIL missing DllCanUnloadNow export\n");
        FreeLibrary(mod);
        return 1;
    }

    HRESULT hr = reinterpret_cast<DllCanUnloadNowFn>(proc)();
    printf("STANDALONE_LOAD_OK DllCanUnloadNow=0x%08lx\n", static_cast<unsigned long>(hr));
    FreeLibrary(mod);
    return 0;
}
