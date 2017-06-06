#include "stdafx.h"
#include "capture.h"
#include <dbt.h>
#include "status.h"



namespace detail {

//capture_stuff* g_instance = nullptr;
std::map<int, capture_stuff*> capture_stuff::self_map_for_timer_ = {};

void ErrMsg(LPTSTR szFormat, ...)
{
	static TCHAR szBuffer[2048] = { 0 };
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	HRESULT hr = StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');
	HWND hWnd = nullptr;
	//if (theApp.m_pMainWnd) {
	//	hWnd = theApp.m_pMainWnd->GetSafeHwnd();
	//} else {
		hWnd = GetDesktopWindow();
	//}

	MessageBox(hWnd, szBuffer, NULL, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
}

std::wstring get_moniker_display_name(IMoniker* pMoniker) 
{
	TCHAR szBuf[512] = {};
	WCHAR *wszDisplayName = NULL;

	if (pMoniker) {
		if (SUCCEEDED(pMoniker->GetDisplayName(0, 0, &wszDisplayName))) {
			if (wszDisplayName) {
				StringCchCopyN(szBuf, NUMELMS(szBuf), wszDisplayName, NUMELMS(szBuf) - 1);
				CoTaskMemFree(wszDisplayName);
				return std::wstring(szBuf);
			}
		}
	}

	return L"";
}



inline std::wstring get_exe_path()
{
	wchar_t path[1024] = { 0 };
	GetModuleFileName(nullptr, path, 1024);
	std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
	return std::wstring(path).substr(0, pos);
}


inline std::wstring get_file_name(const std::wstring& path_with_ext) {
	std::wstring::size_type pos_end = std::wstring(path_with_ext).find_last_of(L".");
	if(std::wstring::size_type(-1) != pos_end)
		return std::wstring(path_with_ext).substr(0, pos_end);
	return path_with_ext;
}


void exec(const std::wstring& cmd, const std::wstring& param)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcessW(NULL, const_cast<LPWSTR>((cmd + param).c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

typedef std::function<void(void)> wait_cb;

inline bool execute(const std::wstring& cmd, wait_cb cb)
{
	STARTUPINFOW si = { sizeof(si) };
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi;
	BOOL bRet = CreateProcessW(NULL, (LPWSTR)(cmd.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (bRet) {
		while (WAIT_OBJECT_0 != WaitForSingleObject(pi.hProcess, 200)) {
			cb();
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return true;
	}
	return false;
}

capture_stuff::capture_stuff()
{
	ZeroMemory(pMonikerVideoMenus_, sizeof(pMonikerVideoMenus_));
	ZeroMemory(pMonikerAudioMenus_, sizeof(pMonikerAudioMenus_));
}

capture_stuff::~capture_stuff()
{
	//g_instance = nullptr;
}

void capture_stuff::IMonRelease(IMoniker *&pm)
{
	if (pm) {
		pm->Release();
		pm = 0;
	}
}

// Make a graph builder object we can use for capture graph building
//
BOOL capture_stuff::MakeSampleCaptureGraphBuilder()
{
	// we have one already
	if (pSampleCaptureGraphBuilder_)
		return TRUE;

	pSampleCaptureGraphBuilder_ = new ISampleCaptureGraphBuilder();
	if (NULL == pSampleCaptureGraphBuilder_) {
		return FALSE;
	}

	return TRUE;
}


BOOL capture_stuff::MakeGraph()
{
	// we have one already
	if (pGraphBuilder_)
		return TRUE;

	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
								  IID_IGraphBuilder, (LPVOID *)&pGraphBuilder_);

	return (hr == NOERROR) ? TRUE : FALSE;
}


void capture_stuff::RemoveDownstream(IBaseFilter *pf)
{
	IPin *pP = 0, *pTo = 0;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;

	if (!pf)
		return;

	HRESULT hr = pf->EnumPins(&pins);
	pins->Reset();

	while (hr == NOERROR) {
		hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP) {
			pP->ConnectedTo(&pTo);
			if (pTo) {
				hr = pTo->QueryPinInfo(&pininfo);
				if (hr == NOERROR) {
					if (pininfo.dir == PINDIR_INPUT) {
						RemoveDownstream(pininfo.pFilter);
						pGraphBuilder_->Disconnect(pTo);
						pGraphBuilder_->Disconnect(pP);
						pGraphBuilder_->RemoveFilter(pininfo.pFilter);
					}
					pininfo.pFilter->Release();
				}
				pTo->Release();
			}
			pP->Release();
		}
	}

	if (pins)
		pins->Release();
}


void capture_stuff::GetDevices()
{
	if (fDeviceMenuPopulated_) {
		return;
	}
	fDeviceMenuPopulated_ = true;
	iNumVCapDevices_ = 0;

	UINT    uIndex = 0;
	HRESULT hr;

	for (int i = 0; i < NUMELMS(pMonikerVideoMenus_); i++) {
		IMonRelease(pMonikerVideoMenus_[i]);
	}
	for (int i = 0; i < NUMELMS(pMonikerAudioMenus_); i++) {
		IMonRelease(pMonikerAudioMenus_[i]);
	}
	video_moniker_friendly_name_map_.clear();
	audio_moniker_friendly_name_map_.clear();

	// enumerate all video capture devices
	ICreateDevEnum *pCreateDevEnum = 0;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
						  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR) {
		ErrMsg(TEXT("Error Creating Device Enumerator"));
		return;
	}

	IEnumMoniker *pEm = 0;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR) {
		ErrMsg(TEXT("Sorry, you have no video capture hardware.\r\n\r\n")
			   TEXT("Video capture will not function properly."));
		goto EnumAudio;
	}

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;

	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK) {
		IPropertyBag *pBag = 0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr)) {
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) {
				device_info di;
				di.friendly_name = var.bstrVal;
				di.display_name = get_moniker_display_name(pM);
				video_moniker_friendly_name_map_[uIndex] = di;
				SysFreeString(var.bstrVal);
				ASSERT(pMonikerVideoMenus_[uIndex] == 0);
				pMonikerVideoMenus_[uIndex] = pM;
				pM->AddRef();
			}
			pBag->Release();
		}

		pM->Release();
		uIndex++;
	}
	pEm->Release();

	iNumVCapDevices_ = uIndex;


EnumAudio:

	// enumerate all audio capture devices
	uIndex = 0;

	ASSERT(pCreateDevEnum != NULL);

	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	pCreateDevEnum->Release();
	if (hr != NOERROR)
		return;
	pEm->Reset();

	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK) {
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr)) {
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) {
				//if (uIndex == 0) {
				//	audio_moniker_friendly_name_map_[uIndex] = L"Mater Voice Input Device";
				//} else {
				device_info di;
				di.friendly_name = var.bstrVal;
				di.display_name = get_moniker_display_name(pM);
					audio_moniker_friendly_name_map_[uIndex] = di;
				//}
				
				SysFreeString(var.bstrVal);

				ASSERT(pMonikerAudioMenus_[uIndex] == 0);
				pMonikerAudioMenus_[uIndex] = pM;
				pM->AddRef();
			}
			pBag->Release();
		}
		pM->Release();
		uIndex++;
	}

	pEm->Release();
}


