#pragma once

#include "utils.h"
#include "mb.h"
#include <unordered_map>
#include <functional>

constexpr char UserAgent[] = "Mozilla/5.0 (Windows NT 10.0; Win32; x86) AppleWebKit/537.36"
							 " (KHTML, like Gecko) Chrome/75.0.0.0 Safari/537.36";

class CuWebView
{
	public:
		typedef std::function<void(CuWebView*, std::string)> NewWindowCallback;
		typedef std::function<void(CuWebView*, std::string)> TitleChangedCallback;
		typedef std::function<void(CuWebView*, std::string)> URLChangedCallback;
		typedef std::function<void(CuWebView*, std::string)> DownloadCallback;

		static void Init()
		{
			wchar_t mb_path[MAX_PATH] = { 0 };
			swprintf(mb_path, UTF16("%s\\miniblink.dll"), GetPath().c_str());
			mbSetMbDllPath(mb_path);
			mbSettings settings{};
			mbInit(&settings);
			mbSetUserAgent(NULL_WEBVIEW, UserAgent);
		}

		CuWebView(const HWND &windowHandler) noexcept : 
			windowHandler_(windowHandler), 
			webView_(NULL_WEBVIEW),
			newWindowCallback_(),
			titleChangedCallback_(),
			urlChangedCallback_(),
			downloadCallback_()
		{ }

		~CuWebView() noexcept
		{
			if (webView_ != NULL_WEBVIEW) {
				mbDestroyWebView(webView_);
			}
		}

		HWND getHandler() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return nullptr;
			}
			return mbGetHostHWND(webView_);
		}

		void createWindow(int x, int y, int width, int height) noexcept
		{
			webView_ = mbCreateWebWindow(MB_WINDOW_TYPE_CONTROL, windowHandler_, x, y, width, height);
			SetWindowLongPtr(mbGetHostHWND(webView_), GWLP_USERDATA, (LONG_PTR)this);
			mbOnCreateView(webView_, &CuWebView::onNewWindow_, nullptr);
			mbOnTitleChanged(webView_, &CuWebView::onTitleChanged_, nullptr);
			mbOnURLChanged(webView_, &CuWebView::onURLChanged_, nullptr);
			mbOnDownload(webView_, &CuWebView::onDownload_, nullptr);
			mbSetContextMenuEnabled(webView_, FALSE);
			mbSetResourceGc(webView_, 60);
			mbShowWindow(webView_, TRUE);
		}

		void setCallbackReceiver(
			NewWindowCallback newWindowCallback, 
			TitleChangedCallback titleChangedCallback,
			URLChangedCallback urlChangedCallback,
			DownloadCallback downloadCallback) noexcept 
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			newWindowCallback_ = newWindowCallback;
			titleChangedCallback_ = titleChangedCallback;
			urlChangedCallback_ = urlChangedCallback;
			downloadCallback_ = downloadCallback;
		}

		void moveWindow(int x, int y, int width, int height) noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			MoveWindow(mbGetHostHWND(webView_), x, y, width, height, TRUE);
		}

		void setFocus() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbSetFocus(webView_);
		}

		void killFocus() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbKillFocus(webView_);
		}

		void showWindow() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbShowWindow(webView_, TRUE);
		}

		void hideWindow() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbShowWindow(webView_, FALSE);
		}

		void wakeBlink() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbWake(webView_);
		}

		void setURL(const std::string &url) noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			std::string currentUrl(url);
			if (currentUrl.find("http://") == std::string::npos && currentUrl.find("https://") == std::string::npos) {
				currentUrl = std::string("http://") + currentUrl;
			}
			mbLoadURL(webView_, currentUrl.c_str());
		}

		void refresh() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbReload(webView_);
		}

		void goBack() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbGoBack(webView_);
		}

		void goForward() noexcept
		{
			if (webView_ == NULL_WEBVIEW) {
				return;
			}
			mbGoForward(webView_);
		}

	private:
		HWND windowHandler_;
		mbWebView webView_;
		NewWindowCallback newWindowCallback_;
		TitleChangedCallback titleChangedCallback_;
		URLChangedCallback urlChangedCallback_;
		DownloadCallback downloadCallback_;

		static mbWebView MB_CALL_TYPE onNewWindow_(
			mbWebView webView, 
			void* param, 
			mbNavigationType navigationType, 
			const utf8* url, 
			const mbWindowFeatures* windowFeatures) 
		{
			auto instance = (CuWebView*)GetWindowLongPtr(mbGetHostHWND(webView), GWLP_USERDATA);
			auto callback = instance->newWindowCallback_;
			if (callback) {
				callback(instance, url);
			}
			return NULL_WEBVIEW;
		}

		static void MB_CALL_TYPE onTitleChanged_(mbWebView webView, void* param, const utf8* title)
		{
			auto instance = (CuWebView*)GetWindowLongPtr(mbGetHostHWND(webView), GWLP_USERDATA);
			auto callback = instance->titleChangedCallback_;
			if (callback) {
				callback(instance, title);
			}
		}

		static void MB_CALL_TYPE onURLChanged_(
			mbWebView webView, 
			void* param, 
			const utf8* url, 
			BOOL canGoBack, 
			BOOL canGoForward
		) {
			auto instance = (CuWebView*)GetWindowLongPtr(mbGetHostHWND(webView), GWLP_USERDATA);
			auto callback = instance->urlChangedCallback_;
			if (callback) {
				callback(instance, url);
			}
		}

		static BOOL MB_CALL_TYPE onDownload_(
			mbWebView webView, 
			void* param, 
			mbWebFrameHandle frameId, 
			const char* url, 
			void* downloadJob
		) {
			auto instance = (CuWebView*)GetWindowLongPtr(mbGetHostHWND(webView), GWLP_USERDATA);
			auto callback = instance->downloadCallback_;
			if (callback) {
				callback(instance, url);
			}
			return FALSE;
		}
};
