//----------------------------------------------------------------------------------------------
// Text2D.h
//----------------------------------------------------------------------------------------------
#ifndef TEXT2D_H
#define TEXT2D_H

class CText2D{

public:

	CText2D() : m_pTextFont(NULL){}
	~CText2D(){ OnDelete(); }

	HRESULT OnRestore(IDirect3DDevice9*, const UINT32, const UINT32, const FLOAT);
	HRESULT OnRender();
	void    OnMove(const LONG lSpeed);
	void    OnDelete();

private:

	ID3DXFont* m_pTextFont;
	RECT m_Rectangle;
	int m_iTextLenght;
	int m_iCharTextLenght;
	LONG m_lWidth;
};

#endif