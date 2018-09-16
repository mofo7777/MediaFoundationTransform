//----------------------------------------------------------------------------------------------
// MFTAsynchronousAudio.h
//----------------------------------------------------------------------------------------------
#ifndef MFTASYNCHRONOUSAUDIO_H
#define MFTASYNCHRONOUSAUDIO_H

class CMFTAsynchronousAudio : public IMFTransform, public IMFMediaEventGenerator, public IMFShutdown{

public:

	// MFTAsynchronousAudio.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - MFTAsynchronousAudio.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFTransform - MFTAsynchronousAudio_Transform.cpp
	STDMETHODIMP GetStreamLimits(DWORD*, DWORD*, DWORD*, DWORD*);
	STDMETHODIMP GetStreamCount(DWORD*, DWORD*);
	STDMETHODIMP GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*);
	STDMETHODIMP GetInputStreamInfo(DWORD, MFT_INPUT_STREAM_INFO*);
	STDMETHODIMP GetOutputStreamInfo(DWORD, MFT_OUTPUT_STREAM_INFO*);
	STDMETHODIMP GetAttributes(IMFAttributes**);
	STDMETHODIMP GetInputStreamAttributes(DWORD, IMFAttributes**);
	STDMETHODIMP GetOutputStreamAttributes(DWORD, IMFAttributes**);
	STDMETHODIMP DeleteInputStream(DWORD);
	STDMETHODIMP AddInputStreams(DWORD, DWORD*);
	STDMETHODIMP GetInputAvailableType(DWORD, DWORD, IMFMediaType**);
	STDMETHODIMP GetOutputAvailableType(DWORD, DWORD, IMFMediaType**);
	STDMETHODIMP SetInputType(DWORD, IMFMediaType*, DWORD);
	STDMETHODIMP SetOutputType(DWORD, IMFMediaType*, DWORD);
	STDMETHODIMP GetInputCurrentType(DWORD, IMFMediaType**);
	STDMETHODIMP GetOutputCurrentType(DWORD, IMFMediaType**);
	STDMETHODIMP GetInputStatus(DWORD, DWORD*);
	STDMETHODIMP GetOutputStatus(DWORD*);
	STDMETHODIMP SetOutputBounds(LONGLONG, LONGLONG);
	STDMETHODIMP ProcessEvent(DWORD, IMFMediaEvent*);
	STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE, ULONG_PTR);
	STDMETHODIMP ProcessInput(DWORD, IMFSample*, DWORD);
	STDMETHODIMP ProcessOutput(DWORD, DWORD, MFT_OUTPUT_DATA_BUFFER*, DWORD*);

	// IMFMediaEventGenerator - MFTAsynchronousAudio_Event.cpp
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFShutdown - MFTAsynchronousAudio_Shutdown.cpp
	STDMETHODIMP GetShutdownStatus(MFSHUTDOWN_STATUS*);
	STDMETHODIMP Shutdown();

private:

	// MFTAsynchronousAudio.cpp
	CMFTAsynchronousAudio(HRESULT& hr);
	virtual ~CMFTAsynchronousAudio();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	IMFMediaEventQueue* m_pEventQueue;
	BOOL m_bShutDown;
	BOOL m_bStreaming;
	BOOL m_bDraining;
	IMFMediaType* m_pInputType;
	IMFMediaType* m_pOutputType;
	IMFSample* m_pOutputSample;
	int m_iInputPendingCount;
	int m_iOutputPendingCount;

	// MFTAsynchronousAudio.cpp
	HRESULT OnStartOfStream();
	HRESULT OnEndOfStream(ULONG_PTR ulParam);
	HRESULT OnDrain();
	void OnFlush();
	HRESULT OnTransformNeedInput();
	HRESULT OnTransformHaveOutput();
	HRESULT OnTransformDrainComplete();
	HRESULT OnTransformMarker(ULONG_PTR);
	void ConvertPCMToFloat(const DWORD, const BYTE*, BYTE*);

	// MFTAsynchronousAudio_Types.cpp
	HRESULT OnCheckMediaType(IMFMediaType*, const BOOL);
	HRESULT OnGetAvailableMediaType(IMFMediaType**, const BOOL);

	// Inline
	HRESULT CheckShutdown() const{ return (m_bShutDown ? MF_E_SHUTDOWN : S_OK); }
};

#endif