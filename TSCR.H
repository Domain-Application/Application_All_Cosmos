#if !defined(AFX_TSCR_H__0AEFB601_AD4B_11D2_B1F7_D9BD24ABFC2B__INCLUDED_)
#define AFX_TSCR_H__0AEFB601_AD4B_11D2_B1F7_D9BD24ABFC2B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Tscr.h : header file
//
#include "tio.h"
/////////////////////////////////////////////////////////////////////////////
// CTscr dialog

class CTscr : public CTio
{
// Construction
public:
   CTscr(UINT IDD,DB *pdf,int Idx,void(*p)(),CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTscr)
	enum { IDD = IDD_TSCR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
    virtual void show_tio();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTscr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTscr)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSCR_H__0AEFB601_AD4B_11D2_B1F7_D9BD24ABFC2B__INCLUDED_)
