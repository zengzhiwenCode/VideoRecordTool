// VideoRecordTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mainwnd.h"
#include <fstream>
#include "config.h"

RECT make_xml()
{
	RECT rc = {};

	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);

#ifdef _DEBUG
	cx = 1024;
	cy = 768;
#endif

	rc.right = cx;
	rc.bottom = cy;
	
	auto sx = to_string(cx);
	auto sy = to_string(cy);

	std::ifstream in(get_exe_path_a() + "\\skin\\mainwnd_template.xml");
	if (!in) { return rc; }
	std::ofstream out(get_exe_path_a() + "\\skin\\mainwnd.xml");
	if (!out) { return rc; }

	string line;
	while (getline(in, line)) {
		if (line == R"(<Window size="cx,cy" mininfo="cx,cy" maxinfo="cx,cy" caption="0,0,0,-1" sizebox="0,0,0,0">)") {
			line = R"(<Window size=")" + sx + "," + sy + R"(" mininfo=")" + sx + "," + sy + R"(" maxinfo=")" + sx + "," + sy + R"(" caption="0,0,0,-1" sizebox="0,0,0,0">)";
		}

		out << line << endl;
	}

	return rc;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE,
					   LPWSTR,
					   int)
{
	jlib::init_logger("VideoRecordTool");

	{
		range_log rl("VideoRecordTool's life cycle");

		HRESULT Hr = ::CoInitialize(nullptr);
		if (FAILED(Hr)) return 0;

		config::get_instance();

		using namespace DuiLib;
		CPaintManagerUI::SetInstance(hInstance);
		CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + L"\\skin");

		auto rc = make_xml();

		auto wnd = std::make_unique<mainwnd>(L"mainwnd.xml");
		wnd->SetRC(rc);
		wnd->Create(nullptr, L"mainwnd", UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
		wnd->ShowModal();

		::CoUninitialize();
	}

	config::release_singleton();

    return 0;
}

