#if !defined(AFX_IT2_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
#define AFX_IT2_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// It1.h : header file
//

#include "fedit.h"
/////////////////////////////////////////////////////////////////////////////
// CIt2 dialog

class CIt2 : public CFEdit
{
// Construction
public:
     CIt2(CWnd* pParent = NULL);   // standard constructor
	 virtual void LoadImages();     
     int iType;
     CString m_vehicle,m_contno,m_tt,msg;
// Dialog Data
     //{{AFX_DATA(CIt2)
     enum { IDD = IDD_IT2 };
           // NOTE: the ClassWizard will add data members here
     //}}AFX_DATA


// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CIt2)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:

        // Generated message map functions
        //{{AFX_MSG(CIt2)
        virtual void OnOK();
        virtual BOOL OnInitDialog();
        afx_msg void OnDestroy();
        afx_msg void OnTimer(UINT nIDEvent);
		afx_msg void OnSnapshots();
        virtual void OnCancel();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_It1_H__79BD2FB0_67E7_4871_8330_07406B8CEF44__INCLUDED_)
