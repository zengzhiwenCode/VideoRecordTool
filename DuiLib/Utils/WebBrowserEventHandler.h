#pragma once
#include <ExDisp.h>
#include <ExDispid.h>
#include <mshtmhst.h>

namespace DuiLib
{
class CWebBrowserEventHandler
{
public:
	CWebBrowserEventHandler() {}
	~CWebBrowserEventHandler() {}

	virtual void BeforeNavigate2(IDispatch *pDisp,
								 VARIANT *&url,
								 VARIANT *&Flags,
								 VARIANT *&TargetFrameName,
								 VARIANT *&PostData,
								 VARIANT *&Headers,
								 VARIANT_BOOL *&Cancel) {
		UNREFERENCED_PARAMETER(pDisp);
		UNREFERENCED_PARAMETER(url);
		UNREFERENCED_PARAMETER(Flags);
		UNREFERENCED_PARAMETER(TargetFrameName);
		UNREFERENCED_PARAMETER(PostData);
		UNREFERENCED_PARAMETER(Headers);
		UNREFERENCED_PARAMETER(Cancel);
	}

	virtual void NavigateError(IDispatch *pDisp,
							   VARIANT * &url,
							   VARIANT *&TargetFrameName,
							   VARIANT *&StatusCode,
							   VARIANT_BOOL *&Cancel) {
		UNREFERENCED_PARAMETER(pDisp);
		UNREFERENCED_PARAMETER(url);
		UNREFERENCED_PARAMETER(TargetFrameName);
		UNREFERENCED_PARAMETER(StatusCode);
		UNREFERENCED_PARAMETER(Cancel);
	}

	virtual void NavigateComplete2(IDispatch *pDisp, VARIANT *&url) {
		UNREFERENCED_PARAMETER(pDisp);
		UNREFERENCED_PARAMETER(url);
	}

	virtual void ProgressChange(LONG nProgress, LONG nProgressMax) {
		UNREFERENCED_PARAMETER(nProgress);
		UNREFERENCED_PARAMETER(nProgressMax);
	}

	virtual void NewWindow3(IDispatch **pDisp, VARIANT_BOOL *&Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl) {
		UNREFERENCED_PARAMETER(pDisp);
		UNREFERENCED_PARAMETER(Cancel);
		UNREFERENCED_PARAMETER(dwFlags);
		UNREFERENCED_PARAMETER(bstrUrlContext);
		UNREFERENCED_PARAMETER(bstrUrl);
	}

	virtual void CommandStateChange(long Command, VARIANT_BOOL Enable) {
		UNREFERENCED_PARAMETER(Command);
		UNREFERENCED_PARAMETER(Enable);
	};

	// interface IDocHostUIHandler
	virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(
		/* [in] */ DWORD dwID,
		/* [in] */ POINT __RPC_FAR *ppt,
		/* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
		/* [in] */ IDispatch __RPC_FAR *pdispReserved)
	{
		UNREFERENCED_PARAMETER(dwID);
		UNREFERENCED_PARAMETER(ppt);
		UNREFERENCED_PARAMETER(pcmdtReserved);
		UNREFERENCED_PARAMETER(pdispReserved);
		//return E_NOTIMPL;
		//返回 E_NOTIMPL 正常弹出系统右键菜单
		return S_OK;
		//返回S_OK 则可屏蔽系统右键菜单
	}

