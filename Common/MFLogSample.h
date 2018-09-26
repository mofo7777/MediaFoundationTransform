//----------------------------------------------------------------------------------------------
// MFLogSample.h
//----------------------------------------------------------------------------------------------
#ifndef MFLOGSAMPLE_H
#define MFLOGSAMPLE_H

#if (_DEBUG && MF_USE_LOGGING)

inline LPCWSTR GetGUIDStringSampleAttributes(const GUID& guid){

	IF_EQUAL_RETURN(guid, GUID_NULL);
	IF_EQUAL_RETURN(guid, MFSampleExtension_3DVideo);
	IF_EQUAL_RETURN(guid, MFSampleExtension_3DVideo_SampleFormat);
	IF_EQUAL_RETURN(guid, MFSampleExtension_BottomFieldFirst);
	IF_EQUAL_RETURN(guid, MFSampleExtension_CameraExtrinsics);
	IF_EQUAL_RETURN(guid, MFSampleExtension_CaptureMetadata);
	IF_EQUAL_RETURN(guid, MFSampleExtension_CleanPoint);
	IF_EQUAL_RETURN(guid, MFSampleExtension_Content_KeyID);
	IF_EQUAL_RETURN(guid, MFSampleExtension_DerivedFromTopField);
	IF_EQUAL_RETURN(guid, MFSampleExtension_DeviceTimestamp);
	IF_EQUAL_RETURN(guid, MFSampleExtension_Discontinuity);
	// Windows10--> IF_EQUAL_RETURN(guid, MFSampleExtension_Encryption_CryptByteBlock);
	// Windows10--> IF_EQUAL_RETURN(guid, MFSampleExtension_Encryption_ProtectionScheme);
	IF_EQUAL_RETURN(guid, MFSampleExtension_Encryption_SampleID);
	// Windows10--> IF_EQUAL_RETURN(guid, MFSampleExtension_Encryption_SkipByteBlock);
	IF_EQUAL_RETURN(guid, MFSampleExtension_Encryption_SubSampleMappingSplit);
	IF_EQUAL_RETURN(guid, MFSampleExtension_FrameCorruption);
	// Windows10--> IF_EQUAL_RETURN(guid, MFSampleExtension_ForwardedDecodeUnits);
	// Windows10--> IF_EQUAL_RETURN(guid, MFSampleExtension_ForwardedDecodeUnitType);
	IF_EQUAL_RETURN(guid, MFSampleExtension_Interlaced);
	IF_EQUAL_RETURN(guid, MFSampleExtension_LongTermReferenceFrameInfo);
	IF_EQUAL_RETURN(guid, MFSampleExtension_MeanAbsoluteDifference);
	IF_EQUAL_RETURN(guid, MFSampleExtension_PacketCrossOffsets);
	IF_EQUAL_RETURN(guid, MFSampleExtension_PhotoThumbnail);
	IF_EQUAL_RETURN(guid, MFSampleExtension_PhotoThumbnailMediaType);
	IF_EQUAL_RETURN(guid, MFSampleExtension_PinholeCameraIntrinsics);
	IF_EQUAL_RETURN(guid, MFSampleExtension_RepeatFirstField);
	IF_EQUAL_RETURN(guid, MFSampleExtension_ROIRectangle);
	IF_EQUAL_RETURN(guid, MFSampleExtension_SingleField);
	// Windows10--> IF_EQUAL_RETURN(guid, MFSampleExtension_TargetGlobalLuminance);
	IF_EQUAL_RETURN(guid, MFSampleExtension_Token);
	IF_EQUAL_RETURN(guid, MFSampleExtension_VideoEncodePictureType);
	IF_EQUAL_RETURN(guid, MFSampleExtension_VideoEncodeQP);

	return NULL;
}

inline void LogSample(IMFSample* pSample){

	if(pSample == NULL){

		TRACE((L"\r\n\tNo sample\r\n"));
		return;
	}

	HRESULT hr;
	UINT32 uiCount = 0;
	LONGLONG llTime = 0;

	TRACE((L"\r\n"));

	if(SUCCEEDED(hr = pSample->GetSampleTime(&llTime))){

		TRACE_NO_END_LINE((L"\tSample time : "));
		MFTimeString(llTime);
	}
	else{

		TRACE((L"\tSample : no sample time"));
	}

	if(SUCCEEDED(hr = pSample->GetSampleDuration(&llTime))){

		TRACE_NO_END_LINE((L"\tSample duration : "));
		MFTimeString(llTime);
	}
	else{

		TRACE((L"\tSample : no sample duration"));
	}

	hr = pSample->GetCount(&uiCount);

	if(SUCCEEDED(hr)){

		for(UINT32 i = 0; i < uiCount; i++)
			LogAttributeValueByIndex(pSample, i, GetGUIDStringSampleAttributes);

		TRACE((L"\r\n"));
	}
	else{

		TRACE((L"\r\n\tSample : no attributes\r\n"));
	}
}

#else
#define LogSample(x)
#endif

#endif
