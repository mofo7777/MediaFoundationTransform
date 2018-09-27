//----------------------------------------------------------------------------------------------
// MFTSimpleDirectxAware_D3DManager.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMFTSimpleDirectxAware::OnSetD3DManager(const ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnSetD3DManager"));

	HRESULT hr = S_OK;

	if(ulParam){

		IF_FAILED_RETURN(hr = (m_pDeviceManager != NULL ? E_UNEXPECTED : S_OK));

		IDirect3DDeviceManager9* pDeviceManager = reinterpret_cast<IDirect3DDeviceManager9*>(ulParam);

		IF_FAILED_RETURN(hr = pDeviceManager->OpenDeviceHandle(&m_hD3d9Device));

		m_pDeviceManager = pDeviceManager;
		m_pDeviceManager->AddRef();
	}
	else{

		OnReleaseD3DManager();
	}

	return hr;
}

void CMFTSimpleDirectxAware::OnReleaseD3DManager(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnReleaseD3DManager"));

	if(m_pVideoSampleAllocator){

		LOG_HRESULT(m_pVideoSampleAllocator->UninitializeSampleAllocator());
		SAFE_RELEASE(m_pVideoSampleAllocator);
	}

	if(m_pDeviceManager){

		LOG_HRESULT(m_pDeviceManager->CloseDeviceHandle(m_hD3d9Device));
		m_hD3d9Device = NULL;

		SAFE_RELEASE(m_pDeviceManager);
	}
}

HRESULT CMFTSimpleDirectxAware::OnAllocateSurface(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnAllocateSurface"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pDeviceManager ? S_OK : E_UNEXPECTED));

	if(m_pVideoSampleAllocator)
		return hr;

	IMFVideoSampleAllocator* pVideoSampleAllocator = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateVideoSampleAllocator(IID_IMFVideoSampleAllocator, reinterpret_cast<void**>(&pVideoSampleAllocator)));
		IF_FAILED_THROW(hr = pVideoSampleAllocator->SetDirectXManager(m_pDeviceManager));
		IF_FAILED_THROW(hr = pVideoSampleAllocator->InitializeSampleAllocator(SAMPLE_ALLOCATED_COUNT, m_pOutputMediaType));

		m_pVideoSampleAllocator = pVideoSampleAllocator;
		m_pVideoSampleAllocator->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pVideoSampleAllocator);

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnReAllocateSurface(){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnReAllocateSurface"));

	HRESULT hr = S_OK;

	if(m_pVideoSampleAllocator){

		IF_FAILED_RETURN(hr = m_pVideoSampleAllocator->UninitializeSampleAllocator());
		SAFE_RELEASE(m_pVideoSampleAllocator);
	}

	IF_FAILED_RETURN(hr = OnAllocateSurface());

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnCopySurfaceToSurface(IDirect3DSurface9* pSrcSurface, IDirect3DSurface9* pDstSurface){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnCopySurfaceToSurface"));

	HRESULT hr = S_OK;
	IDirect3DDevice9* pDevice = NULL;

	D3DLOCKED_RECT SrcLockRect;
	D3DLOCKED_RECT DstLockRect;
	D3DSURFACE_DESC SrcDesc;
	D3DSURFACE_DESC DstDesc;
	BOOL bDeviceLock = FALSE;

	try{

		IF_FAILED_THROW(hr = pSrcSurface->GetDesc(&SrcDesc));
		IF_FAILED_THROW(hr = pDstSurface->GetDesc(&DstDesc));
		IF_FAILED_THROW(hr = (SrcDesc.Format == DstDesc.Format && SrcDesc.Width == DstDesc.Width && SrcDesc.Height == DstDesc.Height ? S_OK : E_UNEXPECTED));

		IF_FAILED_THROW(hr = m_pDeviceManager->TestDevice(m_hD3d9Device));
		IF_FAILED_THROW(hr = m_pDeviceManager->LockDevice(m_hD3d9Device, &pDevice, TRUE));
		bDeviceLock = TRUE;

		//IF_FAILED_THROW(hr = pDevice->UpdateSurface(pSrcSurface, NULL, pDstSurface, NULL));

		IF_FAILED_THROW(hr = pSrcSurface->LockRect(&SrcLockRect, NULL, 0));
		IF_FAILED_THROW(hr = pDstSurface->LockRect(&DstLockRect, NULL, 0));

		memcpy(DstLockRect.pBits, SrcLockRect.pBits, DstLockRect.Pitch * (SrcDesc.Height + (SrcDesc.Height / 2)));

		IF_FAILED_THROW(hr = pSrcSurface->UnlockRect());
		IF_FAILED_THROW(hr = pDstSurface->UnlockRect());

		IF_FAILED_THROW(hr = m_pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));
		bDeviceLock = FALSE;
	}
	catch(HRESULT){}

	if(bDeviceLock){
		LOG_HRESULT(m_pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));
	}

	SAFE_RELEASE(pDevice);

	return hr;
}

HRESULT CMFTSimpleDirectxAware::OnCopyBufferToSurface(IMFMediaBuffer* pBuffer, IDirect3DSurface9* pSurface9){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::OnCopyBufferToSurface"));

	HRESULT hr = S_OK;

	BYTE* pData = NULL;
	DWORD dwLength;
	BOOL bIsSurfaceLock = FALSE;

	BYTE* pDataIn = NULL;
	BYTE* pDataOut = NULL;

	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;
	UINT32 uiYUVHeight = 0;

	D3DSURFACE_DESC Desc;
	D3DLOCKED_RECT LockRect;

	try{

		// todo check size outside OnCopySampleToSurface (both E_UNEXPECTED)
		IF_FAILED_THROW(hr = pSurface9->GetDesc(&Desc));
		IF_FAILED_THROW(hr = MFGetAttributeSize(m_pOutputMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
		IF_FAILED_THROW(hr = (Desc.Width != uiWidth && Desc.Height != uiHeight ? E_UNEXPECTED : S_OK));

		uiYUVHeight = uiHeight + (uiHeight / 2);

		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, &dwLength));
		IF_FAILED_THROW(hr = (dwLength != (uiWidth * uiYUVHeight) ? E_UNEXPECTED : S_OK));
		pDataIn = pData;

		IF_FAILED_THROW(hr = pSurface9->LockRect(&LockRect, NULL, 0));
		bIsSurfaceLock = TRUE;
		pDataOut = (BYTE*)LockRect.pBits;

		if(LockRect.Pitch == (INT)uiWidth){

			memcpy(pDataOut, pDataIn, uiWidth * uiYUVHeight);
		}
		else if(LockRect.Pitch > (INT)uiWidth){

			for(UINT32 ui = 0; ui < uiYUVHeight; ui++){

				memcpy(pDataOut, pDataIn, uiWidth);
				pDataOut += LockRect.Pitch;
				pDataIn += uiWidth;
			}
		}
		else{

			IF_FAILED_THROW(hr = E_NOTIMPL);
		}

		IF_FAILED_THROW(hr = pSurface9->UnlockRect());
		bIsSurfaceLock = FALSE;

		IF_FAILED_THROW(hr = pBuffer->Unlock());
		pData = NULL;
	}
	catch(HRESULT){}

	if(bIsSurfaceLock){
		LOG_HRESULT(pSurface9->UnlockRect());
	}

	if(pBuffer){

		if(pData)
			LOG_HRESULT(pBuffer->Unlock());
	}

	return hr;
}