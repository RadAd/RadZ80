#pragma once
#include <Windows.h>

#include <strsafe.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _DEBUG
#define ASSERT_EQUAL(a, b) ((void)0)
#else
#define ASSERT_EQUAL(a, b) \
            (void)(                                                                                     \
                ((a) == (b)) ||                                                                           \
                (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, L"%ls (%d) != %ls (%d)", _CRT_WIDE(#a), (a), _CRT_WIDE(#b), (b))) || \
                (_CrtDbgBreak(), 0)                                                                     \
            )
#endif

HWND FindOwnedWindow(HWND hOwner, LPCTSTR lpClassName, LPCTSTR lpWindowName);

inline void Replace(TCHAR* str, TCHAR f, TCHAR r)
{
    for (TCHAR* s = str; *s != TEXT('\0'); ++s)
    {
        if (*s == f)
            *s = r;
    }
}

inline RECT Rect(const POINT pt, const SIZE sz)
{
    const RECT rc = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };
    return rc;
}

inline LONG Width(const RECT rc)
{
    return rc.right - rc.left;
}

inline LONG Height(const RECT rc)
{
    return rc.bottom - rc.top;
}

inline BOOL SetDlgItemHexU8(HWND hDlg, int nIDDlgItem, UINT8 value)
{
    TCHAR msg[100];
    StringCchPrintf(msg, ARRAYSIZE(msg), TEXT("%02X"), value);
    return SetDlgItemText(hDlg, nIDDlgItem, msg);
}

inline UINT8 GetDlgItemHexU8(HWND hDlg, int nIDDlgItem)
{
    TCHAR msg[100];
    GetDlgItemText(hDlg, nIDDlgItem, msg, ARRAYSIZE(msg));

    return (UINT8) wcstoul(msg, NULL, 16);
}

inline BOOL SetDlgItemHexU16(HWND hDlg, int nIDDlgItem, UINT16 value)
{
    TCHAR msg[100];
    StringCchPrintf(msg, ARRAYSIZE(msg), TEXT("%04X"), value);
    return SetDlgItemText(hDlg, nIDDlgItem, msg);
}

inline UINT16 GetDlgItemHexU16(HWND hDlg, int nIDDlgItem)
{
    TCHAR msg[100];
    GetDlgItemText(hDlg, nIDDlgItem, msg, ARRAYSIZE(msg));

    return (UINT16) wcstoul(msg, NULL, 16);
}

#if 0
typedef int GetValue(int i, void* context);
inline int BinarySearch(GetValue* gv, void* context, const int n, const int key)
{
    int low = 0;
    int high = n - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        const int value = gv(mid, context);
        if (value < key)
            low = mid + 1;
        else if (value == key)
            return mid;
        else
            high = mid - 1;
    }
    return -1;
}
#endif

#ifdef __cplusplus
}
#endif
