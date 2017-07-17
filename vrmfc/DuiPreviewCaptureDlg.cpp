#include "stdafx.h"
#include "DuiPreviewCaptureDlg.h"
#include <fstream>
#include "vrmfcDlg.h"

DUI_BEGIN_MESSAGE_MAP(CDuiPreviewCaptureDlg, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

bool CDuiPreviewCaptureDlg::make_xml(int cx, int cy)
{
	auto sx = to_string(cx);
	auto sy = to_string(cy);

	std::ifstream in(get_exe_path_a() + "\\skin\\capture_template.xml");
	if (!in) { return false; }
	std::ofstream out(get_exe_path_a() + "\\skin\\capture.xml");
	if (!out) { return false; }

	string line;
	while (getline(in, line)) {
		if (line == R"(<Window size="cx,cy" mininfo="cx,cy" maxinfo="cx,cy" caption="0,0,0,1" sizebox="0,0,0,0">)") {
			line = R"(<Window size=")" + sx + "," + sy + R"(" mininfo=")" + sx + "," + sy + R"(" maxinfo=")" + sx + "," + sy + R"(" caption="0,0,0,1" sizebox="0,0,0,0">)";
		}

		out << line << endl;
	}

	return true;
}

CDuiPreviewCaptureDlg::CDuiPreviewCaptureDlg(const wchar_t* xmlpath)
	: CXMLWnd(xmlpath)
{
}

CDuiPreviewCaptureDlg::~CDuiPreviewCaptureDlg()
{
}

void CDuiPreviewCaptureDlg::InitWindow()
{
	CenterWindow();

	if (!img_.empty()) {
		auto pic = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"pic")); assert(pic);
		if (pic && !img_.empty()) {
			pic->SetBkImage(utf8::mbcs_to_u16((img_)).c_str());
		}
	}

	if (auto_close_) {
		SetTimer(m_hWnd, 1, 800, nullptr);
	}
}

void CDuiPreviewCaptureDlg::Notify(DuiLib::TNotifyUI & msg)
{
	__super::Notify(msg);
}

LRESULT CDuiPreviewCaptureDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	JLOG_INFO("CDuiPreviewCaptureDlg::HandleMessage uMsg 0x{:X} {:d} {:d}", uMsg, wParam, lParam);
	if (uMsg == WM_TIMER && wParam == 1) {
		PostMessage(WM_CLOSE);
	} else if (uMsg == WM_LBUTTONDOWN) {
		//if (wParam == WM_LBUTTONDOWN) {
			auto maindlg = static_cast<CvrmfcDlg*>(AfxGetApp()->GetMainWnd()); assert(maindlg);
			if (maindlg) {
				if (maindlg->do_picview_mode_show_or_hide_tools(show_tip_)) {
					show_tip_ = !show_tip_;
				}
			}
		//}
	}

	return __super::HandleMessage(uMsg, wParam, lParam);
}

void CDuiPreviewCaptureDlg::OnClick(TNotifyUI & msg)
{
	__super::OnClick(msg);
}

void CDuiPreviewCaptureDlg::set_image(const std::string & img)
{
	img_ = img;

	if (m_hWnd) {
		auto pic = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(L"pic")); assert(pic);
		if (pic && !img_.empty()) {
			pic->SetBkImage(utf8::mbcs_to_u16(img_).c_str());
		}
	} 

	show_tip_ = true;
}