BOOL capture_stuff::BuildPreviewGraph()
{
	AUTO_LOG_FUNCTION;
	HRESULT hr;
	AM_MEDIA_TYPE *pmt;

	// we have one already
	if (fPreviewGraphBuilt_)
		return TRUE;

	// No rebuilding while we're running
	if (fCapturing_ || fPreviewing_)
		return FALSE;

	// We don't have the necessary capture filters
	if (pBaseFilterVideoCap_ == NULL)
		return FALSE;
	if (pBaseFilterAudioCap_ == NULL && fCapAudio_)
		return FALSE;

	// we already have another graph built... tear down the old one
	if (fCaptureGraphBuilt)
		TearDownGraph();

	//
	// Render the preview pin - even if there is not preview pin, the capture
	// graph builder will use a smart tee filter and provide a preview.
	//
	// !!! what about latency/buffer issues?

	// NOTE that we try to render the interleaved pin before the video pin, because
	// if BOTH exist, it's a DV filter and the only way to get the audio is to use
	// the interleaved pin.  Using the Video pin on a DV filter is only useful if
	// you don't want the audio.

	if (fMPEG2_) {
		hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Stream, pBaseFilterVideoCap_, NULL, NULL);
		if (FAILED(hr)) {
			ErrMsg(TEXT("Cannot build MPEG2 preview graph!"));
		}

	} else {
		hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, pBaseFilterVideoCap_, NULL, NULL);
		if (hr == VFW_S_NOPREVIEWPIN) {
			// preview was faked up for us using the (only) capture pin
			fPreviewFaked_ = TRUE;
		} else if (hr != S_OK) {
			// maybe it's DV?
			hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pBaseFilterVideoCap_, NULL, NULL);
			if (hr == VFW_S_NOPREVIEWPIN) {
				// preview was faked up for us using the (only) capture pin
				fPreviewFaked_ = TRUE;
			} else if (hr != S_OK) {
				ErrMsg(TEXT("This graph cannot preview!"));
				fPreviewGraphBuilt_ = FALSE;
				return FALSE;
			}
		}

		//
		// Render the closed captioning pin? It could be a CC or a VBI category pin,
		// depending on the capture driver
		//

		if (fCapClosedCaptioning_) {
			hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_CC, NULL, pBaseFilterVideoCap_, NULL, NULL);
			if (hr != NOERROR) {
				hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_VBI, NULL, pBaseFilterVideoCap_, NULL, NULL);
				if (hr != NOERROR) {
					ErrMsg(TEXT("Cannot render closed captioning"));
				}
			}
		}
	}

	//
	// Get the preview window to be a child of our app's window
	//

	// This will find the IVideoWindow interface on the renderer.  It is
	// important to ask the filtergraph for this interface... do NOT use
	// ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
	// know we own the window so it can give us display changed messages, etc.

	hr = pGraphBuilder_->QueryInterface(IID_IVideoWindow, (void **)&pVideoWindow_);
	if (hr != NOERROR) {
		ErrMsg(TEXT("This graph cannot preview properly"));
	} else {
		//Find out if this is a DV stream
		AM_MEDIA_TYPE * pmtDV;

		if (pAMStreamConfigForVideo_ && SUCCEEDED(pAMStreamConfigForVideo_->GetFormat(&pmtDV))) {
			if (pmtDV->formattype == FORMAT_DvInfo) {
				// in this case we want to set the size of the parent window to that of
				// current DV resolution.
				// We get that resolution from the IVideoWindow.
				SmartPtr<IBasicVideo> pBV;

				// If we got here, gcap.pVW is not NULL 
				ASSERT(pVideoWindow_ != NULL);
				hr = pVideoWindow_->QueryInterface(IID_IBasicVideo, (void**)&pBV);

				if (SUCCEEDED(hr)) {
					HRESULT hr1, hr2;
					long lWidth, lHeight;

					hr1 = pBV->get_VideoHeight(&lHeight);
					hr2 = pBV->get_VideoWidth(&lWidth);
					if (SUCCEEDED(hr1) && SUCCEEDED(hr2)) {
						//ResizeWindow(lWidth, abs(lHeight));
					}
				}
			}
		}

		pVideoWindow_->put_Owner((OAHWND)hWndVideo_);    // We own the window now
		pVideoWindow_->put_WindowStyle(WS_CHILD);    // you are now a child
		pVideoWindow_->SetWindowPosition(rcVideo_.left, rcVideo_.top, rcVideo_.right - rcVideo_.left, rcVideo_.bottom - rcVideo_.top); // be this big
		pVideoWindow_->put_Visible(OATRUE);
	}

	// now tell it what frame rate to capture at.  Just find the format it
	// is capturing with, and leave everything alone but change the frame rate
	// No big deal if it fails.  It's just for preview
	// !!! Should we then talk to the preview pin?
	if (pAMStreamConfigForVideo_ && fUseFrameRate_) {
		hr = pAMStreamConfigForVideo_->GetFormat(&pmt);

		// DV capture does not use a VIDEOINFOHEADER
		if (hr == NOERROR) {
			if (pmt->formattype == FORMAT_VideoInfo) {
				VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
				pvi->AvgTimePerFrame = (LONGLONG)(10000000 / FrameRate_);

				hr = pAMStreamConfigForVideo_->SetFormat(pmt);
				if (hr != NOERROR)
					ErrMsg(TEXT("%x: Cannot set frame rate for preview"), hr);
			}
			DeleteMediaType(pmt);
		}
	}

	// make sure we process events while we're previewing!
	hr = pGraphBuilder_->QueryInterface(IID_IMediaEventEx, (void **)&pMediaEventEx_);
	if (hr == NOERROR) {
		pMediaEventEx_->SetNotifyWindow((OAHWND)hWndVideo_, WM_FGNOTIFY, 0);
	}

	// potential debug output - what the graph looks like
	// DumpGraph(gcap.pFg, 1);

	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
	hr = AddGraphToRot(gcap.pGraphBuilder_, &g_dwGraphRegister);
	if (FAILED(hr)) {
		ErrMsg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
		g_dwGraphRegister = 0;
	}
#endif

	// All done.
	fPreviewGraphBuilt_ = TRUE;
	return TRUE;
}


void capture_stuff::TearDownGraph()
{
	AUTO_LOG_FUNCTION;
	SAFE_RELEASE(pFileSinkFilter_);
	SAFE_RELEASE(pConfigAviMux_);
	SAFE_RELEASE(pBaseFilterRender_);
	SAFE_RELEASE(pMediaEventEx_);
	SAFE_RELEASE(pAMDroppedFrames_);

	if (pVideoWindow_) {
		// stop drawing in our window, or we may get wierd repaint effects
		pVideoWindow_->put_Owner(NULL);
		pVideoWindow_->put_Visible(OAFALSE);
		pVideoWindow_->Release();
		pVideoWindow_ = NULL;
	}

	// destroy the graph downstream of our capture filters
	if (pBaseFilterVideoCap_)
		RemoveDownstream(pBaseFilterVideoCap_);
	if (pBaseFilterAudioCap_)
		RemoveDownstream(pBaseFilterAudioCap_);
	if (pBaseFilterVideoCap_)
		pSampleCaptureGraphBuilder_->ReleaseFilters();

	// potential debug output - what the graph looks like
	// if (gcap.pFg) DumpGraph(gcap.pFg, 1);

#ifdef REGISTER_FILTERGRAPH
	// Remove filter graph from the running object table
	if (g_dwGraphRegister) {
		RemoveGraphFromRot(g_dwGraphRegister);
		g_dwGraphRegister = 0;
	}
#endif

	fCaptureGraphBuilt = FALSE;
	fPreviewGraphBuilt_ = FALSE;
	fPreviewFaked_ = FALSE;
}


