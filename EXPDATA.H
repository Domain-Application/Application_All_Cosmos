#if !defined(AFX_EXPDATA_H__F377D1F7_0685_407F_A87E_E8F4BC625A06__INCLUDED_)
#define AFX_EXPDATA_H__F377D1F7_0685_407F_A87E_E8F4BC625A06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExpData.h : header file
//

#include "xShadeButton.h"
/////////////////////////////////////////////////////////////////////////////
// CExpData dialog

class CExpData : public CDialog
{
// Construction
public:
	CExpData(CWnd* pParent = NULL);   // standard constructor
    void OnExport(const char *file);
    CString str;
    DB Exp;
    DB *df;

// Dialog Data
	//{{AFX_DATA(CExpData)
	enum { IDD = IDD_EXPDATA };
	CProgressCtrl	m_progress;
    CxShadeButton m_btn2;
    CxShadeButton m_btn1;
	COleDateTime	m_d1;
	COleDateTime	m_d2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpData)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExpData)
	afx_msg void OnToXLS();
	afx_msg void OnToDbase();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPDATA_H__F377D1F7_0685_407F_A87E_E8F4BC625A06__INCLUDED_)
