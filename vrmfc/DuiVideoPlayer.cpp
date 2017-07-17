#include "stdafx.h"
#include "DuiVideoPlayer.h"
#include "vrmfcDlg.h"

using namespace cv;

namespace {
#define WM_USER_PLAYING         (WM_USER + 1)     // 开始播放文件
#define WM_USER_POS_CHANGED     (WM_USER + 2)     // 文件播放位置改变
#define WM_USER_END_REACHED     (WM_USER + 3)     // 播放完毕

void CallbackPlayer(void *data, UINT uMsg)
{
	CAVPlayer *pAVPlayer = (CAVPlayer *)data;
	if (pAVPlayer) {
		HWND hWnd = pAVPlayer->GetHWND();
		if (::IsWindow(hWnd) && ::IsWindow(::GetParent(hWnd))) {
			::PostMessage(::GetParent(hWnd)/*hWnd*/, uMsg, (WPARAM)data, 0);
		}
	}
}

void CallbackPlaying(void *data)
{
	CallbackPlayer(data, WM_USER_PLAYING);
}

void CallbackPosChanged(void *data)
{
	CallbackPlayer(data, WM_USER_POS_CHANGED);
}

void CallbackEndReached(void *data)
{
	AUTO_LOG_FUNCTION;
	CallbackPlayer(data, WM_USER_END_REACHED);
}
};


CDuiVideoPlayer::CDuiVideoPlayer(const wchar_t* xml)
	: CXMLWnd(xml)
{
}


CDuiVideoPlayer::~CDuiVideoPlayer()
{
}

void CDuiVideoPlayer::InitWindow()
{
	CenterWindow();

	/*cap_.open(video_path_);
	if (cap_.isOpened()) {
		double fps = cap_.get(CAP_PROP_FPS);
		int gap = static_cast<int>(1000.0 / fps);
		SetTimer(m_hWnd, 1, gap, nullptr);
	}*/

	CWndUI *pWnd = static_cast<CWndUI*>(m_PaintManager.FindControl(_T("wndMedia")));
	if (pWnd) {
		player_.SetHWND(pWnd->GetHWND());
		player_.SetCallbackPlaying(CallbackPlaying);
		player_.SetCallbackPosChanged(CallbackPosChanged);
		player_.SetCallbackEndReached(CallbackEndReached);
	} else {
		JLOG_ERRO("CDuiVideoPlayer::InitWindow() cannot find wndMedia!");
	}
}

void CDuiVideoPlayer::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiVideoPlayer::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AUTO_LOG_FUNCTION;
	if (uMsg == WM_TIMER && wParam == 1) {
		
	} else if (uMsg == WM_USER_END_REACHED) {
		OnEndReached(reinterpret_cast<void*>(wParam));

	} else if (uMsg == WM_LBUTTONUP ) {
		
	} else if (uMsg == WM_PARENTNOTIFY) {
		if (wParam == WM_LBUTTONDOWN) {
			JLOG_INFO("CDuiVideoPlayer::HandleMessage WM_PARENTNOTIFY WM_LBUTTONDOWN disable_click_ {}", disable_click_);
			auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
			if (maindlg) {
				if (maindlg->do_video_view_mode_show_or_hide_tools(show_tip_)) {
					show_tip_ = !show_tip_;
				}
			}
		}
	} else if (uMsg == WM_USER_PLAYING) {
		JLOG_INFO("CDuiVideoPlayer::HandleMessage WM_USER_PLAYING {:d} {:d}", wParam, lParam);

	} else if (uMsg == WM_USER_POS_CHANGED) {
		//JLOG_INFO("CDuiVideoPlayer::HandleMessage WM_USER_POS_CHANGED {:d} {:d}", wParam, lParam);
		struct tm   tmTotal, tmCurrent;
		time_t      timeTotal = player_.GetTotalTime() / 1000;
		time_t      timeCurrent = player_.GetTime() / 1000;
		tm_remained_ = timeTotal - timeCurrent;
		if (tm_remained_ <= 1) {
			disable_click_ = true;
			::EnableWindow(m_hWnd, 0);
		}
		JLOG_INFO("playing, remained {}s, disable_click_ {}", tm_remained_, disable_click_);
		TCHAR       szTotal[MAX_PATH], szCurrent[MAX_PATH];
		gmtime_s(&tmTotal, &timeTotal);
		gmtime_s(&tmCurrent, &timeCurrent);
		_tcsftime(szTotal, MAX_PATH, _T("%X"), &tmTotal);
		_tcsftime(szCurrent, MAX_PATH, _T("%X"), &tmCurrent);
		std::wstring cur(szCurrent);
		std::wstring total(szTotal);

		auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
		if (maindlg) {
			maindlg->do_video_view_mode_pos_changed(cur, total, player_.GetPos());
		}
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiVideoPlayer::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}

CControlUI * CDuiVideoPlayer::CreateControl(LPCTSTR pstrClassName)
{
	CDuiString     strXML;
	CDialogBuilder builder;

	if (_tcsicmp(pstrClassName, _T("WndMediaDisplay")) == 0) {
		CWndUI *pUI = new CWndUI();
		HWND hWnd = CreateWindowW(_T("#32770"), _T("WndMediaDisplay"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, m_PaintManager.GetPaintWindow(), (HMENU)0, NULL, NULL);
		pUI->Attach(hWnd);
		return pUI;
	}

	if (!strXML.IsEmpty()) {
		CControlUI* pUI = builder.Create(strXML.GetData(), NULL, NULL, &m_PaintManager, NULL); // 这里必须传入m_PaintManager，不然子XML不能使用默认滚动条等信息。
		return pUI;
	}

	return NULL;
}

void CDuiVideoPlayer::OnEndReached(void *)
{
	AUTO_LOG_FUNCTION;
	stop();

	auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
	if (maindlg) {
		maindlg->do_video_view_mode_pos_changed(L"", L"", -1);
	}

	
}

bool CDuiVideoPlayer::play(const std::string& path)
{
	if (player_.IsPlaying()) {
		player_.Stop();
	}
	if (!path.empty()) {
		video_path_ = path;
		show_tip_ = true;
	}
	disable_click_ = false;
	::EnableWindow(m_hWnd, 1);
	return player_.Play(video_path_);
}

bool CDuiVideoPlayer::pause()
{
	if (player_.IsPlaying()) {
		player_.Pause();
		return true;
	}
	return false;
}

bool CDuiVideoPlayer::resume()
{
	if (player_.IsOpen()) {
		player_.Play();
		return true;
	}
	return false;
}

bool CDuiVideoPlayer::stop()
{
	player_.Stop();
	disable_click_ = false;
	::EnableWindow(m_hWnd, 1);
	return true;
}

bool CDuiVideoPlayer::set_pos(int pos)
{
	if (player_.IsPlaying()) {
		player_.SeekTo(pos);
		return true;
	}
	return false;
}

















DUI_BEGIN_MESSAGE_MAP(CDuiVideoPlayer, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
