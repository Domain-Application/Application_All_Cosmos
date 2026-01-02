//
// Accode.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "mgrid.h"
#include <afxctl.h>
#include "picture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Accode

void ReplaceAccodeGrid(long rec,CMSFlexGrid *grid,DB *df,int IdxItem)
{
    MGRID g(grid,df,rec);
    g.SetText(df,0,1,2,CU_TEL,CU_FAX,-1);
}

void SetAccodeGridTitle(CMSFlexGrid *grid,DB *df,int IdxItem)
{
    MGRID g(grid,df,0,5);
    g.SetTitle(0);

    g.SetTitle("Code ID",1350);
    g.SetTitle("Name",3500);
    g.SetTitle("Tel. No",1250,7);
    g.SetTitle("Fax. No",1250,7);

    grid->SetCol(3);
    CPictureHolder pict;
    pict.CreateFromIcon(IDI_TEL);
    CPicture picture(pict.GetPictureDispatch());
    grid->SetRefCellPicture(picture);
    grid->SetRowHeight(0, 550);

    grid->SetCol(4);
    pict.CreateFromIcon(IDI_FAX);
    CPicture picture2(pict.GetPictureDispatch());
    grid->SetRefCellPicture(picture2);
    grid->SetRowHeight(0, 550);
}

void init_accode(DB *df)
{
    df->e->rep       = ReplaceAccodeGrid;
    df->e->GridTitle = SetAccodeGridTitle;
}
