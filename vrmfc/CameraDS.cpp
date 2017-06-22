//////////////////////////////////////////////////////////////////////
// Video Capture using DirectShow
// Author: Shiqi Yu (shiqi.yu@gmail.com)
// Thanks to:
//		HardyAI@OpenCV China
//		flymanbox@OpenCV China (for his contribution to function CameraName, and frame width/height setting)
// Last modification: April 9, 2009
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// 使用说明：
//   1. 将CameraDS.h CameraDS.cpp以及目录DirectShow复制到你的项目中
//   2. 菜单 Project->Settings->Settings for:(All configurations)->C/C++->Category(Preprocessor)->Additional include directories
//      设置为 DirectShow/Include
//   3. 菜单 Project->Settings->Settings for:(All configurations)->Link->Category(Input)->Additional library directories
//      设置为 DirectShow/Lib
//////////////////////////////////////////////////////////////////////

// CameraDS.cpp: implementation of the CCameraDS class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CameraDS.h"
#include <Streams.h>
#include <comdef.h>

#ifdef _DEBUG
#pragma comment(lib, "strmbasd.lib")
#else
#pragma comment(lib, "strmbase.lib")
#endif // _DEBUG

std::string get_hr_msg(HRESULT hr) {
	_com_error ce(hr);
	return utf8::w2a(ce.ErrorMessage());
}


#pragma comment(lib,"Strmiids.lib") 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCameraDS::CCameraDS()
{
	m_bConnected = m_bLock = m_bChanged = false;
	m_nWidth = m_nHeight = 0;
	m_nBufferSize = 0;

	//m_pFrame = NULL;

	m_pNullFilter = NULL;
	m_pMediaEvent = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pGraph = NULL;

	CoInitialize(NULL);
}

CCameraDS::~CCameraDS()
{
	CloseCamera();
	CoUninitialize();
}

