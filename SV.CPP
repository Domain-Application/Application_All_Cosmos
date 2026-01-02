
// sv.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Daily.h"
#include "multisort.h"

enum sv {Vr_ACCODE=1,Vr_NAME,Vr_TL,Vr_NET,Vr_AMOUNT,Vr_PRICE};

void rep_sv(double total,DB *df,DB *rep)
{
    fetch_rec(rep,df);
    lset(rep->dat[Vr_NAME],accode.dat[2]);
    dtodb(total,rep,Vr_AMOUNT);
    getmet(m,df);
    ltodb(m[4],rep,Vr_NET);
    memwrt(rep);
}

void save_sv(DB *df,DB *rep)
{
    int i,j;
    double total,wt;

    qd(df->dat[DP_ACCODE],&accode);
    total = 0;
    for(i=0;i<8;i++){
        j = i*4+DP_PT1;
            wt = atof(df->dat[j+2]) * atof(df->dat[j+3]);
            total += wt;
    }
    rep_sv(total,df,rep);
}

void fill_sv()
{
    long rec;
    double qty,price;

    for(rec=1; rec<=report.t->sumrecord; rec++){
        readrec(rec, &report);
        if(!isblank(report.dat[1])){
            qty = atof(report.dat[Vr_NET]);
            if(qty) {
                price = atof(report.dat[Vr_AMOUNT]) / qty;
                dtodb(price,&report,Vr_PRICE);
            }
            fastwrt(rec,&report);
        }
    }
}

DB *ptrsv;
void put_sv()
{
    int  field[] = {1,0};
        if(mloop(&daily,ptrsv,&report,DP_DATE,save_sv)){
            sort(&report,field);
            squ(&report,field);
            fill_sv();
            stdplan(&daily,&report);
        }
}
void AcTotalWeight(DB *df,char *rep)
{

    ropen(&daily,"daily.scr");
    ropen(&report,rep);

    ptrsv = df;
    CDaily t1(IDD_DAILY,&daily,0,put_sv);
    t1.DoModal();

    dclose(&report);
    dclose(&daily);
}
