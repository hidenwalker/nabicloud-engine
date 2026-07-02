// standalone_hanja_probe.cpp -- prove clean HanjaDict works without libhangul.

#include <windows.h>
#include <stdio.h>

#include "HanjaDict.h"

static int fail(const char* reason)
{
    fprintf(stderr, "STANDALONE_HANJA_FAIL [%s]\n", reason);
    return 1;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc != 2 || argv[1] == nullptr || argv[1][0] == L'\0')
    {
        fwprintf(stderr, L"usage: standalone_hanja_probe <dictionary-dir>\n");
        return 2;
    }

    DWORD attrs = GetFileAttributesW(argv[1]);
    if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return fail("dictionary dir missing");
    }

    NabiCloud::HanjaDict dict;
    if (!dict.Load(argv[1]) || !dict.IsLoaded())
    {
        return fail("HanjaDict load");
    }

    int gaCount = dict.Match(L"\xAC00"); // "가" from 10-hanja-wiki.txt.
    if (gaCount <= 0 || dict.GetValue(0) == nullptr || dict.GetValue(0)[0] == L'\0')
    {
        return fail("wiki exact match");
    }

    int suffixCount = dict.MatchLongestSuffix(L"\xAC00", 16);
    if (suffixCount <= 0)
    {
        return fail("wiki longest suffix");
    }

    int symbolCount = dict.Match(L"\x3131"); // "ㄱ" from 40-mssymbol-msime.txt.
    if (symbolCount <= 0)
    {
        return fail("mssymbol match");
    }

    if (dict.Match(nullptr) != 0 || dict.Match(L"") != 0)
    {
        return fail("empty guard");
    }

    printf("STANDALONE_HANJA_OK wiki=%d suffix=%d symbol=%d\n",
           gaCount, suffixCount, symbolCount);
    return 0;
}
