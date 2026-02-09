// Wb.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Wb.h"
#include "comm.h"
#include "mgrid.h"
#include "FastSearch.h"
#include "Export4ASCII.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int current_indicator = 1;
/////////////////////////////////////////////////////////////////////////////
// CWb dialog


CWb::CWb(UINT IDD,DB *pdf,int Idx,CWnd* pParent /*=NULL*/)
    : CParent(IDD,pdf,Idx,pParent)
{
	//{{AFX_DATA_INIT(CWb)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    ExtraButton1 = IDC_BUTTON2;
    df->m_WhileEditNoPrint = TRUE;
    csFtpUpload   ="export";
    m_FtpUploadEnabled = TRUE;
    busy = FALSE;
    csPrint  ="Ticket";
    csPrint2 ="D/O";
    m_Print2 = TRUE;
    m_SaveAfterPrint=TRUE;
	
}

void CWb::OnPressPrint2()
{
    double weight2 = df->val(PTARE);
    if(weight2==0) {
        MessageBox("Incomplete data","Error Message",MB_ICONSTOP);
        return;
    }

	
	CString msg;
    msg.Format("Do you want to print D/O : %s ?", df->dat[1]);
    UINT ID = MessageBox(msg, "Confirm it", MB_ICONQUESTION | MB_YESNO);
	if(ID==IDNO) return;
    if(STCR == 1) {
        df->PrintOneFormOnly("wbdo.rpt");
    }
    else if(STCR == 2) {
        df->PrintOneFormOnly("dbdo.rpt");
    }
}

void CWb::DoDataExchange(CDataExchange* pDX)
{
    CParent::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWb)
	DDX_Control(pDX, IDC_WB1, m_wb1);
	DDX_Control(pDX, IDC_WB2, m_wb2);
	//}}AFX_DATA_MAP
}

void CWb::SetOneTimeScreen()
{
}

void CWb::ShowInformation()
{
 	char str[10];
	
    CParent::ShowInformation();

    compute_weight(df,sub);

    dtoa(m[11],str,7,0);
    SetDlgItemText(IDC_PNET,putnum0(str));

    dtoa(m[14],str,9,3);
    SetDlgItemText(IDC_VAR,putnum0(str));

    dtoa(m[15],str,9,3);
    SetDlgItemText(IDC_VAR2,putnum0(str));

    SetDlgItemText( IDC_GROSS_CHANGE, (df->val(FIRST) == df->val(Z1))  ? "" : "*");
    SetDlgItemText( IDC_TARE_CHANGE, (df->val(SECOND) == df->val(Z2))  ? "" : "*");

    int fld = GetDbFieldNo(df ,"Print Counter");
    if(!fld) return;

    double i = df->val(fld);
    if(i>0) {
        GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);
    }
    else {
        GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
    }
    if(df->val(SECOND)) {
        GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(FALSE);
    }
    else {
        GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(TRUE);
    }
    if(df->val(PM2WT)) {
        GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(FALSE);
    }
    else {
        GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON5)->EnableWindow(TRUE);
    }
    if(df->val(PM1WT)) {
        GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
    }
    else {
        GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
    }
	if(!strcmp(df->name, "FIRST"))
	{
		GetDlgItem(IDC_BUTTON7)->EnableWindow(TRUE);
	}
	else {
        GetDlgItem(IDC_BUTTON7)->EnableWindow(FALSE);
    }
}

void CWb::ShowInitFrame(CMSFlexGrid *grid,DB *sub)
{
    fg.Initial(grid,sub,2, 0.8F);
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

void CWb::ReplaceSubGrid(long rec,long irec,CMSFlexGrid *m_grid,DB *sub,int IdxItem)
{
    fg.show(rec);
    fg.SetText(sub,NULL,Wd_DO,Wd_PO1,Wd_PO2,Wd_PQTY,Wd_PT,Wd_PT | 0x100,
          Wd_QTY,Wd_UNIT,Wd_PACKING,Wd_UW,
          Wd_NoPallet,Wd_PalletWt,Wd_TW,
		  -1);
}

BEGIN_MESSAGE_MAP(CWb, CParent)
	//{{AFX_MSG_MAP(CWb)
	ON_BN_CLICKED(IDC_BUTTON1, OnPrimeMoverWeighing)
    ON_WM_DESTROY()
    ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, OnConfirmWeight)
	ON_BN_CLICKED(IDC_BUTTON3, OnFirstWt)
	ON_BN_CLICKED(IDC_BUTTON4, OnSecondWt)
	ON_BN_CLICKED(IDC_BUTTON5, OnPM1WT)
	ON_BN_CLICKED(IDC_BUTTON6, OnPM2WT)
	ON_BN_CLICKED(IDC_BUTTON7, OnSetCompleteTicket)
	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWb message handlers

