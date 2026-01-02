#if !defined(AFX_LORRY_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
#define AFX_LORRY_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Lorry.h : header file
//

#include "staff.h"
#include "labelcontrol.h"
/////////////////////////////////////////////////////////////////////////////
// CLorry dialog

class CLorry : public CStaff
{
// Construction
public:
    CLorry(UINT IDD,DB *pdf,int Idx,CWnd* pParent = NULL);   // standard constructor
    virtual void ShowInformation();
    CString strWM,strWM2,errmsg;
    char cmd1[10],cmd2[10];
    long current_weight;
    BOOL busy;

// Dialog Data
    //{{AFX_DATA(CLorry)
    enum { IDD = IDD_LORRY };
	CLabelControl	m_wb1;
	CLabelControl	m_wb2;
        // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CLorry)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CLorry)
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButton1();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_LORRY_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
