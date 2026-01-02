#if !defined(AFX_RESETPASSWORD_H__BE5C3AA1_28FA_4203_A0E4_08340CDE80D0__INCLUDED_)
#define AFX_RESETPASSWORD_H__BE5C3AA1_28FA_4203_A0E4_08340CDE80D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResetPassword.h : header file
//

#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CResetPassword dialog

class CResetPassword : public CDialog
{
// Construction
public:
	CResetPassword(CWnd* pParent = NULL);   // standard constructor
    void RunSql(CString str);

// Dialog Data
	//{{AFX_DATA(CResetPassword)
	enum { IDD = IDD_RESET_DATABASE };
	CProgressCtrl	m_progress;
	CString	m_password;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResetPassword)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResetPassword)
	virtual void OnOK();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESETPASSWORD_H__BE5C3AA1_28FA_4203_A0E4_08340CDE80D0__INCLUDED_)