void CWb::OnPrimeMoverWeighing() 
{
	vehicle.Run();	
}

BOOL CWb::OnInitDialog()
{
    CParent::OnInitDialog();

#ifdef _DEBUG
    //return FALSE;
#endif //_DEBUG

    // strcpy(cmd1,"W1");
    // strcpy(cmd2,"W2");
    // strWM = profile.dat[Pro_WtServer];
    // strWM.TrimRight();
	// int offset = instr(1, strWM, ",");
	// if(offset==0) {
    //     strWM2 = strWM;
	// }
    // else {
    //     char szTmp[10000];

    //     strcpy(szTmp, strWM);
    //     szTmp[offset-1] = 0;
    //     strWM  = szTmp;
    //     strWM2 = &szTmp[offset];
    //     strcpy(cmd2,"W1");
    // }

    // int flag1 = ClientRequest(strWM, cmd1 , errmsg);
    // if(flag1==-1) {
    //     strWM = "";
    //     CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
    //     GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
    //     GetDlgItem(IDC_WB1)->EnableWindow(FALSE);
    // }
    // else {
    //     SetDlgItemText(IDC_WB1, strWM);
    // }

    // int flag2 = ClientRequest(strWM2, cmd2 , errmsg);
    // if(flag2==-1) {
    //     strWM2 = "";
    //     CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
    //     GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
    //     GetDlgItem(IDC_WB2)->EnableWindow(FALSE);
    // }
    // else {
    //     SetDlgItemText(IDC_WB2, strWM2);
    // }

    // if(flag2==-1 && flag1==-1) {
    //     MessageBox("Weight server not found");
    //     OnCancel();
    //     return TRUE;
    // }
    // else if(flag2==0 && flag1==0) {
    //     if(app->m_strComputerName.CompareNoCase(strWM)==0) {
    //         CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
    //     }
    //     else if(app->m_strComputerName.CompareNoCase(strWM2)==0) {
    //         CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
    //     }
    //     else {
    //         CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+current_indicator-1);
    //     }
    // }

    // SetTimer(1,500,NULL);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CWb::OnDestroy()
{
    CStaff::OnDestroy();
    KillTimer(1);
}

void CWb::OnTimer(UINT nIDEvent)
{
    if(busy) return;

    busy = TRUE;

    char tmp[200];
	int flag,key = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);

    if(strlen(strWM)) {
        flag = ClientRequest(strWM, cmd1 , errmsg);
        sprintf(tmp,"%6ld",tcpWeight);
        m_wb1.SetWindowText( putnum0(tmp));
        if(key==IDC_RADIO1) {
            current_indicator = 1;
            m_wb1.SetForeColor(RGB(0,0,0));
            m_wb1.SetBorderColor(RGB(255,0,0));
            SetDlgItemText(IDC_WT, putnum0(tmp));
			current_weight = tcpWeight;
        }
        else {
            m_wb1.SetForeColor(RGB(164,164,164));
            m_wb1.SetBorderColor(RGB(0,0,0));
            current_indicator = 2;
        }
    }

    if(strlen(strWM2)) {
        flag = ClientRequest(strWM2, cmd2 , errmsg);
        sprintf(tmp,"%6ld",tcpWeight);
        m_wb2.SetWindowText( putnum0(tmp));
        if(key==IDC_RADIO1) {
            m_wb2.SetForeColor(RGB(164,164,164));
            m_wb2.SetBorderColor(RGB(0,0,0));
            current_indicator = 1;
        }
        else {
            m_wb2.SetForeColor(RGB(0,0,0));
            m_wb2.SetBorderColor(RGB(255,0,0));
            SetDlgItemText(IDC_WT, putnum0(tmp));
			current_weight = tcpWeight;
            current_indicator = 2;
        }
    }
	
    busy = FALSE;
}


void CWb::OnConfirmWeight() 
{
    if(SaveFlag == CANNOTSAVE) return;

    if(isblank(df->dat[TIMEIN])) {
        OnFirstWt();
        return;
    }
    if(isblank(df->dat[TIMEOUT])) {
        OnSecondWt();
    }
}