bool CCameraDS::get_info(int nCamID, mi& mi_, procamp& vamp, camera_set& cam)
{
	AUTO_LOG_FUNCTION;
	HRESULT hr = S_OK;

	CoInitialize(NULL);
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&m_pGraph);

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&m_pSampleGrabberFilter);

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pMediaEvent);

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&m_pNullFilter);

	hr = m_pGraph->AddFilter(m_pNullFilter, L"NullRenderer");

	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);

	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	//mt.subtype = MEDIASUBTYPE_YUY2;
	mt.formattype = FORMAT_VideoInfo;
	hr = m_pSampleGrabber->SetMediaType(&mt);
	MYFREEMEDIATYPE(mt);

	m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Grabber");

	// Bind Device Filter.  We know the device because the id was passed in
	BindFilter(nCamID, &m_pDeviceFilter);
	m_pGraph->AddFilter(m_pDeviceFilter, NULL);

	CComPtr<IEnumPins> pEnum;
	m_pDeviceFilter->EnumPins(&pEnum);

	hr = pEnum->Reset();
	hr = pEnum->Next(1, &m_pCameraOutput, NULL);

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pGrabberInput, NULL);

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	pEnum->Skip(1);
	hr = pEnum->Next(1, &m_pGrabberOutput, NULL);

	pEnum = NULL;
	m_pNullFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pNullInputPin, NULL);
	
	IAMStreamConfig *iconfig = NULL;
	hr = m_pCameraOutput->QueryInterface(IID_IAMStreamConfig, (void**)&iconfig);

	AM_MEDIA_TYPE *pmt;
	if (iconfig->GetFormat(&pmt) != S_OK) {
		JLOG_ERRO("iconfig->GetFormat failed {}", get_hr_msg(hr));
		return false;
	}

	IEnumMediaTypes *mediaTypesEnumerator = NULL;
	VIDEOINFOHEADER* videoInfoHeader = NULL;
	hr = m_pCameraOutput->EnumMediaTypes(&mediaTypesEnumerator);
	if (hr != S_OK) {
		JLOG_ERRO("m_pCameraOutput->EnumMediaTypes failed {}", get_hr_msg(hr));
		return false;
	}

	JLOG_INFO("enumming media info");
	while (S_OK == mediaTypesEnumerator->Next(1, &pmt, NULL)) {
		if ((pmt->majortype == MEDIATYPE_Video) &&
			(pmt->formattype == FORMAT_VideoInfo) &&
			(pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
			(pmt->pbFormat != NULL)) {
			pmt->subtype;
			auto name = GuidNames[pmt->subtype];
			videoInfoHeader = (VIDEOINFOHEADER*)pmt->pbFormat;
			videoInfoHeader->bmiHeader.biWidth;  // Supported width
			videoInfoHeader->bmiHeader.biHeight; // Supported height

			if (pmt->subtype == MEDIASUBTYPE_YUY2) {
				mi_[MT_YUY2].type = MT_YUY2;
				misz sz = { videoInfoHeader->bmiHeader.biWidth , videoInfoHeader->bmiHeader.biHeight };
				if (mi_[MT_YUY2].sizes.find(sz) == mi_[MT_YUY2].sizes.end()) {
					mi_[MT_YUY2].sizes.insert(sz);
				} else {
					mi_[MT_YUY2].default_sz = sz;
				}

				JLOG_INFO("MEDIASUBTYPE_YUY2 {}*{}", sz.first, sz.second);
			} else if (pmt->subtype == MEDIASUBTYPE_MJPG) {
				mi_[MT_MJPG].type = MT_MJPG;
				misz sz = { videoInfoHeader->bmiHeader.biWidth , videoInfoHeader->bmiHeader.biHeight };

				if (mi_[MT_MJPG].sizes.find(sz) == mi_[MT_MJPG].sizes.end()) {
					mi_[MT_MJPG].sizes.insert(sz);
				} else {
					mi_[MT_MJPG].default_sz = sz;
				}
				JLOG_INFO("MEDIASUBTYPE_MJPG {}*{}", sz.first, sz.second);
			} else {
				JLOG_INFO("unrecognized media type {} {}*{}", name, videoInfoHeader->bmiHeader.biWidth, videoInfoHeader->bmiHeader.biHeight);
			}
		}
		MYFREEMEDIATYPE(*pmt);
	}

	for (auto& m : mi_) {
		if (m.second.default_sz.first == 0 || m.second.default_sz.second == 0) {
			m.second.default_sz = *m.second.sizes.begin();
		}
	}

	iconfig->Release();
	iconfig = NULL;
	MYFREEMEDIATYPE(*pmt);

	//procamp vamp = {};
	
	hr = m_pDeviceFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pamp);
	if (SUCCEEDED(hr)) {
		/*hr = pamp->GetRange(VideoProcAmp_BacklightCompensation, &vamp.backlight.min_, &vamp.backlight.max_, &vamp.backlight.step_, &vamp.backlight.default_, &vamp.backlight.flags_);
		if (SUCCEEDED(hr)) {
			hr = pamp->Get(VideoProcAmp_BacklightCompensation, &vamp.backlight.val_, &vamp.backlight.flags_);
		}
		if (SUCCEEDED(hr)) {
			vamp.backlight.valid_ = 1;
		}
		*/

#define get_video_property(p, n) \
		hr = pamp->GetRange(p, &vamp.n.min_, &vamp.n.max_, &vamp.n.step_, &vamp.n.default_, &vamp.n.flags_); \
		if (SUCCEEDED(hr)) { \
			hr = pamp->Get(p, &vamp.n.val_, &vamp.n.flags_); \
		} \
		if (SUCCEEDED(hr)) { \
			vamp.n.valid_ = 1; \
		}

		get_video_property(VideoProcAmp_BacklightCompensation, backlight);
		get_video_property(VideoProcAmp_Brightness, brightness);
		get_video_property(VideoProcAmp_Contrast, contrast);
		get_video_property(VideoProcAmp_Gain, gain);
		get_video_property(VideoProcAmp_Gamma, gamma);
		get_video_property(VideoProcAmp_Hue, hue);
		get_video_property(VideoProcAmp_Saturation, saturation);
		get_video_property(VideoProcAmp_Sharpness, sharpness);
		get_video_property(VideoProcAmp_WhiteBalance, white_balance);
		
	} else {
		JLOG_ERRO("m_pDeviceFilter->QueryInterface IID_IAMVideoProcAmp failed {}", get_hr_msg(hr));
	}

	//camera_set cam = {};
	
	hr = m_pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pcam);
	if (SUCCEEDED(hr)) {
		/*hr = pcam->GetRange(CameraControl_Exposure, &cam.exposure.min_, &cam.exposure.max_, &cam.exposure.step_, &cam.exposure.default_, &cam.exposure.flags_);
		if (SUCCEEDED(hr)) {
			hr = pcam->Get(CameraControl_Exposure, &cam.exposure.val_, &cam.exposure.flags_);
		}
		if (SUCCEEDED(hr)) {
			cam.exposure.valid_ = 1;
		}
		*/

#define get_camera_property(p, n) \
		hr = pcam->GetRange(p, &cam.n.min_, &cam.n.max_, &cam.n.step_, &cam.n.default_, &cam.n.flags_); \
		if (SUCCEEDED(hr)) { \
			hr = pcam->Get(p, &cam.n.val_, &cam.n.flags_); \
		} \
		if (SUCCEEDED(hr)) { \
			cam.n.valid_ = 1; \
		}

		get_camera_property(CameraControl_Exposure, exposure);
		get_camera_property(CameraControl_Focus, focus);
		get_camera_property(CameraControl_Iris, iris);
		get_camera_property(CameraControl_Pan, pan);
		get_camera_property(CameraControl_Roll, roll);
		get_camera_property(CameraControl_Tilt, tilt);
		get_camera_property(CameraControl_Zoom, zoom);
	} else {
		JLOG_ERRO("m_pDeviceFilter->QueryInterface IID_IAMCameraControl failed {}", get_hr_msg(hr));
	}

	hr = m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput);

	if (FAILED(hr)) {
		switch (hr) {
		case VFW_S_NOPREVIEWPIN:
			break;
		case E_FAIL:
			break;
		case E_INVALIDARG:
			break;
		case E_POINTER:
			break;
		}

		JLOG_ERRO("m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput) failed {}", get_hr_msg(hr));
	}

	hr = m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin);

	if (FAILED(hr)) {
		switch (hr) {
		case VFW_S_NOPREVIEWPIN:
			break;
		case E_FAIL:
			break;
		case E_INVALIDARG:
			break;
		case E_POINTER:
			break;
		}

		JLOG_ERRO("m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin) failed {}", get_hr_msg(hr));
	}

	m_pSampleGrabber->SetBufferSamples(TRUE);
	m_pSampleGrabber->SetOneShot(TRUE);

	hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr)) {
		JLOG_ERRO("m_pSampleGrabber->GetConnectedMediaType failed {}", get_hr_msg(hr));
		return false;
	}

	VIDEOINFOHEADER *videoHeader;
	videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
	m_nWidth = videoHeader->bmiHeader.biWidth;
	m_nHeight = videoHeader->bmiHeader.biHeight;
	m_bConnected = true;

	pEnum = NULL;

	CloseCamera();

	this->mi_ = mi_;
	vamp_ = vamp;
	cam_ = cam;

	return true;
}