BOOL capture_stuff::StartPreview()
{
	AUTO_LOG_FUNCTION;
	// way ahead of you
	if (fPreviewing_)
		return TRUE;

	BuildPreviewGraph();
	if (!fPreviewGraphBuilt_)
		return FALSE;

	// run the graph
	IMediaControl *pMC = NULL;
	HRESULT hr = pGraphBuilder_->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (SUCCEEDED(hr)) {
		hr = pMC->Run();
		if (FAILED(hr)) {
			// stop parts that ran
			pMC->Stop();
		}
		pMC->Release();
	}
	if (FAILED(hr)) {
		ErrMsg(TEXT("Error %x: Cannot run preview graph"), hr);
		return FALSE;
	}

	fPreviewing_ = TRUE;
	SetTimer(hWndVideo_, 0, 33, OnVideoWindowTimer);
	self_map_for_timer_[0] = this;
	return TRUE;
}


void __stdcall capture_stuff::OnVideoWindowTimer(HWND hWnd, UINT, UINT_PTR nIDEvent, DWORD)
{
	auto g_instance = self_map_for_timer_[nIDEvent];
	if (g_instance && g_instance->hWndVideo_ == hWnd) {
		// update our status bar with #captured, #dropped
		// if we've stopped capturing, don't do it anymore.  Some WM_TIMER
		// messages may come late, after we've destroyed the graph and
		// we'll get invalid numbers.
		if (g_instance->fCapturing_) {
			g_instance->UpdateStatus(FALSE);
		}

		//if(gcap.fPreviewing) {
		//	if (count == 0) {
		//		GetClientRect(ghwndApp, &rc); // window rect in screen coords
		//	} else if (count == 66) {
		//		HRGN rgn = CreateEllipticRgn(rc.left, rc.top, rc.right, rc.bottom/*,
		//																		 rc.right - rc.left, rc.bottom - rc.top*/); // rounded rect w/50 pixel corners

		//																		 //设置这个区域
		//		SetWindowRgn(ghwndApp, rgn, TRUE);
		//	} else if (count == 99) {

		//		HRGN rgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top); // rounded rect w/50 pixel corners

		//																													 //设置这个区域
		//		SetWindowRgn(ghwndApp, rgn, TRUE);
		//	} else if (count == 132) {
		//		POINT ptVertex[5];

		//		ptVertex[0].x = rc.left;
		//		ptVertex[0].y = rc.bottom;
		//		ptVertex[1].x = rc.right;
		//		ptVertex[1].y = rc.bottom;
		//		ptVertex[2].x = rc.right;
		//		ptVertex[2].y = (rc.top + rc.bottom) / 2;
		//		ptVertex[3].x = (rc.right + rc.left) / 2;
		//		ptVertex[3].y = rc.top;
		//		ptVertex[4].x = rc.left;
		//		ptVertex[4].y = (rc.top + rc.bottom) / 2;
		//		HRGN rgn = CreatePolygonRgn(ptVertex, 5, ALTERNATE);
		//		//HRGN rgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top); // rounded rect w/50 pixel corners

		//																													 //设置这个区域
		//		SetWindowRgn(ghwndApp, rgn, TRUE);
		//		count = 0;
		//	}
		//	count++;
		//}

		// is our time limit up?
		if (g_instance->fUseTimeLimit_) {
			if ((timeGetTime() - g_instance->lCapStartTime_) / 1000 >= g_instance->dwTimeLimit_) {
				g_instance->StopCapture();
				if (g_instance->fWantPreview_) {
					g_instance->BuildPreviewGraph();
					g_instance->StartPreview();
				}
			}
		}
	}
}


// how many captured/dropped so far
//
void capture_stuff::UpdateStatus(BOOL fAllStats)
{
	HRESULT hr;
	LONG lDropped, lNot = 0, lAvgFrameSize;
	TCHAR tach[160];

	// we use this interface to get the number of captured and dropped frames
	// NOTE:  We cannot query for this interface earlier, as it may not be
	// available until the pin is connected
	if (!pAMDroppedFrames_) {
		hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved,
														pBaseFilterVideoCap_, IID_IAMDroppedFrames, (void **)&pAMDroppedFrames_);
		if (hr != S_OK)
			hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
															pBaseFilterVideoCap_, IID_IAMDroppedFrames, (void **)&pAMDroppedFrames_);
	}

	// this filter can't tell us dropped frame info.
	if (!pAMDroppedFrames_) {
		statusUpdateStatus(hWndStatus_, TEXT("Filter cannot report capture information"));
		return;
	}

	hr = pAMDroppedFrames_->GetNumDropped(&lDropped);
	if (hr == S_OK)
		hr = pAMDroppedFrames_->GetNumNotDropped(&lNot);
	if (hr != S_OK)
		return;

	lDropped -= lDroppedBase_;
	lNot -= lNotBase_;

	if (!fAllStats) {
		LONG lTime = timeGetTime() - lCapStartTime_;
		hr = StringCchPrintf(tach, 160, TEXT("Captured %d frames (%d dropped) %d.%dsec\0"),
							 lNot, lDropped, lTime / 1000,
							 lTime / 100 - lTime / 1000 * 10);
		statusUpdateStatus(hWndStatus_, tach);
		return;
	}

	// we want all possible stats, including capture time and actual acheived
	// frame rate and data rate (as opposed to what we tried to get).  These
	// numbers are an indication that though we dropped frames just now, if we
	// chose a data rate and frame rate equal to the numbers I'm about to
	// print, we probably wouldn't drop any frames.

	// average size of frame captured
	hr = pAMDroppedFrames_->GetAverageFrameSize(&lAvgFrameSize);
	if (hr != S_OK)
		return;

	// how long capture lasted
	LONG lDurMS = lCapStopTime_ - lCapStartTime_;
	double flFrame;     // acheived frame rate
	LONG lData;         // acheived data rate

	if (lDurMS > 0) {
		flFrame = (double)(LONGLONG)lNot * 1000. /
			(double)(LONGLONG)lDurMS;
		lData = (LONG)(LONGLONG)(lNot / (double)(LONGLONG)lDurMS * 1000. * (double)(LONGLONG)lAvgFrameSize);
	} else {
		flFrame = 0.;
		lData = 0;
	}

	hr = StringCchPrintf(tach, 160, TEXT("Captured %d frames in %d.%d sec (%d dropped): %d.%d fps %d.%d Meg/sec\0"),
						 lNot, lDurMS / 1000, lDurMS / 100 - lDurMS / 1000 * 10,
						 lDropped, (int)flFrame,
						 (int)(flFrame * 10.) - (int)flFrame * 10,
						 lData / 1000000,
						 lData / 1000 - (lData / 1000000 * 1000));
	statusUpdateStatus(hWndStatus_, tach);
}


