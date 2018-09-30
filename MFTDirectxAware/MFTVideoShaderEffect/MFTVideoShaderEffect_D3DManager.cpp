//----------------------------------------------------------------------------------------------
// MFTVideoShaderEffect_D3DManager.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMFTVideoShaderEffect::OnSetD3DManager(const ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnSetD3DManager"));

	HRESULT hr = S_OK;

	if(ulParam){

		IF_FAILED_RETURN(hr = (m_pDeviceManager != NULL ? E_UNEXPECTED : S_OK));

		IDirect3DDevice9* pDevice = NULL;
		ID3DXEffect* pVideoEffect;

		IDirect3DDeviceManager9* pDeviceManager = reinterpret_cast<IDirect3DDeviceManager9*>(ulParam);

		try{

			IF_FAILED_THROW(hr = pDeviceManager->OpenDeviceHandle(&m_hD3d9Device));

			IF_FAILED_THROW(hr = pDeviceManager->TestDevice(m_hD3d9Device));
			IF_FAILED_THROW(hr = pDeviceManager->LockDevice(m_hD3d9Device, &pDevice, TRUE));

			DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER | D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE;
			IF_FAILED_THROW(hr = D3DXCreateEffectFromResource(pDevice, g_hModule, MAKEINTRESOURCE(IDR_EFFECT), NULL, NULL, dwShaderFlags, NULL, &pVideoEffect, NULL));

			IF_FAILED_THROW(hr = m_cVideoFrame.OnRestore(pDevice));

			IF_FAILED_THROW(hr = pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));

			m_pVideoEffect = pVideoEffect;
			m_pVideoEffect->AddRef();

			m_pDeviceManager = pDeviceManager;
			m_pDeviceManager->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoEffect);
		SAFE_RELEASE(pDevice);
	}
	else{

		OnReleaseD3DManager();
	}

	return hr;
}

void CMFTVideoShaderEffect::OnReleaseD3DManager(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnReleaseD3DManager"));

	m_cVideoFrame.OnDelete();
	m_cText2D.OnDelete();

	SAFE_RELEASE(m_pNV12VideoTexture);
	SAFE_RELEASE(m_pNV12VideoSurface);
	SAFE_RELEASE(m_pNV12VideoOffScreenSurface);
	SAFE_RELEASE(m_pVideoEffect);

	if(m_pDeviceManager){

		LOG_HRESULT(m_pDeviceManager->CloseDeviceHandle(m_hD3d9Device));
		m_hD3d9Device = NULL;

		SAFE_RELEASE(m_pDeviceManager);
	}
}

