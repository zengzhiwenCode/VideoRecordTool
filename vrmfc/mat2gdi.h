#pragma once

class PkMatToGDI
{
public:
	////<summary>Standard constructor</summary>
	////<param name="ctrl">Set the CStatic controls where the cv::Mat will be drawn</param>
	////<param name="autofit">
	////<code>true</code>:the cv::Mat will be fitted on width or height based on destination rectangle
	////<code>false</code>:the cv::Mat will be stretched into destination rectangle
	////</param>
	////<remark>In case you don't provide a valid ctrl at construction time you can use SetDestinationControl(...) later</remark>
	PkMatToGDI(HWND ctrl = NULL, bool autofit = true);

	////<summary>Standard de-constructor</summary>
	~PkMatToGDI() {};

	////<summary>Set the CStatic controls where the cv::Mat will be drawn</summary>
	////<param name="ctrl">a valid CStatic object</param>
	////<param name="autofit">set the autofit feature on/off (see SetAutofit() ) </param>
	bool SetDestination(HWND ctrl, bool autofit);

	////<summary>Set the CStatic controls where the cv::Mat will be drawn</summary>
	////<param name="ctrl">a valid CStatic object</param>
	////<remark>autofit feature will not change</remark>
	bool SetDestination(HWND ctrl);

	////<summary>Set autofit features on/off.</summary>
	////<param name="autofit">
	////<code>true</code>:the cv::Mat will be fitted on width or height based on destination rectangle
	////<code>false</code>:the cv::Mat will be stretched into destination rectangle
	////</param>
	void SetAutofit(bool autofit) { m_autofit = autofit; }

	////<summary>Draw a cv::Mat using the DC of current CStatic control.</summary>
	bool PkMatToGDI::DrawImg(const cv::Mat &img);

private:

	////<summary>Repaint the rectangle using current brush</summary>
	void BackgroundClear();

	////<summary>Initialize members.</summary>
	////<return>false if fail</return>
	bool Init(HWND ctrl, bool autofit);

private:
	//Display mode. True=autofit, false=stretch
	bool m_autofit;
	// image used internally to reduce memory allocation due to DWORD padding requirement
	cv::Mat m_internalImg;
	// the CStatic control where to show the image
	HWND m_WinCtrl;
	// Clientrect related to the m_WinCtrl
	RECT m_ctrlRectWin;
	// Utility: same as m_ctrlRectWin but using cv::Rect 
	cv::Rect m_ctrlRectCv;
	//Internal: ratio width/height for m_ctrlRectWin
	double m_ctrlRectRatio;
	//Internal:The image size into m_ctrlRectWin. This might smaller due to autofit
	cv::Rect m_destRectCv;

	//Bitmap header for standard color image
	BITMAPINFO* m_bmiColor;
	uchar _bmiColorBuffer[sizeof(BITMAPINFO)]; //extra space for grey color table

											   //Bitmap header for grey scale image
	BITMAPINFO* m_bmiGrey;
	uchar _bmiGreyBuffer[sizeof(BITMAPINFO) + 256 * 4]; //extra space for grey color table

};


