//----------------------------------------------------------------------------------------------
// DllMain.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK

const WCHAR SZ_DECODER_NAME[] = L"Video Shader Effect Transform";

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

	HRESULT hr;

	if(SUCCEEDED(hr = RegisterObject(g_hModule, CLSID_MFTVideoShaderEffect, SZ_DECODER_NAME, TEXT("Both")))){

		MFT_REGISTER_TYPE_INFO InputTypesInfo[] = {{ MFMediaType_Video, MFVideoFormat_NV12}};
		MFT_REGISTER_TYPE_INFO OutputTypesInfo[] = {{MFMediaType_Video, MFVideoFormat_NV12}};

		hr = MFTRegister(CLSID_MFTVideoShaderEffect, MFT_CATEGORY_VIDEO_EFFECT, const_cast<LPWSTR>(SZ_DECODER_NAME), 0, ARRAY_SIZE(InputTypesInfo),
			InputTypesInfo, ARRAY_SIZE(OutputTypesInfo), OutputTypesInfo, NULL);
	}

	return hr;
}

STDAPI DllUnregisterServer(){

	HRESULT hr = UnregisterObject(CLSID_MFTVideoShaderEffect);

	if(SUCCEEDED(hr))
		// MFTUnregister is buggy : https://social.msdn.microsoft.com/Forums/vstudio/en-US/7d3dc70f-8eae-4ad0-ad90-6c596cf78c80/mftunregister-returns-error-0x80070002-on-dllunregisterserver-of-plugin?forum=mediafoundationdevelopment
		/*hr =*/ MFTUnregister(CLSID_MFTVideoShaderEffect);

	return hr;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv){

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if(clsid == CLSID_MFTVideoShaderEffect){

		ClassFactory* pFactory = NULL;

		pFactory = new (std::nothrow)ClassFactory(CMFTVideoShaderEffect::CreateInstance);

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