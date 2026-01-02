// Pt.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "multisort.h"

void PrintProductCode(long irec,FILE *ptr,DB *df)
{
    ropen(&report,"pprodtype.rep");
    StdPrintTableFormat(&one, df, &report, 0);
    dclose(&report);
}

void init_pt(DB *df)
{
    df->e->prec  = PrintProductCode;
}
