// Pt.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "multisort.h"

void PrintProductCode(long irec,FILE *ptr,DB *df)
{
    ropen(&report,"ppt.rep");

    StdPrintTableFormat(&one, df, &report, 0);

    dclose(&report);
}

void init_product(DB *df)
{
//    df->e->prec  = PrintProductCode;
}
