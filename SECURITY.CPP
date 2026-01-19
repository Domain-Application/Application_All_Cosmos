// Security.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "comm.h"
#include "myjpg.h"
#include "Security.h"
#include "TriggerLPR.h"
#include "TakePhoto.h"
#include "NView.h"
#include "It1.h"
#include "ShowJpg.h"
#include <afxinet.h> 
// #include "ANPRListener.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSecurity dialog


CSecurity::CSecurity(CWnd* pParent /*=NULL*/)
    : CFM20Dlg(CSecurity::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSecurity)
	m_vehicle = _T("");
	m_contno = _T("");
	//}}AFX_DATA_INIT
	iCounter = jpgkey = 0;
	isAutoMode = FALSE;
	isLPR_OK = isCNR_OK = TRUE;
	ManualOpenFrontBarrier = ManualOpenBackBarrier = FALSE;
}


void CSecurity::DoDataExchange(CDataExchange* pDX)
{
    CFM20Dlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecurity)
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_BR2, m_br2);
	DDX_Control(pDX, IDC_BR1, m_br1);
    DDX_Control(pDX, IDC_LED1, m_led1);
    DDX_Control(pDX, IDC_LED2, m_led2);
	DDX_Text(pDX, IDC_L1, m_vehicle);
	DDV_MaxChars(pDX, m_vehicle, 20);
	DDX_Text(pDX, IDC_L2, m_contno);
	DDV_MaxChars(pDX, m_contno, 20);
	//}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_BR1UP, m_led[0]);
    DDX_Control(pDX, IDC_BR1DN, m_led[1]);
    DDX_Control(pDX, IDC_BR2UP, m_led[2]);
    DDX_Control(pDX, IDC_BR2DN, m_led[3]);
    DDX_Control(pDX, IDC_S1, m_S1);
    DDX_Control(pDX, IDC_S2, m_S2);
	m_br1.no = 1;
	m_br2.no = 2;
    m_S1.no = 1;
    m_S2.no = 2;
}


BEGIN_MESSAGE_MAP(CSecurity, CFM20Dlg)
	//{{AFX_MSG_MAP(CSecurity)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnTakePhotos)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
    ON_MESSAGE(WM_PHOTO_PROCESSED, OnPhotoProcessed)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


