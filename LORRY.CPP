// Lorry.cpp : implementation file
//

#include "stdafx.h"
#include "db_ado.h"
#include "openrs.h"
#include "menu.h"
#include "Lorry.h"
#include "comm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLorry dialog


CLorry::CLorry(UINT IDD,DB *pdf,int Idx,CWnd* pParent /*=NULL*/)
    : CStaff(IDD,pdf,Idx,pParent)
{
    //{{AFX_DATA_INIT(CLorry)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    busy = FALSE;
}


void CLorry::DoDataExchange(CDataExchange* pDX)
{
    CStaff::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CLorry)
	DDX_Control(pDX, IDC_WB1, m_wb1);
	DDX_Control(pDX, IDC_WB2, m_wb2);
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLorry, CStaff)
    //{{AFX_MSG_MAP(CLorry)
    ON_WM_DESTROY()
    ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLorry message handlers
void CLorry::ShowInformation()
{
    CStaff::ShowInformation();
}


BOOL CLorry::OnInitDialog()
{
    CStaff::OnInitDialog();

    strcpy(cmd1,"W1");
    strcpy(cmd2,"W2");
    strWM = profile.dat[Pro_WtServer];
    strWM.TrimRight();
	int offset = instr(1, strWM, ",");
	if(offset==0) {
        strWM2 = strWM;
	}
    else {
        char szTmp[10000];

        strcpy(szTmp, strWM);
        szTmp[offset-1] = 0;
        strWM  = szTmp;
        strWM2 = &szTmp[offset];
        strcpy(cmd2,"W1");
    }

    int flag1 = ClientRequest(strWM, cmd1 , errmsg);
    if(flag1==-1) {
        strWM = "";
        CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
        GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
        GetDlgItem(IDC_WB1)->EnableWindow(FALSE);
    }
    else {
        SetDlgItemText(IDC_WB1, strWM);
    }

    int flag2 = ClientRequest(strWM2, cmd2 , errmsg);
    if(flag2==-1) {
        strWM2 = "";
        CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
        GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
        GetDlgItem(IDC_WB2)->EnableWindow(FALSE);
    }
    else {
        SetDlgItemText(IDC_WB2, strWM2);
    }

    if(flag2==-1 && flag1==-1) {
        MessageBox("Weight server not found");
        OnCancel();
        return TRUE;
    }
    else if(flag2==0 && flag1==0) {
        if(app->m_strComputerName.CompareNoCase(strWM)==0) {
            CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
        }
        else if(app->m_strComputerName.CompareNoCase(strWM2)==0) {
            CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
        }
        else {
            CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1+current_indicator-1);
        }
    }

    SetTimer(1,500,NULL);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CLorry::OnDestroy()
{
    CStaff::OnDestroy();
    KillTimer(1);
}

void CLorry::OnTimer(UINT nIDEvent)
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


void AdminDbLorry(DB *df)
{
    if(df->ID==0) {                           
        AfxMessageBox("Database not open");   
        return;                               
    }                                         
    CLorry p1(df->ID,df,df->f[1].idx & 0x01);
    p1.DoModal();                             
}

int Lorry_AdminDb(DB *df,int flag)
{
    AdminDbLorry(df);
    return 1;
}

void init_lorry(DB *df)
{
    df->e->AdminDb = Lorry_AdminDb;
}

void CLorry::OnButton1() 
{
    if(SaveFlag == CANNOTSAVE) return;

    df->FldDefault(Lo_TIME);
    ltodb(current_weight, df, Lo_TARE);
	ShowInformation();
}
