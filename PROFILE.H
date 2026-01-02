#if !defined(AFX_PROFILE_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
#define AFX_PROFILE_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Profile.h : header file
//

#include "staff.h"
/////////////////////////////////////////////////////////////////////////////
// CProfile dialog

class CProfile : public CStaff
{
// Construction
public:
    CProfile(UINT IDD,DB *pdf,int Idx,CWnd* pParent = NULL);   // standard constructor
    virtual void ShowInformation();

// Dialog Data
    //{{AFX_DATA(CProfile)
    enum { IDD = IDD_PROFILE };
        // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CProfile)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CProfile)
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButton1();
	afx_msg void OnGWST();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_PROFILE_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
