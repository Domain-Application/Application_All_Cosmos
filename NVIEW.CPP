// NView.cpp : implementation of the CNView class
//

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

/////////////////////////////////////////////////////////////////////////////
// CNView

IMPLEMENT_DYNCREATE(CNView, CCctvView)

BEGIN_MESSAGE_MAP(CNView, CCctvView)
    //{{AFX_MSG_MAP(CNView)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CNView *pView = NULL;
/////////////////////////////////////////////////////////////////////////////
// CNView construction/destruction

CNView::CNView()
    : CCctvView(app->IDD_DOMVIEW)
{
    //{{AFX_DATA_INIT(CNView)
	//}}AFX_DATA_INIT
	// TODO: add construction code here
    weight_alt_flag = -1;
    TotalCameras = 3;
    m_swGrid = FALSE;
    m_pTcpServer = NULL;
    m_pServerThread = NULL;
    m_nCurrentPort = -1;
    m_strMsgBuffer = _T("");
    requireOpenBackBarrier = FALSE;
    manual_open_barrier1 = weighbridge.lval(weighbridge_MOB1);
    manual_open_barrier2 = weighbridge.lval(weighbridge_MOB2);
    gateOpened_By_Security = FALSE;
    startOpenBarrier = FALSE;
}

CString GetXmlValue(const CString& strXml, const CString& strTag)
{
    CString strStartTag = "<" + strTag + ">";
    CString strEndTag = "</" + strTag + ">";
    int nStart = strXml.Find(strStartTag);
    if (nStart == -1) return "";
    nStart += strStartTag.GetLength();
    int nEnd = strXml.Find(strEndTag, nStart);
    if (nEnd == -1) return "";
    return strXml.Mid(nStart, nEnd - nStart);
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

// Helper function to validate ISO 6346 container number
BOOL ValidateISO6346(const CString& strContainer)
{
    if (strContainer.GetLength() != 11)
        return FALSE;

    CString strUpper = strContainer;
    strUpper.MakeUpper();

    // Calculate check digit
    // Mapping: 0-9 -> 0-9
    // A=10, B=12, ..., Z=38 (skip 11, 22, 33)
    int charValues[26] = {
        10, 12, 13, 14, 15, 16, 17, 18, 19, 20, // A-J
        21, 23, 24, 25, 26, 27, 28, 29, 30, 31, // K-T
        32, 34, 35, 36, 37, 38                  // U-Z
    };

    int sum = 0;
    for (int i = 0; i < 10; ++i)
    {
        char c = strUpper[i];
        int val = 0;

        if (c >= '0' && c <= '9')
            val = c - '0';
        else if (c >= 'A' && c <= 'Z')
            val = charValues[c - 'A'];
        else
            return FALSE; // Invalid character

        // Weight is 2^i
        int weight = 1 << i; 
        sum += val * weight;
    }

    int checkDigit = sum % 11;
    if (checkDigit == 10)
        checkDigit = 0; // ISO standard rule for remainder 10

    int lastDigit = strUpper[10] - '0';
    // If the 11th char is not a digit, it's invalid (standard requires it to be a digit)
    if (strUpper[10] < '0' || strUpper[10] > '9')
        return FALSE;

    return (lastDigit == checkDigit);
}

LRESULT CNView::OnNewMessage(WPARAM wParam, LPARAM lParam)
{
    CString* pStr = (CString*)lParam;
    CString strMsg = *pStr;
    delete pStr; // Clean up the received object immediately

    // Append new data to the buffer
    m_strMsgBuffer += strMsg;

    // Safety check: prevent buffer from growing too large (e.g., 100KB)
    if (m_strMsgBuffer.GetLength() > 100000)
    {
        m_strMsgBuffer.Empty();;
        return 0;
    }
    BOOL bFound = TRUE;
    while (bFound)
    {
        bFound = FALSE;

        // Scenario 1: License Plate
        CString strStartTag = _T("<licensePlate>");
        CString strEndTag = _T("</licensePlate>");
        int nStart = m_strMsgBuffer.Find(strStartTag);
        if (nStart != -1)
        {
            int nEnd = m_strMsgBuffer.Find(strEndTag, nStart);
            if (nEnd != -1)
            {
                int nValStart = nStart + strStartTag.GetLength();
                CString strVal = m_strMsgBuffer.Mid(nValStart, nEnd - nValStart);
                if (!strVal.IsEmpty())
                {
                    CString strMsg;
                    strMsg.Format(_T("License Plate: %s"), strVal);
                    RecordStatusLog(strMsg);
                    #ifdef _DEBUG
                    push_plateno2.Format("TEST123");
                    #endif //_DEBUG
                }
                // Remove this message from buffer
                m_strMsgBuffer.Delete(nStart, (nEnd + strEndTag.GetLength()) - nStart);
                bFound = TRUE;
                continue; // Restart loop to check for more
            }
        }

        // Scenario 2: Container Number
        // We need both Main and Sub to be present to process
        CString strMainStart = _T("<containerMainNum>");
        CString strMainEnd = _T("</containerMainNum>");
        CString strSubStart = _T("<containerSubNum>");
        CString strSubEnd = _T("</containerSubNum>");

        int nMainStart = m_strMsgBuffer.Find(strMainStart);
        int nSubStart = m_strMsgBuffer.Find(strSubStart);

        if (nMainStart != -1 && nSubStart != -1)
        {
            int nMainEnd = m_strMsgBuffer.Find(strMainEnd, nMainStart);
            int nSubEnd = m_strMsgBuffer.Find(strSubEnd, nSubStart);

            if (nMainEnd != -1 && nSubEnd != -1)
            {
                CString strContainerMain = GetXmlValue(m_strMsgBuffer, "containerMainNum");
                CString strContainerSub = GetXmlValue(m_strMsgBuffer, "containerSubNum");
                
                if (!strContainerMain.IsEmpty() && !strContainerSub.IsEmpty())
                {
                    CString strContainerFull = strContainerMain + strContainerSub;
                    CString strMsg;
                    strMsg.Format(_T("Container Full: %s"), strContainerFull);
                    RecordStatusLog(strMsg);
                    if(ValidateISO6346(strContainerFull))
                    {
                        push_plateno2.Format("%s", strContainerFull);
                    }
                    int nMainS = m_strMsgBuffer.Find(strMainStart);
                    int nMainE = m_strMsgBuffer.Find(strMainEnd, nMainS);
                    m_strMsgBuffer.Delete(nMainS, (nMainE + strMainEnd.GetLength()) - nMainS);
                    
                    int nSubS = m_strMsgBuffer.Find(strSubStart);
                    int nSubE = m_strMsgBuffer.Find(strSubEnd, nSubS);
                    m_strMsgBuffer.Delete(nSubS, (nSubE + strSubEnd.GetLength()) - nSubS);
                    
                    bFound = TRUE;
                    continue;
                }
            }
        }
    }

    return 0;
}

struct ServerThreadParam
{
    int port;
    CNView* pView;
};

UINT __cdecl ServerThreadFunc(LPVOID pParam)
{
    ServerThreadParam* pThreadParam = (ServerThreadParam*)pParam;
    int port = pThreadParam->port;
    CNView* pView = pThreadParam->pView;
    delete pThreadParam; 

    // Initialize sockets for this thread
    if (!AfxSocketInit())
        return 0;

    CTcpServer server;
    pView->m_pTcpServer = &server;

    if (!server.StartServer(port))
    {
        pView->m_pTcpServer = NULL;
        return 0;
    }

    CString data;

    // Loop and wait for messages
    while (TRUE)
    {
        // Wait for client
        if (!server.AcceptClient())
        {  
            // AcceptClient returns FALSE if the socket is closed (e.g. OnDestroy)
            break;
        }

        if (!pView->m_bServerRunning)
            break;

        CString cFilename = "lpr.xml";
        DWORD dwReturn;
        HANDLE hFile = CreateFile(cFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
                BOOL flagWrite = WriteFile(hFile, data, len, &dwReturn, NULL);
            }
            // Send message to dialog (UI thread)
            if (::IsWindow(pView->m_hWnd))
                ::PostMessage(pView->m_hWnd, WM_NEW_MESSAGE, len, (LPARAM)new CString(data));
        }
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
        }
        
        // Client disconnected, close client socket and wait for next one
        server.CloseClient();
    }

    server.Close();
    pView->m_pTcpServer = NULL;
    return 0;
}

