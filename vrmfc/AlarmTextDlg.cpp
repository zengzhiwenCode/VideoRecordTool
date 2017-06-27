// AlarmTextDlg.cpp : 实现文件
//

#include "stdafx.h"
//#include "AlarmCenter.h"
#include "AlarmTextDlg.h"
#include "afxdialogex.h"


// CAlarmTextDlg 对话框

IMPLEMENT_DYNAMIC(CAlarmTextDlg, CDialogEx)

CAlarmTextDlg::CAlarmTextDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CAlarmTextDlg::IDD, pParent)
	, m_curPos(0)
	, m_extra(0)
	, m_text(_T(""))
	, m_font()
	, m_bDrawText(TRUE)
	, m_text_color(RGB(255, 255, 255))
	, m_bk_color(RGB(237, 125, 49))
	, m_bAlreadyAddBlank(FALSE)
{

}

CAlarmTextDlg::~CAlarmTextDlg()
{}

void CAlarmTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAlarmTextDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAlarmTextDlg::OnBnClickedOk)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


// CAlarmTextDlg 消息处理程序


BOOL CAlarmTextDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	::SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) ^ WS_EX_LAYERED);
	::SetLayeredWindowAttributes(m_hWnd, RGB(255, 0, 255), 0, LWA_COLORKEY);

	const TCHAR* fonts[12] = {
		L"微软雅黑",
		L"黑体",
		L"Consolas",
		L"楷体",
		L"仿宋体",
		L"Arial",
		L"Courier",
		L"FixedSys",
		L"Roman",
		L"Symbol",
		L"Terminal",
		L"Times New Roman",
	};

	for (int i = 0; i < 11; i++) {
		if (m_font.CreateFont(
			detail::ALARM_TEXT_HEIGHT, // nHeight
			0, // nWidth
			0, // nEscapement
			0, // nOrientation
			FW_NORMAL, //FW_BOLD, // nWeight
			FALSE, // bItalic
			FALSE, // bUnderline
			0, // cStrikeOut
			DEFAULT_CHARSET, // nCharSet
			OUT_DEFAULT_PRECIS, // nOutPrecision
			CLIP_DEFAULT_PRECIS, // nClipPrecision
			DEFAULT_QUALITY, // nQuality
			DEFAULT_PITCH | FF_SWISS,
			fonts[i] // nPitchAndFamily Arial
			)) {
			break;
		}
	}

	m_bDrawText = TRUE;
	m_bAlreadyAddBlank = FALSE;
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CAlarmTextDlg::OnBnClickedOk()
{}

BOOL CAlarmTextDlg::OnEraseBkgnd(CDC *pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}

void CAlarmTextDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rc;
	GetWindowRect(rc);
	ScreenToClient(rc);

	if (!m_bDrawText) {
		//return;
	}

	CFont *pOldFont = dc.SelectObject(&m_font);
	dc.SetTextColor(m_text_color);
	dc.SetBkMode(TRANSPARENT);
	CBrush brush;
	brush.CreateSolidBrush(m_bk_color);//RGB(237, 125, 49)
	dc.FillRect(rc, &brush);
	dc.DrawTextW(m_text, rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	CSize sz = dc.GetTextExtent(m_text);
	if (sz.cx >= rc.Width()) {
		auto_timer timer(m_hWnd, 1, 3500);
		if (!m_bAlreadyAddBlank) {
			m_text += _T("    ");
			m_bAlreadyAddBlank = TRUE;
		}
		CString text = _T("");
		text = m_text.Right(m_text.GetLength() - 1);
		text += m_text.GetAt(0);
		m_text = text;
	}

	dc.SelectObject(pOldFont);

	//CFont *pOldFont = dc.SelectObject(&m_font);
	//dc.SetTextColor(m_clr);
	//dc.SetBkMode(TRANSPARENT);
	////dc.TextOutW(0, 0, m_text);
	//dc.DrawText(m_text, rc, DT_LEFT);
	//CSize sz = dc.GetTextExtent(m_text);
	//if (sz.cx >= rc.Width()) {
	//	auto_timer timer(m_hWnd, 1, 3500);
	//	if (!m_bAlreadyAddBlank) {
	//		m_text += _T("    ");
	//		m_bAlreadyAddBlank = TRUE;
	//	}
	//	CString text = _T("");
	//	text = m_text.Right(m_text.GetLength() - 1);
	//	text += m_text.GetAt(0);
	//	m_text = text;
	//}

	//dc.SelectObject(pOldFont);
}


void CAlarmTextDlg::OnTimer(UINT_PTR nIDEvent)
{
	auto_timer timer(m_hWnd, 1, 500);
	m_bDrawText = FALSE;
	CRect Rect;
	GetWindowRect(&Rect);
	GetParent()->ScreenToClient(&Rect);
	GetParent()->InvalidateRect(&Rect);
	GetParent()->UpdateWindow();
	m_bDrawText = TRUE;
	RedrawWindow();

	CDialogEx::OnTimer(nIDEvent);
}


void CAlarmTextDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	m_font.DeleteObject();
}


HBRUSH CAlarmTextDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}


HBRUSH CAlarmTextDlg::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

