// Gst.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "multisort.h"

void PrintGst(long irec,FILE *ptr,DB *df)
{
    ropen(&report,"pgst.rep");
    StdPrintTableFormat(&one, df, &report, 0);
    dclose(&report);
}

void init_gst(DB *df)
{
    df->e->prec  = PrintGst;
}
