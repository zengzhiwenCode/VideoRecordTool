#include "stdafx.h"
#include "mainwnd.h"
//#include "capture.h"
#include "config.h"
#include "mat2gdi.h"
#include "serial/serial.h"

#ifdef _DEBUG
#pragma comment(lib, "serial/lib/Debug/serial.lib")
#else
#pragma comment(lib, "serial/lib/Release/serial.lib")
#endif // _DEBUG


namespace detail {

//std::shared_ptr<capture_stuff> g_cap = nullptr;
serial::

}

using namespace ::detail;
using namespace cv;

DUI_BEGIN_MESSAGE_MAP(mainwnd, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

mainwnd::mainwnd(LPCTSTR pszXMLPath)
	: CXMLWnd(pszXMLPath)
{
}

mainwnd::~mainwnd()
{
}

void mainwnd::InitWindow()
{
	CenterWindow();

	//g_cap = std::make_shared<capture_stuff>();

	CWndUI *client = static_cast<CWndUI*>(m_PaintManager.FindControl(L"wndMedia")); assert(client);
	auto hwnd = client->GetHWND();
	//RECT rc;
	////rc = client->GetPos();//
	//GetWindowRect(hwnd, &rc);
	//rc.right = rc.left + this->GetRC().right - this->GetRC().left;
	//rc.bottom = rc.top + this->GetRC().bottom - this->GetRC().top;
	//rc = rc_;
	//g_cap->set_video_window(hwnd, nullptr, rc);

	MoveWindow(hwnd, rc_.left, rc_.top, rc_.right - rc_.left, rc_.bottom - rc_.top, TRUE);

	auto cfg = config::get_instance();
	/*std::wstring v_dis, a_dis;
	auto v = g_cap->get_video_devices();
	if (!v.empty()) {
		auto v_iter = v.find(cfg->get_vid());
		if (v_iter != v.end()) {
			v_dis = v_iter->second.display_name;
		} else {
			v_dis = v.begin()->second.display_name;
			cfg->set_vid(v.begin()->first);
		}
	}

	auto a = g_cap->get_audio_devices();
	if (!a.empty()) {
		auto a_iter = a.find(cfg->get_aid());
		if (a_iter != a.end()) {
			a_dis = a_iter->second.display_name;
		} else {
			a_dis = a.begin()->second.display_name;
			cfg->set_aid(a.begin()->first);
		}
	}

	g_cap->ChooseDevices(v_dis.c_str(), a_dis.c_str());
	g_cap->StartPreview();*/

	drawer_ = std::make_shared<PkMatToGDI>(hwnd, false);
	if (!capture_.open(cfg->get_vid())) {
		if (capture_.open(0)) {
			cfg->set_vid(0);
		}
	}

	if (capture_.isOpened()) {
		SetTimer(m_hWnd, 1, 1, nullptr);
	}

	ShowWindow();
}

void mainwnd::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT mainwnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_KEYUP:
		if (wParam == VK_ESCAPE) {
			KillTimer(m_hWnd, 1);
			capture_.release();
			PostMessage(WM_QUIT);
		}
		break;

	case WM_TIMER:
	{
		Mat frame;
		if (capture_.isOpened() && capture_.read(frame) && !frame.empty()) {
			drawer_->DrawImg(frame);
		}
	}
		break;
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void mainwnd::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}

CControlUI * mainwnd::CreateControl(LPCTSTR pstrClassName)
{
	CDuiString     strXML;
	CDialogBuilder builder;

	if (_tcsicmp(pstrClassName, _T("WndMediaDisplay")) == 0) {
		CWndUI *pUI = new CWndUI;
		HWND   hWnd = CreateWindow(_T("#32770"), _T("WndMediaDisplay"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, m_PaintManager.GetPaintWindow(), (HMENU)0, NULL, NULL);
		pUI->Attach(hWnd);
		return pUI;
	}

	if (!strXML.IsEmpty()) {
		CControlUI* pUI = builder.Create(strXML.GetData(), NULL, NULL, &m_PaintManager, NULL); // 这里必须传入m_PaintManager，不然子XML不能使用默认滚动条等信息。
		return pUI;
	}

	return nullptr;
}
