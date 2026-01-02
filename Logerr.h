#if !defined(AFX_LOGERR_H__960CAE82_37B8_11D3_B36C_00001CD5F6A7__INCLUDED_)
#define AFX_LOGERR_H__960CAE82_37B8_11D3_B36C_00001CD5F6A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Logerr.h : header file
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CLogerr dialog

class CLogerr : public CDialog
{
// Construction
public:
	CLogerr(CWnd* pParent = NULL);   // standard constructor
	~CLogerr();
    int level;
    DB  *df;
// Dialog Data
	//{{AFX_DATA(CLogerr)
	enum { IDD = IDD_LOGERR };
	CString	m_user;
	CString	m_pass;
	CString	m_purpose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogerr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLogerr)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGERR_H__960CAE82_37B8_11D3_B36C_00001CD5F6A7__INCLUDED_)
