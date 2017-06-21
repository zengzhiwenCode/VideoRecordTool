#include "stdafx.h"
#include "DuiVideoPos.h"
#include "vrmfcDlg.h"

CDuiVideoPos::CDuiVideoPos(const wchar_t* xml)
	: CXMLWnd(xml)
{
}


CDuiVideoPos::~CDuiVideoPos()
{
}

void CDuiVideoPos::InitWindow()
{
	slider_ = static_cast<CSliderUI*>(m_PaintManager.FindControl(L"sliderPlay")); assert(slider_);
}

void CDuiVideoPos::Notify(DuiLib::TNotifyUI & msg)
{
	if (msg.sType == L"valuechanged" && msg.pSender->GetName() == L"sliderPlay") {
		int pos = slider_->GetValue();
		auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
		if (maindlg) {
			maindlg->do_video_view_mode_user_change_pos(pos);
		}
	}
	__super::Notify(msg);
}

LRESULT CDuiVideoPos::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiVideoPos::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}

void CDuiVideoPos::SetPos(int pos)
{
	if (slider_) {
		slider_->SetValue(pos);
	}
}









DUI_BEGIN_MESSAGE_MAP(CDuiVideoPos, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
