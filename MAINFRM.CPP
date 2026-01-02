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
}

void CMainFrame::OnOpenOtherFiles()
{
    CString WeightServer = profile.dat[Pro_WtServer];
    WeightServer.TrimRight();

    if(WeightServer.GetLength()==0) {
        bCommOpen = TRUE;

        HWND hWnd = GetSafeHwnd();
        BOOL bOpen = SetCommThread(hWnd,FALSE);
        if(bOpen==FALSE) {
        }
    }
    else {
        UINT flag = GetCurrentDirectory(1000,lpBuffer);
        if(flag) {
            if(!WeightServer.CompareNoCase(app->m_strComputerName)) {
                char szPort[20];
                CString errmsg;
                strcpy(szPort,"W1");
                flag = ClientRequest(WeightServer, szPort , errmsg);
                    if(flag==-1) {
                    CString lpFile = lpBuffer;
                    lpFile += "\\wmtcp.exe";
                    HINSTANCE flag = ShellExecute( NULL, "OPEN", lpFile, NULL, NULL, SW_SHOW);
                }
            }
        }
    }
}

void CMainFrame::OnClose()
{
    CloseCommThread();
    CMyFrame::OnClose();
}