bool CCameraDS::update_video(VideoProcAmpProperty p, int value)
{
	do {
		if (!pamp)break;

#define set_video_property(ppp) \
	if (vamp_.ppp.val_ != value) { \
		hr = pamp->Set(p, value, vamp_.ppp.flags_); \
		if (SUCCEEDED(hr)) { \
			vamp_.ppp.val_ = value; \
			return true; \
		} \
	}

		HRESULT hr = S_FALSE;
		switch (p) {
		case VideoProcAmp_Brightness:
			set_video_property(brightness);
			break;
		case VideoProcAmp_Contrast:
			set_video_property(contrast);
			break;
		case VideoProcAmp_Hue:
			set_video_property(hue);
			break;
		case VideoProcAmp_Saturation:
			set_video_property(saturation);
			break;
		case VideoProcAmp_Sharpness:
			set_video_property(sharpness);
			break;
		case VideoProcAmp_Gamma:
			set_video_property(gamma);
			break;
		//case VideoProcAmp_ColorEnable:
		//	set_video_property(brightness);
		//	break;
		case VideoProcAmp_WhiteBalance:
			set_video_property(white_balance);
			break;
		case VideoProcAmp_BacklightCompensation:
			set_video_property(backlight);
			break;
		case VideoProcAmp_Gain:
			set_video_property(gain);
			break;
		default:
			break;
		}
	} while (false);
	return false;
}

