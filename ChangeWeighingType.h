#if !defined(AFX_CHANGEWEIGHINGTYPE_H__CF1887D0_14B6_41BA_A7E0_97E3FE9981B4__INCLUDED_)
#define AFX_CHANGEWEIGHINGTYPE_H__CF1887D0_14B6_41BA_A7E0_97E3FE9981B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeWeighingType.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeWeighingType dialog

class CChangeWeighingType : public CDialog
{
// Construction
public:
	CChangeWeighingType(CWnd* pParent = NULL);   // standard constructor
	DB *df;
    CString g,s;

// Dialog Data
	//{{AFX_DATA(CChangeWeighingType)
	enum { IDD = IDD_ChangeWeighingType };
	CString	m_vehicle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeWeighingType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeWeighingType)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEWEIGHINGTYPE_H__CF1887D0_14B6_41BA_A7E0_97E3FE9981B4__INCLUDED_)
