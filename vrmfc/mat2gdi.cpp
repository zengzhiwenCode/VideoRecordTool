#include "stdafx.h"
#include "mat2gdi.h"

namespace detail {
class clientdc {
private:
	HWND hwnd_ = nullptr;
	HDC hdc_ = nullptr;
public:
	explicit clientdc(HWND hwnd) {
		hwnd_ = hwnd;
		hdc_ = GetDC(hwnd);
	}

	~clientdc() {
		ReleaseDC(hwnd_, hdc_);
	}

	operator HDC() {
		return hdc_;
	}
};
}

PkMatToGDI::PkMatToGDI(HWND ctrl, bool autofit)
{
	Init(ctrl, autofit);
}

bool PkMatToGDI::SetDestination(HWND ctrl, bool autofit)
{
	return Init(ctrl, autofit);
}

bool PkMatToGDI::SetDestination(HWND ctrl)
{
	return Init(ctrl, m_autofit);
}

bool PkMatToGDI::DrawImg(const cv::Mat & img)
{
	AUTO_LOG_FUNCTION;
	if (m_WinCtrl == NULL || img.empty())
		return false;

	detail::clientdc hDC(m_WinCtrl);
	int bpp = 8 * img.elemSize();
	assert((bpp == 8 || bpp == 24 || bpp == 32));

	int img_w = img.cols;
	int img_h = img.rows;

	cv::Rect rr;
	if (m_autofit) {
		if (m_ctrlRectRatio > (1.0*img_w / img_h)) {
			// fit height
			rr.height = m_ctrlRectCv.height;
			rr.width = (int)floor(1.0*img_w * m_ctrlRectCv.height / img_h);
			//align center
			rr.x = (int)floor((m_ctrlRectCv.width - rr.width) / 2.0);
			rr.y = m_ctrlRectCv.y;
		} else {
			// fit width
			rr.width = m_ctrlRectCv.width;
			rr.height = (int)floor(1.0*img_h * m_ctrlRectCv.width / img_w);
			//align middle
			rr.x = m_ctrlRectCv.x;
			rr.y = (int)floor((m_ctrlRectCv.height - rr.height) / 2);
		}
	} else {
		//stretch
		rr = m_ctrlRectCv;
	}


	// The image must be padded 4bytes and must be continuous
	//int stride = ((((img.cols * bpp) + 31) & ~31) >> 3);
	int padding = 0;
	// 32 bit image is always DWORD aligned because each pixel requires 4 bytes
	if (bpp == 32)
		padding = 0;
	else if ((img.cols % 4) > 0)
		padding = 4 - (img.cols % 4);
	else
		padding = 0;

	cv::Mat tempimg;
	if (padding > 0 || img.isContinuous() == false) {
		// Adding needed columns on the right (max 3 px)

		// we use internal image to reuse the memory. Avoid to alloc new memory at each call due to img size changes rarely
		cv::copyMakeBorder(img, m_internalImg, 0, 0, 0, padding, cv::BORDER_CONSTANT, 0);
		tempimg = m_internalImg;
		// ignore (do not shows) the just added border
		//img_w = tempimg.cols;
	} else {
		tempimg = img;
	}

	BITMAPINFO* bmi;
	BITMAPINFOHEADER* bmih;
	if (bpp == 8) {
		bmi = m_bmiGrey;
	} else {
		bmi = m_bmiColor;
	}

	bmih = &(bmi->bmiHeader);
	bmih->biHeight = -tempimg.rows;
	bmih->biWidth = tempimg.cols;
	bmih->biBitCount = bpp;

	//------------------
	// DRAW THE IMAGE 

	//if source and destination are same size
	if (tempimg.size() == m_ctrlRectCv.size()) {
		// tranfer memory block
		// NOTE: the padding border will be shown here. Anyway it will be max 3px width
		int numLines = SetDIBitsToDevice(hDC,
										 m_ctrlRectCv.x, m_ctrlRectCv.y, m_ctrlRectCv.width, m_ctrlRectCv.height,
										 0, 0, 0, tempimg.rows, tempimg.data, bmi, DIB_RGB_COLORS);
		if (numLines == 0)
			return false;

		m_destRectCv = m_ctrlRectCv;
		// all done
		return true;
	}

	//if destination rect is smaller of previous we need to clear the background
	if (m_destRectCv.width <= 0) {
		m_destRectCv = rr;
	} else if (rr != m_destRectCv) {
		BackgroundClear();
		m_destRectCv = rr;
	}

	//if destination width less than source width
	else if (m_destRectCv.width < img_w) {
		SetStretchBltMode(hDC, HALFTONE);
	} else {
		SetStretchBltMode(hDC, COLORONCOLOR);
	}

	//copy and stretch the image
	int numLines = StretchDIBits(hDC,
								 m_destRectCv.x, m_destRectCv.y, m_destRectCv.width, m_destRectCv.height,
								 0, 0, img_w, img_h,
								 tempimg.data, bmi, DIB_RGB_COLORS, SRCCOPY);
	if (numLines == 0)
		return false;

	//all done
	return true;
}

void PkMatToGDI::BackgroundClear()
{
	detail::clientdc hDC(m_WinCtrl);
	//the rectangle is outlined by using the current pen and filled by using the current brush
	::Rectangle(hDC, m_ctrlRectWin.left, m_ctrlRectWin.top, m_ctrlRectWin.right, m_ctrlRectWin.bottom);
}

bool PkMatToGDI::Init(HWND ctrl, bool autofit)
{
	m_WinCtrl = ctrl;
	if (m_WinCtrl == NULL)
		return false;

	m_autofit = autofit;

	//m_WinCtrl->GetClientRect(&m_ctrlRectWin);
	GetClientRect(m_WinCtrl, &m_ctrlRectWin);

	m_ctrlRectCv.x = m_ctrlRectWin.left;
	m_ctrlRectCv.y = m_ctrlRectWin.top;
	m_ctrlRectCv.width = m_ctrlRectWin.right - m_ctrlRectWin.left;
	m_ctrlRectCv.height = m_ctrlRectWin.bottom - m_ctrlRectWin.top;
	m_ctrlRectRatio = 1.0*m_ctrlRectCv.width / m_ctrlRectCv.height;
	m_destRectCv = m_ctrlRectCv;

	BITMAPINFOHEADER*	bmih;
	//standard colour bitmapinfo
	m_bmiColor = (BITMAPINFO*)_bmiColorBuffer;
	bmih = &(m_bmiColor->bmiHeader);
	memset(bmih, 0, sizeof(*bmih));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = 0;
	bmih->biHeight = 0;
	bmih->biPlanes = 1;
	bmih->biBitCount = 0;
	bmih->biSizeImage = bmih->biWidth * bmih->biHeight * bmih->biBitCount / 8;
	bmih->biCompression = BI_RGB;

	//grey scale bitmapinfo
	m_bmiGrey = (BITMAPINFO*)_bmiGreyBuffer;
	bmih = &(m_bmiGrey->bmiHeader);
	memset(bmih, 0, sizeof(*bmih));
	bmih->biSize = sizeof(BITMAPINFOHEADER);
	bmih->biWidth = 0;
	bmih->biHeight = 0;
	bmih->biPlanes = 1;
	bmih->biBitCount = 8;
	bmih->biSizeImage = bmih->biWidth * bmih->biHeight * bmih->biBitCount / 8;
	bmih->biCompression = BI_RGB;

	//create a grey scale palette
	RGBQUAD* palette = m_bmiGrey->bmiColors;
	int i;
	for (i = 0; i < 256; i++) {
		palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
		palette[i].rgbReserved = 0;
	}
	return true;
}
