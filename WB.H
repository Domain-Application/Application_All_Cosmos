//{{AFX_INCLUDES()
#include "labelcontrol.h"
//}}AFX_INCLUDES
#if !defined(AFX_WB_H__8B172581_7B26_11D3_B370_00001CD5F6A7__INCLUDED_)
#define AFX_WB_H__8B172581_7B26_11D3_B370_00001CD5F6A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wb.h : header file
//
#include "Parent.h"
/////////////////////////////////////////////////////////////////////////////
// CWb dialog

class CWb : public CParent
{
// Construction
public:
    CWb(UINT IDD,DB *pdf,int Idx,CWnd* pParent = NULL);   // standard constructor
    virtual void OnFtpUpload();
    void OnPressPrint2();
    BOOL IsCanExit();
    void OnPrint();
    void SetOneTimeScreen();
    void ShowInformation();
    void ShowInitFrame(CMSFlexGrid *grid,DB *sub);
    void ReplaceSubGrid(long rec,long irec,CMSFlexGrid *m_grid,DB *sub,int IdxItem);
    void OnSearch();
    void OnAfterPrint();
	virtual void OnNew();
    CString strWM,strWM2,errmsg;
    CString WeightServer;
    char cmd1[10],cmd2[10];
    long current_weight;
    BOOL busy;
// Dialog Data
	//{{AFX_DATA(CWb)
	enum { IDD = IDD_WB };
	CLabelControl	m_wb1;
	CLabelControl	m_wb2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWb)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWb)
	afx_msg void OnPrimeMoverWeighing();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnConfirmWeight();
	afx_msg void OnFirstWt();
	afx_msg void OnSecondWt();
	afx_msg void OnPM1WT();
	afx_msg void OnPM2WT();
	afx_msg void OnSetCompleteTicket();
;	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WB_H__8B172581_7B26_11D3_B370_00001CD5F6A7__INCLUDED_)
