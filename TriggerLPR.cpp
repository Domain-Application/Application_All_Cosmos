// TriggerLPR.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "comm.h"
#include "TriggerLPR.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString test_plateno;
BOOL DeletePic(const char *file)
{
	CString pic;

	pic.Format("%s\\%s", app->picFolder, file);
	return DeleteFile(pic);
}

/////////////////////////////////////////////////////////////////////////////
// CTriggerLPR dialog


CTriggerLPR::CTriggerLPR(CWnd* pParent /*=NULL*/)
	: CDialog(CTriggerLPR::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerLPR)
	m_stable = FALSE;
	//}}AFX_DATA_INIT

    iTryCount = 1;
    Initial();
}


void CTriggerLPR::Initial()
{
    iCounter = 0;
    oldwt = -10;
    iStep = 1;
    busy = 0;
    no = 1;
    m_bUseExternalData = FALSE;

    memset(lpr_status, 0, sizeof(lpr_status));
    memset(vehicle_rec, 0, sizeof(vehicle_rec));
	plateno = plateno1 = plateno2 = "";
    m_strExternalInfo2 = "";
}

void CTriggerLPR::SetExternalData(CString strInfo1, CString strInfo2, CString strImage1, CString strImage2, CString strImage3, CString strImage4)
{
    m_bUseExternalData = TRUE;
    m_strExternalInfo = strInfo1;
    m_strExternalInfo2 = strInfo2;
    m_strExternalImage = strImage1;
    m_strExternalImage2 = strImage2;
    m_strExternalImage3 = strImage3;
    m_strExternalImage4 = strImage4;
}

void CTriggerLPR::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggerLPR)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggerLPR, CDialog)
	//{{AFX_MSG_MAP(CTriggerLPR)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_L1, OnL1)
    ON_BN_CLICKED(IDC_L2, OnL2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTriggerLPR message handlers

void CTriggerLPR::OnDestroy() 
{
	KillTimer(1);
	CDialog::OnDestroy();
	
}

BOOL CTriggerLPR::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    for(int i=1; i<=4;i++) {
        CWnd* previewWnd = GetDlgItem(IDC_PREVIEW+i-1);
        if(!previewWnd) continue;
        previewWnd->GetWindowRect(PreviewRect[i]);
        ScreenToClient(&PreviewRect[i]);
    }


    SetTimer(1,250,NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTriggerLPR::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    for(int i=1; i<=4;i++) {
		CWnd* previewWnd = GetDlgItem(IDC_PREVIEW+i-1);
        if(!previewWnd) continue;

		m_jpg[i].Draw(&dc,PreviewRect[i].left,PreviewRect[i].top,
                   PreviewRect[i].Width(),PreviewRect[i].Height());
    }

   
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CTriggerLPR::OnTimer(UINT nIDEvent) 
{
    if(busy) return;

    busy = 1;
	CDialog::OnTimer(nIDEvent);


    iCounter++;
    if(iCounter>10) iCounter = 10;


	picPlate.Format("plate%2.2i.jpg", no);
	picLPR.Format("lpr%2.2i.jpg", no);

	if(iStep==1) {
         if (m_bUseExternalData)
         {
             plateno1 = m_strExternalInfo;
             if(plateno1.GetLength()>2) {
                 s.Format("%s",plateno1);
                 SetDlgItemText(IDC_L1,s);
             }

             plateno2 = m_strExternalInfo2;
             if(plateno2.GetLength()>2) {
                 s.Format("%s",plateno2);
                 SetDlgItemText(IDC_L2,s);
             }
             
             // Load the external images
             // Load the external images from both cameras
             // Cam A: Crop -> m_jpg[1], Full -> m_jpg[2]
             // Cam B: Crop -> m_jpg[3], Full -> m_jpg[4]
             if (!m_strExternalImage.IsEmpty()) {
                 if(m_jpg[1].Load(m_strExternalImage)) InvalidateRect(PreviewRect[1]);
             }
             if (!m_strExternalImage2.IsEmpty()) {
                 if(m_jpg[2].Load(m_strExternalImage2)) InvalidateRect(PreviewRect[2]);
             }
             if (!m_strExternalImage3.IsEmpty()) {
                 if(m_jpg[3].Load(m_strExternalImage3)) InvalidateRect(PreviewRect[3]);
             }
             if (!m_strExternalImage4.IsEmpty()) {
                 if(m_jpg[4].Load(m_strExternalImage4)) InvalidateRect(PreviewRect[4]);
             }
             
             iStep+=3;
             busy = 0;
             return;
         }

         int option = 3;
         DeletePic(picPlate);
         DeletePic(picLPR);
         lpr_status[1] = app->OnGetPlateNo(no,plateno1,option,app->picFolder);
#ifdef _DEBUG
		 if(no==2 && test_plateno.GetLength()) {
			plateno1 = test_plateno;        //for testing only
			lpr_status[1] = 1;
		 }
#endif //_DEBUG

		 direction = 1;
		 Save2loglpr(plateno1,1);

		 lset(lorry.dat[Lo_LPRNO], plateno1);
		 long xrec = lorry.Search(lorry.dat[Lo_LPRNO],Lo_LPRNO,1);
		 if(xrec>0) {
			 if( strcmp(lorry.dat[1],lorry.dat[Lo_LPRNO])) {
				plateno1 = lorry.dat[1];
			 }
		 }

		 if(plateno1.GetLength()>2) {
             s.Format("%s",plateno1);
			 SetDlgItemText(IDC_L1,s);
		 }
		 s.Format("%s\\%s", app->picFolder,picPlate);
		 if(m_jpg[1].Load( s )) {
			 InvalidateRect( PreviewRect[1] );
		 }

		 s.Format("%s\\%s", app->picFolder,picLPR);
		 if(m_jpg[2].Load( s )) {
			 InvalidateRect( PreviewRect[2] );
		 }
		 iStep+=3;
		 busy = 0;
		 return;
	}
	

    busy = 0;
}

void CTriggerLPR::OnOK() 
{
}

void CTriggerLPR::OnCancel() 
{
	CDialog::OnCancel();
}


void CTriggerLPR::Save2loglpr(const char *plateno1,int no)
{
	CString s;
	
	char szDirection[2] = {0};
	
	szDirection[0] = '0'+direction;
/*
	s.Format("Insert into LogLPR (plateno,push_plateno1,push_plateno2,weight,DIRECTION,wb,[DeviceIndex],mode) values ('%s','%s','%s',%ld,'%s',%i,%i,'C')", 
		plateno1, push_plateno1, push_plateno2,cweight, szDirection, program4,no);
	Runsql(s);
*/
}

void CTriggerLPR::OnL1() 
{
    plateno = plateno1;
	CDialog::OnOK();
}

void CTriggerLPR::OnL2() 
{
    plateno = plateno2;
	CDialog::OnOK();
}

void CTriggerLPR::OnManual(int no)
{
    int id = IDC_EDIT1 + no - 1;
    DWORD tt = GetDlgItem(id)->GetStyle() & WS_VISIBLE;
    if(tt) {
		CString s;

		GetDlgItemText(id,s);

		UINT ID = MessageBox(s,"Confirm it?", MB_ICONQUESTION | MB_YESNO);
		if(ID==IDYES) {
             plateno = s;
             CDialog::OnOK();
		}
		return;
    }

    GetDlgItem(IDC_L1 + no - 1)->ShowWindow(SW_HIDE);
    GetDlgItem(id)->ShowWindow(SW_SHOW);
    GetDlgItem(id)->SetFocus();
}

void CTriggerLPR::OnButton1() 
{
    OnManual(1);
}
