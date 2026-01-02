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

int tell_packing(UINT nChar,int &flag,int &fld,DB *df,int sq)
{
    switch(fld) {
    case Pk_WEIGHT:
        flag = DO;
        break;
    }
    double total = atof(df->dat[Pk_NETWT])  + atof(df->dat[Pk_PW]);
    dtodb(total,df,Pk_WEIGHT);
    return 2;
}

void init_packing(DB *df)
{
    df->e->tell    = tell_packing;
}
