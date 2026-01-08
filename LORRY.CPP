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

    int i = df->lval(Lo_INDICATOR);
    SetDlgItemText(IDC_VAR2, strIndicator(i) );

}


BOOL CLorry::OnInitDialog()
{
    CStaff::OnInitDialog();

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

	char tmp[99];
    sprintf(tmp,"%6ld",cweight);
    SetDlgItemText(IDC_WT, putnum0(tmp));
	current_weight = cweight;
	
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

int Lorry_initial(DB *df,int *fld)                //initial setting
{
    int j= 0;
    fld[j++] = Lo_INDICATOR;
    if(atoi(user.dat[UR_LEVEL])<99) {
        fld[j++] = Lo_LPRNO;
        fld[j++] = Lo_TARE;
    }
	return 1;
}

int lorry_save(DB *df,int flagi)
{
    if( df->IsBlank(Lo_LPRNO) ) {
		lset( df->dat[Lo_LPRNO], df->dat[1]);
    }
    return 1;
}

void init_lorry(DB *df)
{
    df->e->AdminDb = Lorry_AdminDb;
    df->e->initial = Lorry_initial;
    df->e->save     = lorry_save;
}

void CLorry::OnButton1() 
{
    if(SaveFlag == CANNOTSAVE) return;

    df->FldDefault(Lo_TIME);
    ltodb(cweight, df, Lo_TARE);

    ltodb ( program4, df, Lo_INDICATOR);
	ShowInformation();
}
