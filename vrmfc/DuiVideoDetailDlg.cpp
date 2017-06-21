#include "stdafx.h"
#include "DuiVideoDetailDlg.h"
#include "config.h"
#include "vrmfc.h"

CDuiVideoDetailDlg::CDuiVideoDetailDlg(const wchar_t* xml)
	: CXMLWnd(xml)
{
}


CDuiVideoDetailDlg::~CDuiVideoDetailDlg()
{
}

void CDuiVideoDetailDlg::InitWindow()
{
	CenterWindow();

	get_ctrl(CLabelUI, filename);
	get_ctrl(CLabelUI, file_size);
	get_ctrl(CLabelUI, resolution);
	get_ctrl(CLabelUI, file_format);
	get_ctrl(CLabelUI, video_length);
	get_ctrl(CLabelUI, fps);
	get_ctrl(CLabelUI, create_time);

	get_ctrl(CLabelUI, filename_val);
	get_ctrl(CLabelUI, file_size_val);
	get_ctrl(CLabelUI, resolution_val);
	get_ctrl(CLabelUI, file_format_val);
	get_ctrl(CLabelUI, video_length_val);
	get_ctrl(CLabelUI, fps_val);
	get_ctrl(CLabelUI, create_time_val);

	filename->SetText(trw(IDS_STRING_FILENAME).c_str());
	file_size->SetText(trw(IDS_STRING_FILESIZE).c_str());
	resolution->SetText(trw(IDS_STRING_RESOLUTION).c_str());
	file_format->SetText(trw(IDS_STRING_FILEEXT).c_str());
	video_length->SetText(trw(IDS_STRING_VIDEOLEN).c_str());
	fps->SetText(trw(IDS_STRING_FPS).c_str());
	create_time->SetText(trw(IDS_STRING_CREATETIME).c_str());

	filename_val->SetText(videoinfo_.name.c_str());
	file_size_val->SetText(videoinfo_.filesize.c_str());
	resolution_val->SetText(videoinfo_.resolution.c_str());
	file_format_val->SetText(videoinfo_.ext.c_str());
	video_length_val->SetText(videoinfo_.video_length.c_str());
	fps_val->SetText(videoinfo_.fps.c_str());
	create_time_val->SetText(videoinfo_.create_time.c_str());

	get_ctrl(CLabelUI, title);
	get_ctrl(CButtonUI, closebtn);
	title->SetText(trw(IDS_STRING_VIDEOINFO).c_str());
	closebtn->SetText(trw(IDS_STRING_CLOSE).c_str());
}

void CDuiVideoDetailDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiVideoDetailDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiVideoDetailDlg::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}




DUI_BEGIN_MESSAGE_MAP(CDuiVideoDetailDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
