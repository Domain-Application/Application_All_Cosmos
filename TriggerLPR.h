#if !defined(AFX_TRIGGERLPR_H__A68318CE_6C0B_4C29_8231_1AF9872AAE86__INCLUDED_)
#define AFX_TRIGGERLPR_H__A68318CE_6C0B_4C29_8231_1AF9872AAE86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TriggerLPR.h : header file
//

#include <jpgpicture.h>
/////////////////////////////////////////////////////////////////////////////
// CTriggerLPR dialog

class CTriggerLPR : public CDialog
{
// Construction
public:
    CTriggerLPR(CWnd* pParent = NULL);   // standard constructor
    void Initial();
    void OnManual(int no);
	void Save2loglpr(const char *plateno1,int no);
    // New method for external data
    void SetExternalData(CString strInfo, CString strImage, CString strImage2 = "");

    int iStep,oldwt,iCounter,busy,lpr_status[9];
    int direction,iTryCount;
    long vehicle_rec[9];
	int no;
	CString s,plateno,plateno1,plateno2;
	CString picPlate,picLPR;
    CRect PreviewRect[9];
    CJpgPicture m_jpg[9];

    // New members
    BOOL m_bUseExternalData;
    CString m_strExternalInfo;
    CString m_strExternalImage;
    CString m_strExternalImage2;

// Dialog Data
	//{{AFX_DATA(CTriggerLPR)
	enum { IDD = IDD_TriggerLPR };
	BOOL	m_stable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTriggerLPR)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTriggerLPR)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnL1();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRIGGERLPR_H__A68318CE_6C0B_4C29_8231_1AF9872AAE86__INCLUDED_)
