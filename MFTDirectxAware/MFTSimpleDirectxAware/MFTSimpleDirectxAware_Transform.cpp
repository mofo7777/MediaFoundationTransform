//----------------------------------------------------------------------------------------------
// MFTSimpleDirectxAware_Transform.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMFTSimpleDirectxAware::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetStreamLimits"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

	*pdwInputMinimum = 1;
	*pdwInputMaximum = 1;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetStreamCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetStreamIDs"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetInputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES
		| MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER
		| MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE
		| MFT_INPUT_STREAM_DOES_NOT_ADDREF;

	pStreamInfo->hnsMaxLatency = 0;
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 0;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetOutputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES
		| MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER
		| MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE
		| MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

	pStreamInfo->cbAlignment = 0;
	pStreamInfo->cbSize = 0;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetAttributes(IMFAttributes** ppAttributes){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetAttributes"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppAttributes == NULL ? E_POINTER : S_OK));

	IMFAttributes* pAttributes = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateAttributes(&pAttributes, 2));
		IF_FAILED_THROW(hr = pAttributes->SetUINT32(MF_SA_D3D_AWARE, TRUE));
		IF_FAILED_THROW(hr = pAttributes->SetUINT32(MFT_SUPPORT_DYNAMIC_FORMAT_CHANGE, TRUE));

		*ppAttributes = pAttributes;
		(*ppAttributes)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pAttributes);

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetInputStreamAttributes(DWORD, IMFAttributes**){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetInputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::GetOutputStreamAttributes(DWORD, IMFAttributes**){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetOutputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::DeleteInputStream(DWORD){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::DeleteInputStream"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::AddInputStreams(DWORD, DWORD*){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::AddInputStreams"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::GetInputAvailableType(DWORD dwInputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetInputAvailableType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_pInputMediaType == NULL){
		IF_FAILED_RETURN(hr = OnGetAvalaibleType(ppType));
	}
	else{

		IF_FAILED_RETURN(hr = OnCloneMediaType(m_pInputMediaType, ppType));
	}

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetOutputAvailableType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_pInputMediaType == NULL){
		IF_FAILED_RETURN(hr = MF_E_TRANSFORM_TYPE_NOT_SET);
	}
	else{

		IMFMediaType* pMediatype = m_pOutputMediaType ? m_pOutputMediaType : m_pInputMediaType;
		IF_FAILED_RETURN(hr = OnCloneMediaType(pMediatype, ppType));
	}

	return hr;
}

HRESULT CMFTSimpleDirectxAware::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::SetInputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	if(pType){

		IF_FAILED_RETURN(hr = OnCheckMediaType(pType));
	}

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	if(bReallySet){

		AutoLock lock(m_CriticSection);

		if(m_pVideoSampleAllocator){

			IF_FAILED_RETURN(hr = CheckFormatChange(pType));
		}

		SAFE_RELEASE(m_pInputMediaType);

		if(pType){

			IF_FAILED_RETURN(hr = OnCloneMediaType(pType, &m_pInputMediaType));
			LogMediaType(pType);
		}
	}

	return hr;
}

HRESULT CMFTSimpleDirectxAware::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::SetOutputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	if(pType){

		IF_FAILED_RETURN(hr = OnCheckMediaType(pType));
	}

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	if(bReallySet){

		AutoLock lock(m_CriticSection);

		SAFE_RELEASE(m_pOutputMediaType);

		if(pType){

			IF_FAILED_RETURN(hr = OnCloneMediaType(pType, &m_pOutputMediaType));
			LogMediaType(pType);

			if(m_bFormatHasChange){

				IF_FAILED_RETURN(hr = OnReAllocateSurface());
				m_bFormatHasChange = FALSE;
			}
		}
	}

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetInputCurrentType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pInputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	IF_FAILED_RETURN(hr = OnCloneMediaType(m_pInputMediaType, ppType));

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetOutputCurrentType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	IF_FAILED_RETURN(hr = OnCloneMediaType(m_pOutputMediaType, ppType));

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetInputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	*pdwFlags = m_pOutputSample ? 0 : MFT_INPUT_STATUS_ACCEPT_DATA;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::GetOutputStatus(DWORD* pdwFlags){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::GetOutputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	*pdwFlags = m_pOutputSample ? MFT_OUTPUT_STATUS_SAMPLE_READY : 0;

	return hr;
}

HRESULT CMFTSimpleDirectxAware::SetOutputBounds(LONGLONG, LONGLONG){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::SetOutputBounds"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::ProcessEvent(DWORD, IMFMediaEvent*){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::ProcessEvent"));

	return E_NOTIMPL;
}

