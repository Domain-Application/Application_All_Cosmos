#if !defined(AFX_IT1_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
#define AFX_IT1_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// It1.h : header file
//

#include "fedit.h"
/////////////////////////////////////////////////////////////////////////////
// CIt1 dialog

class CIt1 : public CFEdit
{
// Construction
public:
     CIt1(CWnd* pParent = NULL);   // standard constructor
	 virtual void LoadImages();     
	 int iType;
     CString m_contno,m_vehicle,m_tt;

// Dialog Data
     //{{AFX_DATA(CIt1)
     enum { IDD = IDD_IT1 };
           // NOTE: the ClassWizard will add data members here
     //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CIt1)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CIt1)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
    virtual void OnCancel();
	afx_msg void OnSnapshots();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_It1_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