void capture_stuff::ChooseDevices(const TCHAR *szVideo, const TCHAR *szAudio)
{
	WCHAR wszVideo[1024], wszAudio[1024];

	StringCchCopyN(wszVideo, NUMELMS(wszVideo), szVideo, NUMELMS(wszVideo) - 1);
	StringCchCopyN(wszAudio, NUMELMS(wszAudio), szAudio, NUMELMS(wszAudio) - 1);
	wszVideo[1023] = wszAudio[1023] = 0;    // Null-terminate

	IBindCtx *lpBC = 0;
	IMoniker *pmVideo = 0, *pmAudio = 0;

	HRESULT hr = CreateBindCtx(0, &lpBC);
	if (SUCCEEDED(hr)) {
		DWORD dwEaten;
		hr = MkParseDisplayName(lpBC, wszVideo, &dwEaten, &pmVideo);
		hr = MkParseDisplayName(lpBC, wszAudio, &dwEaten, &pmAudio);

		lpBC->Release();
	}

	// Handle the case where the video capture device used for the previous session
	// is not available now.
	BOOL bFound = FALSE;

	if (pmVideo != NULL) {
		for (int i = 0; i < NUMELMS(pMonikerVideoMenus_); i++) {
			if (pMonikerVideoMenus_[i] != NULL && S_OK == pMonikerVideoMenus_[i]->IsEqual(pmVideo)) {
				bFound = TRUE;
				break;
			}
		}
	}

	if (!bFound) {
		if (iNumVCapDevices_ > 0) {
			IMonRelease(pmVideo);
			ASSERT(pMonikerVideoMenus_[0] != NULL);
			pmVideo = pMonikerVideoMenus_[0];
			pmVideo->AddRef();
		} else
			goto CleanUp;
	}

	ChooseDevices(pmVideo, pmAudio);

CleanUp:
	IMonRelease(pmVideo);
	IMonRelease(pmAudio);
}


// Check the devices we're currently using and make filters for them
//
void capture_stuff::ChooseDevices(IMoniker *pmVideo, IMoniker *pmAudio)
{
#define VERSIZE 40
#define DESCSIZE 80

	int versize = VERSIZE;
	int descsize = DESCSIZE;
	WCHAR wachVer[VERSIZE] = { 0 }, wachDesc[DESCSIZE] = { 0 };
	TCHAR tachStatus[VERSIZE + DESCSIZE + 5] = { 0 };

	// they chose a new device. rebuild the graphs
	if (pMonikerVideo_ != pmVideo || pMonikerAudio_ != pmAudio) {
		if (pmVideo) {
			pmVideo->AddRef();
		}
		if (pmAudio) {
			pmAudio->AddRef();
		}

		IMonRelease(pMonikerVideo_);
		IMonRelease(pMonikerAudio_);
		pMonikerVideo_ = pmVideo;
		pMonikerAudio_ = pmAudio;

		if (fPreviewing_)
			StopPreview();
		if (fCaptureGraphBuilt || fPreviewGraphBuilt_)
			TearDownGraph();

		FreeCapFilters();
		InitCapFilters();

		if (fWantPreview_) {   // were we previewing?
			BuildPreviewGraph();
			StartPreview();
		}

	}

	// Put the video driver name in the status bar - if the filter supports
	// IAMVideoCompression::GetInfo, that's the best way to get the name and
	// the version.  Otherwise use the name we got from device enumeration
	// as a fallback.
	if (pAMVideoCompression_) {
		HRESULT hr = pAMVideoCompression_->GetInfo(wachVer, &versize, wachDesc, &descsize, NULL, NULL, NULL, NULL);
		if (hr == S_OK) {
			// It's possible that the call succeeded without actually filling
			// in information for description and version.  If these strings
			// are empty, just display the device's friendly name.
			if (wcslen(wachDesc) && wcslen(wachVer)) {
				hr = StringCchPrintf(tachStatus, VERSIZE + DESCSIZE + 5, TEXT("%s - %s\0"), wachDesc, wachVer);
				statusUpdateStatus(hWndStatus_, tachStatus);
				return;
			}
		}
	}

	// Since the GetInfo method failed (or the interface did not exist),
	// display the device's friendly name.
	statusUpdateStatus(hWndStatus_, FriendlyName_);
}


// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
//
BOOL capture_stuff::InitCapFilters()
{
	HRESULT hr = S_OK;
	BOOL f;

	fClosedCaptioningAvailable_ = FALSE;  // assume no closed captioning support

	f = MakeSampleCaptureGraphBuilder();
	if (!f) {
		ErrMsg(TEXT("Cannot instantiate graph builder"));
		return FALSE;
	}

	//
	// First, we need a Video Capture filter, and some interfaces
	//
	pBaseFilterVideoCap_ = NULL;

	if (pMonikerVideo_ != 0) {
		IPropertyBag *pBag;
		FriendlyName_[0] = 0;

		hr = pMonikerVideo_->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr)) {
			VARIANT var;
			var.vt = VT_BSTR;

			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) {
				hr = StringCchCopyW(FriendlyName_, sizeof(FriendlyName_) / sizeof(FriendlyName_[0]), var.bstrVal);
				SysFreeString(var.bstrVal);
			}

			pBag->Release();
		}

		hr = pMonikerVideo_->BindToObject(0, 0, IID_IBaseFilter, (void**)&pBaseFilterVideoCap_);
	}

	if (pBaseFilterVideoCap_ == NULL) {
		ErrMsg(TEXT("Error %x: Cannot create video capture filter"), hr);
		goto InitCapFiltersFail;
	}

	//
	// make a filtergraph, give it to the graph builder and put the video
	// capture filter in the graph
	//

	f = MakeGraph();
	if (!f) {
		ErrMsg(TEXT("Cannot instantiate filtergraph"));
		goto InitCapFiltersFail;
	}

	hr = pSampleCaptureGraphBuilder_->SetFiltergraph(pGraphBuilder_);
	if (hr != NOERROR) {
		ErrMsg(TEXT("Cannot give graph to builder"));
		goto InitCapFiltersFail;
	}

	// Add the video capture filter to the graph with its friendly name
	hr = pGraphBuilder_->AddFilter(pBaseFilterVideoCap_, FriendlyName_);
	if (hr != NOERROR) {
		ErrMsg(TEXT("Error %x: Cannot add vidcap to filtergraph"), hr);
		goto InitCapFiltersFail;
	}

	// Calling FindInterface below will result in building the upstream
	// section of the capture graph (any WDM TVTuners or Crossbars we might
	// need).

	// we use this interface to get the name of the driver
	// Don't worry if it doesn't work:  This interface may not be available
	// until the pin is connected, or it may not be available at all.
	// (eg: interface may not be available for some DV capture)
	hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pBaseFilterVideoCap_,
														 IID_IAMVideoCompression, (void **)&pAMVideoCompression_);
	if (hr != S_OK) {
		hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pBaseFilterVideoCap_,
															 IID_IAMVideoCompression, (void **)&pAMVideoCompression_);
	}

	// !!! What if this interface isn't supported?
	// we use this interface to set the frame rate and get the capture size
	hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pBaseFilterVideoCap_,
														 IID_IAMStreamConfig, (void **)&pAMStreamConfigForVideo_);

	if (hr != NOERROR) {
		hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pBaseFilterVideoCap_,
															 IID_IAMStreamConfig, (void **)&pAMStreamConfigForVideo_);
		if (hr != NOERROR) {
			// this means we can't set frame rate (non-DV only)
			ErrMsg(TEXT("Error %x: Cannot find VCapture:IAMStreamConfig"), hr);
		}
	}

	fCapAudioIsRelevant_ = TRUE;

	AM_MEDIA_TYPE *pmt;

	// default capture format
	if (pAMStreamConfigForVideo_ && pAMStreamConfigForVideo_->GetFormat(&pmt) == S_OK) {
		// DV capture does not use a VIDEOINFOHEADER
		if (pmt->formattype == FORMAT_VideoInfo) {
			// resize our window to the default capture size
			//ResizeWindow(HEADER(pmt->pbFormat)->biWidth, ABS(HEADER(pmt->pbFormat)->biHeight));
		}
		if (pmt->majortype != MEDIATYPE_Video) {
			// This capture filter captures something other that pure video.
			// Maybe it's DV or something?  Anyway, chances are we shouldn't
			// allow capturing audio separately, since our video capture
			// filter may have audio combined in it already!
			fCapAudioIsRelevant_ = FALSE;
			fCapAudio_ = FALSE;
		}
		DeleteMediaType(pmt);
	}

	// we use this interface to bring up the 3 dialogs
	// NOTE:  Only the VfW capture filter supports this.  This app only brings
	// up dialogs for legacy VfW capture drivers, since only those have dialogs
	hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pBaseFilterVideoCap_,
														 IID_IAMVfwCaptureDialogs, (void **)&pAMVfwCaptureDialogs_);

	// Use the crossbar class to help us sort out all the possible video inputs
	// The class needs to be given the capture filters ANALOGVIDEO input pin
	{
		IPin        *pPin = 0;
		IEnumPins   *pEnumPins = 0;
		ULONG        n;
		PIN_INFO     pinInfo;
		BOOL         Found = FALSE;
		IKsPropertySet *pKsPropertySet = 0;
		GUID guid;
		DWORD dw;
		BOOL fMatch = FALSE;

		pCrossbar_ = NULL;

		if (SUCCEEDED(pBaseFilterVideoCap_->EnumPins(&pEnumPins))) {
			while (!Found && (S_OK == pEnumPins->Next(1, &pPin, &n))) {
				if (S_OK == pPin->QueryPinInfo(&pinInfo)) {
					if (pinInfo.dir == PINDIR_INPUT) {
						// is this pin an ANALOGVIDEOIN input pin?
						if (pPin->QueryInterface(IID_IKsPropertySet, (void **)&pKsPropertySet) == S_OK) {
							if (pKsPropertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &guid, sizeof(GUID), &dw) == S_OK) {
								if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
									fMatch = TRUE;
							}
							pKsPropertySet->Release();
						}

						if (fMatch) {
							HRESULT hrCreate = S_OK;
							pCrossbar_ = new CCrossbar(pPin, &hrCreate);
							if (!pCrossbar_ || FAILED(hrCreate))
								break;

							hr = pCrossbar_->GetInputCount(&NumberOfVideoInputs_);
							Found = TRUE;
						}
					}
					pinInfo.pFilter->Release();
				}
				pPin->Release();
			}
			pEnumPins->Release();
		}
	}

	// there's no point making an audio capture filter
	if (fCapAudioIsRelevant_ == FALSE)
		goto SkipAudio;

	// create the audio capture filter, even if we are not capturing audio right
	// now, so we have all the filters around all the time.

	//
	// We want an audio capture filter and some interfaces
	//

	if (pMonikerAudio_ == 0) {
		// there are no audio capture devices. We'll only allow video capture
		fCapAudio_ = FALSE;
		goto SkipAudio;
	}
	pBaseFilterAudioCap_ = NULL;


	hr = pMonikerAudio_->BindToObject(0, 0, IID_IBaseFilter, (void**)&pBaseFilterAudioCap_);

	if (pBaseFilterAudioCap_ == NULL) {
		// there are no audio capture devices. We'll only allow video capture
		fCapAudio_ = FALSE;
		ErrMsg(TEXT("Cannot create audio capture filter"));
		goto SkipAudio;
	}

	//
	// put the audio capture filter in the graph
	//
	{
		WCHAR wachAudioFriendlyName[256];
		IPropertyBag *pBag;

		wachAudioFriendlyName[0] = 0;

		// Read the friendly name of the filter to assist with remote graph
		// viewing through GraphEdit
		hr = pMonikerAudio_->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr)) {
			VARIANT var;
			var.vt = VT_BSTR;

			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR) {
				hr = StringCchCopyW(wachAudioFriendlyName, 256, var.bstrVal);
				SysFreeString(var.bstrVal);
			}

			pBag->Release();
		}

		// We'll need this in the graph to get audio property pages
		hr = pGraphBuilder_->AddFilter(pBaseFilterAudioCap_, wachAudioFriendlyName);
		if (hr != NOERROR) {
			ErrMsg(TEXT("Error %x: Cannot add audio capture filter to filtergraph"), hr);
			goto InitCapFiltersFail;
		}
	}

	// Calling FindInterface below will result in building the upstream
	// section of the capture graph (any WDM TVAudio's or Crossbars we might
	// need).

	// !!! What if this interface isn't supported?
	// we use this interface to set the captured wave format
	hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, pBaseFilterAudioCap_,
														 IID_IAMStreamConfig, (void **)&pAMStreamConfigForAudio_);;

	if (hr != NOERROR) {
		ErrMsg(TEXT("Cannot find ACapture:IAMStreamConfig"));
	}

SkipAudio:

	// Can this filter do closed captioning?
	IPin *pPin;
	hr = pSampleCaptureGraphBuilder_->FindPin(pBaseFilterVideoCap_, PINDIR_OUTPUT, &PIN_CATEGORY_VBI, NULL, FALSE, 0, &pPin);;
	if (hr != S_OK)
		hr = pSampleCaptureGraphBuilder_->FindPin(pBaseFilterVideoCap_, PINDIR_OUTPUT, &PIN_CATEGORY_CC, NULL, FALSE, 0, &pPin);
	if (hr == S_OK) {
		pPin->Release();
		fClosedCaptioningAvailable_ = TRUE;
	} else {
		fCapClosedCaptioning_ = FALSE;    // can't capture it, then
	}

	// potential debug output - what the graph looks like
	// DumpGraph(gcap.pGraphBuilder_, 1);

	return TRUE;

