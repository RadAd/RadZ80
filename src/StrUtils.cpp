#include "StrUtils.h"

#include <algorithm>
#include <cctype>

namespace
{
    LPCTSTR strBegin(LPCTSTR s) { return s; }
    LPCTSTR strEnd(LPCTSTR s) { return s + lstrlen(s); }

    bool StrCompare(LPCTSTR s1, LPCTSTR s2)
    {
        return lstrcmp(s1, s2) == 0;
    }

    bool StrCompareI(LPCTSTR s1, LPCTSTR s2)
    {
        return lstrcmpi(s1, s2) == 0;
    }

    bool StrFind(LPCTSTR s1, LPCTSTR s2)
    {
        auto it = std::search(
            strBegin(s1), strEnd(s1),
            strBegin(s2), strEnd(s2),
            [](TCHAR ch1, TCHAR ch2) { return ch1 == ch2; }
        );
        return (it != strEnd(s1));
    }

    bool StrFindI(LPCTSTR s1, LPCTSTR s2)
    {
        auto it = std::search(
            strBegin(s1), strEnd(s1),
            strBegin(s2), strEnd(s2),
            [](TCHAR ch1, TCHAR ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        );
        return (it != strEnd(s1));
    }
}

StrCompareT GeStrComparator(bool bMatchCase, bool bMatchWholeWord)
{
    if (bMatchWholeWord)
        if (bMatchCase)
            return StrCompare;
        else
            return StrCompareI;
    else
        if (bMatchCase)
            return StrFind;
        else
            return StrFindI;
}
