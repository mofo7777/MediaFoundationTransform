//----------------------------------------------------------------------------------------------
// DllMain.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK

const WCHAR SZ_DECODER_NAME[] = L"Simple Video Effect Transform";

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/){

	switch(ul_reason_for_call){

	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls((HMODULE)hModule);
		g_hModule = (HMODULE)hModule;
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

STDAPI DllCanUnloadNow(){

	if(!ClassFactory::IsLocked()){
		return S_OK;
	}
	else{
		return S_FALSE;
	}
}

STDAPI DllRegisterServer(){

	HRESULT hr = S_OK;

	if(SUCCEEDED(hr = RegisterObject(g_hModule, CLSID_MFTSimpleDirectxAware, SZ_DECODER_NAME, TEXT("Both")))){

		MFT_REGISTER_TYPE_INFO InputTypesInfo[] = {{ MFMediaType_Video, MFVideoFormat_YV12}};
		MFT_REGISTER_TYPE_INFO OutputTypesInfo[] = {{MFMediaType_Video, MFVideoFormat_YV12}};

		hr = MFTRegister(CLSID_MFTSimpleDirectxAware, MFT_CATEGORY_VIDEO_EFFECT, const_cast<LPWSTR>(SZ_DECODER_NAME), 0, ARRAY_SIZE(InputTypesInfo),
			InputTypesInfo, ARRAY_SIZE(OutputTypesInfo), OutputTypesInfo, NULL);
	}

	return hr;
}

STDAPI DllUnregisterServer(){

	UnregisterObject(CLSID_MFTSimpleDirectxAware);
	MFTUnregister(CLSID_MFTSimpleDirectxAware);

	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv){

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if(clsid == CLSID_MFTSimpleDirectxAware){

		ClassFactory* pFactory = NULL;

		pFactory = new (std::nothrow)ClassFactory(CMFTSimpleDirectxAware::CreateInstance);

		if(pFactory){

			hr = pFactory->QueryInterface(riid, ppv);

			SAFE_RELEASE(pFactory);
		}
		else{

			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}