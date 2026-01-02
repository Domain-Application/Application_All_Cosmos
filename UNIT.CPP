// unit.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "multisort.h"

void PrintUnit(long irec,FILE *ptr,DB *df)
{
    ropen(&report,"punit.rep");

    StdPrintTableFormat(&one, df, &report, 0);

    dclose(&report);
}

void init_unit(DB *df)
{
    df->e->prec  = PrintUnit;
}
