#pragma once

#include <Windows.h>

typedef bool (*StrCompareT)(LPCTSTR s1, LPCTSTR s2);

StrCompareT GeStrComparator(bool bMatchCase, bool bMatchWholeWord);