bool CCameraDS::reset_video()
{
	do {
		if (!pamp)break;
		HRESULT hr = S_FALSE;

#define reset_video_property(p, pp) \
	if (vamp_.pp.valid_ && vamp_.pp.val_ != vamp_.pp.default_) { \
		hr = pamp->Set(p, vamp_.pp.default_, vamp_.pp.flags_); \
		if (SUCCEEDED(hr)) { \
			vamp_.pp.val_ = vamp_.pp.default_; \
		}  \
	}

		reset_video_property(VideoProcAmp_Brightness, brightness);
		reset_video_property(VideoProcAmp_Contrast, contrast);
		reset_video_property(VideoProcAmp_Hue, hue);
		reset_video_property(VideoProcAmp_Saturation, saturation);
		reset_video_property(VideoProcAmp_Sharpness, sharpness);
		reset_video_property(VideoProcAmp_Gamma, gamma);
		reset_video_property(VideoProcAmp_WhiteBalance, white_balance);
		reset_video_property(VideoProcAmp_BacklightCompensation, backlight);
		reset_video_property(VideoProcAmp_Gain, gain);

		return true;
	} while (false);
	
	return false;
}

bool CCameraDS::update_camera(CameraControlProperty p, int value)
{
	do {
		if (!pcam) { break; }
		HRESULT hr = S_FALSE;
		if (cam_.exposure.val_ != value) {
			hr = pcam->Set(p, value, cam_.exposure.flags_);
			if (SUCCEEDED(hr)) {
				cam_.exposure.val_ = value;
				return true;
			}
		}

	} while (false);
	return false;
}

bool CCameraDS::reset_camera()
{
	if (!pcam) { return false; }

	if (cam_.exposure.val_ != cam_.exposure.default_) {
		HRESULT hr = pcam->Set(CameraControl_Exposure, cam_.exposure.default_, cam_.exposure.flags_);
		if (SUCCEEDED(hr)) {
			cam_.exposure.val_ = cam_.exposure.default_;
			return true;
		}
	}

	return false;
}

void CCameraDS::CloseCamera()
{
	if (m_bConnected) {
		m_pMediaControl->Stop();
	}

	m_pGraph = NULL;
	m_pDeviceFilter = NULL;
	m_pMediaControl = NULL;
	m_pSampleGrabberFilter = NULL;
	m_pSampleGrabber = NULL;
	m_pGrabberInput = NULL;
	m_pGrabberOutput = NULL;
	m_pCameraOutput = NULL;
	m_pMediaEvent = NULL;
	m_pNullFilter = NULL;
	m_pNullInputPin = NULL;

	//if (m_pFrame) {
	//	cvReleaseImage(&m_pFrame);
	//}

	m_bConnected = m_bLock = m_bChanged = false;
	m_nWidth = m_nHeight = 0;
	m_nBufferSize = 0;
}

