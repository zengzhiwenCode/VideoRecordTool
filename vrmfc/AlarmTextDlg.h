#pragma once
#include "resource.h"

namespace detail {
const int ALARM_TEXT_HEIGHT = 20;
};

// CAlarmTextDlg 对话框

class CAlarmTextDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAlarmTextDlg)

public:
	CAlarmTextDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CAlarmTextDlg();

	// 对话框数据
	enum { IDD = IDD_DIALOG_ALARM_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
private:
	int m_curPos;
	int m_extra;
	CString m_text;
	CFont m_font;
	BOOL m_bDrawText;
	CBitmap m_bitmap;
	CDC m_memDC;
	COLORREF m_text_color;
	COLORREF m_bk_color;
	BOOL m_bAlreadyAddBlank;
public:

	inline void SetText(const CString& text)
	{
		m_text = text;
	}

	CString GetText() const 
	{
		return m_text;
	}

	inline void Show()
	{
		ShowWindow(SW_SHOW);
		//GetParent()->UpdateWindow();
	}

	inline void Hide()
	{
		KillTimer(1);
		ShowWindow(SW_HIDE);
	}

	inline void SetTextColor(COLORREF color)
	{
		if (color != m_text_color) {
			m_text_color = color;
			//GetParent()->UpdateWindow();
		}
	}

	inline void SetBkColor(COLORREF color)
	{
		if (color != m_bk_color) {
			m_bk_color = color;
			//GetParent()->UpdateWindow();
		}
	}

	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
