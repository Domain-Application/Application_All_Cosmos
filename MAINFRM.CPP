// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "menu.h"
#include "MainFrm.h"
#include "Staff.h"
#include "about.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

extern void CheckUserRight(DB *user,int key);
IMPLEMENT_DYNCREATE(CMainFrame, CMyFrame)

BEGIN_MESSAGE_MAP(CMainFrame, CMyFrame)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_INITMENU()
    ON_COMMAND(ID_USER, OnUser)
    ON_COMMAND(ID00031, OnPassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    m_System.Format("%s Weighing System Ver 1.00", title);
    app->m_auto_purge_logfiles_after_days = 30;
    bCommOpen = TRUE;
}

void CMainFrame::OnOpenOtherFiles()
{
    #ifdef _DEBUG
    ADAMSimulator = 1;
    #else
    ADAMSimulator = 2;
    #endif
    if (app->m_strComputerName.CompareNoCase("domain-chan") == 0 ||
		app->m_strComputerName.CompareNoCase("ooi") == 0 ||
		app->m_strComputerName.CompareNoCase("kelvin") == 0 ||
		app->m_strComputerName.CompareNoCase("brian-pc") == 0 ||
		app->m_strComputerName.CompareNoCase("torry") == 0
		)
	{
		ADAMSimulator = 1;
	}
    ADAM_flag = 0;
    current_indicator = program4;
    HWND hWnd = GetSafeHwnd();
    BOOL bOpen = SetCommThread(hWnd,FALSE);
}

void CMainFrame::OnClose()
{
    CloseCommThread();
    CMyFrame::OnClose();
}