void CWb::OnSearch()
{
    CFastSearch h1;
    h1.df = df;
    int flag = h1.DoModal();
    if(flag==IDC_BUTTON1) {
        CStaff::OnSearch();
    }
    else if(flag==IDOK) {
        ShowInformation();
    }
}

void CWb::OnFirstWt() 
{
    if(SaveFlag == CANNOTSAVE) return;
    if(!isblank(df->dat[TIMEOUT])) return;

    df->FldDefault(TIMEIN);
    ltodb(current_weight, df, PGROSS);
    ltodb(current_weight, df, W1);
    ltodb(current_weight, df, Z1);
    lset(df->dat[USER1], user.dat[1]);

    char szIndicator[2];

    szIndicator[0] = 'A' + current_indicator - 1;
    szIndicator[1] = 0;
    lset(df,"I1", szIndicator);
    ShowInformation();
}

void CWb::OnSecondWt() 
{
    if(SaveFlag == CANNOTSAVE) return;

    int fld = GetDbFieldNo(df ,"Print Counter");
    if(!fld) return;

    double i = df->val(fld);
    if(i>0) return;

    df->FldDefault(TIMEOUT);
    ltodb(current_weight, df, PTARE);
    ltodb(current_weight, df, W2);
    ltodb(current_weight, df, Z2);
    lset(df->dat[USER2], user.dat[1]);

    char szIndicator[2];

    szIndicator[0] = 'A' + current_indicator - 1;
    szIndicator[1] = 0;
    lset(df,"I2", szIndicator);
    ShowInformation();
}

void CWb::OnAfterPrint()
{
    long x1 = qd(df->dat[PM1NO], &vehicle);
    if(x1) {
    }
}

BOOL CWb::IsCanExit()
{
    return TRUE;
}


void CWb::OnPM1WT() 
{
    if(SaveFlag == CANNOTSAVE) return;
    if(isblank(df->dat[PM1NO])) return;

    int fld = GetDbFieldNo(df ,"Print Counter");
    if(!fld) return;

    double i = df->val(PM2WT);
    if(i>0) return;

    df->FldDefault(PM1TIME);
    ltodb(current_weight, df, PM1WT);
    lset(df->dat[USER3], user.dat[1]);
    char szIndicator[2];

    szIndicator[0] = 'A' + current_indicator - 1;
    szIndicator[1] = 0;
    lset(df,"I3", szIndicator);
    ShowInformation();
}

void CWb::OnPM2WT() 
{
    if(SaveFlag == CANNOTSAVE) return;
    if(isblank(df->dat[PM2NO])) return;

    int fld = GetDbFieldNo(df ,"Print Counter");
    if(!fld) return;

    double i = df->val(SECOND);
    if(i>0) return;

    df->FldDefault(PM2TIME);
    ltodb(current_weight, df, PM2WT);
    lset(df->dat[USER4], user.dat[1]);
    char szIndicator[2];

    szIndicator[0] = 'A' + current_indicator - 1;
    szIndicator[1] = 0;
    lset(df,"I4", szIndicator);
    ShowInformation();
}

void CWb::OnPrint()
{
    CString str,msg;

    double weight2 = df->val(PTARE);
    if(weight2==0) {
        return;
    }

	msg.Format("Do you want to print ticket : %s ?", df->dat[1]);
    UINT ID = MessageBox(msg, "Confirm it", MB_ICONQUESTION | MB_YESNO);
	if(ID==IDNO) return;
	
	long xrec = qd(df->dat[TOLERANT], &tolerant);
    if(xrec) {
        double maxvar = tolerant.val(4);
        double var = abs(m[14]) - maxvar;
        if(var>0) {
            str.Format("Ticket No: %s\r\n\r\nTotal variance :%8.2f kg\r\nMax. tolerant weight: %8.2f kg\r\nOver %8.2f kg.\r\n\r\n",
                df->dat[1], m[14], maxvar, var);
            int flag = UserPasswordLoginAccepted(df,9,str);
            if(flag==0) return;
        }
    }

    CStaff::OnPrint();
}

void CWb::OnFtpUpload()
{
    CExport4ASCII h1;

    h1.df = df;
    h1.sub = sub;
    h1.DoModal();
}

