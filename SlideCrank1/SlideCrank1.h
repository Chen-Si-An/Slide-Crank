
// SlideCrank1.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CSlideCrank1App:
// �аѾ\��@�����O�� SlideCrank1.cpp
//

class CSlideCrank1App : public CWinApp
{
public:
	CSlideCrank1App();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CSlideCrank1App theApp;