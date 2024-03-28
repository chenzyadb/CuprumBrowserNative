#pragma once

#include "utils.h"
#include <unordered_map>
#include <functional>

class BrowserMenuWindow
{
	public:
		BrowserMenuWindow(HWND windowHandler, const std::wstring &name, int x, int y, int width, int height) : 
			windowHandler_(windowHandler), 
			menuHandler_()
		{
			RegisterWindowClass_();
			RECT rect{};
			GetWindowRect(windowHandler_, &rect);
			int windowLeft = rect.left + 10 + x, windowTop = rect.top + 30 + y;
			menuHandler_ = CreateWindowEx(WS_EX_TOPMOST, UTF16("BrowserMenuWindow"), name.c_str(), WS_POPUPWINDOW | WS_VISIBLE,
										  windowLeft, windowTop, width, height, windowHandler_, nullptr, nullptr, nullptr);
			SetWindowLongPtr(menuHandler_, GWLP_USERDATA, (LONG_PTR)this);
			SetFocus(menuHandler_);
		}

		~BrowserMenuWindow()
		{
			DestroyWindow(menuHandler_);
		}

		HWND getHandler() noexcept
		{
			return menuHandler_;
		}

	private:
		HWND windowHandler_;
		HWND menuHandler_;

		static LRESULT CALLBACK WndProc_(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg) {
				case WM_KILLFOCUS:
					delete (BrowserMenuWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
					break;
				case WM_DESTROY:
					return 0;
				default:
					break;
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

		static void RegisterWindowClass_(void) noexcept
		{
			static LPWNDCLASSEX lpwcex = nullptr;
			if (lpwcex == nullptr) {
				lpwcex = new WNDCLASSEX();
				lpwcex->cbSize = sizeof(WNDCLASSEX);
				lpwcex->lpszClassName = UTF16("BrowserMenuWindow");
				lpwcex->style = CS_HREDRAW | CS_VREDRAW | CS_ENABLE;
				lpwcex->cbClsExtra = 0;
				lpwcex->cbWndExtra = 0;
				lpwcex->lpszMenuName = nullptr;
				lpwcex->lpfnWndProc = &BrowserMenuWindow::WndProc_;
				lpwcex->hInstance = GetModuleHandle(nullptr);
				lpwcex->hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
				RegisterClassEx(lpwcex);
			}
		}
};
