#include "stdafx.h"
#include "DuiPicDetailDlg.h"
#include "config.h"
#include "vrmfc.h"

CDuiPicDetailDlg::CDuiPicDetailDlg(const wchar_t* xml)
	: CXMLWnd(xml)
{
}


CDuiPicDetailDlg::~CDuiPicDetailDlg()
{
}

void CDuiPicDetailDlg::InitWindow()
{
	CenterWindow();

	get_ctrl(CLabelUI, filename);
	get_ctrl(CLabelUI, file_size);
	get_ctrl(CLabelUI, resolution);
	get_ctrl(CLabelUI, file_format);
	get_ctrl(CLabelUI, create_time);

	get_ctrl(CLabelUI, filename_val);
	get_ctrl(CLabelUI, file_size_val);
	get_ctrl(CLabelUI, resolution_val);
	get_ctrl(CLabelUI, file_format_val);
	get_ctrl(CLabelUI, create_time_val);

	filename->SetText(trw(IDS_STRING_FILENAME).c_str());
	file_size->SetText(trw(IDS_STRING_FILESIZE).c_str());
	resolution->SetText(trw(IDS_STRING_RESOLUTION).c_str());
	file_format->SetText(trw(IDS_STRING_FILEEXT).c_str());
	create_time->SetText(trw(IDS_STRING_CREATETIME).c_str());

	filename_val->SetText(picinfo_.name.c_str());
	file_size_val->SetText(picinfo_.filesize.c_str());
	resolution_val->SetText(picinfo_.resolution.c_str());
	file_format_val->SetText(picinfo_.ext.c_str());
	create_time_val->SetText(picinfo_.create_time.c_str());

	get_ctrl(CLabelUI, title);
	get_ctrl(CButtonUI, closebtn);
	title->SetText(trw(IDS_STRING_PICINFO).c_str());
	closebtn->SetText(trw(IDS_STRING_CLOSE).c_str());
}

void CDuiPicDetailDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiPicDetailDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiPicDetailDlg::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}




DUI_BEGIN_MESSAGE_MAP(CDuiPicDetailDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