bool CCameraDS::OpenCamera(int nCamID, bool bDisplayProperties, int nWidth, int nHeight, const std::string& mstype)
{
	AUTO_LOG_FUNCTION;
	HRESULT hr = S_OK;

	CoInitialize(NULL);
	// Create the Filter Graph Manager.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&m_pGraph);

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&m_pSampleGrabberFilter);

	hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pMediaEvent);

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&m_pNullFilter);

	hr = m_pGraph->AddFilter(m_pNullFilter, L"NullRenderer");

	hr = m_pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pSampleGrabber);

	AM_MEDIA_TYPE   mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	//mt.subtype = MEDIASUBTYPE_YUY2;
	mt.formattype = FORMAT_VideoInfo;
	hr = m_pSampleGrabber->SetMediaType(&mt);
	MYFREEMEDIATYPE(mt);

	m_pGraph->AddFilter(m_pSampleGrabberFilter, L"Grabber");

	// Bind Device Filter.  We know the device because the id was passed in
	BindFilter(nCamID, &m_pDeviceFilter);
	m_pGraph->AddFilter(m_pDeviceFilter, NULL);

	CComPtr<IEnumPins> pEnum;
	m_pDeviceFilter->EnumPins(&pEnum);

	hr = pEnum->Reset();
	hr = pEnum->Next(1, &m_pCameraOutput, NULL);

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pGrabberInput, NULL);

	pEnum = NULL;
	m_pSampleGrabberFilter->EnumPins(&pEnum);
	pEnum->Reset();
	pEnum->Skip(1);
	hr = pEnum->Next(1, &m_pGrabberOutput, NULL);

	pEnum = NULL;
	m_pNullFilter->EnumPins(&pEnum);
	pEnum->Reset();
	hr = pEnum->Next(1, &m_pNullInputPin, NULL);

	//SetCrossBar();

	if (bDisplayProperties) {
		CComPtr<ISpecifyPropertyPages> pPages;

		HRESULT hr = m_pCameraOutput->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pPages);
		if (SUCCEEDED(hr)) {
			PIN_INFO PinInfo;
			m_pCameraOutput->QueryPinInfo(&PinInfo);

			CAUUID caGUID;
			pPages->GetPages(&caGUID);

			OleCreatePropertyFrame(NULL, 0, 0,
								   L"Property Sheet", 1,
								   (IUnknown **)&(m_pCameraOutput.p),
								   caGUID.cElems, caGUID.pElems,
								   0, 0, NULL);

			CoTaskMemFree(caGUID.pElems);
			PinInfo.pFilter->Release();
		}
		pPages = NULL;
	} else {
		//////////////////////////////////////////////////////////////////////////////
		// 加入由 lWidth和lHeight设置的摄像头的宽和高 的功能，默认320*240
		// by flymanbox @2009-01-24
		//////////////////////////////////////////////////////////////////////////////
		IAMStreamConfig *iconfig = NULL;
		hr = m_pCameraOutput->QueryInterface(IID_IAMStreamConfig, (void**)&iconfig);

		AM_MEDIA_TYPE *pmt;
		if (iconfig->GetFormat(&pmt) != S_OK) {
			//printf("GetFormat Failed ! \n");
			JLOG_ERRO("iconfig->GetFormat failed {}", get_hr_msg(hr));
			return false;
		}
		
		// 3、考虑如果此时的的图像大小正好是 nWidth * nHeight，则就不用修改了。
		//if ((pmt->lSampleSize != (nWidth * nHeight * 3)) && (pmt->formattype == FORMAT_VideoInfo)) {
			VIDEOINFOHEADER *phead = (VIDEOINFOHEADER*)(pmt->pbFormat);
			phead->bmiHeader.biWidth = nWidth;
			phead->bmiHeader.biHeight = nHeight;

			JLOG_INFO("setting width={}, height={}", nWidth, nHeight);

			if (mstype == MT_YUY2) {
				pmt->subtype = MEDIASUBTYPE_YUY2;
			} else if (mstype == MT_MJPG) {
				pmt->subtype = MEDIASUBTYPE_MJPG;
			}
			
			if ((hr = iconfig->SetFormat(pmt)) != S_OK) {
				JLOG_ERRO("iconfig->SetFormat failed {}", get_hr_msg(hr));
				return false;
			}
		//}		

		iconfig->Release();
		iconfig = NULL;
		MYFREEMEDIATYPE(*pmt);
	}

	hr = m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput);
	if (FAILED(hr)) {
		switch (hr) {
		case VFW_S_NOPREVIEWPIN:
			break;
		case E_FAIL:
			break;
		case E_INVALIDARG:
			break;
		case E_POINTER:
			break;
		}
		JLOG_ERRO("m_pGraph->Connect(m_pCameraOutput, m_pGrabberInput) failed {}", get_hr_msg(hr));
	}

	hr = m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin);

	if (FAILED(hr)) {
		switch (hr) {
		case VFW_S_NOPREVIEWPIN:
			break;
		case E_FAIL:
			break;
		case E_INVALIDARG:
			break;
		case E_POINTER:
			break;
		}
		JLOG_ERRO("m_pGraph->Connect(m_pGrabberOutput, m_pNullInputPin) failed {}", get_hr_msg(hr));
	}

	m_pSampleGrabber->SetBufferSamples(TRUE);
	m_pSampleGrabber->SetOneShot(TRUE);

	hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr)) {
		JLOG_ERRO("m_pSampleGrabber->GetConnectedMediaType failed {}", get_hr_msg(hr));
		return false;
	}

	VIDEOINFOHEADER *videoHeader;
	videoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
	m_nWidth = videoHeader->bmiHeader.biWidth;
	m_nHeight = videoHeader->bmiHeader.biHeight;
	m_bConnected = true;

	m_pDeviceFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pamp);
	m_pDeviceFilter->QueryInterface(IID_IAMCameraControl, (void**)&pcam);

	pEnum = NULL;
	return true;
}