HRESULT CMFTSimpleDirectxAware::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::ProcessMessage : %s", MFTMessageString(eMessage)));

	AutoLock lock(m_CriticSection);

	HRESULT hr = S_OK;

	switch(eMessage){

		case MFT_MESSAGE_COMMAND_FLUSH:
			hr = OnFlush();
			break;

		case MFT_MESSAGE_COMMAND_DRAIN:
			hr = OnDrain();
			break;

		case MFT_MESSAGE_NOTIFY_END_STREAMING:
			hr = OnEndOfStream(0);
			break;

		case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
			hr = OnEndOfStream(ulParam);
			break;

		case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
			hr = OnStartStream();
			break;

		case MFT_MESSAGE_COMMAND_MARKER:
			// todo : send marker after current output
			//-->hr = OnTransformMarker(ulParam);
			break;

		case MFT_MESSAGE_SET_D3D_MANAGER:
			hr = OnSetD3DManager(ulParam);
			break;

		/*case MFT_MESSAGE_DROP_SAMPLES:
		case MFT_MESSAGE_COMMAND_TICK:
		case MFT_MESSAGE_NOTIFY_RELEASE_RESOURCES:
		case MFT_MESSAGE_NOTIFY_REACQUIRE_RESOURCES:
		case MFT_MESSAGE_NOTIFY_EVENT:
		case MFT_MESSAGE_COMMAND_SET_OUTPUT_STREAM_STATE:
		case MFT_MESSAGE_COMMAND_FLUSH_OUTPUT_STREAM:
		// Nothing to do : S_OK
		break;*/
	}

	return hr;
}

HRESULT CMFTSimpleDirectxAware::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::ProcessInput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = (m_pOutputSample || m_bDraining || m_bFormatHasChange ? MF_E_NOTACCEPTING : S_OK));
	IF_FAILED_RETURN(hr = (m_pVideoSampleAllocator ? S_OK : E_UNEXPECTED));

	//LogSample(pSample);

	IMFSample* pOutputSample = NULL;
	IMFMediaBuffer* pOutputBuffer = NULL;
	IDirect3DSurface9* pOutputSurface = NULL;

	IMFMediaBuffer* pInputBuffer = NULL;
	IDirect3DSurface9* pInputSurface = NULL;

	try{

		IF_FAILED_THROW(hr = m_pVideoSampleAllocator->AllocateSample(&pOutputSample));
		IF_FAILED_THROW(hr = pOutputSample->GetBufferByIndex(0, &pOutputBuffer));
		IF_FAILED_THROW(hr = MFGetService(pOutputBuffer, MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), reinterpret_cast<void**>(&pOutputSurface)));

		IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pInputBuffer));

		if(SUCCEEDED(hr = MFGetService(pInputBuffer, MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), reinterpret_cast<void**>(&pInputSurface)))){

			IF_FAILED_THROW(hr = OnCopySurfaceToSurface(pInputSurface, pOutputSurface));
		}
		else{

			IF_FAILED_THROW(hr = OnCopyBufferToSurface(pInputBuffer, pOutputSurface));
		}
		
		IF_FAILED_THROW(hr = OnCloneSample(pSample, pOutputSample));

		//LogSample(pOutputSample);

		m_pOutputSample = pOutputSample;
		m_pOutputSample->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pInputSurface);
	SAFE_RELEASE(pInputBuffer);

	SAFE_RELEASE(pOutputSurface);
	SAFE_RELEASE(pOutputBuffer);
	SAFE_RELEASE(pOutputSample);

	return hr;
}

HRESULT CMFTSimpleDirectxAware::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

	TRACE_TRANSFORM((L"CMFTSimpleDirectxAware::ProcessOutput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (cOutputBufferCount != 1 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].dwStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample != NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].dwStatus != 0 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].pEvents != NULL ? E_INVALIDARG : S_OK));
	*pdwStatus = 0;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pInputMediaType && m_pOutputMediaType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	// We just handle one sample
	if(m_bDraining)
		m_bDraining = FALSE;

	if(m_pOutputSample){

		pOutputSamples[0].pSample = m_pOutputSample;
		pOutputSamples[0].pSample->AddRef();

		SAFE_RELEASE(m_pOutputSample);
	}
	else{

		if(m_bFormatHasChange){

			pOutputSamples[0].dwStatus = MFT_OUTPUT_DATA_BUFFER_FORMAT_CHANGE;
			SAFE_RELEASE(m_pOutputMediaType);
			hr = MF_E_TRANSFORM_STREAM_CHANGE;
		}
		else{

			hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
		}
	}

	return hr;
}