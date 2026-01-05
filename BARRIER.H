#if !defined(AFX_BARRIER_H__3F435C35_899A_418A_9ACD_26D802129748__INCLUDED_)
#define AFX_BARRIER_H__3F435C35_899A_418A_9ACD_26D802129748__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Barrier.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBarrier window

class CBarrier : public CStatic
{
// Construction
public:
    CBarrier();
    HICON m_hIcon;
    void ShowIcon();
    void CloseBarrier();
    void OnStartCloseTimer();
    void OnStartOpenTimer();
    void OpenBarrier();
    void FindIOAddress();
    void DoOpenBarrier();
    void DoCloseBarrier();
    BOOL flagDisable;
    BYTE adamData[128];
    BOOL busy,InProgress,MControl;
    UINT TimerEvent;
    int  minwt;
    int  no, fldBarrierOpen, fldBarrierClose,fldBarrierStatus, iCount;
    int  closeCounter,openCounter;
    long LastWt,iStableWtCounter;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBarrier)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBarrier();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBarrier)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BARRIER_H__3F435C35_899A_418A_9ACD_26D802129748__INCLUDED_)