UINT PhotoThreadProc(LPVOID pParam)
{
    PhotoThreadParam* p = (PhotoThreadParam*)pParam;
    CMenuApp* app = (CMenuApp*)AfxGetApp(); 

    CString plateno1;
    int option = 3;
    
    //TODO: fix this for camera 1 and camera 3 mapping to license plate
    for(int i=1; i<=p->TotalCameras; i++) {
        CString ActualFile;
		ActualFile.Format("%s\\plate0%d.jpg",CString(p->picFolder),i);
        ::DeleteFile(ActualFile);
        // Blocking camera call
        int flag = app->OnGetPlateNo(i, plateno1, option, p->picFolder);
        
        CString file(p->strPhoto[i-1]);
        
        CString* pResultStr = new CString();
        
        bool doSnapshot = false;
        if( (i == 1 && !push_plateno1.IsEmpty()) || (i == 2 && !push_plateno2.IsEmpty()) || i == 3 )
        {
             app->OnSnapshot(i, file);
             
             if(i == 1) *pResultStr = push_plateno1;
             else if(i == 2) *pResultStr = push_plateno2;
        }
        else if (i == 1)
        {
            *pResultStr = plateno1;
        }
        else if (i == 2)
        {
            *pResultStr = plateno1;
        }
        if(i==9 || i==10)
		{
			app->OnSnapshot(i, file);
			*pResultStr = "";
		}
        // Post message to UI thread
        // We pass the string pointer in LPARAM. Receiver must delete it.
        ::PostMessage(p->hWnd, WM_PHOTO_PROCESSED, (WPARAM)i, (LPARAM)pResultStr);
    }
    
    // Signal completion with 0
    ::PostMessage(p->hWnd, WM_PHOTO_PROCESSED, 0, 0);
    
    delete p;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSecurity message handlers
void CSecurity::OnButton5() 
{
}

void CSecurity::OnDestroy() 
{
    KillTimer(1);
	KillTimer(2);
	KillTimer(9001);
	//If not removing this while closing the system. System will keep remaining the container number in motion detect while container reverse and tanker coming in
	if(!isblank(push_plateno2))
	{
		UINT ID = MessageBox("Closing will remove detected container number from motion detect","Confirm it", MB_ICONQUESTION | MB_YESNO);
		if(ID==IDYES) 
		{
			push_plateno2.Format(""); //20251218
		}
	}
	
    CFM20Dlg::OnDestroy();
}

BOOL isWBDualMode()
{
	return weighbridge.Boolean(GetDbFieldNo(&weighbridge, "MANUAL_MODE"));
}

BOOL isManualSetting()
{
	return weighbridge.Boolean(GetDbFieldNo(&weighbridge, "SET_MANUAL_FLAG"));
}


BOOL CSecurity::OnInitDialog() 
{
    CFM20Dlg::OnInitDialog();
    SetWindowLocation( this );

    CheckRadioButton( IDC_RADIO1, IDC_RADIO4, IDC_RADIO1);
    SetTimer(1,100,NULL);
    SetTimer(3, 200, NULL);
    SetupIcons();
    for(int i=1; i<=10;i++) {
        CWnd* previewWnd = GetDlgItem(IDC_PREVIEW+i-1);
        if(!previewWnd) continue;
        previewWnd->GetWindowRect(PreviewRect[i]);
        ScreenToClient(&PreviewRect[i]);
    }	
	CEdit *r = (CEdit *) GetDlgItem(IDC_L3);
	r->SetReadOnly(FALSE);
	r->SetFocus();

	if(!isManualSetting())
	{
		CEdit *p = (CEdit *) GetDlgItem(IDC_L1);
		p->SetReadOnly(TRUE);
		p->SetFocus();
		CEdit *q = (CEdit *) GetDlgItem(IDC_L2);
		q->SetReadOnly(TRUE);
		q->SetFocus();
	}
	else
	{	
		CEdit *p = (CEdit *) GetDlgItem(IDC_L1);
		p->SetReadOnly(FALSE);
		p->SetFocus();
		CEdit *q = (CEdit *) GetDlgItem(IDC_L2);
		q->SetReadOnly(FALSE);
		q->SetFocus();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSecurity::OnGW()
{
    char tmp[80];

	if(cweight==-1) {
		sprintf(tmp,"No signal");
	}
	else {
		sprintf(tmp,"%5.5ld kg",cweight);
	}
    SetDlgItemText(IDC_TEXTBOX1, tmp );
    HWND hWnd = ::GetDlgItem( GetSafeHwnd(), IDC_ZERO_FLAG);
    UINT BST = (IsZeroWt == FALSE) ? BST_UNCHECKED : BST_CHECKED;
    ::SendMessage(hWnd, BM_SETCHECK, BST, 0L);
	long var =  20;
	long count = 5;
    flagStable = IsHistoryWtStable(var, count);
    HWND hWnd2 = ::GetDlgItem( GetSafeHwnd(), IDC_STABLE_FLAG);
    BST = flagStable ? BST_CHECKED : BST_UNCHECKED;
    ::SendMessage(hWnd2, BM_SETCHECK, BST, 0L);
}

void CSecurity::OnTimer(UINT nIDEvent) 
{

	if(nIDEvent==9001) {
		iCounter--;
		m_progress.SetPos(iCounter);
		KillTimer(nIDEvent);
        CFM20Dlg::OnOK();
		return;
	}

	if(nIDEvent==1) {
		KillTimer(nIDEvent);
        OnGW();
        SetTimer(2,100,NULL);
		return;
	}
    else if(nIDEvent==2) {
		KillTimer(nIDEvent);
		OnTakePhotos();
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		return;
	}
    else if(nIDEvent==3)
    {
        int flag = readADAMData(adamData, 0); //Review this if in future going change from ADAM to PLC
        m_S1.Show();
        m_S2.Show();
    }
    OnGW();
    CFM20Dlg::OnTimer(nIDEvent);
}

void CSecurity::SetupIcons()
{
    CWnd *pS1 = (CWnd *)GetDlgItem(IDC_S1);
    m_S1.SetLED(pS1,ID_LED_RED,ID_SHAPE_SQUARE,500);
    GetDlgItem(IDC_S1)->ShowWindow(SW_HIDE);

    CWnd *pS2 = (CWnd *)GetDlgItem(IDC_S2);
    m_S2.SetLED(pS2,ID_LED_RED,ID_SHAPE_SQUARE,500);
    GetDlgItem(IDC_S2)->ShowWindow(SW_HIDE);
}

void CSecurity::OnCancel() 
{
    CFM20Dlg::OnCancel();
}

void CSecurity::OnProceed() 
{
	UpdateData();
 	if(IsZeroWt == FALSE){
		MessageBox("Weight without from zero.","Error", MB_ICONSTOP);
		return;
	}

	if (flagStable == FALSE) {
		MessageBox("Weight unstable.","Error", MB_ICONSTOP);
		return;
	}
	if(cweight < 1000) {
		MessageBox("Weight too light.","Error", MB_ICONSTOP);
		return;
	}

	if( isblank( m_vehicle )) {
		MessageBox("Vehicle Not found.","Error", MB_ICONSTOP);
		return;
	}
	for(int i=1; i<=2; i++) 
	{
		CString str;
		if(SensorStatus(i)) 
		{
			str.Format("Sensor %i blocked\n\nDo you want to continue ?",i);
			UINT ID = MessageBox(str,"Error Message", MB_ICONQUESTION | MB_YESNO);
			if(ID==IDNO) {
				CDialog::OnOK();
				return;
			}
		}
	}
	
	GetDlgItemText(IDC_L1, m_vehicle);
	GetDlgItemText(IDC_L2, m_contno);
	m_contno.TrimRight();
	if(m_contno.GetLength()==11) {
        int flag = CheckContainerDigit( m_contno );
        if(flag != TRUE) {
			CheckContainerDigitErrorMsg += "\n\nDo you want to continue";
			UINT ID = MessageBox(CheckContainerDigitErrorMsg,"Error Message",MB_ICONQUESTION | MB_YESNO);
			if(ID==IDNO) return;
		}
	}
    int key = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO4);
    if(key==IDC_RADIO1) {
        OnRadio1(isLPR_OK, isCNR_OK);
    }
    else if(key==IDC_RADIO2) {
        OnRadio2(isLPR_OK, isCNR_OK);
    }
    else if(key==IDC_RADIO3) {
        OnRadio3();
    }
    else if(key==IDC_RADIO4) {
        OnRadio4();
    }
}

void CSecurity::OnOK() 
{
	SetDlgItemText( IDOK, "Proceeding...");
	GetDlgItem( IDOK )->EnableWindow(FALSE);
	OnProceed();
	SetDlgItemText( IDOK, "F5->  Confirm Weight");
	GetDlgItem( IDOK )->EnableWindow(TRUE);
}

// Image extraction helper remains local as it's specific to the API multipart response

// Helper to extract specific image from multipart buffer
// Returns size of extracted image, or 0 if not found
long ExtractMultipartImage(char* pBuf, DWORD dwTotalSize, const char* pszFilename, CString strDestPath)
{
    CString strSig;
    strSig.Format("%s\"", pszFilename); // Look for filename="name.jpg"
    
    long nFoundPos = -1;
    for (long i = 0; i < (long)dwTotalSize - strSig.GetLength(); i++)
    {
        if (memcmp(pBuf + i, strSig, strSig.GetLength()) == 0)
        {
            nFoundPos = i;
            break;
        }
    }

    if (nFoundPos != -1)
    {
        long nDataStart = -1;
        for (long i = nFoundPos; i < (long)dwTotalSize - 4; i++)
        {
            if (pBuf[i] == '\r' && pBuf[i+1] == '\n' && pBuf[i+2] == '\r' && pBuf[i+3] == '\n')
            {
                nDataStart = i + 4;
                break;
            }
        }

        if (nDataStart != -1)
        {
            long nDataEnd = dwTotalSize;
            for (long i = nDataStart; i < (long)dwTotalSize - 4; i++)
            {
                 // Find next boundary
                if (pBuf[i] == '\r' && pBuf[i+1] == '\n' && pBuf[i+2] == '-' && pBuf[i+3] == '-')
                {
                    nDataEnd = i;
                    break;
                }
            }
            
            long nSize = nDataEnd - nDataStart;
            if (nSize > 0)
            {
                CFile imgFile;
                if (imgFile.Open(strDestPath, CFile::modeCreate | CFile::modeWrite))
                {
                    imgFile.Write(pBuf + nDataStart, nSize);
                    imgFile.Close();
                    return nSize;
                }
            }
        }
    }
    return 0;
}

void CSecurity::ProcessCameraRequest(int nIndex)
{
    CMenuApp* app = (CMenuApp*)AfxGetApp();
    CString strResultNumA, strResultNumB, title;
    
    // Paths for up to 2 cameras (Cam A and Cam B)
    CString strCamACrop, strCamAFull;
    CString strCamBCrop, strCamBFull;

    int camIdxA = (nIndex == 1) ? 1 : 2;
    int camIdxB = (nIndex == 1) ? 3 : 4;

    int camIndices[2] = {camIdxA, camIdxB};
    for (int k = 0; k < 2; k++)
    {
        int currentCam = camIndices[k];
        CString strFullImgPath, strCropImgPath;
        strFullImgPath.Format("%s\\detectionPicture_%d.jpg", app->picFolder, currentCam);
        strCropImgPath.Format("%s\\plate0%d.jpg", app->picFolder, currentCam);
        ::DeleteFile(strFullImgPath);
        ::DeleteFile(strCropImgPath);

        CString cameraIP, cameraUserName, cameraPassword;
        CString strCamCode;
        strCamCode.Format("0%d", currentCam);
        
        lset(camera.dat[1], strCamCode);
        if(qd(camera.dat[1], &camera) == 0) continue;
        
        cameraIP = camera.dat[GetDbFieldNo(&camera, "IPADD")];
        cameraUserName = camera.dat[GetDbFieldNo(&camera, "USER")];
        cameraPassword = camera.dat[GetDbFieldNo(&camera, "PASSWORD")];
        cameraIP.TrimRight();
        cameraUserName.TrimRight();
        cameraPassword.TrimRight();

        CInternetSession session("HikvisionClient");
        CHttpConnection* pConnection = NULL;
        CHttpFile* pFile = NULL;
        char* pBuf = NULL;
        DWORD dwTotalSize = 0;

        try {
            pConnection = session.GetHttpConnection(cameraIP, INTERNET_DEFAULT_HTTP_PORT, cameraUserName, cameraPassword);
            pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, "/ISAPI/Traffic/MNPR/channels/1", 
                                             NULL, 1, NULL, NULL, 
                                             INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION);
            pFile->SendRequest();
            DWORD dwRet;
            pFile->QueryInfoStatusCode(dwRet);
            
            if (dwRet == HTTP_STATUS_OK) {
                DWORD dwBufSize = 2097152; // 2MB
                pBuf = new char[dwBufSize];
                char* pCurr = pBuf;
                while (dwTotalSize < dwBufSize) {
                     DWORD dwBytesToRead = (dwBufSize - dwTotalSize < 8192) ? (dwBufSize - dwTotalSize) : 8192;
                     UINT nRead = pFile->Read(pCurr, dwBytesToRead);
                     if (nRead == 0) break;
                     dwTotalSize += nRead;
                     pCurr += nRead;
                }
            }
        } catch (CInternetException* pEx) { pEx->Delete(); }

        if (dwTotalSize > 0 && pBuf) {
            CString strContent;
            int nTextLen = (dwTotalSize > 20000) ? 20000 : (int)dwTotalSize;
            char* pTemp = new char[nTextLen + 1];
            memcpy(pTemp, pBuf, nTextLen);
            pTemp[nTextLen] = 0;
            strContent = pTemp;
            delete[] pTemp;

            CString strPlate, strContainer;
            int res = ProcessAnprXml(strContent, strPlate, strContainer);

            if (nIndex == 1 && (res & ANPR_RESULT_PLATE)) {
                if (k == 0) strResultNumA = strPlate;
                else strResultNumB = strPlate;
                title = _T("Trigger License Plate Recognition");
            } else if (nIndex == 2 && (res & ANPR_RESULT_CONTAINER)) {
                if (k == 0) strResultNumA = strContainer;
                else strResultNumB = strContainer;
                title = _T("Trigger Container Number Recognition");
            }

            // Save images
            ExtractMultipartImage(pBuf, dwTotalSize, "detectionPicture.jpg", strFullImgPath);
            ExtractMultipartImage(pBuf, dwTotalSize, "licensePlatePicture.jpg", strCropImgPath);

            if (k == 0) {
                strCamACrop = strCropImgPath;
                strCamAFull = strFullImgPath;
            } else {
                strCamBCrop = strCropImgPath;
                strCamBFull = strFullImgPath;
            }
        }

        if (pBuf) delete[] pBuf;
        if (pFile) { pFile->Close(); delete pFile; }
        if (pConnection) { pConnection->Close(); delete pConnection; }
        session.Close();
    }

    CTriggerLPR h;
    h.no = nIndex;
    h.SetExternalData(strResultNumA, strResultNumB, strCamACrop, strCamAFull, strCamBCrop, strCamBFull);
    
    UINT ID = h.DoModal();
    if(ID != IDCANCEL)
    {
        if (nIndex == 1) {
             m_vehicle = h.plateno;
             SetDlgItemText(IDC_L1, m_vehicle);
             if (m_jpg[1].Load(strCamAFull)) InvalidateRect(PreviewRect[1]);
             if (m_jpg[5].Load(strCamACrop)) InvalidateRect(PreviewRect[5]);
             if (m_jpg[3].Load(strCamBFull)) InvalidateRect(PreviewRect[3]);
             if (m_jpg[6].Load(strCamBCrop)) InvalidateRect(PreviewRect[6]);
        }
        if (nIndex == 2) {
             m_contno = h.plateno;
             SetDlgItemText(IDC_L2, m_contno);
             if (m_jpg[2].Load(strCamAFull)) InvalidateRect(PreviewRect[2]);
             if (m_jpg[7].Load(strCamACrop)) InvalidateRect(PreviewRect[7]);
             if (m_jpg[4].Load(strCamBFull)) InvalidateRect(PreviewRect[4]);
             if (m_jpg[8].Load(strCamBCrop)) InvalidateRect(PreviewRect[8]);
        }
        UpdateData(FALSE);
    }
}

void CSecurity::OnButton1() 
{
    ProcessCameraRequest(1);
}

void CSecurity::OnButton2() 
{
    ProcessCameraRequest(2);
}

void CSecurity::OnButton7()  //brian to do//prompt viewidx master list to let user choose
{
}
void CSecurity::SaveImages(DB *df,const char *JPGKEY)
{
    if(jpgkey==0) {
        jpgkey = app->CreateNewJpgFile();
    }
    CString fldName;
	int flag,TotalCameras = pView->TotalCameras;
    for(int i=1; i<=TotalCameras; i++) {
        fldName.Format("jpg%2.2i",i+10);
        CMyJpg p;

	    CString file = pView->strPhoto[i];

        flag = p.textcopy("jpgnew", fldName, jpgkey , file, 'I');
        if(flag==0) {
        }
    }

    ltodb(jpgkey, df, JPGKEY);
    flag = pView->CopyPhotoToServer(jpgkey);
	jpgkey = 0;
}



void CSecurity::OnTakePhotos() 
{
    // Disable UI to prevent re-entrancy
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);

    PhotoThreadParam* p = new PhotoThreadParam;
    p->hWnd = GetSafeHwnd();
    p->TotalCameras = pView->TotalCameras;
    strcpy(p->picFolder, app->picFolder);
    
    for(int i=0; i<p->TotalCameras && i<10; i++) {
        strcpy(p->strPhoto[i], pView->strPhoto[i+1]);
    }
    
    m_progress.SetRange32(1, p->TotalCameras);
    m_progress.SetPos(0);
    
    AfxBeginThread(PhotoThreadProc, p);
}

