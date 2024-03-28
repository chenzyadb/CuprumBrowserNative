#include "Resource.h"
#include "CuWebView.h"
#include "CuButton.h"
#include "CuEditText.h"
#include "BrowserMenuWindow.h"

constexpr wchar_t windowName[] = UTF16("CuprumBrowserNative");

static HINSTANCE appInstance = nullptr;
static HWND window_hwnd = nullptr;

static CuButton* function_button = nullptr;
static CuButton* goback_button = nullptr;
static CuButton* goforward_button = nullptr;
static CuButton* refresh_button = nullptr;
static CuEditText* url_edittext = nullptr;
static CuWebView* topWebView = nullptr;

void CreateMenuWindow()
{
	auto menuWindow = new BrowserMenuWindow(window_hwnd, UTF16("MENU"), 5, 40, 200, 400);
}

void CreateWidgets()
{
	RECT rect{};
	GetClientRect(window_hwnd, &rect);
	function_button = new CuButton(window_hwnd, LoadBitmap(appInstance, MAKEINTRESOURCE(IDB_BITMAP4)), 5, 5, 30, 30);
	function_button->setCallbackReceiver([](CuButton* instance) {
		CreateMenuWindow();
	});
	goback_button = new CuButton(window_hwnd, LoadBitmap(appInstance, MAKEINTRESOURCE(IDB_BITMAP1)), 40, 5, 30, 30);
	goback_button->setCallbackReceiver([](CuButton* instance) {
		if (topWebView != nullptr) {
			topWebView->goBack();
		}
	});
	goforward_button = new CuButton(window_hwnd, LoadBitmap(appInstance, MAKEINTRESOURCE(IDB_BITMAP2)), 75, 5, 30, 30);
	goforward_button->setCallbackReceiver([](CuButton* instance) {
		if (topWebView != nullptr) {
			topWebView->goForward();
		}
	});
	refresh_button = new CuButton(window_hwnd, LoadBitmap(appInstance, MAKEINTRESOURCE(IDB_BITMAP3)), 110, 5, 30, 30);
	refresh_button->setCallbackReceiver([](CuButton* instance) {
		if (topWebView != nullptr) {
			topWebView->refresh();
		}
	});
	url_edittext = new CuEditText(window_hwnd, UTF16(""), 145, 5, rect.right - rect.left - 150, 30);
	auto font = CreateFont(24, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, UTF16("Segoe UI"));
	url_edittext->setFont(font);
	url_edittext->setCallbackReceiver([]() {
		topWebView->setURL(U16ToU8(url_edittext->getText()));
	});
}

void DestroyWidgets()
{
	delete function_button;
	function_button = nullptr;
	delete goback_button;
	goback_button = nullptr;
	delete goforward_button;
	goforward_button = nullptr;
	delete refresh_button;
	refresh_button = nullptr;
	delete url_edittext;
	url_edittext = nullptr;
	delete topWebView;
	topWebView = nullptr;
}

void AddWebViewWindow(const std::string &url)
{
	static const auto newWindowCallback = [](CuWebView* instance, std::string url) {
		// AddWebViewWindow(url);
		instance->setURL(url);
	};
	static const auto titleChangedCallback = [](CuWebView* instance, std::string title) {
		if (instance == topWebView) {
			SetWindowText(window_hwnd, (U8ToU16(title) + UTF16("  -  CuprumBrowserNative")).c_str());
		}
	};
	static const auto urlChangedCallback = [](CuWebView* instance, std::string url) { 
		if (instance == topWebView) { 
			url_edittext->setText(U8ToU16(url));
		}
	};
	static const auto downloadCallback = [](CuWebView* instance, std::string url) { };

	if (topWebView != nullptr) {
		topWebView->killFocus();
		topWebView->hideWindow();
	}
	RECT rect{};
	GetClientRect(window_hwnd, &rect);
	topWebView = new CuWebView(window_hwnd);
	topWebView->createWindow(0, 40, rect.right - rect.left, rect.bottom - rect.top - 40);
	topWebView->setCallbackReceiver(newWindowCallback, titleChangedCallback, urlChangedCallback, downloadCallback);
	topWebView->setFocus();
	topWebView->setURL(url);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_SIZE:
			{
				int width = LOWORD(lParam), height = HIWORD(lParam);
				if (topWebView != nullptr) {
					topWebView->moveWindow(0, 40, width, height - 40);
				}
				if (url_edittext != nullptr) {
					url_edittext->moveWidget(145, 5, width - 150, 30);
				}
			}
			break;
		case WM_DESTROY:
			DestroyWidgets();
			PostQuitMessage(0);
			return 0;
		default:
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	if (!SetTaskHighPrio()) {
		MessageBox(nullptr, UTF16("Failed to set priority."), UTF16("Warn"), S_OK);
	}
	appInstance = hInstance;
	CuWebView::Init();

	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpszClassName = windowName;
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_ENABLE;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.lpszMenuName = nullptr;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = appInstance;
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.hIcon = LoadIcon(appInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hIconSm = LoadIcon(appInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(appInstance, MAKEINTRESOURCE(IDC_POINTER));
	RegisterClassEx(&wcex);

	window_hwnd = CreateWindowEx(WS_EX_APPWINDOW, wcex.lpszClassName, windowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
							     CW_USEDEFAULT, 800, 600, nullptr, nullptr, wcex.hInstance, nullptr);
	ShowWindow(window_hwnd, nCmdShow);
	CreateWidgets();
	UpdateWindow(window_hwnd);

	AddWebViewWindow("www.baidu.com");
	
	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (topWebView != nullptr) {
			topWebView->wakeBlink();
		}
	}
	return (int)msg.wParam;
}
