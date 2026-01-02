// Packing.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Wb.h"
#include "mgrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//

int tell_wbdata(UINT nChar,int &flag,int &fld,DB *df,int sq)
{
    switch(fld) {
    case Wd_PT:
        if(sq) {
            lset(df->dat[Wd_PACKING], prodtype.dat[PT_PACKING]);
            qd(df->dat[Wd_PACKING], &packing);
            double uw = packing.val(Pk_WEIGHT);
            dtodb(uw, df, Wd_UW);
        }
        break;
    case Wd_PACKING:
        if(sq) {
            double uw = packing.val(Pk_WEIGHT);
            dtodb(uw, df, Wd_UW);
        }
        break;
    case Wd_TW:
        flag = DO;
        break;
    }
    double total = df->val(Wd_QTY) * df->val(Wd_UW) + df->val(Wd_PalletWt);
    dtodb(total,df,Wd_TW);
    return 2;
}

void init_wbdata(DB *df)
{
    df->e->tell = tell_wbdata;
}
