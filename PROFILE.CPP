// Profile.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Profile.h"
#include "db_ado.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProfile dialog


CProfile::CProfile(UINT IDD,DB *pdf,int Idx,CWnd* pParent /*=NULL*/)
    : CStaff(IDD,pdf,Idx,pParent)
{
    //{{AFX_DATA_INIT(CProfile)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CProfile::DoDataExchange(CDataExchange* pDX)
{
    CStaff::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CProfile)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProfile, CStaff)
    //{{AFX_MSG_MAP(CProfile)
    ON_WM_DESTROY()
    ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnGWST)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfile message handlers
void CProfile::ShowInformation()
{
    CStaff::ShowInformation();
}


BOOL CProfile::OnInitDialog()
{
    CStaff::OnInitDialog();

    int ShiftKeyStatus = GetKeyState(VK_SHIFT);
 
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CProfile::OnDestroy()
{
    CStaff::OnDestroy();

    // TODO: Add your message handler code here

}

void CProfile::OnTimer(UINT nIDEvent)
{
    // TODO: Add your message handler code here and/or call default

    CStaff::OnTimer(nIDEvent);
}


void AdminDbProfile(DB *df)
{
    if(df->ID==0) {                           
        AfxMessageBox("Database not open");   
        return;                               
    }                                         
    CProfile p1(df->ID,df,df->f[1].idx & 0x01);
    p1.DoModal();                             
}

#include "Keyin1Fld.h"

void CProfile::OnButton1() 
{
    CKeyin1Fld h;

    h.m_title = "Company Reg. No:";
    if(h.DoModal()!=IDOK) return;

	CString str;
	
	str.Format("Update profile set [%s] = %s", 
        df->fieldName[1], DbText(df,1, h.m_amt));
	RunSql(str);

	df->ReOpen();
	CDialog::OnOK();
}

void CProfile::OnGWST() 
{
/*
	gst.ReOpen();
	gst.read(1);
    int nID = gst.fedit(0,TRUE);
	if(nID==IDOK) {
		gst.SqlUpdate();
    } */
}
