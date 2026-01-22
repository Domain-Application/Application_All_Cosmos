#include "stdafx.h"
#include "MENU.h"
#include "comm.h"
#include "NView.h"
#include "initial.h"
#include "Security.h"
#include "ChangeWeighingType.h"
#include "tcpServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int iStatus = 0;
long release_wt = 30;
int iDOCounter=0;

IMPLEMENT_DYNCREATE(CNView, CCctvView)

BEGIN_MESSAGE_MAP(CNView, CCctvView)
	ON_BN_CLICKED(IDOK, OnGo)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
    ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_RestartCameras, OnRestartCameras)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
    ON_MESSAGE(WM_NEW_MESSAGE, OnNewMessage)
END_MESSAGE_MAP()

CNView *pView = NULL;

CNView::CNView(): CCctvView(app->IDD_DOMVIEW)
{
    weight_alt_flag = -1;
    TotalCameras = 10;
    m_swGrid = FALSE;
    requireOpenBackBarrier = FALSE;
    manual_open_barrier1 = weighbridge.lval(weighbridge_MOB1);
    manual_open_barrier2 = weighbridge.lval(weighbridge_MOB2);
    gateOpened_By_Security = FALSE;
    startOpenBarrier = FALSE;
}

void RecordStatusLog(const CString str)//Ver 2.05
{
	CString strFileName;
	strFileName.Format("status%s.trc", &today[2]);
	FILE* ptr = fopen(strFileName, "a");
	if (ptr) {
		fprintf(ptr, str);
		fclose(ptr);
	}
}

 LRESULT CNView::OnNewMessage(WPARAM wParam, LPARAM lParam)
{
    ServerInfo* pInfo = (ServerInfo*)wParam;
    CString* pStr = (CString*)lParam;
    if (!pInfo || !pStr) return 0;

    CString strMsg = *pStr;
    delete pStr;

    // Use the buffer specific to this server instance
    pInfo->strBuffer += strMsg;

    if (pInfo->strBuffer.GetLength() > 100000)
    {
        pInfo->strBuffer.Empty();
        return 0;
    }

    BOOL bFound = TRUE;
    while (bFound)
    {
        bFound = FALSE;
        CString strPlate, strContainer;
        int result = ProcessAnprXml(pInfo->strBuffer, strPlate, strContainer);

        if (result & ANPR_RESULT_PLATE)
        {
            CString strLog;
            strLog.Format(_T("[%s] License Plate: %s"), pInfo->portName, strPlate);
            RecordStatusLog(strLog);
            if (IsMalaysiaPlate(strPlate))
            {
                // Push to global if needed, similar to security.cpp
            }
            bFound = TRUE;
        }

        if (result & ANPR_RESULT_CONTAINER)
        {
            CString strLog;
            strLog.Format(_T("[%s] Container Full: %s"), pInfo->portName, strContainer);
            RecordStatusLog(strLog);
            if(ValidateISO6346(strContainer))
            {
                push_plateno2.Format("%s", strContainer);
            }
            bFound = TRUE;
        }
    }

    return 0;
}

