// caegory.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "multisort.h"

void PrintCat(long irec,FILE *ptr,DB *df)
{
    ropen(&report,"pcat.rep");
    StdPrintTableFormat(&one, df, &report, 0);
    dclose(&report);
}

void init_category(DB *df)
{
    df->e->prec  = PrintCat;
}