LRESULT CSecurity::OnPhotoProcessed(WPARAM wParam, LPARAM lParam)
{
    int i = (int)wParam;
    if (i == 0) {
        UpdateData(FALSE);
        m_showjpg = 1;
        GetDlgItem(IDOK)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
        return 0;
    }
    CString* pStr = (CString*)lParam;
    if (pStr) {
        if (!pStr->IsEmpty()) {
            if (i == 1 || i == 3) {
                if (IsMalaysiaPlate(*pStr)) {
                    m_vehicle = *pStr;
                    SetDlgItemText(IDC_L1, m_vehicle);
                }
                // Allow display even if it doesn't match a standard plate (or maybe always true for camera 3?)
            } else if (i == 2 || i == 4) {
                if(ValidateISO6346(*pStr)) {    
                    m_contno = *pStr;
                    SetDlgItemText(IDC_L2, m_contno);
                }
            }
        }
        delete pStr;
    }
    if(i == 1 || i == 3) {
        CString picPlate, pic_Path;
        picPlate.Format("plate0%d.jpg", i);
        pic_Path.Format("%s\\%s", app->picFolder, picPlate);
        int idx = i == 1 ? 5 : 6;
        if(m_jpg[idx].Load( pic_Path )) {
             InvalidateRect( PreviewRect[idx] );
        }
    }
    else if (i == 2) {
        CString picPlate, pic_Path;
        picPlate.Format("plate%2.2i.jpg", i);
        pic_Path.Format("%s\\%s", app->picFolder, picPlate);
        int idx = i == 2 ? 7 : 8;
        if(m_jpg[idx].Load( pic_Path )) {
             InvalidateRect( PreviewRect[idx] );
        }
    }
    /*CString file = pView->strPhoto[i];
    if(m_jpg[i].Load( file )) {
         InvalidateRect( PreviewRect[i] );
    }*/

	if (i >= 1 && i <= 10) { 
        CString file = pView->strPhoto[i]; 
        
        // Load the image into the array
        if(m_jpg[i].Load( file )) {
			// Redraw the specific rectangle for this preview
			InvalidateRect( PreviewRect[i] );
        }
    }
    m_progress.SetPos(i);    
    return 0;
}

