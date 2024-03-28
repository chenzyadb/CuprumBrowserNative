#pragma once

#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <string>
#include <Windows.h>
#include <CommCtrl.h>

#define UTF16(u8_str) L##u8_str

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif // _MSC_VER

inline std::wstring U8ToU16(const std::string &u8_str)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, u8_str.data(), u8_str.size() + 1, nullptr, 0);
	if (len > 0) {
		wchar_t* buffer = new wchar_t[len + 1]{};
		MultiByteToWideChar(CP_UTF8, 0, u8_str.data(), u8_str.size() + 1, buffer, len + 1);
		std::wstring u16_str(buffer);
		delete[] buffer;
		return u16_str;
	}
	return {};
}

inline std::string U16ToU8(const std::wstring &u16_str)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, u16_str.data(), u16_str.size() + 1, nullptr, 0, nullptr, 0);
	if (len > 0) {
		char* buffer = new char[len + 1]{};
		WideCharToMultiByte(CP_UTF8, 0, u16_str.data(), u16_str.size() + 1, buffer, len + 1, nullptr, 0);
		std::string u8_str(buffer);
		delete[] buffer;
		return u8_str;
	}
	return {};
}

inline std::wstring GetPath()
{
	WCHAR path_buf[MAX_PATH] = { 0 };
	GetModuleFileName(nullptr, path_buf, MAX_PATH);
	std::wstring path(path_buf);
	return path.substr(0, path.rfind('\\'));
}

inline HMENU CreateFakeHMenu() noexcept
{
	static HMENU hMenu = (HMENU)UINT16_MAX;
	return hMenu--;
}

inline bool SetTaskHighPrio() noexcept
{
	if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == TRUE && 
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == TRUE) 
	{
		return true;
	}
	return false;
}
