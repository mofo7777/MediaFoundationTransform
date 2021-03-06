//----------------------------------------------------------------------------------------------
// MFTSimpleDirectxAware.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

CMFTSimpleDirectxAware::CMFTSimpleDirectxAware() :
	m_nRefCount(1),
	m_pInputMediaType(NULL),
	m_pOutputMediaType(NULL),
	m_pOutputSample(NULL),
	m_bDraining(FALSE),
	m_bFormatHasChange(FALSE),
	m_pVideoSampleAllocator(NULL),
	m_pDeviceManager(NULL),
	m_hD3d9Device(NULL)
{
	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::CTOR"));
}

CMFTSimpleDirectxAware::~CMFTSimpleDirectxAware(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::DTOR"));

	OnRelease();
}

HRESULT CMFTSimpleDirectxAware::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMFTSimpleDirectxAware* pMFT = new (std::nothrow)CMFTSimpleDirectxAware();

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CMFTSimpleDirectxAware::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CMFTSimpleDirectxAware::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFTSimpleDirectxAware::Release(){

	ULONG ulCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CMFTSimpleDirectxAware::Release m_nRefCount = %d", ulCount));

	if(ulCount == 0) {
		delete this;
	}

	return ulCount;
}

HRESULT CMFTSimpleDirectxAware::QueryInterface(REFIID riid, void** ppv){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {QITABENT(CMFTSimpleDirectxAware, IMFTransform), {0}};

	return QISearch(this, qit, riid, ppv);
}

void CMFTSimpleDirectxAware::OnRelease(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnRelease"));

	AutoLock lock(m_CriticSection);

	OnReleaseD3DManager();
	SAFE_RELEASE(m_pInputMediaType);
	SAFE_RELEASE(m_pOutputMediaType);
	SAFE_RELEASE(m_pOutputSample);

	m_bDraining = FALSE;
	m_bFormatHasChange = FALSE;
}

HRESULT CMFTSimpleDirectxAware::OnCheckMediaType(IMFMediaType* pMediaType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnCheckMediaType"));

	HRESULT hr;

	GUID majortype = {0};
	GUID subtype = {0};

	IF_FAILED_RETURN(hr = pMediaType->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subtype));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Video ? MF_E_INVALIDMEDIATYPE : S_OK));
	IF_FAILED_RETURN(hr = (subtype != MFVideoFormat_NV12 ? MF_E_INVALIDMEDIATYPE : S_OK));

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnGetAvalaibleType(IMFMediaType** ppMediaType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnGetAvalaibleType"));

	HRESULT hr = S_OK;
	IMFMediaType* pMediaType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pMediaType));
		IF_FAILED_THROW(hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));

		*ppMediaType = pMediaType;
		(*ppMediaType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);

	return hr;
}

HRESULT CMFTSimpleDirectxAware::CheckFormatChange(IMFMediaType* pType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::CheckFormatChange"));

	HRESULT hr = S_OK;

	if(pType == NULL)
		return hr;

	if(m_pInputMediaType){

		UINT32 uiWidth = 0;
		UINT32 uiHeight = 0;
		UINT32 uiNewWidth = 0;
		UINT32 uiNewHeight = 0;

		IF_FAILED_RETURN(hr = MFGetAttributeSize(m_pInputMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
		IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uiNewWidth, &uiNewHeight));

		if(uiWidth != uiNewWidth || uiHeight != uiNewHeight)
			m_bFormatHasChange = TRUE;
	}

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnCloneMediaType(IMFMediaType* pSrcType, IMFMediaType** ppDestType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnCloneMediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pMediaType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pMediaType));
		IF_FAILED_THROW(hr = pSrcType->CopyAllItems(pMediaType));

		*ppDestType = pMediaType;
		(*ppDestType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnCloneSample(IMFSample* pSrcSample, IMFSample* pDstSample){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnCloneSample"));

	LONGLONG llSampleTime = 0;
	LONGLONG llSampleDuration = 0;

	HRESULT hr;
	IF_FAILED_RETURN(hr = pSrcSample->GetSampleTime(&llSampleTime));
	IF_FAILED_RETURN(hr = pSrcSample->GetSampleDuration(&llSampleDuration));

	IF_FAILED_RETURN(hr = pDstSample->SetSampleTime(llSampleTime));
	IF_FAILED_RETURN(hr = pDstSample->SetSampleDuration(llSampleDuration));

	IF_FAILED_RETURN(hr = pSrcSample->CopyAllItems(pDstSample));

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnStartStream(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnStartStream"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = OnAllocateSurface());

	m_bDraining = FALSE;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnFlush(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnFlush"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	SAFE_RELEASE(m_pOutputSample);
	m_bDraining = FALSE;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnDrain(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnDrain"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	m_bDraining = TRUE;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnEndOfStream(const ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnEndOfStream"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = (ulParam != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	SAFE_RELEASE(m_pOutputSample);
	m_bDraining = FALSE;

	return hr;
}