UINT __cdecl ServerThreadFunc(LPVOID pParam)
{
    ServerInfo* pInfo = (ServerInfo*)pParam;
    if (!pInfo) return 0;

    int port = pInfo->port;
    CNView* pView = pInfo->pView;

    // Initialize sockets for this thread
    if (!AfxSocketInit())
        return 0;

    CTcpServer server;
    pInfo->pServer = &server;

    if (!server.StartServer(port))
    {
        pInfo->pServer = NULL;
        return 0;
    }

    CString data;
    CString portLog;
    portLog.Format("%s_lpr.xml", pInfo->portName);

    // Loop and wait for messages
    while (pInfo->bRunning)
    {
        // Wait for client
        if (!server.AcceptClient())
        {  
            break;
        }

        if (!pInfo->bRunning)
            break;

        DWORD dwReturn;
        HANDLE hFile = CreateFile(portLog, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        while (TRUE)
        {
            data.Empty();
            int len = server.ReceiveData(data);

            if (len <= 0)
            {
                break; // client disconnected
            }

            if (hFile != INVALID_HANDLE_VALUE)
            {
                WriteFile(hFile, data, len, &dwReturn, NULL);
            }
            // Send message to dialog (UI thread)
            if (::IsWindow(pView->m_hWnd))
                ::PostMessage(pView->m_hWnd, WM_NEW_MESSAGE, (WPARAM)pInfo, (LPARAM)new CString(data));
        }
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        
        server.CloseClient();
    }

    server.Close();
    pInfo->pServer = NULL;
    return 0;
}

void CNView::OnTimer(UINT nIDEvent) 
{
    CCctvView::OnTimer(nIDEvent);
}

void CNView::OnSetPhotosName()
{
    for(int i=1; i<=TotalCameras; i++) {
        strPhoto[i].Format("%s\\lpr%2.2i.jpg",app->picFolder, i);
    }
}

void CNView::DoDataExchange(CDataExchange* pDX)
{
    CCctvView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CNView)
	DDX_Control(pDX, IDC_MSFLEXGRID1, m_grid);
	DDX_Control(pDX, IDC_MSFLEXGRID2, m_grid2);
	DDX_Control(pDX, IDC_LIST1, m_list);
	//}}AFX_DATA_MAP
    m_br1.no = 1;
	m_br2.no = 2;

}

