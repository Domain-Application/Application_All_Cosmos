// NView.h : interface of the CNView class
//
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INCLUDES()
#include "msflexgrid.h"
#include "labelcontrol.h"
//}}AFX_INCLUDES

#if !defined(AFX_NView_H__01A41CA5_3A95_9713_AA5A_118DB73CFB3E__INCLUDED_)
#define AFX_NView_H__01A41CA5_3A95_9713_AA5A_118DB73CFB3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786) // Suppress identifier truncation warnings (common with STL)

#include "menuDoc.h"
#include "mgrid.h"
#include "cctvview.h"
#include "DynamicLED.h"
#include "Barrier.h"

#define timer_video 1
#define timer_weight 2

#include "tcpServer.h"
#include <afxmt.h>
#include <map>

#define WM_NEW_MESSAGE (WM_USER + 100)

struct ServerInfo {
    int port;
    CString portName;
    BOOL bRunning;
    CWinThread* pThread;
    CTcpServer* pServer;
    CString strBuffer;
    class CNView* pView;
};

class CNView : public CCctvView
{
public: // create from serialization only
    CNView();
    virtual void SetCompanyName(DB *df,int no);
    virtual void OnSetPhotosName();
    virtual void ShowInitFrame();
    virtual void ShowInformation();
    virtual void SetActiveStatus(DB *df,long rec, int type);
    virtual void OnSecond();
    virtual BOOL CopyPhotoToServer(int jpgkey);
    virtual void OnW2(UINT IDD, DB *df);
	CString s;
    int     weight_alt_flag;
    MGRID  fg;
	void OnTimer(UINT nIDEvent);
	CBarrier	m_br2;
	CBarrier	m_br1;
	BOOL startOpenBarrier;

    DECLARE_DYNCREATE(CNView)
    
    CCriticalSection m_csServer;
    std::map<CString, ServerInfo*> m_serverInstances;
    
    void OnCreateTCPServer(const CString& portFieldName);
	BOOL requireOpenBackBarrier;
	int manual_open_barrier1;
    int manual_open_barrier2;
	BOOL gateOpened_By_Security;
	void InitiateDualMode();

public:
    //{{AFX_DATA(CNView)
	enum { IDD = IDD_MENU_FORM };
	CMSFlexGrid	m_grid;
	CMSFlexGrid	m_grid2;
	CListBox	m_list;
	CLabelControl	m_title;
	//}}AFX_DATA

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CNView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual int OnGw();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
		virtual void displaySystemMode();
	//}}AFX_VIRTUAL

// Implementation
public:

protected:

// Generated message map functions
public:
    //{{AFX_MSG(CNView)
	afx_msg void OnGo();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
    afx_msg void OnRadio3();
	afx_msg void OnButton1();
	afx_msg void OnClickMsflexgrid1();
	afx_msg void OnDblClickMsflexgrid1();
	afx_msg void OnMouseDownMsflexgrid1(short Button, short Shift, long x, long y);
	afx_msg LRESULT OnNewMessage(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CNView *pView;
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NView_H__01A41CA5_3A95_9713_AA5A_118DB73CFB3E__INCLUDED_)
