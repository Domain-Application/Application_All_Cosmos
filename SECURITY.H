#if !defined(AFX_SECURITY_H__4F0AA5B4_29BB_416B_A1D2_697DCEF5875E__INCLUDED_)
#define AFX_SECURITY_H__4F0AA5B4_29BB_416B_A1D2_697DCEF5875E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Security.h : header file
//

#include "DynamicLED.h"
#include <jpgpicture.h>
#include "Barrier.h"
#include "FM20Dlg.h"
#include "Sensor.h"

#define WM_PHOTO_PROCESSED (WM_USER + 500)

struct PhotoThreadParam {
    HWND hWnd;
    int TotalCameras;
    char picFolder[MAX_PATH];
    char strPhoto[10][MAX_PATH]; 
};

/////////////////////////////////////////////////////////////////////////////
// CSecurity dialog

class CSecurity : public CFM20Dlg
{
// Construction
public:
	CSecurity(CWnd* pParent = NULL);   // standard constructor
	void SaveImages(DB *df,const char *JPGKEY);
    void OnUpdateLorry();
    BOOL OnFirstWt(int type, BOOL LPR_Status, BOOL CNR_Status);
    BOOL OnSecond(BOOL LPR_Status, BOOL CNR_Status);
    void OnGW();
	void OnOpenBarrier(UINT ID);
	void OnProceed();
	void UpdatePrimeMover1stWeight();
	void UpdatePrimeMover2ndWeight();
	void UpdatePrimeMoverWeighingData(BOOL canAssign2ndPrimeMover);
    void ProcessCameraRequest(int nIndex);

	CSensor m_S1;
    CSensor m_S2;
	void SetupIcons();
    CRect PreviewRect[9];
    CJpgPicture m_jpg[9];
	int jpgkey, flagStable, iCounter, iBarrier;
    DB  *df;
	CString s;
	BOOL isAutoMode;
	BOOL ManualModeActive();
	BOOL ManualOpenFrontBarrier, ManualOpenBackBarrier;

// Dialog Data
	//{{AFX_DATA(CSecurity)
	enum { IDD = IDD_SECURITY };
	CProgressCtrl	m_progress;
	CBarrier	m_br2;
	CBarrier	m_br1;
	CStatic	m_tl2;
	CStatic	m_tl1;
    CStatic m_led1;
    CStatic m_led2;
	CString	m_vehicle;
	CString	m_contno;
	CString m_bin;
	//}}AFX_DATA
    CDynamicLED m_led[9];
	BOOL isLPR_OK, isCNR_OK;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSecurity)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSecurity)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	afx_msg void OnRadio1(BOOL LPR_Status, BOOL CNR_Status);
	afx_msg void OnRadio2(BOOL LPR_Status, BOOL CNR_Status);
	virtual void OnOK();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnTakePhotos();
	afx_msg void OnPaint();
	afx_msg void OnRadio3();
	afx_msg void OnRadio4();
	afx_msg void OnButton4();
	afx_msg void OnButton6();
	afx_msg void OnButton7();
	afx_msg void OnButton8();
    afx_msg void OnPic01();
    afx_msg void OnPic02();
    afx_msg void OnPic03();
	afx_msg void OnButton5();
    afx_msg LRESULT OnPhotoProcessed(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECURITY_H__4F0AA5B4_29BB_416B_A1D2_697DCEF5875E__INCLUDED_)
