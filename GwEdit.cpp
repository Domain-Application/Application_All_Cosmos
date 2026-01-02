// GwEdit.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "GwEdit.h"
#include "GetWt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGwEdit

CGwEdit::CGwEdit()
{
    flag = 0;
    OldChar = 0;
}

CGwEdit::~CGwEdit()
{
}


BEGIN_MESSAGE_MAP(CGwEdit, CEdit)
	//{{AFX_MSG_MAP(CGwEdit)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGwEdit message handlers

UINT CGwEdit::OnGetDlgCode() 
{
	return CEdit::OnGetDlgCode() | DLGC_WANTALLKEYS;
}

void CGwEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CGwEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if(nChar>=1 && nChar<=10) {
		if (nChar!=8){
			GwstWeight = atol(gst.dat[nChar]);
		}
		else{
			if (nFlags==35) GwstWeight = atol(gst.dat[nChar]);
			//else GwstWeight = 0;
		}
    }
    else if(nChar==32 && OldChar==32) {
        GwstWeight = 0;
    }
	CEdit::OnChar(nChar, nRepCnt, nFlags);
    OldChar = nChar;
}

void CGwEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    CGetWt *p;

    p = (CGetWt *) GetParent();
    if(nChar==VK_RETURN && flag==1) {
         p->OnOK();
	}
    else if(nChar==VK_ESCAPE) {
        p->OnCancel();
	}
	CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
    flag = 1;
}