void CSecurity::OnPaint() 
{
    CPaintDC dc(this);
    for(int i=1; i<=10;i++) {
		CWnd* previewWnd = GetDlgItem(IDC_PREVIEW+i-1);
        if(!previewWnd) continue;
		m_jpg[i].Draw(&dc,PreviewRect[i].left,PreviewRect[i].top,
                   PreviewRect[i].Width(),PreviewRect[i].Height());
    }
}

BOOL CSecurity::ManualModeActive()
{
	return TRUE;
}

void CSecurity::OnOpenBarrier(UINT ID)
{
}

void CSecurity::OnButton4() 
{
	CString str;
	int level = 9;
	str.Format("Manual input vehicle number.");
	int flag = UserPasswordLoginAccepted(df,level,str);
	if(flag==0) return;
	CEdit *p = (CEdit *) GetDlgItem(IDC_L1);
	p->SetReadOnly(FALSE);
	p->SetFocus();
	isLPR_OK = FALSE;
}

void CSecurity::OnButton6() 
{
	CString str;
	int level = 9;
	str.Format("Manual input container number.");
	int flag = UserPasswordLoginAccepted(df,level,str);
	if(flag==0) return;
	CEdit *p = (CEdit *) GetDlgItem(IDC_L2);
	p->SetReadOnly(FALSE);
	p->SetFocus();
	isCNR_OK = FALSE;
}