HRESULT CMFTVideoShaderEffect::OnAllocateSurface(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnAllocateSurface"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pDeviceManager ? S_OK : E_UNEXPECTED));

	// Already allocated, for exemple when MFT_MESSAGE_NOTIFY_BEGIN_STREAMING and MFT_MESSAGE_NOTIFY_START_OF_STREAM are both called
	if(m_pNV12VideoOffScreenSurface)
		return hr;

	IDirect3DDevice9* pDevice = NULL;
	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;

	try{

		// m_pOutputMediaType is not NULL (see OnStartStream)
		IF_FAILED_RETURN(hr = MFGetAttributeSize(m_pOutputMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));

		IF_FAILED_THROW(hr = m_pDeviceManager->TestDevice(m_hD3d9Device));
		IF_FAILED_THROW(hr = m_pDeviceManager->LockDevice(m_hD3d9Device, &pDevice, TRUE));

		// Texture for the shader, we will copy the input video surface on it
		IF_FAILED_THROW(hr = pDevice->CreateTexture(uiWidth, uiHeight + (uiHeight / 2), 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &m_pNV12VideoTexture, NULL));
		// Surface for the final video, after shader. We can't use this surface to create the video output sample, because EVR does not handle D3DFMT_L8 as NV12
		IF_FAILED_THROW(hr = pDevice->CreateRenderTarget(uiWidth, uiHeight + (uiHeight / 2), D3DFMT_L8, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pNV12VideoSurface, NULL));
		// Surface for the video output sample,  we will copy the final video render target
		IF_FAILED_THROW(hr = pDevice->CreateOffscreenPlainSurface(uiWidth, uiHeight, (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2'), D3DPOOL_DEFAULT, &m_pNV12VideoOffScreenSurface, NULL));

		IF_FAILED_THROW(hr = m_cText2D.OnRestore(pDevice, uiWidth, uiHeight, 20.0f));

		// We need special texel size so that tex2D in shader works correctly with D3DFMT_L8
		D3DXVECTOR2 vTexelSize((1.0f / float(uiWidth)) / 2.0f, (1.0f / float(uiHeight)) / 2.0f);

		IF_FAILED_THROW(hr = m_pVideoEffect->SetValue("g_vTexelSize", &vTexelSize, sizeof(D3DXVECTOR2)));

		IF_FAILED_THROW(hr = m_pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pDevice);

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnReAllocateSurface(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnReAllocateSurface"));

	m_cText2D.OnDelete();

	SAFE_RELEASE(m_pNV12VideoTexture);
	SAFE_RELEASE(m_pNV12VideoSurface);
	SAFE_RELEASE(m_pNV12VideoOffScreenSurface);

	HRESULT hr;
	IF_FAILED_RETURN(hr = OnAllocateSurface());

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnRender(){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnRender"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pDeviceManager ? S_OK : E_UNEXPECTED));

	IDirect3DDevice9* pDevice = NULL;

	try{

		IF_FAILED_THROW(hr = m_pDeviceManager->TestDevice(m_hD3d9Device));
		IF_FAILED_THROW(hr = m_pDeviceManager->LockDevice(m_hD3d9Device, &pDevice, TRUE));

		IF_FAILED_THROW(hr = m_pVideoEffect->SetTexture("g_pNV12Texture", m_pNV12VideoTexture));

		IF_FAILED_THROW(hr = pDevice->SetRenderTarget(0, m_pNV12VideoSurface));
		IF_FAILED_THROW(hr = pDevice->Clear(0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L));

		IF_FAILED_THROW(hr = pDevice->BeginScene());

		UINT uiPasses;

		IF_FAILED_THROW(hr = m_pVideoEffect->Begin(&uiPasses, 0));

		IF_FAILED_THROW(hr = m_pVideoEffect->BeginPass(0));
		IF_FAILED_THROW(hr = m_cVideoFrame.OnRender(pDevice));
		IF_FAILED_THROW(hr = m_pVideoEffect->EndPass());

		IF_FAILED_THROW(hr = m_pVideoEffect->End());

		// Rendering text on a RT surface with D3DFMT_L8 does not provide correct color for the font.
		// It should be use with RGB format, but it is just for the demonstration
		IF_FAILED_THROW(hr = m_cText2D.OnRender());

		IF_FAILED_THROW(hr = pDevice->EndScene());

		IF_FAILED_THROW(hr = m_pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pDevice);

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnCopySurfaceToTexture(IDirect3DSurface9* pSrcSurface, IDirect3DTexture9* pDstTexture){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnCopySurfaceToTexture"));

	HRESULT hr = S_OK;

	D3DSURFACE_DESC SrcDesc;
	D3DSURFACE_DESC DstDesc;
	D3DLOCKED_RECT SrcLockRect;
	D3DLOCKED_RECT DstLockRect;
	UINT32 uiHeight;

	try{

		IF_FAILED_THROW(hr = pSrcSurface->GetDesc(&SrcDesc));
		IF_FAILED_THROW(hr = pDstTexture->GetLevelDesc(0, &DstDesc));

		IF_FAILED_THROW(hr = pSrcSurface->LockRect(&SrcLockRect, NULL, D3DLOCK_READONLY));
		IF_FAILED_THROW(hr = pDstTexture->LockRect(0, &DstLockRect, NULL, 0));

		// Better when SrcLockRect.Pitch == DstLockRect.Pitch
		if(SrcLockRect.Pitch == DstLockRect.Pitch){

			// Get min Height
			uiHeight = min(SrcDesc.Height, DstDesc.Height);
			memcpy(DstLockRect.pBits, SrcLockRect.pBits, SrcLockRect.Pitch * (uiHeight + (uiHeight / 2)));
		}
		else{

			// Get min Pitch
			INT iMinPitch = min(SrcLockRect.Pitch, DstLockRect.Pitch);
			// Get max Height
			uiHeight = max(SrcDesc.Height, DstDesc.Height);
			BYTE* pSrcData = (BYTE*)SrcLockRect.pBits;
			BYTE* pDstData = (BYTE*)DstLockRect.pBits;

			for(UINT ui = 0; ui < uiHeight; ui++){

				memcpy(pDstData, pSrcData, iMinPitch);
				pSrcData += SrcLockRect.Pitch;
				pDstData += DstLockRect.Pitch;
			}
		}

		IF_FAILED_THROW(hr = pSrcSurface->UnlockRect());
		IF_FAILED_THROW(hr = pDstTexture->UnlockRect(0));
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnCopySurfaceToSurface(IDirect3DSurface9* pSrcSurface, IDirect3DSurface9* pDstSurface){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnCopySurfaceToSurface"));

	HRESULT hr = S_OK;

	D3DSURFACE_DESC SrcDesc;
	D3DSURFACE_DESC DstDesc;
	D3DLOCKED_RECT SrcLockRect;
	D3DLOCKED_RECT DstLockRect;
	UINT32 uiHeight;

	try{

		IF_FAILED_THROW(hr = pSrcSurface->GetDesc(&SrcDesc));
		IF_FAILED_THROW(hr = pDstSurface->GetDesc(&DstDesc));

		IF_FAILED_THROW(hr = pSrcSurface->LockRect(&SrcLockRect, NULL, D3DLOCK_READONLY));
		IF_FAILED_THROW(hr = pDstSurface->LockRect(&DstLockRect, NULL, 0));

		// Better when SrcLockRect.Pitch == DstLockRect.Pitch
		if(SrcLockRect.Pitch == DstLockRect.Pitch){

			// Get min Height
			uiHeight = min(SrcDesc.Height, DstDesc.Height);
			memcpy(DstLockRect.pBits, SrcLockRect.pBits, SrcLockRect.Pitch * (uiHeight + (uiHeight / 2)));
		}
		else{

			// Get min Pitch
			INT iMinPitch = min(SrcLockRect.Pitch, DstLockRect.Pitch);
			// Get max Height
			uiHeight = max(SrcDesc.Height, DstDesc.Height);
			BYTE* pSrcData = (BYTE*)SrcLockRect.pBits;
			BYTE* pDstData = (BYTE*)DstLockRect.pBits;

			for(UINT ui = 0; ui < uiHeight; ui++){

				memcpy(pDstData, pSrcData, iMinPitch);
				pSrcData += SrcLockRect.Pitch;
				pDstData += DstLockRect.Pitch;
			}
		}

		IF_FAILED_THROW(hr = pSrcSurface->UnlockRect());
		IF_FAILED_THROW(hr = pDstSurface->UnlockRect());
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CMFTVideoShaderEffect::OnCopyBufferToTexture(IMFMediaBuffer* pBuffer, IDirect3DTexture9* pTexture9){

	TRACE_TRANSFORM((L"CMFTVideoShaderEffect::OnCopyBufferToTexture"));

	HRESULT hr = S_OK;

	D3DSURFACE_DESC Desc;
	D3DLOCKED_RECT LockRect;
	BOOL bIsSurfaceLock = FALSE;

	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;

	BYTE* pData = NULL;
	DWORD dwLength;

	try{

		IF_FAILED_THROW(hr = pTexture9->GetLevelDesc(0, &Desc));
		IF_FAILED_THROW(hr = MFGetAttributeSize(m_pOutputMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));

		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, &dwLength));
		IF_FAILED_THROW(hr = (dwLength != (Desc.Width * Desc.Height) ? E_UNEXPECTED : S_OK));

		IF_FAILED_THROW(hr = pTexture9->LockRect(0, &LockRect, NULL, 0));
		bIsSurfaceLock = TRUE;

		// Better when uiWidth == LockRect.Pitch
		if(uiWidth == (UINT32)LockRect.Pitch){

			// Get min Height
			uiHeight = min(Desc.Height, uiHeight);
			memcpy(LockRect.pBits, pData, uiWidth * (uiHeight + (uiHeight / 2)));
		}
		else{

			// Get min Pitch
			INT iMinPitch = min((UINT32)LockRect.Pitch, uiWidth);
			// Get max Height
			uiHeight = max(Desc.Height, uiHeight);
			BYTE* pSrcData = (BYTE*)pData;
			BYTE* pDstData = (BYTE*)LockRect.pBits;

			for(UINT ui = 0; ui < uiHeight; ui++){

				memcpy(pDstData, pSrcData, iMinPitch);
				pSrcData += uiWidth;
				pDstData += LockRect.Pitch;
			}
		}

		IF_FAILED_THROW(hr = pTexture9->UnlockRect(0));
		bIsSurfaceLock = FALSE;

		IF_FAILED_THROW(hr = pBuffer->Unlock());
		pData = NULL;
	}
	catch(HRESULT){}

	if(bIsSurfaceLock){
		LOG_HRESULT(pTexture9->UnlockRect(0));
	}

	if(pBuffer){

		if(pData)
			LOG_HRESULT(pBuffer->Unlock());
	}

	return hr;
}