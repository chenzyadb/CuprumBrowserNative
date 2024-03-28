#pragma once

#include "utils.h"
#include <functional>

class CuEditText
{
	public:
		typedef std::function<void(void)> KeyReturnDownCallback;

		CuEditText(HWND windowHandler, const std::wstring &text, int x, int y, int width, int height) :
			windowHandler_(windowHandler),
			editHandler_(nullptr),
			keyReturnDownCallback_()
		{
			editHandler_ = CreateWindow(UTF16("EDIT"), text.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
										x, y, width, height, windowHandler_, CreateFakeHMenu(), nullptr, nullptr);
			SetWindowLongPtr(editHandler_, GWLP_USERDATA, (LONG_PTR)this);
			SetWindowSubclass(editHandler_, &CuEditText::SubclassProc_, 1, 0);
			SendMessage(editHandler_, EM_SETLIMITTEXT, (WPARAM)MAX_PATH, 0);
		}

		~CuEditText()
		{
			DestroyWindow(editHandler_);
		}

		HWND getHandler() noexcept
		{
			return editHandler_;
		}

		void setFont(HFONT font) noexcept
		{
			SendMessage(editHandler_, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
		}

		void moveWidget(int x, int y, int width, int height) noexcept
		{
			MoveWindow(editHandler_, x, y, width, height, TRUE);
		}

		void setCallbackReceiver(KeyReturnDownCallback keyReturnDownCallback)
		{
			keyReturnDownCallback_ = keyReturnDownCallback;
		}

		void setText(const std::wstring &text)
		{
			if (text.size() <= MAX_PATH) {
				SetWindowText(editHandler_, text.c_str());
			} else {
				SetWindowText(editHandler_, text.substr(0, MAX_PATH).c_str());
			}
		}

		std::wstring getText()
		{
			wchar_t* buffer = new wchar_t[MAX_PATH + 1];
			GetWindowText(editHandler_, buffer, MAX_PATH + 1);
			std::wstring text(buffer);
			delete[] buffer;
			return text;
		}

	private:
		HWND windowHandler_;
		HWND editHandler_;
		KeyReturnDownCallback keyReturnDownCallback_;

		static LRESULT CALLBACK SubclassProc_(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			UINT_PTR uIdSubclass,
			DWORD_PTR dwRefData)
		{
			if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
				auto instance = (CuEditText*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				auto callback = instance->keyReturnDownCallback_;
				if (callback) {
					callback();
				}
			}
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
};