void CNView::OnOpenOppositeBarrier()
{
    int br2_status = weighbridge.lval(weighbridge_BR2), loop2_status = weighbridge.lval(weighbridge_LOOP2);
    int br1_status = weighbridge.lval(weighbridge_BR1), loop1_status = weighbridge.lval(weighbridge_LOOP1);
    
	if(cweight == 0 && DiData[br2_status-1] && !DiData[loop2_status-1] && program4 == 1 && !gateOpened_By_Security)
	{
		m_br1.OpenBarrier();	
	}
    else if(cweight == 0 && DiData[br1_status-1] && !DiData[loop1_status-1] && program4 == 2 && !gateOpened_By_Security)
    {
        m_br2.OpenBarrier();
    }
    else if(gateOpened_By_Security && cweight == 0 && program4 == 1)
    {
        m_br2.OpenBarrier();
        m_br1.CloseBarrier();
    }
    else if(isWBDualMode() && gateOpened_By_Security && cweight == 0 && program4 == 2)
    {
        m_br1.OpenBarrier();
        m_br2.CloseBarrier();
    }
    startOpenBarrier = FALSE;
    gateOpened_By_Security = FALSE;
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

void CNView::OnCreateTCPServer()
{
    CString msg;
	
    
    int newPort = weighbridge.lval(GetDbFieldNo(&weighbridge, "ANPR_PORT"));

    if (m_pServerThread)
    {
        DWORD dwExitCode;
        GetExitCodeThread(m_pServerThread->m_hThread, &dwExitCode);

        if (dwExitCode == STILL_ACTIVE)
        {
            if (newPort == m_nCurrentPort)
            {
                // AfxMessageBox(_T("Server is already running on this port."));
                return;
            }
            else
            {
                // Stop current server
                m_bServerRunning = FALSE;

                // Connect a dummy socket to unblock Accept()
                CSocket killerSocket;
                if (killerSocket.Create())
                {
                    killerSocket.Connect(_T("127.0.0.1"), m_nCurrentPort);
                    killerSocket.Close();
                }
                
                // Wait for thread to exit
                WaitForSingleObject(m_pServerThread->m_hThread, 5000);
                delete m_pServerThread;
                m_pServerThread = NULL;
                m_pTcpServer = NULL;
            }
        }
        else
        {
            // Thread finished
            delete m_pServerThread;
            m_pServerThread = NULL;
            m_pTcpServer = NULL;
        }
    }

    ServerThreadParam* pParam = new ServerThreadParam;
    pParam->port = newPort;
    pParam->pView = this;

    m_bServerRunning = TRUE;
    // Start background listener thread
    m_pServerThread = AfxBeginThread(ServerThreadFunc, pParam);
    if (m_pServerThread)
    {
        m_pServerThread->m_bAutoDelete = FALSE; // We want to wait for it
        m_nCurrentPort = newPort;
    }
}

void CNView::OnDestroy()
{
    m_bServerRunning = FALSE;

    if (m_pServerThread)
    {
        // Connect a dummy socket to unblock Accept()
        CSocket killerSocket;
        if (killerSocket.Create())
        {
            killerSocket.Connect(_T("127.0.0.1"), m_nCurrentPort);
            killerSocket.Close();
        }

        WaitForSingleObject(m_pServerThread->m_hThread, 5000); // Wait up to 5 seconds
        delete m_pServerThread;
        m_pServerThread = NULL;
    }
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
	app->picFolder.Format("c:\\data\\vance");
	CreateDirectory(app->picFolder, NULL);


    CCctvView::OnInitialUpdate();

    pView = this;
	SetDlgItemText(IDC_TEXTBOX1,"Initialize...");
	displaySystemMode();//brian//2025-12-22//add display manual mode or dual mode
    SetTimer(timer_video,250,NULL);
	OnRadio1();
    ShowInitFrame();
    ShowInformation();
    OnCreateTCPServer();
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
    int mod = iDOCounter % 4;
    if( mod==0 ) {
        int flag2 = readADAMData(DoData,1);
//        ShowDO();
    }
    else {
        int flag2 = readADAMData(DiData,2);

        mod = CheckManualOpenBarrier(iDOCounter,m_list);
		if(mod) {
		}

//        ShowDI();
    }


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
    fg.SetTitle("Vehicle/Container No.",1650,1);
    fg.SetTitle("AC Name",3600,1);
    fg.SetTitle("Product",2600,1);

    fg.SetTitle("Prime Mover",1200,1);
    fg.SetTitle("Date/Time In",1500,4);
    fg.SetTitle("Weight (kg)",1100,7);
    fg.SetTitle("Ticket",1,1);

    fg.SetTitle("W. Type",870,4);
    fg.SetTitle("Remarks", 3000, 1);
	fg.SetTitle("Bin", 2000, 1);

	MGRID fg2(&m_grid2, NULL, 0, 7);
    fg2.SetTitle("No", 500, 1);
    fg2.SetTitle("Bin No.", 1200);
	fg2.SetTitle("Ticket",1300,1);
    fg2.SetTitle("Lorry Carry In", 1500);
    fg2.SetTitle("Date/Time In", 1600);
    fg2.SetTitle("Lorry Carry Out", 1500);
	fg2.SetTitle("Date/Time Out", 1500);
}

void CNView::SetActiveStatus(DB *df,long rec)
{
	if(!stricmp(df->name, "FIRST"))
	{
		MGRID g(&m_grid,df,rec);
		g.SetText(df,0,LORRY,ACCODE | 0x100,PT | 0x100, PM1NO,TIMEIN,FIRST,1,-1);
		
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
		g.SetText(df, REM,BIN | 0x100, -1);
		return;
	}
}

void CNView::ShowInformation()
{
    if(m_swGrid) return;
    m_swGrid = TRUE;
	//brian//20251231
	CString strSQL;
	strSQL.Format("select * from FIRST ORDER BY TIMEIN ASC");
    first.ReOpen(strSQL);
    long srec = read_sumindex(0,&first);
    m_grid.SetRedraw(FALSE);
    m_grid.SetRows(srec+1);
    for(long rec=1; rec<=srec; rec++) {
        readrec(rec,&first);
        m_grid.SetRow(rec);
        SetActiveStatus(&first,rec);
    } 
    m_grid.SetRedraw(TRUE);
    CString str;
    str.Format("%ld",srec);
    SetDlgItemText(IDC_TOTAL_VEHICLE_IN_YARD,str);
    strSQL.Format("select Final_Count from Container_In_Yard");
    SetDlgItemText(IDC_CONT_IN_YARD, SqlText(strSQL));
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
	int row = m_grid.GetMouseRow();
	if(row==0) return;
	//if(UserLevel()<9) return; //BRIAN//TEST


	
//	if(!app->SecondUserPassword) return;
	
	CMenu menu;
	int pMenuID = 0;
	
    VERIFY( menu.LoadMenu(IDR_MFIRST_MULTI_WEIGHING) );
	CMenu* pPopup = menu.GetSubMenu(pMenuID);
	ASSERT(pPopup != NULL);

	if (UserLevel()<9)
	{
      pPopup->RemoveMenu(ID_VEHICLEINYARD_EDIT, MF_BYCOMMAND);
      pPopup->RemoveMenu(ID_VEHICLEINYARD_PRINT, MF_BYCOMMAND);
	  pPopup->RemoveMenu(ID_VEHICLEINYARD_DELETE, MF_BYCOMMAND);
	}
	POINT pp;
	GetCursorPos(&pp);
	UINT SelectionMade = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY |TPM_RETURNCMD,pp.x,pp.y,this);
	pPopup->DestroyMenu();
	if(SelectionMade==0) return;

	s.Format("Button %i Shift %i  x %i y %i selection %i", Button, Shift, x, y, SelectionMade);
	TRACE("%s\n",s);
    CString str = m_grid.GetTextMatrix(row, 7);
    CString vehno = m_grid.GetTextMatrix(row, 1);
    lset(first.dat[1], str);
	
    if(SelectionMade==ID_VEHICLEINYARD_EDIT) {
        lset(first.dat[1], str);
        first.Enq(first.dat[1], TRUE, 1);
	}
    else if(SelectionMade==ID_VEHICLEINYARD_PRINT) {
        s.Format("Change Weighing Type for vehicle no.:%s", vehno);
		UINT ID = MessageBox(s,"Confirm it", MB_ICONQUESTION | MB_YESNO);
		if(ID==IDYES) {
             lset(first.dat[1], str);
             qd(first.dat[1], &first);

             CChangeWeighingType h;
			 h.df = &first;
			 h.m_vehicle = vehno;
             h.DoModal();
		}
    }
    else if(SelectionMade==ID_VEHICLEINYARD_DELETE) {
        s.Format("Delete vehicle no:%s in yard", vehno);
		UINT ID = MessageBox(s,"Confirm it", MB_ICONQUESTION | MB_YESNO);
		if(ID==IDYES) {
			s.Format("Delete from [%s] where [%s] = '%s'", 
                first.name, first.fieldName[1], first.dat[1]);
			RunSql(s);
		}
	}
	if(SelectionMade==ID_VEHICLEINYARD_VIEW) {
        lset(first.dat[1], str);
		first.m_ViewOnly=TRUE;
        first.Enq(first.dat[1], FALSE, 1);
	}
	else {
		return;
	}
	ShowInformation();	
	
}

void CNView::displaySystemMode() //brian//2025-12-22//add display manual mode or dual mode
{
    CString strSQL,pcname,mode,wbname;
	pcname=app->m_strComputerName;
	pcname.TrimRight();
	if(program4==1)
	{
		wbname="WB A";
	}
	else if(program4==2)
	{
		wbname="WB B";
	}	
	CString wbmode,manual_setting;
	strSQL.Format("SELECT TOP(1) MANUAL_MODE,SET_MANUAL_FLAG FROM WEIGHBRIDGE WHERE COMPUTER_NAME='%s' and NAME='%s'",pcname,wbname);
	if(SqlValue(strSQL,1)==1)
	{
		wbmode="Dual";
	}
	else
	{
		wbmode="Normal";
	}
	if(SqlValue(strSQL,2)==1)
	{
		manual_setting="Manual";
	}
	else
	{
		manual_setting="Auto";
	}
	mode.Format("%s Mode (%s)",wbmode,manual_setting);
	

	if(program4==0)
	{
		mode= _T("Admin");
	}
	SetDlgItemText(IDC_MODELABEL,mode);
}
