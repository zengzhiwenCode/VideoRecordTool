#pragma once

class PkMatToGDI;

class mainwnd : public CXMLWnd
{
public:
	//CMainWnd();
	virtual ~mainwnd();
	explicit mainwnd(LPCTSTR pszXMLPath);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void InitWindow() override;
	virtual LPCTSTR GetWindowClassName() const override { return L"mainwnd"; }
	virtual void Notify(DuiLib::TNotifyUI& msg) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnClick(TNotifyUI& msg) override;
	virtual CControlUI* CreateControl(LPCTSTR pstrClassName) override;

public:
	void SetRC(RECT rc) { rc_ = rc; }
	RECT GetRC() const { return rc_; }

protected:
	RECT rc_ = {};
	std::shared_ptr<PkMatToGDI> drawer_ = {};
	cv::VideoCapture capture_ = {};


};