InitCapFiltersFail:
	FreeCapFilters();
	return FALSE;
}


// all done with the capture filters and the graph builder
//
void capture_stuff::FreeCapFilters()
{
	SAFE_RELEASE(pGraphBuilder_);

	if (pSampleCaptureGraphBuilder_) {
		delete pSampleCaptureGraphBuilder_;
		pSampleCaptureGraphBuilder_ = NULL;
	}

	SAFE_RELEASE(pBaseFilterVideoCap_);
	SAFE_RELEASE(pBaseFilterAudioCap_);
	SAFE_RELEASE(pAMStreamConfigForAudio_);
	SAFE_RELEASE(pAMStreamConfigForVideo_);
	SAFE_RELEASE(pAMVideoCompression_);
	SAFE_RELEASE(pAMVfwCaptureDialogs_);

	if (pCrossbar_) {
		delete pCrossbar_;
		pCrossbar_ = NULL;
	}
}


// stop the preview graph
//
BOOL capture_stuff::StopPreview()
{
	// way ahead of you
	if (!fPreviewing_) {
		return FALSE;
	}

	// stop the graph
	IMediaControl *pMC = NULL;
	HRESULT hr = pGraphBuilder_->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (SUCCEEDED(hr)) {
		hr = pMC->Stop();
		pMC->Release();
	}
	if (FAILED(hr)) {
		ErrMsg(TEXT("Error %x: Cannot stop preview graph"), hr);
		return FALSE;
	}

	fPreviewing_ = FALSE;

	// get rid of menu garbage
	InvalidateRect(hWndVideo_, NULL, TRUE);

	return TRUE;
}


void capture_stuff::set_video_window(HWND hWndVideo, HWND hWndStatus, RECT rcVideo)
{
	hWndVideo_ = hWndVideo;
	hWndStatus_ = hWndStatus;
	rcVideo_ = rcVideo;
	statusInit(DuiLib::CPaintManagerUI::GetInstance(), nullptr);
}


// start the capture graph
//
BOOL capture_stuff::StartCapture()
{
	if (fPreviewing_)
		StopPreview();
	if (fPreviewGraphBuilt_)
		TearDownGraph();

	BuildCaptureGraph();

	BOOL fHasStreamControl;
	HRESULT hr;

	// way ahead of you
	if (fCapturing_)
		return TRUE;

	// or we'll get confused
	if (fPreviewing_)
		StopPreview();

	// or we'll crash
	if (!fCaptureGraphBuilt)
		return FALSE;

	// This amount will be subtracted from the number of dropped and not
	// dropped frames reported by the filter.  Since we might be having the
	// filter running while the pin is turned off, we don't want any of the
	// frame statistics from the time the pin is off interfering with the
	// statistics we gather while the pin is on
	lDroppedBase_ = 0;
	lNotBase_ = 0;

	REFERENCE_TIME start = MAXLONGLONG, stop = MAXLONGLONG;

	// don't capture quite yet...
	hr = pSampleCaptureGraphBuilder_->ControlStream(&PIN_CATEGORY_CAPTURE, NULL, NULL, &start, NULL, 0, 0);

	// Do we have the ability to control capture and preview separately?
	fHasStreamControl = SUCCEEDED(hr);

	// prepare to run the graph
	IMediaControl *pMC = NULL;
	hr = pGraphBuilder_->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (FAILED(hr)) {
		ErrMsg(TEXT("Error %x: Cannot get IMediaControl"), hr);
		return FALSE;
	}

	// If we were able to keep capture off, then we can
	// run the graph now for frame accurate start later yet still showing a
	// preview.   Otherwise, we can't run the graph yet without capture
	// starting too, so we'll pause it so the latency between when they
	// press a key and when capture begins is still small (but they won't have
	// a preview while they wait to press a key)

	if (fHasStreamControl)
		hr = pMC->Run();
	else
		hr = pMC->Pause();
	if (FAILED(hr)) {
		// stop parts that started
		pMC->Stop();
		pMC->Release();
		ErrMsg(TEXT("Error %x: Cannot start graph"), hr);
		return FALSE;
	}

	// Start capture NOW!
	if (fHasStreamControl) {
		// we may not have this yet
		if (!pAMDroppedFrames_) {
			hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pBaseFilterVideoCap_,
																 IID_IAMDroppedFrames, (void **)&pAMDroppedFrames_);
			if (hr != NOERROR)
				hr = pSampleCaptureGraphBuilder_->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pBaseFilterVideoCap_,
																	 IID_IAMDroppedFrames, (void **)&pAMDroppedFrames_);
		}

		// turn the capture pin on now!
		hr = pSampleCaptureGraphBuilder_->ControlStream(&PIN_CATEGORY_CAPTURE, NULL, NULL, NULL, &stop, 0, 0);
		// make note of the current dropped frame counts

		if (pAMDroppedFrames_) {
			pAMDroppedFrames_->GetNumDropped(&lDroppedBase_);
			pAMDroppedFrames_->GetNumNotDropped(&lNotBase_);
		}
	} else {
		hr = pMC->Run();
		if (FAILED(hr)) {
			// stop parts that started
			pMC->Stop();
			pMC->Release();
			ErrMsg(TEXT("Error %x: Cannot run graph"), hr);
			return FALSE;
		}
	}

	pMC->Release();

	// when did we start capture?
	lCapStartTime_ = timeGetTime();

	// update status bar 30 times per second - #captured, #dropped
	auto nIDEvent = SetTimer(hWndVideo_, 0, 33, OnVideoWindowTimer);
	self_map_for_timer_[nIDEvent] = this;

	fCapturing_ = TRUE;
	return TRUE;
}


// stop the capture graph
//
BOOL capture_stuff::StopCapture()
{
	AUTO_LOG_FUNCTION;
	// way ahead of you
	if (!fCapturing_) {
		return FALSE;
	}

	// stop the graph
	IMediaControl *pMC = NULL;
	HRESULT hr = pGraphBuilder_->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (SUCCEEDED(hr)) {
		hr = pMC->Stop();
		pMC->Release();
	}
	if (FAILED(hr)) {
		ErrMsg(TEXT("Error %x: Cannot stop graph"), hr);
		return FALSE;
	}

	// when the graph was stopped
	lCapStopTime_ = timeGetTime();

	// no more status bar updates
	KillTimer(hWndVideo_, 1);

	// one last time for the final count and all the stats
	UpdateStatus(TRUE);

	fCapturing_ = FALSE;

	// get rid of menu garbage
	InvalidateRect(hWndVideo_, NULL, TRUE);

	// compress avi
	std::wstring file = GetCaptureFile();
	auto name = get_file_name(file);
	auto out = name + L"_compress.avi";
	auto exe_path = get_exe_path();
	std::wstring cmd = L"\"";
	std::wstringstream ss;
	ss << cmd;
	ss << exe_path + L"\\ffmpeg.exe\" -i \"" + file + L"\"  " << /*w_ << "x" << h_ <<*/ " -r 10 -qscale 6 -vol 500 \"" << out + L"\"";
	//auto param = L" -i " + file + L" - vcodec msmpeg4v2 " + out;
	//std::system(utf8::w2a(cmd).c_str());
	//exec(cmd, param);]
	cmd = ss.str();
	//JLOG(cmd.c_str());

	static int step = 0;
	step = 1;

	auto cb = [this]() {
		std::wstring str = (L"正在压缩视频，请稍候");
		for (int i = 0; i < step; i++) {
			str += L"。";
		}
		if (++step > 6) {
			step = 1;
		}
		statusUpdateStatus(this->hWndStatus_, str.c_str());
	};

	execute((cmd), cb);
	std::remove(utf8::w2a(file).c_str());
	std::rename(utf8::w2a(out).c_str(), utf8::w2a(file).c_str());
	statusUpdateStatus(this->hWndStatus_, L"视频压缩完成");
	return TRUE;
}

