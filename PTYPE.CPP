// ptype.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "multisort.h"

void PrintPType(long irec,FILE *ptr,DB *df)
{
    ropen(&report,"pptype.rep");
    StdPrintTableFormat(&one, df, &report, 0);
    dclose(&report);
}

void init_ptype(DB *df)
{
    df->e->prec  = PrintPType;
}
