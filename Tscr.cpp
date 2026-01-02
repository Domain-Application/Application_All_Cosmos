// Tscr.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Tscr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTscr dialog

CTscr::CTscr(UINT IDD,DB *pdf,int Idx = 0,void(*p)() = NULL,CWnd* pParent /*=NULL*/)
    : CTio(IDD,pdf,Idx,p,pParent)
{
	//{{AFX_DATA_INIT(CTscr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTscr::DoDataExchange(CDataExchange* pDX)
{
    CTio::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTscr)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTscr, CTio)
	//{{AFX_MSG_MAP(CTscr)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTscr message handlers

void CTscr::show_tio()
{
    long rec;

    TmpTio[0] = 0;
    rec    = qd(tscr.dat[Ts_PT],&pt);
    if(rec>0){
        strcpy(TmpTio,"Product : ");
        strcat(TmpTio,pt.dat[2]);
    }
    rec    = qd(tscr.dat[Ts_MILL],&mill);
    if(rec>0){
        strcat(TmpTio,"\nDelivery To: ");
        strcat(TmpTio,mill.dat[2]);
    }
    _PL[1]  = TmpTio;
}


int DailyTransferFilter(DB *df)
{
    if ( !wildcmp(df->dat[TR_MILL],tscr.dat[Ts_MILL])) {
        if ( !wildcmp(df->dat[TR_VEHICLE],tscr.dat[Ts_VECHICLE])) {
            if ( !wildcmp(df->dat[TR_PT],tscr.dat[Ts_PT])) {
                return 1;
            }
        }
    }
    return 0;
}