void CNView::OnCreateTCPServer(const CString& portFieldName)
{
    CSingleLock lock(&m_csServer, TRUE);

    int newPort = weighbridge.lval(GetDbFieldNo(&weighbridge, (char*)(LPCTSTR)portFieldName));
    if (newPort <= 0) return;

    ServerInfo* pInfo = NULL;
    std::map<CString, ServerInfo*>::iterator it = m_serverInstances.find(portFieldName);
    if (it != m_serverInstances.end())
    {
        pInfo = (*it).second;
    }

    if (pInfo)
    {
        DWORD dwExitCode;
        if (pInfo->pThread && GetExitCodeThread(pInfo->pThread->m_hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE)
        {
            if (newPort == pInfo->port)
            {
                return; // Already running on this port
            }
            else
            {
                // Stop current server for this port field
                pInfo->bRunning = FALSE;

                // Connect a dummy socket to unblock Accept()
                CSocket killerSocket;
                if (killerSocket.Create())
                {
                    killerSocket.Connect(_T("127.0.0.1"), pInfo->port);
                    killerSocket.Close();
                }
                
                WaitForSingleObject(pInfo->pThread->m_hThread, 5000);
                delete pInfo->pThread;
                pInfo->pThread = NULL;
                pInfo->pServer = NULL;
            }
        }
        else
        {
            if (pInfo->pThread) delete pInfo->pThread;
            pInfo->pThread = NULL;
            pInfo->pServer = NULL;
        }
    }
    else
    {
        pInfo = new ServerInfo;
        pInfo->portName = portFieldName;
        pInfo->pView = this;
        pInfo->strBuffer = _T("");
        m_serverInstances[portFieldName] = pInfo;
    }

    pInfo->port = newPort;
    pInfo->bRunning = TRUE;
    pInfo->pThread = AfxBeginThread(ServerThreadFunc, pInfo);
    if (pInfo->pThread)
    {
        pInfo->pThread->m_bAutoDelete = FALSE;
    }
}

void CNView::OnDestroy()
{
    CSingleLock lock(&m_csServer, TRUE);

    for (std::map<CString, ServerInfo*>::iterator it = m_serverInstances.begin(); it != m_serverInstances.end(); ++it)
    {
        ServerInfo* pInfo = (*it).second;
        if (pInfo)
        {
            pInfo->bRunning = FALSE;
            if (pInfo->pThread)
            {
                CSocket killerSocket;
                if (killerSocket.Create())
                {
                    killerSocket.Connect(_T("127.0.0.1"), pInfo->port);
                    killerSocket.Close();
                }
                WaitForSingleObject(pInfo->pThread->m_hThread, 5000);
                delete pInfo->pThread;
            }
            delete pInfo;
        }
    }
    m_serverInstances.clear();

    CCctvView::OnDestroy();
}

void CNView::InitiateDualMode()
{
    int br2_status = weighbridge.lval(weighbridge_BR2), loop2_status = weighbridge.lval(weighbridge_LOOP2);
    int br1_status = weighbridge.lval(weighbridge_BR1), loop1_status = weighbridge.lval(weighbridge_LOOP1);
    if(cweight == 0 && DiData[br2_status-1] && !DiData[loop2_status-1] && DiData[br1_status-1] && !DiData[loop1_status-1] && program4 == 1)
    {
        m_br1.OpenBarrier();	
    }
    else if(cweight == 0 && isWBDualMode() && DiData[br1_status-1] && !DiData[loop1_status-1] && DiData[br2_status-1] && !DiData[loop2_status-1] && program4 == 2)
    {
        m_br2.OpenBarrier();
    }
}

void CNView::OnInitialUpdate()
{
	app->picFolder.Format("c:\\data");
	CreateDirectory(app->picFolder, NULL);
    CCctvView::OnInitialUpdate();
    pView = this;
	SetDlgItemText(IDC_TEXTBOX1,"Initialize...");
    SetTimer(timer_video,250,NULL);
	OnRadio1();
    ShowInitFrame();
    ShowInformation();
    OnCreateTCPServer("ANPR_PORT");
    OnCreateTCPServer("ANPR_PORT2");
}

int CNView::OnGw()
{
    char tmp[80];

    ActualWeight =  cweight;
	weight = adjust_wt(cweight);

	if(cweight==-1) {
		sprintf(tmp,"No signal");
	}
	else {
		sprintf(tmp,"%5.5ld kg",weight);
	}
    SetDlgItemText(IDC_TEXTBOX1, tmp );

    flagNoZero = (IsZeroWt == FALSE);
	
    HWND hWnd = ::GetDlgItem( GetSafeHwnd(), IDC_ZERO_FLAG);
    UINT BST = flagNoZero ? BST_UNCHECKED : BST_CHECKED;
    ::SendMessage(hWnd, BM_SETCHECK, BST, 0L);
	
	long var =  20;
	long count = 5;
    flagStable = IsHistoryWtStable(var, count);
    HWND hWnd2 = ::GetDlgItem( GetSafeHwnd(), IDC_STABLE_FLAG);
    BST = flagStable ? BST_CHECKED : BST_UNCHECKED;
    ::SendMessage(hWnd2, BM_SETCHECK, BST, 0L);
    iDOCounter++;
    int weight_status = weighbridge.lval(weighbridge_ALT);
    if(weight_status) {

        int wflag = cweight > 30;
        if(wflag != weight_alt_flag) {
            weight_alt_flag = wflag;
            WriteCoil(weight_status, wflag);
        }
	}
    InitiateDualMode();
    return TRUE;
}


BOOL CNView::PreTranslateMessage(MSG* pMsg)
{
    if( m_ToolTipIsActive) {
		m_ToolTip.RelayEvent(pMsg);
	}
    UINT nChar = pMsg->wParam;

	if (pMsg->message == WM_CHAR)	{
        if(nChar==32 && lastkey==32) {
			GwstPercent = GwstWeight = 0;
        }
        lastkey = nChar;
        if(nChar==32) {
            return TRUE;
		}
	}
	else if (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_KEYDOWN)	{
        if(nChar==VK_RETURN || nChar==VK_F5) {
            OnButton1();
            return TRUE;
        }
	} 
    return CCctvView::PreTranslateMessage(pMsg);
}

void CNView::SetCompanyName(DB *df,int no)
{
}

void CNView::OnRadio1()
{
}

void CNView::OnRadio2()
{
}

void CNView::OnRadio3()
{
}

void CNView::OnGo() 
{
}

void CNView::ShowInitFrame()
{
    fg.Initial(&m_grid,NULL,2,1.1f);
    int screen_width = app->longWindowWidth;
    if(screen_width >= 1900) fg.ratio = 1.23f;

    fg.SetTitle("No.",450,4);
    fg.SetTitle("Lorry.",1650,1);
    fg.SetTitle("AC Name",3600,1);
    fg.SetTitle("Product",2600,1);

    fg.SetTitle("Date/Time In",1500,4);
    fg.SetTitle("Weight (kg)",1100,7);
    fg.SetTitle("Ticket",1,1);

    fg.SetTitle("W. Type",870,4);
    fg.SetTitle("Remarks", 3000, 1);
	
	MGRID fg2(&m_grid2, NULL, 0, 5);
    fg2.SetTitle("No", 500, 1);
	fg2.SetTitle("Ticket",1300,1);
    fg2.SetTitle("Vehicle", 1500);
    fg2.SetTitle("Date/Time In", 1600);
    fg2.SetTitle("Weight (kg)", 1100, 7);
}

void CNView::SetActiveStatus(DB *df,long rec, int type)
{
	if(type == 1)
	{
		MGRID g(&m_grid,df,rec);
		g.SetText(df,0,LORRY,ACCODE | 0x100,PT | 0x100,TIMEIN,FIRST,1,-1);
		
		int fld = GetDbFieldNo(df ,_szPrintCounter);
		int no = df->lval(fld);
		if(no==1) {
			g.SetText("Receiving");
		}
		else if(no==2) {
			g.SetText("Despatch");
		}
		else {
			g.SetText("Auto");
		}
		g.SetText(df, REM, -1);
		return;
	}
    if(type == 2 )
	{
		MGRID g(&m_grid2, df, rec);
		g.show(rec);
		g.SetText(df, 0, 1 , LORRY, TIMEIN, FIRST, -1);
		return;
	}
}

void CNView::ShowInformation()
{
    if(m_swGrid) return;
    m_swGrid = TRUE;
	//brian//20251231
	CString strSQL;
	strSQL.Format("select * from FIRST where container is null ORDER BY TIMEIN ASC");
    first.ReOpen(strSQL);
    long srec = read_sumindex(0,&first);
    m_grid.SetRedraw(FALSE);
    m_grid.SetRows(srec+1);
    for(long rec=1; rec<=srec; rec++) {
        readrec(rec,&first);
        m_grid.SetRow(rec);
        SetActiveStatus(&first,rec, 1);
    } 
    m_grid.SetRedraw(TRUE);
    strSQL.Format("select * from FIRST where container is not null ORDER BY TIMEIN ASC");
    first.ReOpen(strSQL);
    srec = read_sumindex(0,&first);
    m_grid2.SetRedraw(FALSE);
    m_grid2.SetRows(srec+1);
    for(rec=1; rec<=srec; rec++) {
        readrec(rec,&first);
        m_grid2.SetRow(rec);
        SetActiveStatus(&first,rec, 2);
    } 
    m_grid2.SetRedraw(TRUE);
    CString str;
    str.Format("%ld",srec);
    SetDlgItemText(IDC_TOTAL_VEHICLE_IN_YARD,str);
    strSQL.Format("select Final_Count from Container_In_Yard");
    SetDlgItemText(IDC_CONT_IN_YARD, SqlText(strSQL));
    strSQL.Format("select count(*) from first where container is null");
    SetDlgItemText(IDC_TANK_IN_YARD, SqlText(strSQL));
    m_swGrid = FALSE;
}

void CNView::OnSecond()
{
}

void CNView::OnW2(UINT IDD, DB *df)
{
}

BOOL CNView::CopyPhotoToServer(int jpgkey)
{

	int fld  = GetDbFieldNo(&logo,"BACKUP_PHOTO_FOLDER");
	if(fld==0) return FALSE;

	CString s = logo.dat[fld];
	s.TrimRight();
	int len = s.GetLength();
	if(len==0) return FALSE;

	char ch = s.GetAt(len-1);
	if(!(ch=='/' || ch=='\\')) {
		s += "\\";
	}

	CString k;
	k.Format("%8.8i",jpgkey);

	s+= k;

	BOOL flag = CreateDirectory(s,NULL);
	if(flag==FALSE) return FALSE;

    for(int i=1; i<=TotalCameras; i++) {

        CString p = strPhoto[i];

		CString file = FileOnly(strPhoto[i]);
		k.Format("%s\\%s",s,file);
		flag = CopyFile(p, k, FALSE);
		if(flag==FALSE) return FALSE;
		DeleteFile(p);
	}

	return TRUE;
}

//FIXME: disabling the button causing unable to continue pressing it
void log_proceeding(const char *msg)
{
    FILE *ptr = log_file("trc");
    if(!ptr) return;

    fprintf(ptr,"%s %s\n", nowMs(), msg);

    fclose(ptr);
}

void CNView::OnButton1() 
{
	log_proceeding("start");
	SetDlgItemText( IDC_BUTTON1, "Proceeding..");
	// GetDlgItem( IDC_BUTTON1 )->EnableWindow(FALSE);
	OnSetPhotosName();

	CSecurity p;

    p.df = &first;
	p.DoModal();

	ShowInformation();
	SetDlgItemText( IDC_BUTTON1, "F5->  Weighing Process");
	// GetDlgItem( IDC_BUTTON1 )->EnableWindow(TRUE);
	log_proceeding("End");

}

BEGIN_EVENTSINK_MAP(CNView, CCctvView)
    //{{AFX_EVENTSINK_MAP(CNView)
	ON_EVENT(CNView, IDC_MSFLEXGRID1, -600 /* Click */, OnClickMsflexgrid1, VTS_NONE)
	ON_EVENT(CNView, IDC_MSFLEXGRID1, -601 /* DblClick */, OnDblClickMsflexgrid1, VTS_NONE)
	ON_EVENT(CNView, IDC_MSFLEXGRID1, -605 /* MouseDown */, OnMouseDownMsflexgrid1, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CNView, IDC_MSFLEXGRID2, -605 /* MouseDown */, OnMouseDownMsflexgrid2, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNView::OnClickMsflexgrid1() 
{
	// TODO: Add your control notification handler code here
	
}

void CNView::OnDblClickMsflexgrid1() 
{
	// TODO: Add your control notification handler code here
	
}

void CNView::OnMouseDownMsflexgrid1(short Button, short Shift, long x, long y) 
{
	CString s;

	if(Button !=2) return;
	RightClickGrid(m_grid, 6, 1);	
	
}

void CNView::OnMouseDownMsflexgrid2(short Button, short Shift, long x, long y) 
{
	CString s;
	
	if(Button !=2) return;
	RightClickGrid(m_grid2, 1, 2);
	
}

void CNView::RightClickGrid(CMSFlexGrid &grid, int idCol, int vehicleCol)
{
    CString s;
	
    int row = grid.GetMouseRow();
    if(row == 0) return;
	
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_MFIRST_MULTI_WEIGHING));
	
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);
	
    if(UserLevel() < 9)
    {
        pPopup->RemoveMenu(ID_VEHICLEINYARD_EDIT, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_VEHICLEINYARD_PRINT, MF_BYCOMMAND);
        pPopup->RemoveMenu(ID_VEHICLEINYARD_DELETE, MF_BYCOMMAND);
    }
	
    POINT pt;
    GetCursorPos(&pt);
	
    UINT SelectionMade = pPopup->TrackPopupMenu(
        TPM_LEFTALIGN |
        TPM_LEFTBUTTON |
        TPM_NONOTIFY |
        TPM_RETURNCMD,
        pt.x, pt.y, this);
	
    if(SelectionMade == 0) return;
	
    CString str   = grid.GetTextMatrix(row, idCol);
    CString vehno = grid.GetTextMatrix(row, vehicleCol);
	
    lset(first.dat[1], str);
	
    if(SelectionMade == ID_VEHICLEINYARD_EDIT)
    {
        first.Enq(first.dat[1], TRUE, 1);
    }
    else if(SelectionMade == ID_VEHICLEINYARD_PRINT)
    {
        s.Format("Change Weighing Type for vehicle no.:%s", vehno);
		
        if(MessageBox(s, "Confirm it", MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            qd(first.dat[1], &first);
			
            CChangeWeighingType dlg;
            dlg.df = &first;
            dlg.m_vehicle = vehno;
            dlg.DoModal();
        }
    }
    else if(SelectionMade == ID_VEHICLEINYARD_DELETE)
    {
        s.Format("Delete vehicle no:%s in yard", vehno);
		
        if(MessageBox(s, "Confirm it", MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            s.Format("Delete from [%s] where [%s] = '%s'",
				first.name,
				first.fieldName[1],
				first.dat[1]);
			
            RunSql(s);
        }
    }
    else if(SelectionMade == ID_VEHICLEINYARD_VIEW)
    {
        first.m_ViewOnly = TRUE;
        first.Enq(first.dat[1], FALSE, 1);
    }
    else
    {
        return;
    }
	
    ShowInformation();
}