void CSecurity::OnButton8() 
{
	CEdit *r = (CEdit *) GetDlgItem(IDC_L3);
	r->SetReadOnly(FALSE);
	r->SetFocus();
}
void CSecurity::OnPic01()
{
    CShowJpg h1(IDD_SHOW_JPG);
    h1.df = NULL;
    h1.jpgName = pView->strPhoto[1];
    h1.DoModal();
}

void CSecurity::OnPic02()
{
    CShowJpg h1(IDD_SHOW_JPG);

    h1.df = NULL;
    h1.jpgName = pView->strPhoto[2];
    h1.DoModal();
}

void CSecurity::OnPic03()
{
    CShowJpg h1(IDD_SHOW_JPG);
    h1.df = NULL;
    h1.jpgName = pView->strPhoto[3];
    h1.DoModal();
}

BOOL CSecurity::PreTranslateMessage(MSG* pMsg) 
{
    UINT nChar = pMsg->wParam;
	UINT nFlags = (pMsg->lParam >> 16) & 0x00ff;	

    if (pMsg->message == WM_SYSKEYDOWN || pMsg->message == WM_KEYDOWN)	{

        if(nChar==VK_F5) {
             OnOK();
             return TRUE;
         }
	} 
	return CFM20Dlg::PreTranslateMessage(pMsg);
}
