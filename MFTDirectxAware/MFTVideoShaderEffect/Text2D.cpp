//----------------------------------------------------------------------------------------------
// Text2D.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

#define TEXT_TO_DISPLAY L"\
This sample demonstrate a Direct3D-Aware Transform with dynamic format change. \
The MFT uses the IDirect3DDeviceManager9 to query the IDirect3DDevice9. With IDirect3DDevice9, \
we can then use shader (ID3DXEffect) and text (ID3DXFont), like here. We could also use 3D Models."

HRESULT CText2D::OnRestore(IDirect3DDevice9* pDevice, const UINT32 uiWidth, const UINT32 uiHeight, const FLOAT fFontSize){

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pDevice == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (m_pTextFont ? E_UNEXPECTED : S_OK));

	ID3DXFont* pTextFont = NULL;
	HDC hDC = NULL;
	int iLogPixelsY;
	int iFontSize;
	int iHeight;

	try{

		hDC = GetDC(NULL);
		IF_FAILED_THROW(hDC ? S_OK : E_POINTER);

		iLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
		iFontSize = static_cast<int>(uiHeight / fFontSize);
		iHeight = -MulDiv(iFontSize, iLogPixelsY, 72);

		IF_FAILED_THROW(hr = D3DXCreateFont(pDevice,
			iHeight,
			0,
			FW_NORMAL,
			1,
			TRUE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Arial",
			&pTextFont
		));

		m_iCharTextLenght = (int)wcslen(TEXT_TO_DISPLAY);

		IF_FAILED_THROW(hr = pTextFont->PreloadText(TEXT_TO_DISPLAY, m_iCharTextLenght));

		ReleaseDC(NULL, hDC);
		hDC = NULL;
		SIZE size;

		IF_FAILED_THROW((hDC = pTextFont->GetDC()) == NULL ? E_OUTOFMEMORY : S_OK);

		IF_FAILED_THROW(hr = (GetTextExtentPoint32(hDC, TEXT_TO_DISPLAY, m_iCharTextLenght, &size)) == false ? E_FAIL : S_OK);

		m_iTextLenght = -size.cx;
		m_lWidth = uiWidth;
		m_Rectangle.left = m_lWidth;
		m_Rectangle.top = uiHeight - (iFontSize + 32);
		m_Rectangle.right = 0;
		m_Rectangle.bottom = 0;

		m_pTextFont = pTextFont;
		m_pTextFont->AddRef();
	}
	catch(HRESULT){}

	if(hDC)
		ReleaseDC(NULL, hDC);

	SAFE_RELEASE(pTextFont);

	return hr;
}

void CText2D::OnMove(const LONG lPixelCount){

	if(m_Rectangle.left < m_iTextLenght){
		m_Rectangle.left = m_lWidth;
	}
	else{
		m_Rectangle.left -= lPixelCount;
	}
}

HRESULT CText2D::OnRender(){

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pTextFont ? S_OK : E_UNEXPECTED));

	m_pTextFont->DrawText(NULL, TEXT_TO_DISPLAY, m_iCharTextLenght, &m_Rectangle, DT_NOCLIP, 0xffff0000);

	return hr;
}

void CText2D::OnDelete(){

	if(m_pTextFont){

		m_pTextFont->OnLostDevice();
		SAFE_RELEASE(m_pTextFont);
	}
}