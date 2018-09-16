//----------------------------------------------------------------------------------------------
// MFTAsynchronousAudio_Shutdown.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMFTAsynchronousAudio::GetShutdownStatus(MFSHUTDOWN_STATUS* pStatus){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetShutdownStatus"));

	HRESULT hr;

	IF_FAILED_RETURN(hr = (pStatus == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_bShutDown){

		*pStatus = MFSHUTDOWN_COMPLETED;
	}
	else{

		hr = MF_E_INVALIDREQUEST;
	}

	return hr;
}

HRESULT CMFTAsynchronousAudio::Shutdown(){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr = S_OK;

	if(m_bShutDown)
		return hr;

	SAFE_RELEASE(m_pInputType);
	SAFE_RELEASE(m_pOutputType);
	SAFE_RELEASE(m_pOutputSample);

	if(m_pEventQueue){
		hr = m_pEventQueue->Shutdown();
	}

	SAFE_RELEASE(m_pEventQueue);

	m_iInputPendingCount = 0;
	m_iOutputPendingCount = 0;

	m_bStreaming = FALSE;
	m_bDraining = FALSE;
	m_bShutDown = TRUE;

	return hr;
}