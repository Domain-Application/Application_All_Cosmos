// It2.cpp : implementation file
//
 
#include "stdafx.h"
#include "menu.h"
#include "it2.h"
#include "Security.h"
#include "comm.h"
#include "nview.h"
 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 
/////////////////////////////////////////////////////////////////////////////
// CIt2 dialog
 
 
CIt2::CIt2(CWnd* pParent /*=NULL*/)
    : CFEdit(CIt2::IDD, pParent)
{
    //{{AFX_DATA_INIT(CIt2)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    iType = 0;
    askSave=-1;
}
 
 
void CIt2::DoDataExchange(CDataExchange* pDX)
{
    CFEdit::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CIt2)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    if (GetDlgItem(IDC_MSFLEXGRID1))
        DDX_Control(pDX, IDC_MSFLEXGRID1, m_grid1);
    //}}AFX_DATA_MAP
}
 
 
BEGIN_MESSAGE_MAP(CIt2, CFEdit)
    //{{AFX_MSG_MAP(CIt2)
    ON_WM_DESTROY()
    ON_WM_TIMER()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
 
/////////////////////////////////////////////////////////////////////////////
// CIt2 message handlers
void CIt2::OnOK()
{
    if(iType==1) {			//receiving only
		if(m_contno.GetLength() != 11 && df->val(SWT) <= 0) 
		{
			UINT ID = MessageBox("Cannot weighing!!!\n\nDo you want to continue?", "Supplier weight is required for tanker", MB_ICONQUESTION | MB_YESNO);
			if(ID==IDNO) return;
		}
	}

	ReplaceRecord();
    
    if (IsSaveConditionOk(df)==0) return;
    int key = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
    push_plateno2.Format("");
    if(program4 == 1) {
		CDialog::OnOK();
    }
    else {
		CDialog::EndDialog(IDC_BUTTON1);
    }
}
 
