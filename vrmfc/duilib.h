#pragma once
#include "../DuiLib/UIlib.h"
//using namespace DuiLib;
using DuiLib::WindowImplBase;
using DuiLib::CDuiString;
using DuiLib::CControlUI;
using DuiLib::TNotifyUI;
using DuiLib::CPaintManagerUI;
using DuiLib::DUI_MSGMAP;
using DuiLib::DUI_MSGMAP_ENTRY;
using DuiLib::DUI_PMSG;
using DuiLib::DuiSig_end;
using DuiLib::DuiSig_vn;
using DuiLib::CHorizontalLayoutUI;
using DuiLib::CVerticalLayoutUI;
using DuiLib::CButtonUI;
using DuiLib::CTabLayoutUI;
using DuiLib::COptionUI;
using DuiLib::CSliderUI;
using DuiLib::CLabelUI;
using DuiLib::CWindowWnd;
using DuiLib::TDrawInfo;
using DuiLib::CRenderEngine;
using DuiLib::INotifyUI;
using DuiLib::CDuiRect;
using DuiLib::CDialogBuilder;
using DuiLib::CListUI;
using DuiLib::CProgressUI;

#define get_ctrl(type, name) auto name = static_cast<type*>(m_PaintManager.FindControl(utf8::a2w(#name).c_str())); assert(name);


//#ifdef _DEBUG
//#   pragma comment(lib, "../Debug/DuiLib_Debug.lib")
//#else
#   pragma comment(lib, "../Release/DuiLib_Release.lib")
//#endif

namespace DuiLib {

// 以XML生成界面的窗口基类
class CXMLWnd : public WindowImplBase
{
public:
	explicit CXMLWnd(LPCTSTR pszXMLPath)
		: m_strXMLPath(pszXMLPath) {}

public:
	virtual LPCTSTR GetWindowClassName() const { return _T("XMLWnd"); }

	virtual CDuiString GetSkinFile() { return m_strXMLPath; }

	virtual CDuiString GetSkinFolder() { return _T(""); }

protected:
	CDuiString m_strXMLPath;
};


// 将HWND显示到CControlUI上面
class CWndUI : public CControlUI
{
public:
	CWndUI() : m_hWnd(nullptr) {}

	virtual void SetInternVisible(bool bVisible = true) {
		__super::SetInternVisible(bVisible);
		::ShowWindow(m_hWnd, bVisible);
	}

	virtual void SetPos(RECT rc) {
		__super::SetPos(rc);
		::SetWindowPos(m_hWnd, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	BOOL Attach(HWND hWndNew) {
		if (!::IsWindow(hWndNew)) { return FALSE; }

		m_hWnd = hWndNew;
		return TRUE;
	}

	HWND Detach() {
		HWND hWnd = m_hWnd; m_hWnd = nullptr; return hWnd;
	}

	HWND GetHWND() { return m_hWnd; }

protected:
	HWND m_hWnd;
};

inline void DuiSleep(UINT milliseconds) {
	MSG msg;
	auto begin = std::chrono::steady_clock::now();
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		auto now = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - begin);
		if (diff.count() >= milliseconds) {
			break;
		}
	}
}

}

using DuiLib::CXMLWnd;
using DuiLib::CWndUI;
using DuiLib::DuiSleep;
