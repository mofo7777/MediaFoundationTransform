//----------------------------------------------------------------------------------------------
// MFTAsynchronousAudio_Transform.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMFTAsynchronousAudio::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetStreamLimits"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

	*pdwInputMinimum = 1;
	*pdwInputMaximum = 1;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetStreamCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetStreamIDs"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetInputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES | MFT_INPUT_STREAM_DOES_NOT_ADDREF;
	pStreamInfo->hnsMaxLatency = 0;
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 0;

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetOutputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbAlignment = 0;

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetAttributes(IMFAttributes** ppAttributes){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetAttributes"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppAttributes == NULL ? E_POINTER : S_OK));

	IMFAttributes* pAttributes = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateAttributes(&pAttributes, 2));

		IF_FAILED_THROW(hr = pAttributes->SetUINT32(MF_TRANSFORM_ASYNC, TRUE));

		// I don't understand why an asynchronous MFT must handle dynamic format change :
		// https://docs.microsoft.com/en-us/windows/desktop/medfound/asynchronous-mfts
		IF_FAILED_THROW(hr = pAttributes->SetUINT32(MFT_SUPPORT_DYNAMIC_FORMAT_CHANGE, TRUE));

		*ppAttributes = pAttributes;
		(*ppAttributes)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pAttributes);

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetInputStreamAttributes(DWORD, IMFAttributes**){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetInputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::GetOutputStreamAttributes(DWORD, IMFAttributes**){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetOutputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::DeleteInputStream(DWORD){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::DeleteInputStream"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::AddInputStreams(DWORD, DWORD*){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::AddInputStreams"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::GetInputAvailableType(DWORD dwInputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetInputAvailableType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	IF_FAILED_RETURN(hr = OnGetAvailableMediaType(ppType, TRUE));

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetOutputAvailableType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pInputType == NULL){
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else{
		hr = OnGetAvailableMediaType(ppType, FALSE);
	}

	return hr;
}

HRESULT CMFTAsynchronousAudio::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::SetInputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	// todo : handle dynamic format change
	assert(m_bStreaming == FALSE);

	if(pType){
		hr = OnCheckMediaType(pType, TRUE);
	}

	if(SUCCEEDED(hr)){

		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		if(bReallySet){

			SAFE_RELEASE(m_pInputType);

			if(pType){

				// todo : clone mediatype
				m_pInputType = pType;
				m_pInputType->AddRef();
			}
		}
	}

	return hr;
}

HRESULT CMFTAsynchronousAudio::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::SetOutputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	// todo : handle dynamic format change
	assert(m_bStreaming == FALSE);
	
	if(pType){
		
		hr = OnCheckMediaType(pType, FALSE);
	}

	if(SUCCEEDED(hr)){

		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		if(bReallySet){

			SAFE_RELEASE(m_pOutputType);

			if(pType){

				// todo : clone mediatype
				m_pOutputType = pType;
				m_pOutputType->AddRef();
			}
		}
	}

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetInputCurrentType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_pInputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	// todo : clone mediatype
	// hr = MFCreateMediaType(&pType);
	// hr = m_pInputType->CopyAllItems(pType);
	// *ppType = pType;
	// (*ppType)->AddRef();
	// SAFE_RELEASE(pType);
	*ppType = m_pInputType;
	(*ppType)->AddRef();

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetOutputCurrentType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_pOutputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	// todo : clone mediatype
	// hr = MFCreateMediaType(&pType);
	// hr = m_pOutputType->CopyAllItems(pType);
	// *ppType = pType;
	// (*ppType)->AddRef();
	// SAFE_RELEASE(pType);
	*ppType = m_pOutputType;
	(*ppType)->AddRef();

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetInputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());
	IF_FAILED_RETURN(hr = (m_pInputType && m_pOutputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	*pdwFlags = m_pOutputSample ? 0 : MFT_INPUT_STATUS_ACCEPT_DATA;

	return hr;
}

HRESULT CMFTAsynchronousAudio::GetOutputStatus(DWORD* pdwFlags){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::GetOutputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());
	IF_FAILED_RETURN(hr = (m_pInputType && m_pOutputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	*pdwFlags = m_pOutputSample ? MFT_OUTPUT_STATUS_SAMPLE_READY : 0;

	return hr;
}

HRESULT CMFTAsynchronousAudio::SetOutputBounds(LONGLONG, LONGLONG){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::SetOutputBounds"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::ProcessEvent(DWORD, IMFMediaEvent*){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::ProcessEvent"));

	return E_NOTIMPL;
}

HRESULT CMFTAsynchronousAudio::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::ProcessMessage : %s", MFTMessageString(eMessage)));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());
	IF_FAILED_RETURN(hr = (m_pInputType && m_pOutputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));

	switch(eMessage){

		case MFT_MESSAGE_COMMAND_FLUSH:
			OnFlush();
			break;

		case MFT_MESSAGE_NOTIFY_END_STREAMING:
			hr = OnEndOfStream(0);
			break;

		case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
			hr = OnEndOfStream(ulParam);
			break;

		case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
			hr = OnStartOfStream();
			break;

		case MFT_MESSAGE_COMMAND_DRAIN:
			hr = OnDrain();
			break;

		case MFT_MESSAGE_COMMAND_MARKER:
			// todo : send marker after current output
			hr = OnTransformMarker(ulParam);
			break;

		/*case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		case MFT_MESSAGE_SET_D3D_MANAGER:
		case MFT_MESSAGE_DROP_SAMPLES:
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

HRESULT CMFTAsynchronousAudio::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::ProcessInput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());
	IF_FAILED_RETURN(hr = (m_pInputType && m_pOutputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = (m_bStreaming && m_iInputPendingCount ? S_OK : MF_E_NOTACCEPTING));

	m_iInputPendingCount--;

	assert(m_pOutputSample == NULL);

	//LogSample(pSample);

	LONGLONG llTime = 0;
	LONGLONG llDuration = 0;

	hr = pSample->GetSampleTime(&llTime);

	if(FAILED(hr))
		IF_FAILED_RETURN(hr = MF_E_NO_SAMPLE_TIMESTAMP);

	hr = pSample->GetSampleDuration(&llDuration);

	if(FAILED(hr))
		IF_FAILED_RETURN(hr = MF_E_NO_SAMPLE_DURATION);

	IMFSample* pOutputSample = NULL;
	IMFMediaBuffer* pInputBuffer = NULL;
	IMFMediaBuffer* pOutputBuffer = NULL;

	BYTE* pInputData = NULL;
	DWORD dwInputSize = 0;

	BYTE* pOutputData = NULL;
	DWORD dwOutputSize = 0;

	try{

		IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pInputBuffer));
		IF_FAILED_THROW(hr = pInputBuffer->Lock(&pInputData, NULL, &dwInputSize));

		dwOutputSize = dwInputSize * 2;

		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwOutputSize, &pOutputBuffer));
		IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(dwOutputSize));
		IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pOutputData, NULL, &dwOutputSize));

		ConvertPCMToFloat(dwInputSize, pInputData, pOutputData);

		IF_FAILED_THROW(hr = pInputBuffer->Unlock());
		pInputData = NULL;
		IF_FAILED_THROW(hr = pOutputBuffer->Unlock());
		pOutputData = NULL;

		IF_FAILED_THROW(hr = MFCreateSample(&pOutputSample));
		IF_FAILED_THROW(hr = pOutputSample->AddBuffer(pOutputBuffer));

		IF_FAILED_THROW(hr = pOutputSample->SetSampleTime(llTime));
		IF_FAILED_THROW(hr = pOutputSample->SetSampleDuration(llDuration));
		IF_FAILED_THROW(hr = pOutputSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));

		IF_FAILED_THROW(hr = OnTransformHaveOutput());

		m_pOutputSample = pOutputSample;
		m_pOutputSample->AddRef();
	}
	catch(HRESULT){}

	if(pOutputBuffer && pOutputData){
		LOG_HRESULT(pOutputBuffer->Unlock());
	}

	if(pInputBuffer && pInputData){
		LOG_HRESULT(pInputBuffer->Unlock());
	}

	SAFE_RELEASE(pOutputBuffer);
	SAFE_RELEASE(pInputBuffer);
	SAFE_RELEASE(pOutputSample);

	/*IUnknown* pToken = NULL;
	IMFMediaEvent* pMediaEvent = NULL;
	UINT32 uiValue = 0;

	try{

		IF_FAILED_THROW(hr = pSample->GetUnknown(MFSampleExtension_Token, IID_IUnknown, reinterpret_cast<void**>(&pToken)));

		hr = pMediaEvent->GetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, &uiValue);
	}
	catch(HRESULT){}*/

	return hr;
}

HRESULT CMFTAsynchronousAudio::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::ProcessOutput"));

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

	IF_FAILED_RETURN(hr = CheckShutdown());
	IF_FAILED_RETURN(hr = (m_pInputType && m_pOutputType ? S_OK : MF_E_TRANSFORM_TYPE_NOT_SET));
	IF_FAILED_RETURN(hr = (m_iOutputPendingCount ? S_OK : E_UNEXPECTED));
	
	m_iOutputPendingCount--;

	assert(m_pOutputSample);

	pOutputSamples[0].pSample = m_pOutputSample;
	pOutputSamples[0].pSample->AddRef();

	SAFE_RELEASE(m_pOutputSample);

	if(m_bDraining){

		m_bDraining = FALSE;

		IF_FAILED_RETURN(hr = OnTransformDrainComplete());
	}
	else{

		IF_FAILED_RETURN(hr = OnTransformNeedInput());
	}

	return hr;
}