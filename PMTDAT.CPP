// pmtdat.cpp : implementation file
//

#include "stdafx.h"
#include "db_ado.h"
#include "openrs.h"
#include "menu.h"
#include "pmtdat.h"
#include "picgrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Cpmtdat dialog


Cpmtdat::Cpmtdat(UINT IDD,DB *pdf,int Idx,CWnd* pParent /*=NULL*/)
    : CStaff(IDD,pdf,Idx,pParent)
{
    //{{AFX_DATA_INIT(Cpmtdat)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void Cpmtdat::DoDataExchange(CDataExchange* pDX)
{
    CStaff::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(Cpmtdat)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Cpmtdat, CStaff)
    //{{AFX_MSG_MAP(Cpmtdat)
    ON_WM_DESTROY()
    ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Cpmtdat message handlers
void Cpmtdat::ShowInformation()
{
    CStaff::ShowInformation();

    char ch = toupper( df->dat[Pm_DIRECTION][0] );
    CString s = "";
    if(ch=='I') {
        s = "(Prime Mover In) (Weighing Sq 3)";
    }
    else if(ch=='O') {
        s = "(Prime Mover Out) (Weighing Sq 2)";
    }
    SetDlgItemText(IDC_LABEL7, s);
}


BOOL Cpmtdat::OnInitDialog()
{
    CStaff::OnInitDialog();

    // TODO: Add extra initialization here

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void Cpmtdat::OnDestroy()
{
    CStaff::OnDestroy();

    // TODO: Add your message handler code here

}

void Cpmtdat::OnTimer(UINT nIDEvent)
{
    // TODO: Add your message handler code here and/or call default

    CStaff::OnTimer(nIDEvent);
}


void AdminDbpmtdat(DB *df)
{
    if(df->ID==0) {
        AfxMessageBox("Database not open");
        return;
    }
    Cpmtdat p1(df->ID,df,df->f[1].idx & 0x01);
    p1.DoModal();
}

int pmtdat_AdminDb(DB *df,int flag)
{
    AdminDbpmtdat(df);
    return 1;
}

int tell_pmtdat(UINT nChar,int &id,int &fld,DB *df,int sq)
{
    return 1;
}

int start_pmtdat(DB *df)
{
    df->SetCurrentTime(Pm_TIMEIN);
    df->SetCurrentTime(Pm_DATE);
    return 1;
}

int initial_pmtdat(DB *df,int *fld)
{
    int j = 0;
    if(atoi(user.dat[UR_LEVEL])<9) {
        fld[j++] = Pm_TIMEIN;
        fld[j++] = Pm_WEIGHT;
        fld[j++] = Pm_DIRECTION;
        fld[j++] = Pm_TICKET;
        fld[j++] = Pm_DATE;
    }
    fld[j++] = 2;
    fld[j++] = 5;
    fld[j++] = 6;
    return 1;
}

int save_pmtdat(DB *df,int flag)
{
    return 1;
}

void Relationships_pmtdat(DB *df)
{
}

void init_pmtdat(DB *df)
{
    df->e->AdminDb = pmtdat_AdminDb;
    df->e->tell = tell_pmtdat;
    df->e->start = start_pmtdat;
    df->e->initial = initial_pmtdat;
    df->e->save = save_pmtdat;
    df->e->Relationships = Relationships_pmtdat;
}



void PrintPm1(DB *df)
{
}

void PrintPm2(DB *df)
{
}

void Cpmtdat::OnButton1() 
{
	int fld = GetDbFieldNo(df,"JPGKEY1");
    int jpgkey = sub->lval( fld);
    if(jpgkey==0) return;

    CPicGrid p;

    p.jpgkey = jpgkey;
    p.TotalPhotos = 3;
    p.m_caption.Format("View Photos %i", jpgkey);
    p.DoModal();

}