void CWb::OnSetCompleteTicket()
{
	if(!strcmp(df->name, "FIRST")) 
	{
		CString strSQL,ticket;
		DB *maintable;
		DB sub_table;
		CString tablename,msn,s;
		ticket=df->dat[TICKET];
		char prefix = ticket.GetAt(0);

		if (prefix == 'D')
		{
			maintable=&despatch;
			tablename="DBDATA";
			msn="Despatch";
		}
		else if(prefix == 'K')
		{
			maintable=&wb;
			tablename="WBDATA";
			msn="Receive";
		}
		s.Format("Confirm to complete ticket %s to %s Entry ?",ticket,msn);
        if(MessageBox(s, "Confirm it", MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			lsetblank(maintable);		
			fetch(maintable, df);
			maintable->AddNew();
			
			strSQL.Format("Delete from [%s] where [%s] = '%s'", df->name, df->fieldName[1], df->dat[1]);
			RunSql(strSQL);

			tado(&sub_table, tablename);
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
			dclose(&sub_table);
		}
	}
	else
	{
		AfxMessageBox("Cannot convert ticket in this entry");
		return;
	}
	
}




void CWb::OnNew() 
{
	
		CString s,ticketprefix,ticketfield,strSQL,ticketformat;
		int print;
		if(!strcmp(df->name, "FIRST"))
		{
			s.Format("Add new ticket for receive (No for despatch) ?");
			if(MessageBox(s, "Confirm it", MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				ticketprefix="K";
				ticketfield="receive_ticket_info";
				print=1;
			}
			else
			{
				ticketprefix="D";
				ticketfield="despatch_ticket_info";
				print=2;
			}
			strSQL.Format("SELECT [NEXT NO] FROM TicketNo where [recordset]='%s'",ticketfield);
			int ticketno=SqlValue(strSQL);
			ticketformat.Format("%s%09d", ticketprefix, ticketno);
		}
		app->Footmark("  OnNew() - begin");
		int tmp_saveflag = SaveFlag;
		
		if(m_Enabled==FALSE) return;
		if(Status==view__only){
			ErrorBox(IDS_NO_RIGHT, 3018);
			return;                   //View Only
		}
		else if(Status==edit__only){
			ErrorBox(IDS_NO_RIGHT, 3019);
			return;                   //Edit Only
		}
		if(df->m_CannotAdd) {
			//		ErrorBox(IDS_NO_RIGHT, 3017);		//2020-02-22 cannot block still can new
			//		return;
		}
		
		
		//    if (IsOldPeriodLock()) return ;     // 11.7.2013
		
		df->SaveFlag = SaveFlag = NEWFILE;
		
		ShowRefno(SW_HIDE);
		
		NewFlag  = NEW_MODE;
		if(df->NewIsBlank){
			df->lsetblank();
		}
		lset(df->dat[1],"NEW");
		if(!strcmp(df->name, "FIRST"))
		{
			lset(df->dat[1], ticketformat);
			int fldPrintCounter = GetDbFieldNo(df ,_szPrintCounter);
			if(fldPrintCounter > 0) 
			{
				ltodb(print, df, fldPrintCounter);
			}
		}
		backup(df);
		if(OneRecordOnly == TRUE) {                         //21.1.2009
			if(df->m_strReplacePrimaryKey.GetLength()>0) {
				DB adoTmp;
				CString str;
				
				str.Format("select * from [%s] where [%s]='%s' and [%s] like '%s%%' ",
					df->name, df->fieldName[1], adoString(df->m_strReplacePrimaryKey),df->fieldName[1],ticketprefix);
				
				BOOL flagOpen = ado(&adoTmp, "", NULL, str);
				if(!flagOpen) return;
				
				if(adoTmp.t->sumrecord>0) {
					adoTmp.read(1);
					fetch(df, &adoTmp);
					lset(df->dat[1],"NEW");
				}
				dclose(&adoTmp);
			}
		}
		df->e->start(df);
		df->e->InitialValue(df);
		
		if (IsOldPeriodLock()){
			df->SaveFlag = SaveFlag = tmp_saveflag;			//must restore old flag 2019-09-13
			return ;     // 3.9.2013
		}
		OnRecordDisplay();
		SetAttribute(FALSE,TRUE);
		
		AssignReadOnlyFields();
		SetReadOnlyFields();
		//    backup(df);
		HideNoUseButtonMenu();
		int f_fld = GetFirstAvailableField();
		GotoEdit(f_fld);
		ShowSaveButton();
		m_FirstSave = FALSE;
		m_AddEdit = FALSE;
		app->Footmark("  OnNew() - end");
	
}




