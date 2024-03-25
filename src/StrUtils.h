#pragma once

#ifdef UNICODE
#define tstring wstring
#define tstringstream wstringstream
#define tifstream wifstream
#else
#define tstring string
#define tstringstream stringstream
#define tifstream ifstream
#endif

#include <Windows.h>

#include <string>
#include <vector>
#include <sstream>

typedef bool (*StrCompareT)(LPCTSTR s1, LPCTSTR s2);

StrCompareT GeStrComparator(bool bMatchCase, bool bMatchWholeWord);

inline std::vector<std::tstring> split(const std::tstring& str, TCHAR delim)
{
    std::vector<std::tstring> split;
    std::tstringstream ss(str);
    std::tstring sstr;
    while (std::getline(ss, sstr, delim))
        if (!sstr.empty())
            split.push_back(sstr);
    return split;
}

inline std::tstring ltrim(std::tstring s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](TCHAR ch) {
        return !std::isspace(ch);
        }));
    return s;
}

// trim from end (in place)
inline std::tstring rtrim(std::tstring s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](TCHAR ch) {
        return !std::isspace(ch);
        }).base(), s.end());
    return s;
}
