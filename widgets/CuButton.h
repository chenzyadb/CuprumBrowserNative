#pragma once

#include "utils.h"
#include <unordered_map>
#include <functional>

class CuButton
{
	public:
		typedef std::function<void(CuButton*)> ButtonClickedCallback;

		CuButton(HWND windowHandler, const std::wstring &text, int x, int y, int width, int height) :
			windowHandler_(windowHandler),
			buttonHandler_(nullptr),
			buttonClickedCallback_()
		{
			HookWindow_();
			buttonHandler_ = CreateWindow(UTF16("BUTTON"), text.c_str(), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT | BS_VCENTER,
										  x, y, width, height, windowHandler_, CreateFakeHMenu(), nullptr, nullptr);
			SetWindowLongPtr(buttonHandler_, GWLP_USERDATA, (LONG_PTR)this);
		}

		CuButton(HWND windowHandler, HBITMAP bitmap, int x, int y, int width, int height) :
			windowHandler_(windowHandler),
			buttonHandler_(),
			buttonClickedCallback_()
		{
			HookWindow_();
			buttonHandler_ = CreateWindow(UTF16("BUTTON"), UTF16(""), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT | BS_BITMAP,
										  x, y, width, height, windowHandler_, CreateFakeHMenu(), nullptr, nullptr);
			SendMessage(buttonHandler_, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bitmap);
			SetWindowLongPtr(buttonHandler_, GWLP_USERDATA, (LONG_PTR)this);
		}

		~CuButton()
		{
			DestroyWindow(buttonHandler_);
		}

		HWND getHandler() noexcept
		{
			return buttonHandler_;
		}

		void setCallbackReceiver(ButtonClickedCallback buttonClickedCallback) noexcept
		{
			buttonClickedCallback_ = buttonClickedCallback;
		}

		void setFont(HFONT font) noexcept
		{
			SendMessage(buttonHandler_, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
		}

		void moveWidget(int x, int y, int width, int height) noexcept
		{
			MoveWindow(buttonHandler_, x, y, width, height, TRUE);
		}

		void setEnable() noexcept
		{
			EnableWindow(buttonHandler_, TRUE);
		}

		void setDisable() noexcept
		{
			EnableWindow(buttonHandler_, FALSE);
		}

		void setText(const std::wstring &text) noexcept
		{
			SetWindowText(buttonHandler_, text.c_str());
		}

	private:
		HWND windowHandler_;
		HWND buttonHandler_;
		ButtonClickedCallback buttonClickedCallback_;

		static LRESULT CALLBACK HookProc_(int nCode, WPARAM wParam, LPARAM lParam)
		{ 
			if (nCode >= 0) {
				auto cwp = (CWPRETSTRUCT*)lParam;
				if (cwp->message == WM_COMMAND && HIWORD(cwp->wParam) == BN_CLICKED) {
					auto hwnd = GetDlgItem(cwp->hwnd, LOWORD(cwp->wParam));
					auto instance = (CuButton*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					auto callback = instance->buttonClickedCallback_;
					if (callback) {
						callback(instance);
					}
				}
			}
			return CallNextHookEx(nullptr, nCode, wParam, lParam);
		}

		static void HookWindow_() noexcept
		{
			static HOOKPROC hookProc = nullptr;
			if (hookProc == nullptr) {
				hookProc = std::addressof(CuButton::HookProc_);
				SetWindowsHookEx(WH_CALLWNDPROCRET, hookProc, nullptr, GetCurrentThreadId());
			}
		}
};