BOOL CIt2::OnInitDialog()
{
    CFEdit::OnInitDialog();

	int wtype = (cweight > df->val(PGROSS) )  ? 2 : 1;

	if( wtype != iType) {
		if(iType==0) {
			iType = wtype;
		}
		else {
			CString tMsg = (wtype==1) ? "Receiving" : "Despatch";

            msg.Format("Should be %s weighing type", tMsg);
            UINT ID = MessageBox(msg, "Wrong Weighing Type", MB_ICONQUESTION | MB_YESNO);
			if(ID==IDYES) {
				iType = wtype;
			}
		}
	}
	CString t = "Receiving Weighing";
    if(iType==2) {
        t = "Despatch Weighing";
		SetDlgItemText( StaticId(CUST), "Customer");
		SetDlgItemText( StaticId(SWT), "Buyer Weight");
    }
	SetDlgItemText( IDC_TITLE, t);

    CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
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
    ShowInitFrame();
	OnRefresh();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CIt2::ShowInitFrame()
{
    MGRID fg(&m_grid1, NULL, 0, 3);
    fg.SetTitle("No",400);
    fg.SetTitle("D/N No",1000);
    fg.SetTitle("P/O No 1",1000);
    fg.SetTitle("P/O No 2",1000);
    fg.SetTitle("P/O Qty",1000, 7);
    fg.SetTitle("Item Code",2100);
    fg.SetTitle("Item Description",4800);
    fg.SetTitle("Qty",600,7);
    fg.SetTitle("Unit",600);
    fg.SetTitle("Packing",1300);
    fg.SetTitle("Unit Wt.",1100,7);
    fg.SetTitle("No pallets.",1050,7);
    fg.SetTitle("Pallet Wt",1050,7);
    fg.SetTitle("Theoretical Wt.",1750,7);
}

void CIt2::OnRefresh()
{
    DB adoTmp;
    CString strSQL;
    strSQL.Format("select * from fdata where REFNO = '%s'", df->dat[1]);
    ado(&adoTmp, "", NULL, strSQL);
	long rec1, last1 = read_sumindex(0, &adoTmp);
	m_grid1.SetRedraw(FALSE);
	m_grid1.SetRows(last1 + 1);
	for (rec1 = 1; rec1 <= last1; rec1++)
	{
		readrec(rec1, &adoTmp);
		m_grid1.SetRow(rec1);
		ReplaceSubGrid(rec1, last1, &m_grid1, &adoTmp, 1);
	}
	m_grid1.SetRedraw(TRUE);
}

void CIt2::ReplaceSubGrid(long rec,long irec,CMSFlexGrid *m_grid,DB *sub,int IdxItem)
{
    MGRID fg(m_grid, sub, rec);
    fg.show(rec);
    fg.SetText(sub,NULL,Wd_DO,Wd_PO1,Wd_PO2,Wd_PQTY,Wd_PT,Wd_PT | 0x100,
          Wd_QTY,Wd_UNIT,Wd_PACKING,Wd_UW,
          Wd_NoPallet,Wd_PalletWt,Wd_TW,
		  -1);
}

void CIt2::LoadImages()
{
     int TotalCameras = 10;
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


void CIt2::OnSnapshots() 
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

void CIt2::OnDestroy()
{
	KillTimer(1);
    CFEdit::OnDestroy();
}

void CIt2::OnTimer(UINT nIDEvent)
{
    long ActualWeight =  cweight;
	long weight = adjust_wt(cweight);
	ltodb(weight, df, PTARE);
	ltodb(ActualWeight, df, W2);
	ltodb(weight, df, Z2);
    ShowFieldContent( PTARE );
	char str[10];
    compute_weight(df,sub);
    dtoa(m[11],str,7,0);
    SetDlgItemText(IDC_PNET,putnum0(str));
    dtoa(m[14],str,9,0);
    SetDlgItemText(IDC_VAR,putnum0(str));
    dtoa(m[15],str,9,3);
    SetDlgItemText(IDC_VAR2,putnum0(str));
    if(m_tt.Compare(df->dat[TT])) {
        UINT ID = df->dat[TT][0]=='Y' ? IDC_RADIO2 : IDC_RADIO1;
        CheckRadioButton( IDC_RADIO1, IDC_RADIO2, ID);
        m_tt = df->dat[TT];
    }
    CFEdit::OnTimer(nIDEvent);
}

BOOL CSecurity::OnSecond(BOOL LPR_Status, BOOL CNR_Status)
{
	CString strSQL;
	if(m_contno.GetLength()==11) {
		lset(df->dat[WB_Container], m_contno);
        //lset(df->dat[LORRY], m_vehicle);
        lset(lorry.dat[1], m_vehicle);


        qd(lorry.dat[1], &lorry);
        ltodb(lorry.val(GetDbFieldNo(&lorry, "TARE")), df, PM2WT);
        char szIndicator[2];
        szIndicator[0] = 'A' + program4 - 1;
        szIndicator[1] = 0;
        lset(df, "IT4", szIndicator);
        lset(df->dat[PM2NO], lorry.dat[1]);
        lset(df->dat[PM2TIME], lorry.dat[Lo_TIME]);
        lset(df->dat[USER4], lorry.dat[GetDbFieldNo(&lorry, "LAST_WEIGHT_BY")]);
	}
	else {
		lset(df->dat[LORRY], m_vehicle);
	}
	int fldPrintCounter = GetDbFieldNo(df ,_szPrintCounter);
    int type =  df->lval(fldPrintCounter);
	ltodb(cweight, df, PTARE);
	ltodb(cweight, df, W2);
	ltodb(cweight, df, Z2);
	lset(df,"I2", strIndicator(program4) );
    lset(df->dat[USER2], user.dat[1]);
	df->SetCurrentTime(TIMEOUT);
	df->SqlUpdate();
    CIt2 h;
	h.m_contno = m_contno;
	h.m_vehicle = m_vehicle;
    h.df = df;
    h.iType = type;

    UINT ID = h.DoModal();
	if(ID==IDCANCEL) return FALSE;

	STCR = type;
    DB *pf = (type==1) ? &wb : &despatch;

	lsetblank(pf);
	ltodb(0, df, fldPrintCounter);			//clear print counter
    fetch(pf, df);
	SaveImages(pf,"JPGKEY2");
    ltodb(LPR_Status, pf,  GetDbFieldNo(pf, "OUT_LPR_SUCCESS"));
    ltodb(CNR_Status, pf,  GetDbFieldNo(pf, "OUT_CNR_SUCCESS"));
    pf->AddNew();
    IsZeroWt = FALSE;
    CString msg, normal_report_name, do_report_name;
    normal_report_name.Format("%s.rpt", pf->name);
	do_report_name.Format("%sdo.rpt", pf->name);
	pf->PrintOneFormOnly(normal_report_name);
	pf->PrintOneFormOnly(do_report_name);
	s.Format("Delete from [%s] where [%s] = '%s'", df->name, df->fieldName[1], df->dat[1]);
    RunSql(s);
    DB sub_table;
    CString sub_Table_Name;
    if(!strcmp(pf->name, "WB")) {
        sub_Table_Name = "wbdata";
    }
    else {
        sub_Table_Name = "dbdata";
    }
    tado(&sub_table, sub_Table_Name);
    strSQL.Format("select * from fdata where refno = '%s'", df->dat[1]);
    DB adoTmp;
    ado(&adoTmp, "", NULL, strSQL);
    long rec  = read_sumindex(0, &adoTmp);
    if(rec > 0)
    {
        for(int i = 1; i <= rec; i++)
        {
            readrec(i, &adoTmp);
            fetch(&sub_table, &adoTmp);
            sub_table.AddNew();
        }   
        strSQL.Format("delete fdata where refno = '%s'", df->dat[1]);
        RunSql(strSQL);
    }
    dclose(&adoTmp);
    dclose(&sub_table); //TODO: investigate why require instantiate another DB pointer or else once Added New, The weighing entry facing system crash
    return TRUE;
}

void CIt2::OnCancel()
{
    int flag = MessageBox("Exit","Confirm it ?",MB_YESNO | MB_ICONQUESTION);
    if(flag == IDYES) {
        CDialog::OnCancel();
    }
}