bool CCameraDS::BindFilter(int nCamID, IBaseFilter **pFilter)
{
	if (nCamID < 0) {
		return false;
	}

	// enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR) {
		return false;
	}

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) {
		return false;
	}

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	int index = 0;
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK, index <= nCamID) {
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr)) {
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) {
				if (index == nCamID) {
					pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
				}
				SysFreeString(var.bstrVal);
			}
			pBag->Release();
		}
		pM->Release();
		index++;
	}

	pCreateDevEnum = NULL;
	return true;
}

//将输入crossbar变成PhysConn_Video_Composite
void CCameraDS::SetCrossBar()
{
	int i;
	IAMCrossbar *pXBar1 = NULL;
	ICaptureGraphBuilder2 *pBuilder = NULL;

	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pBuilder);

	if (SUCCEEDED(hr)) {
		hr = pBuilder->SetFiltergraph(m_pGraph);
	}

	hr = pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, m_pDeviceFilter, IID_IAMCrossbar, (void**)&pXBar1);

	if (SUCCEEDED(hr)) {
		long OutputPinCount, InputPinCount;
		long PinIndexRelated, PhysicalType;
		long inPort = 0, outPort = 0;

		pXBar1->get_PinCounts(&OutputPinCount, &InputPinCount);
		for (i = 0; i < InputPinCount; i++) {
			pXBar1->get_CrossbarPinInfo(TRUE, i, &PinIndexRelated, &PhysicalType);
			if (PhysConn_Video_Composite == PhysicalType) {
				inPort = i;
				break;
			}
		}
		for (i = 0; i < OutputPinCount; i++) {
			pXBar1->get_CrossbarPinInfo(FALSE, i, &PinIndexRelated, &PhysicalType);
			if (PhysConn_Video_VideoDecoder == PhysicalType) {
				outPort = i;
				break;
			}
		}

		if (S_OK == pXBar1->CanRoute(outPort, inPort)) {
			pXBar1->Route(outPort, inPort);
		}
		pXBar1->Release();
	}
	pBuilder->Release();
}

/*
The returned image can not be released.
*/
cv::Mat CCameraDS::QueryFrame()
{
	long evCode, size = 0;

	m_pMediaControl->Run();
	m_pMediaEvent->WaitForCompletion(INFINITE, &evCode);

	m_pSampleGrabber->GetCurrentBuffer(&size, NULL);

	//if the buffer size changed
	if (size != m_nBufferSize) {
		//if (m_pFrame) {
		//	cvReleaseImage(&m_pFrame);
		//}

		m_nBufferSize = size;
		//m_pFrame = cvCreateImage(cvSize(m_nWidth, m_nHeight), IPL_DEPTH_8U, 3);
		//if (!frame.data) {
		frame.create(m_nHeight, m_nWidth, CV_8UC3);
	}

	m_pSampleGrabber->GetCurrentBuffer(&m_nBufferSize, (long*)frame.data);
	//cvFlip(m_pFrame);
	cv::flip(frame, frame, 0);

	return frame;
}

int CCameraDS::CameraCount()
{
	int count = 0;
	CoInitialize(NULL);

	// enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) {
		return count;
	}

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK) {
		count++;
	}

	pCreateDevEnum = NULL;
	pEm = NULL;
	return count;
}

int CCameraDS::CameraName(int nCamID, char* sName, int nBufferSize)
{
	int count = 0;
	CoInitialize(NULL);

	// enumerate all video capture devices
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) return 0;

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK) {
		if (count == nCamID) {
			IPropertyBag *pBag = 0;
			hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
			if (SUCCEEDED(hr)) {
				VARIANT var;
				var.vt = VT_BSTR;
				hr = pBag->Read(L"FriendlyName", &var, NULL); //还有其他属性,像描述信息等等...
				if (hr == NOERROR) {
					//获取设备名称			
					WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, sName, nBufferSize, "", NULL);

					SysFreeString(var.bstrVal);
				}
				pBag->Release();
			}
			pM->Release();

			break;
		}
		count++;
	}

	pCreateDevEnum = NULL;
	pEm = NULL;

	return 1;
}
