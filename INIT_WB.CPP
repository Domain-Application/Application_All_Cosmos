// Wb.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Wb.h"
#include "comm.h"
#include "mgrid.h"

/////////////////////////////////////////////////////////////////////////////
// CWb dialog

void ReplaceReceiveGrid(long rec,CMSFlexGrid *grid,DB *df,int IdxItem)
{
    MGRID g(grid,df,rec);

    g.SetText(df,0,1,Date,ACCODE,ACCODE | 0x100,LORRY,TRANSP | 0x100,-1);
    compute_weight(df, df->sub);
    g.SetText(m[11],6,0);
}

void SetReceiveGridTitle(CMSFlexGrid *grid,DB *df,int IdxItem)
{
    MGRID g(grid,df,0,8);

    g.ratio = float(1.0);
    g.SetTitle(df,0,1,Date,ACCODE,-1);
    g.SetTitle("Name",2500);
    g.SetTitle("Vehicle No",1000);
    g.SetTitle("Product Description",3000);
    g.SetTitle("Net Weight",900,7);
}

int WB_initial(DB *df,int *fld)                //initial setting
{

    fld[0] = I1;
    fld[1] = I2;
    fld[2] = I3;
    fld[3] = I4;
    if(atoi(user.dat[UR_LEVEL])<10) {
        fld[4] = PM1TIME;
        fld[5] = PM1WT;
        fld[6] = PM2TIME;
        fld[7] = PM2WT;
        fld[8] = PGROSS;
        fld[9] = PTARE;
        fld[10] = TIMEIN;
        fld[11] = TIMEOUT;
    }
	return 1;
}

int tell_receive(UINT nChar,int &id,int &fld,DB *df,int sq)
{
    compute_weight(df,df->sub);
	switch(fld) {
    case PM1NO:
        if(sq) {
            long wt = pm1no.val(Lo_TARE);
            ltodb(wt, df, fld+2);
            lset(df->dat[fld+1], pm1no.dat[Lo_TIME]);
            lset(df->dat[USER3], user.dat[1]);
        }
        break;
    case PM2NO:
        if(sq) {
            long wt = pm2no.val(Lo_TARE);
            ltodb(wt, df, fld+2);
            lset(df->dat[fld+1], pm2no.dat[Lo_TIME]);
            lset(df->dat[USER4], user.dat[1]);
        }
        break;
    }
    if(isblank(df->dat[PM1NO])) {
        fillblank(PM1TIME,df);
        fillblank(PM1WT,df);
    }
    if(isblank(df->dat[PM2NO])) {
        fillblank(PM2TIME,df);
        fillblank(PM2WT,df);
    }
    return 2;
}

int start_receive(DB *df)
{
    dbasedate(df->dat[Date]);
    lset(df->dat[STATUS],"P");
    lset(df->dat[TT],"N");
    return 1;
}

int receive_save(DB *df,int flagi)
{
    if( df->val(PM2WT)==0) {
        lset(df->dat[PM2TIME],"");
    }
    if( df->val(PM1WT)==0) {
        lset(df->dat[PM1TIME],"");
    }
    return 1;
}

void init_wb(DB *df)
{
    df->e->rep       = ReplaceReceiveGrid;
    df->e->GridTitle = SetReceiveGridTitle;
    df->e->tell      = tell_receive;
    df->e->start     = start_receive;
    df->e->save      = receive_save;
    df->e->initial   = WB_initial;
}

double m[16];
void compute_weight(DB *df, DB *sub)
{
    memset(m,0,sizeof(m));
    if(isblank(df->dat[TIMEOUT])) {
        return;
    }

    double weight1 = df->val(PGROSS);
    double weight2 = df->val(PTARE);
    double wastage = df->val(WASTAGE);

    m[0] = df->val(SWT);
    m[7] = df->val(PM1WT);
    m[8] = df->val(PM2WT);
    m[9] = fabs(m[7]-m[8]);

    m[1] = m[2] = weight1 - m[7];
    m[4] = m[5] = weight2 - m[8];
    m[3] = wastage;

    m[10] = m[11] =  fabs(m[5]-m[2]) - wastage;


    m[15] = m[11] - m[0];
}