BOOL capture_stuff::OpenFileDialog(HWND hWnd, LPTSTR pszName, DWORD cchName)
{
	OPENFILENAME ofn;
	LPTSTR p;
	TCHAR  szFileName[_MAX_PATH];
	TCHAR  szBuffer[_MAX_PATH];

	if (pszName == NULL) {
		return FALSE;
	}

	// start with capture file as current file name
	szFileName[0] = 0;
	(void)StringCchCopy(szFileName, NUMELMS(szFileName), wszCaptureFile_);

	// Get just the path info
	// Terminate the full path at the last backslash
	(void)StringCchCopy(szBuffer, NUMELMS(szBuffer), szFileName);
	for (p = szBuffer + lstrlen(szBuffer); p > szBuffer; p--) {
		if (*p == '\\') {
			*(p + 1) = '\0';
			break;
		}
	}
	szBuffer[_MAX_PATH - 1] = 0;  // Null-terminate

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = TEXT("Microsoft AVI\0*.avi\0\0");
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrTitle = TEXT("Set Capture File");
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = szBuffer;
	ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		// We have a capture file name
		std::wstring w = szFileName;
		auto name = get_file_name(w) + L".avi";
		StringCchCopy(pszName, cchName, name.c_str());
		return TRUE;
	} else {
		return FALSE;
	}
}


BOOL capture_stuff::SetCaptureFile(HWND hWnd)
{
	if (OpenFileDialog(hWnd, wszCaptureFile_, _MAX_PATH)) {
		auto file = GetCaptureFile();
		std::remove(utf8::w2a(file).c_str());
	} else {
		return FALSE;
	}

	// tell the file writer to use the new filename
	if (pFileSinkFilter_) {
		pFileSinkFilter_->SetFileName(wszCaptureFile_, NULL);
	}

	SetAppCaption();

	return TRUE;
}

// build the capture graph
//
BOOL capture_stuff::BuildCaptureGraph()
{
	HRESULT hr;
	BOOL f;
	AM_MEDIA_TYPE *pmt = 0;

	// we have one already
	if (fCaptureGraphBuilt)
		return TRUE;

	// No rebuilding while we're running
	if (fCapturing_ || fPreviewing_)
		return FALSE;

	// We don't have the necessary capture filters
	if (pBaseFilterVideoCap_ == NULL)
		return FALSE;
	if (pBaseFilterAudioCap_ == NULL && fCapAudio_)
		return FALSE;

	// no capture file name yet... we need one first
	//if (wszCaptureFile_[0] == 0) {
		f = SetCaptureFile(hWndVideo_);
		if (!f)
			return f;
	//}

	// we already have another graph built... tear down the old one
	if (fPreviewGraphBuilt_)
		TearDownGraph();

	//
	// We need a rendering section that will write the capture file out in AVI
	// file format
	//

	GUID guid;
	if (fMPEG2_) {
		guid = MEDIASUBTYPE_Mpeg2;
	} else {
		guid = MEDIASUBTYPE_Avi;
	}

	hr = pSampleCaptureGraphBuilder_->SetOutputFileName(&guid, wszCaptureFile_, &pBaseFilterRender_, &pFileSinkFilter_);
	if (hr != NOERROR) {
		ErrMsg(TEXT("Cannot set output file"));
		goto SetupCaptureFail;
	}

	// Now tell the AVIMUX to write out AVI files that old apps can read properly.
	// If we don't, most apps won't be able to tell where the keyframes are,
	// slowing down editing considerably
	// Doing this will cause one seek (over the area the index will go) when
	// you capture past 1 Gig, but that's no big deal.
	// NOTE: This is on by default, so it's not necessary to turn it on

	// Also, set the proper MASTER STREAM

	if (!fMPEG2_) {
		hr = pBaseFilterRender_->QueryInterface(IID_IConfigAviMux, (void **)&pConfigAviMux_);
		if (hr == NOERROR && pConfigAviMux_) {
			pConfigAviMux_->SetOutputCompatibilityIndex(TRUE);
			if (fCapAudio_) {
				hr = pConfigAviMux_->SetMasterStream(1);
				if (hr != NOERROR)
					ErrMsg(TEXT("SetMasterStream failed!"));
			}
		}
	}

	//
	// Render the video capture and preview pins - even if the capture filter only
	// has a capture pin (and no preview pin) this should work... because the
	// capture graph builder will use a smart tee filter to provide both capture
	// and preview.  We don't have to worry.  It will just work.
	//

	// NOTE that we try to render the interleaved pin before the video pin, because
	// if BOTH exist, it's a DV filter and the only way to get the audio is to use
	// the interleaved pin.  Using the Video pin on a DV filter is only useful if
	// you don't want the audio.

	if (!fMPEG2_) {
		hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pBaseFilterVideoCap_, NULL, pBaseFilterRender_);
		if (hr != NOERROR) {
			hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pBaseFilterVideoCap_, NULL, pBaseFilterRender_);
			if (hr != NOERROR) {
				ErrMsg(TEXT("Cannot render video capture stream"));
				goto SetupCaptureFail;
			}
		}

		if (fWantPreview_) {
			hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, pBaseFilterVideoCap_, NULL, NULL);
			if (hr == VFW_S_NOPREVIEWPIN) {
				// preview was faked up for us using the (only) capture pin
				fPreviewFaked_ = TRUE;
			} else if (hr != S_OK) {
				hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pBaseFilterVideoCap_, NULL, NULL);
				if (hr == VFW_S_NOPREVIEWPIN) {
					// preview was faked up for us using the (only) capture pin
					fPreviewFaked_ = TRUE;
				} else if (hr != S_OK) {
					ErrMsg(TEXT("Cannot render video preview stream"));
					goto SetupCaptureFail;
				}
			}
		}
	} else {
		SmartPtr< IBaseFilter > sink;
		if (&pFileSinkFilter_) {
			pFileSinkFilter_->QueryInterface(IID_IBaseFilter, reinterpret_cast<void **>(&sink));
		}

		hr = pSampleCaptureGraphBuilder_->RenderStream(NULL, &MEDIATYPE_Stream, pBaseFilterVideoCap_, NULL, sink);
	}

	//
	// Render the audio capture pin?
	//

	if (!fMPEG2_ && fCapAudio_) {
		hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
															pBaseFilterAudioCap_, NULL, pBaseFilterRender_);
		if (hr != NOERROR) {
			ErrMsg(TEXT("Cannot render audio capture stream"));
			goto SetupCaptureFail;
		}
	}

	//
	// Render the closed captioning pin? It could be a CC or a VBI category pin,
	// depending on the capture driver
	//

	if (!fMPEG2_  && fCapClosedCaptioning_) {
		hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_CC, NULL, pBaseFilterVideoCap_, NULL, pBaseFilterRender_);
		if (hr != NOERROR) {
			hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_VBI, NULL, pBaseFilterVideoCap_, NULL, pBaseFilterRender_);
			if (hr != NOERROR) {
				ErrMsg(TEXT("Cannot render closed captioning"));
				// so what? goto SetupCaptureFail;
			}
		}
		// To preview and capture VBI at the same time, we can call this twice
		if (fWantPreview_) {
			hr = pSampleCaptureGraphBuilder_->RenderStream(&PIN_CATEGORY_VBI, NULL, pBaseFilterVideoCap_, NULL, NULL);
		}
	}

	//
	// Get the preview window to be a child of our app's window
	//

	// This will find the IVideoWindow interface on the renderer.  It is
	// important to ask the filtergraph for this interface... do NOT use
	// ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
	// know we own the window so it can give us display changed messages, etc.

	if (!fMPEG2_ && fWantPreview_) {
		hr = pGraphBuilder_->QueryInterface(IID_IVideoWindow, (void **)&pVideoWindow_);
		if (hr != NOERROR) {
			ErrMsg(TEXT("This graph cannot preview"));
		} else {
			pVideoWindow_->put_Owner((OAHWND)hWndVideo_);    // We own the window now
			pVideoWindow_->put_WindowStyle(WS_CHILD);    // you are now a child

															  // give the preview window all our space but where the status bar is

			pVideoWindow_->SetWindowPosition(rcVideo_.left, rcVideo_.top, rcVideo_.right - rcVideo_.left, rcVideo_.bottom - rcVideo_.top); // be this big
			pVideoWindow_->put_Visible(OATRUE);
		}
	}

	// now tell it what frame rate to capture at.  Just find the format it
	// is capturing with, and leave everything alone but change the frame rate
	if (!fMPEG2_) {
		hr = fUseFrameRate_ ? E_FAIL : NOERROR;
		if (pAMStreamConfigForVideo_ && fUseFrameRate_) {
			hr = pAMStreamConfigForVideo_->GetFormat(&pmt);

			// DV capture does not use a VIDEOINFOHEADER
			if (hr == NOERROR) {
				if (pmt->formattype == FORMAT_VideoInfo) {
					VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
					pvi->AvgTimePerFrame = (LONGLONG)(10000000 / FrameRate_);
					hr = pAMStreamConfigForVideo_->SetFormat(pmt);
				}
				DeleteMediaType(pmt);
			}
		}
		if (hr != NOERROR)
			ErrMsg(TEXT("Cannot set frame rate for capture"));
	}

	// now ask the filtergraph to tell us when something is completed or aborted
	// (EC_COMPLETE, EC_USERABORT, EC_ERRORABORT).  This is how we will find out
	// if the disk gets full while capturing
	hr = pGraphBuilder_->QueryInterface(IID_IMediaEventEx, (void **)&pMediaEventEx_);
	if (hr == NOERROR) {
		pMediaEventEx_->SetNotifyWindow((OAHWND)hWndVideo_, WM_FGNOTIFY, 0);
	}

	// potential debug output - what the graph looks like
	// DumpGraph(gcap.pFg, 1);

	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
	hr = AddGraphToRot(gcap.pGraphBuilder_, &g_dwGraphRegister);
	if (FAILED(hr)) {
		ErrMsg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
		g_dwGraphRegister = 0;
	}
