//////////////////////////////////////////////////////////////////////+
// Video Capture using DirectShow
// Author: Shiqi Yu (shiqi.yu@gmail.com)
// Thanks to:
//		HardyAI@OpenCV China
//		flymanbox@OpenCV China (for his contribution to function CameraName, and frame width/height setting)
// Last modification: April 9, 2009
//
// 使用说明：
//   1. 将CameraDS.h CameraDS.cpp以及目录DirectShow复制到你的项目中
//   2. 菜单 Project->Settings->Settings for:(All configurations)->C/C++->Category(Preprocessor)->Additional include directories
//      设置为 DirectShow/Include
//   3. 菜单 Project->Settings->Settings for:(All configurations)->Link->Category(Input)->Additional library directories
//      设置为 DirectShow/Lib
//////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "camerads.h"

#include <opencv/highgui.h>

const char *g_szTitle = "Camera";

int main()
{
	// 1、考虑到已经存在了显示图像的窗口，那就不必再次驱动摄像头了，即便往下驱动那也是摄像头已被占用。
	if (IsWindowVisible(FindWindow(NULL, g_szTitle))) {
		return (-1);
	}

	//仅仅获取摄像头数目
	int m_iCamCount = CCameraDS::CameraCount();
	printf("There are %d cameras.\n", m_iCamCount);

	if (m_iCamCount == 0) {
		return (-1);
	}

	CCameraDS m_CamDS;

	//获取所有摄像头的名称
	for (int i = 0; i < m_iCamCount; i++) {
		char szCamName[1024];

		int retval = m_CamDS.CameraName(i, szCamName, sizeof(szCamName));

		if (retval > 0) {
			printf("Camera #%d's Name is '%s'.\n", i, szCamName);
		} else {
			printf("Can not get Camera #%d's name.\n", i);
		}
	}

	// 2、考虑到如果有多个摄像头，或者又有其中某个或某几个正在被其它程序占有，故需要逐个遍历，
	// 直到找到可用的为止。
	int m_iCamNum = 0; // 摄像头编号

	//IplImage *pFrame = NULL;
	cv::Mat frame;
	while (m_iCamNum < m_iCamCount) {
		if ((!m_CamDS.OpenCamera(m_iCamNum, false, 960, 720))) {
			m_iCamNum++;
		} else { // 找到合适的摄像头，退出循环。
			break;
		}

		// 关闭摄像头，必须要关闭，因为即将要进行下一次的检测，检测前要清空当前的占用空间。
		m_CamDS.CloseCamera();
	}

	if (m_iCamNum == m_iCamCount) {
		fprintf(stderr, "Can not open camera or is used by another app.\n");

		return (-1);
	}

	cv::namedWindow(g_szTitle);
	//显示
	//cvShowImage(g_szTitle, frame);
	//cv::imshow(g_szTitle, frame);

	while (1) {
		//获取一帧
		frame = m_CamDS.QueryFrame();

		//显示
		cv::imshow(g_szTitle, frame);

		if (cvWaitKey(20) == 'q') {
			break;
		}
	}

	m_CamDS.CloseCamera(); //可不调用此函数，CCameraDS析构时会自动关闭摄像头

	//cvDestroyWindow(g_szTitle);

	return 0;
}
