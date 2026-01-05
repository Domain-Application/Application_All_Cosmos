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
 
CString isOverToleranceWeight(DB *df)
{
	CString test;
	test.Format("%f %f %f", df->val(PGROSS), df->val(PTARE), df->val(WASTAGE));
    double nett_wt = abs(df->val(PGROSS) - df->val(PTARE));
    double supplier_wt = df->val(SWT); 
    double tolerance_range = profile.val(GetDbFieldNo(&profile, "TOLERANCE_RANGE"));
    if(tolerance_range <= 0 || supplier_wt <= 0) 
        return "";
    double tolerance_percent;
    if(supplier_wt >= nett_wt)
    {
        tolerance_percent = (nett_wt/supplier_wt) * 100;
    }
    else
    {
        tolerance_percent = (supplier_wt/nett_wt) * 100;
    }
    CString msg;
    msg.Format("Nett Weight: %.0f kg\nSupplier Weight: %.0f\nTolerance Percentage Setting: %.0f%%\nTolerance Percentage: %.0f%%\nPlease check.", nett_wt, supplier_wt, tolerance_range, tolerance_percent);
    if(tolerance_percent > tolerance_range) 
        return msg;
    return "";
}

void CIt2::OnOK()
{
#ifdef _DEBUG 
	
#else
   if( BarrierStatus(1) ) {
		MessageBox("Cannot weighing!!!","Barrier 1 still open", MB_ICONSTOP);
		return;
	}

	if( BarrierStatus(2) ) {
		MessageBox("Cannot weighing!!!","Barrier 2 still open", MB_ICONSTOP);
		return;
	}
#endif
	CString message = IsBlacklistedDriver(df,df->dat[DRIVER]); //brian//check blacklist for driver1 and driver2//20251223
	if(!isblank(message) && UserPasswordLoginAccepted(df, 9, message) == 0)
	{
		return;
	}
	
	CString binnumber;
	binnumber=df->dat[BIN];
	binnumber.TrimRight();
	if(binnumber.GetLength()>0)
	{
		if (OnBin_WeightOut_Info(df,m_vehicle)==FALSE)//20251229
		{
			return;
		}
		else
		{
			CString strsql;
			strsql.Format("UPDATE [BIN] SET INYARD = 0, OUTYARD_TIME = GETDATE() where CODE='%s'",df->dat[BIN]);
			RunSql(strsql);
		}
	}

    if(iType==1) {			//receiving only
		if(m_contno.GetLength() != 11 && df->val(SWT) <= 0) 
		{
			UINT ID = MessageBox("Cannot weighing!!!\n\nDo you want to continue?", "Supplier weight is required for tanker", MB_ICONQUESTION | MB_YESNO);
			if(ID==IDNO) return;
		}
	}

	ReplaceRecord();
    
    if (IsSaveConditionOk(df)==0) return;
    CString msg = isOverToleranceWeight(df);
    if(m_contno.GetLength() != 11 && !isblank(msg) && UserPasswordLoginAccepted(df, 9, msg) == 0)
    {
		return;
    }
    int key = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
    push_plateno2.Format("");
    if(program4 == 1) {
		CDialog::OnOK();
        requireAutoOpenFrontBarrier = TRUE;
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

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CIt2::LoadImages()
{
     int TotalCameras = 3;
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

BOOL isShuntingLorry(CString lorryNo)
{
    lsetblank(&lorry);
    lset(lorry.dat[1], lorryNo);
    if(qd(lorry.dat[1], &lorry) > 0 && lorry.Boolean(GetDbFieldNo(&lorry, "IS_SHUNTING_LORRY"))) {
        return TRUE;
    }
    return FALSE;
}


void CSecurity::OnSecond(BOOL LPR_Status, BOOL CNR_Status)
{
	CString strSQL,binticket,pmnumber;
	strSQL.Format("SELECT top 1 ticket,[PM1NO] FROM [first] where lorry='%s' and BIN is not null order by timein desc", m_vehicle);
	binticket=SqlText(strSQL,1);
	pmnumber=SqlText(strSQL,2);
	binticket.TrimRight();

	
	pmnumber.TrimRight();

	if(m_contno.GetLength()==11) {
		lset(df->dat[LORRY], m_contno);
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
	else if(binticket.GetLength()>0) {
		if(pmnumber.GetLength()>0)
		{
			lset(df->dat[LORRY], m_vehicle);
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

	}
	else {
		lset(df->dat[LORRY], m_vehicle);
        //lset(df->dat[PM1NO], "");			//2025-12-18
        //lset(df->dat[PM2NO], "");
	}
	
    int fldPrintCounter = GetDbFieldNo(df ,_szPrintCounter);
    int type =  df->lval(fldPrintCounter);
	
	ltodb(cweight, df, PTARE);
	ltodb(cweight, df, W2);
	ltodb(cweight, df, Z2);

	lset(df,"I2", strIndicator(program4) );
    lset(df->dat[USER2], user.dat[1]);

	//df->SetCurrentTime(Date); //brian//20251220
	df->SetCurrentTime(TIMEOUT);
	df->SqlUpdate();

    CIt2 h;
	h.m_contno = m_contno;
	h.m_vehicle = m_vehicle;
    h.df = df;
    h.iType = type;

    UINT ID = h.DoModal();
	if(ID==IDCANCEL) return;

	STCR = type;
    DB *pf = (type==1) ? &wb : &despatch;

	lsetblank(pf);
	ltodb(0, df, fldPrintCounter);			//clear print counter
    fetch(pf, df);
    //FindAddInfoTicketNo(pf,1); //first time create add info, second no need create
	SaveImages(pf,"JPGKEY2");
    ltodb(LPR_Status, pf,  GetDbFieldNo(pf, "OUT_LPR_SUCCESS"));
    ltodb(CNR_Status, pf,  GetDbFieldNo(pf, "OUT_CNR_SUCCESS"));
    pf->AddNew();
    IsZeroWt = FALSE;
    // pView->OnPrintTicket(pf);
    CString msg, normal_report_name, a5_report_name;
    normal_report_name.Format("%s.rpt", pf->name);
    a5_report_name.Format("%s_A5.rpt", pf->name);

	CString tMsg = (type==1) ? "Receiving" : "Despatch";
    msg.Format("Print %s Ticket No.: %s? Press Yes for Dot Matrix paper, No for Triplicate A5 paper", 
		tMsg, pf->dat[1]);

    CString tCaption;
	tCaption.Format("Confirm %s Weighing", tMsg);

	UINT print = MessageBox(msg, tCaption, MB_ICONQUESTION | MB_YESNOCANCEL);
    if(print == IDYES) {
        pf->PrintOneFormOnly(normal_report_name);
    }
    else if(print == IDNO) {
        for(int i=1;i<=3;i++) 
        {
            pf->PrintOneFormOnly(a5_report_name);
        }
    }

	s.Format("Delete from [%s] where [%s] = '%s'", df->name, df->fieldName[1], df->dat[1]); //brian//20251230//no use first table so no need delete
	runsql(s);
	OnOpenBarrier(ID);
}

void CIt2::OnCancel()
{
    int flag = MessageBox("Exit","Confirm it ?",MB_YESNO | MB_ICONQUESTION);
    if(flag == IDYES) {
        CDialog::OnCancel();
    }
}


CString IsBlacklistedDriver(DB *df,CString drivercode) //brian//2025-12-22//add check blacklisted driver function
{
	CString strSQL,message;
	strSQL.Format("SELECT BLACKLISTED FROM DRIVER WHERE CODE='%s'",drivercode);
	message.Format("Driver %s is blacklisted driver.\nPlease check.",drivercode);
	if(SqlValue(strSQL)==1)
	{
		return message;
	}
	else
	{
		return "";
	}
}

BOOL OnBin_WeightOut_Info(DB *df,CString m_vehicle) //brian//28-12-2925
{
	
	CString strsql,str,binno,outlorry,ticket;
	binno=df->dat[BIN];
	binno.TrimRight();
	strsql.Format("SELECT REFNO,TICKET, INYARD_FLAG from BINDAT where [BIN]='%s' order by date desc",binno);
	ticket=SqlText(strsql,2);
	if(SqlValue(strsql,3)==1) 
	{	
		//strsql.Format("SELECT TOP(1) IS_RORO from PMTDAT where [PMNO]='%s' AND DIRECTION='I' order by date desc",m_vehicle);
	
			strsql.Format("UPDATE BINDAT SET LORRYOUT='%s', LORRYOUT_TIME=GETDATE(), OUTDRIVER='%s',INYARD_FLAG=1 WHERE TICKET='%s'",m_vehicle,df->dat[DRIVER],ticket);
			RunSql(strsql);
			/*lset(bindat.dat[Bin_LORRYOUT],df->dat[LORRY]);
			lset(bindat.dat[Bin_LORRYOUT],df->dat[TIMEOUT]);
			lset(bindat.dat[Bin_OUTDRIVER],df->dat[DRIVER]);
			ltodb(0,&bindat,Bin_INYARD_FLAG);
			bindat.Update();*/
			return TRUE;
		
	/*	else
		{
			str.Format("Prime mover %s is not roro type.",m_vehicle);
			AfxMessageBox(str);
			return FALSE;
		}*/
	
	}
	else
	{
		//check bin is outside yard means no existing bin to let lorry carry out
		str.Format("Bin No %s is still out side yard.", binno);
		AfxMessageBox(str);
		return FALSE;
	}
	
}