	virtual HRESULT STDMETHODCALLTYPE GetHostInfo(
		/* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo)
	{
		UNREFERENCED_PARAMETER(pInfo);
		// 		if (pInfo != NULL)
		// 		{
		// 			pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DBORDER;
		// 		}
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ShowUI(
		/* [in] */ DWORD dwID,
		/* [in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
		/* [in] */ IOleCommandTarget __RPC_FAR *pCommandTarget,
		/* [in] */ IOleInPlaceFrame __RPC_FAR *pFrame,
		/* [in] */ IOleInPlaceUIWindow __RPC_FAR *pDoc)
	{
		UNREFERENCED_PARAMETER(dwID);
		UNREFERENCED_PARAMETER(pActiveObject);
		UNREFERENCED_PARAMETER(pCommandTarget);
		UNREFERENCED_PARAMETER(pFrame);
		UNREFERENCED_PARAMETER(pDoc);
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE HideUI(void)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE UpdateUI(void)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE EnableModeless(
		/* [in] */ BOOL fEnable)
	{
		UNREFERENCED_PARAMETER(fEnable);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate(
		/* [in] */ BOOL fActivate)
	{
		UNREFERENCED_PARAMETER(fActivate);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(
		/* [in] */ BOOL fActivate)
	{
		UNREFERENCED_PARAMETER(fActivate);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ResizeBorder(
		/* [in] */ LPCRECT prcBorder,
		/* [in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
		/* [in] */ BOOL fRameWindow)
	{
		UNREFERENCED_PARAMETER(prcBorder);
		UNREFERENCED_PARAMETER(pUIWindow);
		UNREFERENCED_PARAMETER(fRameWindow);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(
		/* [in] */ LPMSG lpMsg,
		/* [in] */ const GUID __RPC_FAR *pguidCmdGroup,
		/* [in] */ DWORD nCmdID)
	{
		UNREFERENCED_PARAMETER(lpMsg);
		UNREFERENCED_PARAMETER(pguidCmdGroup);
		UNREFERENCED_PARAMETER(nCmdID);
		return S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath(
		/* [out] */ LPOLESTR __RPC_FAR *pchKey,
		/* [in] */ DWORD dw)
	{
		UNREFERENCED_PARAMETER(pchKey);
		UNREFERENCED_PARAMETER(dw);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDropTarget(
		/* [in] */ IDropTarget __RPC_FAR *pDropTarget,
		/* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget)
	{
		UNREFERENCED_PARAMETER(pDropTarget);
		UNREFERENCED_PARAMETER(ppDropTarget);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetExternal(
		/* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
	{
		UNREFERENCED_PARAMETER(ppDispatch);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE TranslateUrl(
		/* [in] */ DWORD dwTranslate,
		/* [in] */ OLECHAR __RPC_FAR *pchURLIn,
		/* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
	{
		UNREFERENCED_PARAMETER(dwTranslate);
		UNREFERENCED_PARAMETER(pchURLIn);
		UNREFERENCED_PARAMETER(ppchURLOut);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE FilterDataObject(
		/* [in] */ IDataObject __RPC_FAR *pDO,
		/* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet)
	{
		UNREFERENCED_PARAMETER(pDO);
		UNREFERENCED_PARAMETER(ppDORet);
		return S_OK;
	}

	// 	virtual HRESULT STDMETHODCALLTYPE GetOverrideKeyPath( 
	// 		/* [annotation][out] */ 
	// 		__deref_out  LPOLESTR *pchKey,
	// 		/* [in] */ DWORD dw)
	// 	{
	// 		return E_NOTIMPL;
	// 	}

	// IDownloadManager
	virtual HRESULT STDMETHODCALLTYPE Download(
		/* [in] */ IMoniker *pmk,
		/* [in] */ IBindCtx *pbc,
		/* [in] */ DWORD dwBindVerb,
		/* [in] */ LONG grfBINDF,
		/* [in] */ BINDINFO *pBindInfo,
		/* [in] */ LPCOLESTR pszHeaders,
		/* [in] */ LPCOLESTR pszRedir,
		/* [in] */ UINT uiCP)
	{
		UNREFERENCED_PARAMETER(pmk);
		UNREFERENCED_PARAMETER(pbc);
		UNREFERENCED_PARAMETER(dwBindVerb);
		UNREFERENCED_PARAMETER(grfBINDF);
		UNREFERENCED_PARAMETER(pBindInfo);
		UNREFERENCED_PARAMETER(pszHeaders);
		UNREFERENCED_PARAMETER(pszRedir);
		UNREFERENCED_PARAMETER(uiCP);
		return S_OK;
	}
};
}
