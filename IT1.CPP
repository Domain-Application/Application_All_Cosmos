// It1.cpp : implementation file
//
 
#include "stdafx.h"
#include "menu.h"
#include "Security.h"
#include "comm.h"
#include "nview.h"
#include "It1.h"
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 
/////////////////////////////////////////////////////////////////////////////
// CIt1 dialog
 
 
CIt1::CIt1(CWnd* pParent /*=NULL*/)
    : CFEdit(CIt1::IDD, pParent)
{
    //{{AFX_DATA_INIT(CIt1)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    iType = 0;
    askSave=-1;
}
 
 
void CIt1::DoDataExchange(CDataExchange* pDX)
{
    CFEdit::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CIt1)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}
 
 
BEGIN_MESSAGE_MAP(CIt1, CFEdit)
    //{{AFX_MSG_MAP(CIt1)
    ON_WM_DESTROY()
    ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SNAPSHOTS, OnSnapshots)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

 
/////////////////////////////////////////////////////////////////////////////
// CIt1 message handlers
 
void CIt1::OnOK()
{
	ReplaceRecord();
    if (IsSaveConditionOk(df)==0) return;

    int key = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
    push_plateno2.Format("");
    if(program4 == 2) {
		CDialog::OnOK();
    }
    else {
		CDialog::EndDialog(IDC_BUTTON1);
    }
}
 
BOOL CIt1::OnInitDialog()
{
    CFEdit::OnInitDialog();

	CString t = "Receiving Weighing";
    if(iType==2) {
        t = "Despatch Weighing";
		SetDlgItemText( StaticId(CUST), "Customer");
		SetDlgItemText( StaticId(SWT), "Buyer Weight");
    }
	SetDlgItemText( IDC_TITLE, t);

    CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
    m_tt = df->dat[TT];

	SetTimer(1,250,NULL);

	if(m_contno.GetLength()==11) {
		t.Format("Container Number: %s", m_contno);
	}
	else {
		t.Format("Vehicle Number: %s", m_vehicle);
	}
	SetWindowText(t);
	LoadImages();


    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
 
void CIt1::OnDestroy()
{
	KillTimer(1);
    CFEdit::OnDestroy();
}

void CIt1::OnTimer(UINT nIDEvent)
{
    long ActualWeight =  cweight;
	long weight = adjust_wt(cweight);

	ltodb(weight, df, PGROSS);
	ltodb(ActualWeight, df, W1);
	ltodb(weight, df, Z1);

    ShowFieldContent( PGROSS );

    if(m_tt.Compare(df->dat[TT])) {
        UINT ID = df->dat[TT][0]=='Y' ? IDC_RADIO2 : IDC_RADIO1;

        CheckRadioButton( IDC_RADIO1, IDC_RADIO2, ID);
        m_tt = df->dat[TT];
    }

    CFEdit::OnTimer(nIDEvent);
}

BOOL CSecurity::OnFirstWt(int type, BOOL LPR_Status, BOOL CNR_Status)
{
    lsetblank(df);
	long xrec;
	CString strsql;
	(*df->e->start)(df);

	m_contno.TrimRight();

	if(m_contno.GetLength()==11) 
	{
		xrec = df->Search(m_contno, WB_Container, TRUE );
	}
	else
	{
		xrec = df->Search(m_vehicle , LORRY, TRUE );
	}

	if(xrec) {
		return OnSecond(LPR_Status, CNR_Status);
	}
	
	if(m_contno.GetLength()==11) {
		lset(df->dat[WB_Container], m_contno);
		lset(df->dat[LORRY], m_vehicle);
        lset(df->dat[PM1NO], m_vehicle);
        long x1 = qd(df->dat[PM1NO], &lorry);
	}
	else {
		lset(df->dat[LORRY], m_vehicle);
	}

	ltodb(cweight, df, PGROSS);
	ltodb(cweight, df, W1);
	ltodb(cweight, df, Z1);
    lset(df->dat[USER1], user.dat[1]);
	df->SetCurrentTime(TIMEIN);

	lset(df,"I1", strIndicator(program4) );

    int fld = GetDbFieldNo(df ,_szPrintCounter);
	if(type==1)
	{
		df->m_strTicketNo = receive_ticket_info.m_strTicketNo; 
		df->m_strRootTable = receive_ticket_info.m_strTicketNo; 

	}
	else
	{
		df->m_strTicketNo =	despatch_ticket_info.m_strTicketNo; 
		df->m_strRootTable = despatch_ticket_info.m_strTicketNo; 
	}
	FindAddInfoTicketNo(df,1);
    ltodb(type, df, fld);

    CIt1 h;
	h.m_contno = m_contno;
	h.m_vehicle = m_vehicle;
    h.df = df;
    h.iType = type;

    UINT ID = h.DoModal();
	if(ID==IDCANCEL) return FALSE;

	SaveImages(df,"JPGKEY1");
    ltodb(LPR_Status, df,  GetDbFieldNo(df, "IN_LPR_SUCCESS"));
    ltodb(CNR_Status, df,  GetDbFieldNo(df, "IN_CNR_SUCCESS"));
	df->SetCurrentTime(TIMEIN);
	df->SetCurrentTime(Date);
    df->AddNew();
	IsZeroWt = FALSE;
	return TRUE;
}


void CSecurity::OnRadio1(BOOL LPR_Status, BOOL CNR_Status) 
{
    BOOL is_Done = OnFirstWt( 1, LPR_Status, CNR_Status);
    isLPR_OK = TRUE;
    isCNR_OK = TRUE;
	if(is_Done)
	{
		CFM20Dlg::OnOK();
	}
}

void CSecurity::OnRadio2(BOOL LPR_Status, BOOL CNR_Status) 
{
    BOOL is_Done = OnFirstWt( 2, LPR_Status, CNR_Status);
    isLPR_OK = TRUE;
    isCNR_OK = TRUE;
	if(is_Done)
	{
		CFM20Dlg::OnOK();
	}
}

void CIt1::LoadImages()
{
     int TotalCameras = 4;
	 CString s;

     for(int i=1; i<=TotalCameras; i++) {
         CWnd* previewWnd = GetDlgItem(IDC_PREVIEW+i-1);
         if(!previewWnd) continue;
         previewWnd->GetWindowRect(PreviewRect[i]);
         ScreenToClient(&PreviewRect[i]);
		 s = pView->strPhoto[i];
         if(m_jpg[i].Load(s)) {
             InvalidateRect( PreviewRect[i] );
         }
     }
}


void CIt1::OnSnapshots() 
{
	int TotalCameras = pView->TotalCameras;
    for(int i=1; i<=TotalCameras; i++) {

		 CString file = pView->strPhoto[i];

		 int flag = app->OnSnapshot(i, file);

         if(m_jpg[i].Load( file )) {
			 InvalidateRect( PreviewRect[i] );
		 }
		 RestMsg();
	}
    UpdateData(FALSE);
	
}

void CIt1::OnCancel()
{
    int flag = MessageBox("Exit","Confirm it ?",MB_YESNO | MB_ICONQUESTION);
    if(flag == IDYES) {
        CDialog::OnCancel();
    }
}

BOOL OnBin_WeightIn_Info(DB *df,CString binno) //brian//28-12-2925
{
	return TRUE;
}