#endif

	// All done.
	fCaptureGraphBuilt = TRUE;
	return TRUE;

SetupCaptureFail:
	TearDownGraph();
	return FALSE;
}


void capture_stuff::OnFgNotify()
{
	// uh-oh, something went wrong while capturing - the filtergraph
	// will send us events like EC_COMPLETE, EC_USERABORT and the one
	// we care about, EC_ERRORABORT.
	if (pMediaEventEx_) {
		LONG event;
		LONG_PTR l1, l2;
		HRESULT hrAbort = S_OK;
		BOOL bAbort = FALSE;
		while (pMediaEventEx_->GetEvent(&event, &l1, &l2, 0) == S_OK) {
			pMediaEventEx_->FreeEventParams(event, l1, l2);
			if (event == EC_ERRORABORT) {
				StopCapture();
				bAbort = TRUE;
				hrAbort = static_cast<HRESULT>(l1);
				continue;
			} else if (event == EC_DEVICE_LOST) {
				// Check if we have lost a capture filter being used.
				// lParam2 of EC_DEVICE_LOST event == 1 indicates device added
				//                                 == 0 indicates device removed
				if (l2 == 0) {
					IBaseFilter *pBaseFilter;
					IUnknown *punk = (IUnknown *)l1;
					if (S_OK == punk->QueryInterface(IID_IBaseFilter, (void **)&pBaseFilter)) {
						if (AreComObjectsEqual(pBaseFilterVideoCap_, pBaseFilter)) {
							pBaseFilter->Release();
							bAbort = FALSE;
							StopCapture();
							TCHAR szError[100];
							HRESULT hr = StringCchCopy(szError, 100,
													   TEXT("Stopping Capture (Device Lost). Select New Capture Device\0"));
							ErrMsg(szError);
							break;
						}
						pBaseFilter->Release();
					}
				}
			}
		} // end while
		if (bAbort) {
			if (fWantPreview_) {
				BuildPreviewGraph();
				StartPreview();
			}
			TCHAR szError[100];
			HRESULT hr = StringCchPrintf(szError, 100, TEXT("ERROR during capture, error code=%08x\0"), hrAbort);
			ErrMsg(szError);
		}
	}
}


void capture_stuff::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	do {
		// We are interested in only device arrival & removal events
		if (DBT_DEVICEARRIVAL != wParam && DBT_DEVICEREMOVECOMPLETE != wParam)
			break;

		PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR)lParam;
		if (pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE) {
			break;
		}

		PDEV_BROADCAST_DEVICEINTERFACE pdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
		// Check for capture devices.
		if (pdbi->dbcc_classguid != AM_KSCATEGORY_CAPTURE) {
			break;
		}

		// Check for device arrival/removal.
		if (DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam) {
			fDeviceMenuPopulated_ = false;
		}
		break;

	} while (0);
}


void capture_stuff::SetAppCaption()
{
	TCHAR tach[_MAX_PATH + 80];

	//GetWindowText(hWndVideo_, tach, _MAX_PATH + 80);
	StringCchCopy(tach, NUMELMS(tach), title_.c_str());

	if (wszCaptureFile_[0] != 0) {
		HRESULT hr = StringCchCat(tach, _MAX_PATH + 80, TEXT(" - "));
		hr = StringCchCat(tach, _MAX_PATH + 80, wszCaptureFile_);
	}
	SetWindowText(hWndVideo_, tach);
}

}
