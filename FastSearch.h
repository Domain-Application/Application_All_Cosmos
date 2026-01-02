#if !defined(AFX_FASTSEARCH_H__B013C653_E2BF_4D68_A9AD_8092DA0BF21A__INCLUDED_)
#define AFX_FASTSEARCH_H__B013C653_E2BF_4D68_A9AD_8092DA0BF21A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FastSearch.h : header file
//

#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CFastSearch dialog

class CFastSearch : public CDialog
{
// Construction
public:
	CFastSearch(CWnd* pParent = NULL);   // standard constructor
    void OnFetchData();
    void DisplayTicket();
    long irec,last;
    DB*  df,adoTmp;

// Dialog Data
	//{{AFX_DATA(CFastSearch)
	enum { IDD = IDD_FASTSEARCH };
	CComboBox	m_cb1;
	COleDateTime	m_date;
	CString	m_str;
	BOOL	m_unprint;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFastSearch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    CString str;

	// Generated message map functions
	//{{AFX_MSG(CFastSearch)
	afx_msg void OnAdvanceSearch();
	virtual BOOL OnInitDialog();
	afx_msg void OnDatetimechangeDatetimepicker1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkCombo1();
	virtual void OnOK();
	afx_msg void OnCheck1();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FASTSEARCH_H__B013C653_E2BF_4D68_A9AD_8092DA0BF21A__INCLUDED_)
