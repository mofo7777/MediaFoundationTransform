//----------------------------------------------------------------------------------------------
// MFTAsynchronousAudio_Types.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

// MF_MT_MAJOR_TYPE			MFMediaType_Audio
// MF_MT_SUBTYPE			MFAudioFormat_PCM
// MF_MT_AUDIO_SAMPLES_PER_SECOND	48000
// MF_MT_AUDIO_NUM_CHANNELS		2
// MF_MT_AUDIO_BITS_PER_SAMPLE		16
// MF_MT_AUDIO_BLOCK_ALIGNMENT		4
// MF_MT_AUDIO_AVG_BYTES_PER_SECOND	192000
// MF_MT_ALL_SAMPLES_INDEPENDENT	1

// MF_MT_MAJOR_TYPE			MFMediaType_Audio
// MF_MT_SUBTYPE			MFAudioFormat_Float
// MF_MT_AUDIO_SAMPLES_PER_SECOND	48000
// MF_MT_AUDIO_NUM_CHANNELS		2
// MF_MT_AUDIO_BITS_PER_SAMPLE		32
// MF_MT_AUDIO_BLOCK_ALIGNMENT		8
// MF_MT_AUDIO_AVG_BYTES_PER_SECOND	384000
// MF_MT_ALL_SAMPLES_INDEPENDENT	1

HRESULT CMFTAsynchronousAudio::OnCheckMediaType(IMFMediaType* pmt, const BOOL bInput){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnCheckMediaType"));

	HRESULT hr;

	GUID majortype = {0};
	GUID subtype = {0};
	UINT32 uiSamplePerSec = 0;
	UINT32 uiChannels = 0;
	UINT32 uiBitsPerSample = 0;
	UINT32 uiBlocAligment = 0;
	UINT32 uiAvgBytesPerSecond = 0;
	UINT32 uiSampleIndependant = 0;

	//LogMediaType(pmt);

	IF_FAILED_RETURN(hr = pmt->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &uiSamplePerSec));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uiChannels));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &uiBitsPerSample));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &uiBlocAligment));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &uiAvgBytesPerSecond));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, &uiSampleIndependant));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Audio ? MF_E_INVALIDMEDIATYPE : S_OK));
	IF_FAILED_RETURN(hr = (uiSamplePerSec == 48000 ? S_OK : MF_E_INVALIDTYPE));
	IF_FAILED_RETURN(hr = (uiChannels == 2 ? S_OK : MF_E_INVALIDTYPE));
	IF_FAILED_RETURN(hr = (uiSampleIndependant == FALSE ? MF_E_INVALIDTYPE : S_OK));

	if(bInput){

		IF_FAILED_RETURN(hr = (subtype != MFAudioFormat_PCM ? MF_E_INVALIDMEDIATYPE : S_OK));
		IF_FAILED_RETURN(hr = (uiBitsPerSample == 16 ? S_OK : MF_E_INVALIDTYPE));
		IF_FAILED_RETURN(hr = (uiBlocAligment == 4 ? S_OK : MF_E_INVALIDTYPE));
		IF_FAILED_RETURN(hr = (uiAvgBytesPerSecond == 192000 ? S_OK : MF_E_INVALIDTYPE));
	}
	else{

		IF_FAILED_RETURN(hr = (subtype != MFAudioFormat_Float ? MF_E_INVALIDMEDIATYPE : S_OK));
		IF_FAILED_RETURN(hr = (uiBitsPerSample == 32 ? S_OK : MF_E_INVALIDTYPE));
		IF_FAILED_RETURN(hr = (uiBlocAligment == 8 ? S_OK : MF_E_INVALIDTYPE));
		IF_FAILED_RETURN(hr = (uiAvgBytesPerSecond == 384000 ? S_OK : MF_E_INVALIDTYPE));
	}

	return hr;
}

HRESULT CMFTAsynchronousAudio::OnGetAvailableMediaType(IMFMediaType** ppType, const BOOL bInput){

	TRACE_TRANSFORM((L"CMFTAsynchronousAudio::OnGetAvailableMediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));

		if(bInput){

			IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
			IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16));
			IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4));
			IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 192000));
		}
		else{

			IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
			IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32));
			IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 8));
			IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 384000));
		}

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);

	return hr;
}
