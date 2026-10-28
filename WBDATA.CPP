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

// Detail-line grid columns, shared by the ticket screen and the lane second-weight screen.
void SetDetailGridTitles(MGRID &fg)
{
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
    fg.SetTitle("DO Qty (Kg)",1300,7);
    fg.SetTitle("Bin Location",1300);
}

void SetDetailGridRow(MGRID &fg, DB *sub)
{
    fg.SetText(sub,NULL,Wd_DO,Wd_PO1,Wd_PO2,Wd_PQTY,Wd_PT,Wd_PT | 0x100,
          Wd_QTY,Wd_UNIT,Wd_PACKING,Wd_UW,
          Wd_NoPallet,Wd_PalletWt,Wd_TW,Wd_DO_QTY,Wd_BIN_LOCATION,
          -1);
}
