//----------------------------------------------------------------------------------------------
// MFTVideoShaderEffect.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

CMFTVideoShaderEffect::CMFTVideoShaderEffect() :
	m_nRefCount(1),
	m_pInputMediaType(NULL),
	m_pOutputMediaType(NULL),
	m_pInputSample(NULL),
	m_bDraining(FALSE),
	m_bFormatHasChange(FALSE),
	m_pDeviceManager(NULL),
	m_hD3d9Device(NULL),
	m_pNV12VideoTexture(NULL),
	m_pNV12VideoSurface(NULL),
	m_pNV12VideoOffScreenSurface(NULL),
	m_pVideoEffect(NULL)
{
	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::CTOR"));
}

CMFTVideoShaderEffect::~CMFTVideoShaderEffect(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::DTOR"));

	OnRelease();
}

HRESULT CMFTVideoShaderEffect::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMFTVideoShaderEffect* pMFT = new (std::nothrow)CMFTVideoShaderEffect();

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CMFTVideoShaderEffect::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CMFTVideoShaderEffect::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFTVideoShaderEffect::Release(){

	ULONG ulCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"CMFTVideoShaderEffect::Release m_nRefCount = %d", ulCount));

	if(ulCount == 0) {
		delete this;
	}

	return ulCount;
}

HRESULT CMFTVideoShaderEffect::QueryInterface(REFIID riid, void** ppv){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = { QITABENT(CMFTVideoShaderEffect, IMFTransform), {0} };

	return QISearch(this, qit, riid, ppv);
}

void CMFTVideoShaderEffect::OnRelease(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnRelease"));

	AutoLock lock(m_CriticSection);

	OnReleaseD3DManager();
	SAFE_RELEASE(m_pInputMediaType);
	SAFE_RELEASE(m_pOutputMediaType);
	SAFE_RELEASE(m_pInputSample);

	m_bDraining = FALSE;
	m_bFormatHasChange = FALSE;
}

HRESULT CMFTVideoShaderEffect::OnCheckMediaType(IMFMediaType* pMediaType){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnCheckMediaType"));

	HRESULT hr;

	GUID majortype = {0};
	GUID subtype = {0};
	UINT32 uiValue = 0;
	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;

	IF_FAILED_RETURN(hr = pMediaType->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subtype));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Video ? MF_E_INVALIDMEDIATYPE : S_OK));
	IF_FAILED_RETURN(hr = (subtype != MFVideoFormat_NV12 ? MF_E_INVALIDMEDIATYPE : S_OK));

	IF_FAILED_RETURN(hr = pMediaType->GetUINT32(MF_MT_INTERLACE_MODE, &uiValue));
	IF_FAILED_RETURN(hr = (uiValue != MFVideoInterlace_Progressive && uiValue != MFVideoInterlace_MixedInterlaceOrProgressive ? MF_E_INVALIDTYPE : S_OK));

	IF_FAILED_RETURN(hr = pMediaType->GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, &uiValue));
	IF_FAILED_RETURN(hr = (uiValue != TRUE ? MF_E_INVALIDTYPE : S_OK));

	IF_FAILED_RETURN(hr = pMediaType->GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, &uiValue));
	IF_FAILED_RETURN(hr = (uiValue != TRUE ? MF_E_INVALIDTYPE : S_OK));

	IF_FAILED_RETURN(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
	IF_FAILED_RETURN(hr = ((uiWidth == 0 || uiHeight == 0) ? MF_E_INVALIDTYPE : S_OK));

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnGetAvalaibleType(IMFMediaType** ppMediaType){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnGetAvalaibleType"));

	HRESULT hr = S_OK;
	IMFMediaType* pMediaType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pMediaType));
		IF_FAILED_THROW(hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
		IF_FAILED_THROW(hr = pMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pMediaType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));

		*ppMediaType = pMediaType;
		(*ppMediaType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);

	return hr;
}

HRESULT CMFTVideoShaderEffect::CheckFormatChange(IMFMediaType* pType){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::CheckFormatChange"));

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

HRESULT CMFTVideoShaderEffect::OnCloneMediaType(IMFMediaType* pSrcType, IMFMediaType** ppDestType){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnCloneMediaType"));

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

HRESULT CMFTVideoShaderEffect::OnCloneSample(IMFSample* pSrcSample, IMFSample* pDstSample){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnCloneSample"));

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

HRESULT CMFTVideoShaderEffect::OnStartStream(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnStartStream"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = OnAllocateSurface());

	m_bDraining = FALSE;

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnFlush(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnFlush"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	SAFE_RELEASE(m_pInputSample);
	m_bDraining = FALSE;

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnDrain(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnDrain"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	m_bDraining = TRUE;

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnEndOfStream(const ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnEndOfStream"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = (ulParam != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	SAFE_RELEASE(m_pInputSample);
	m_bDraining = FALSE;

	return hr;
}