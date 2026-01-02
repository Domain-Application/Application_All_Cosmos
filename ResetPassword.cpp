// ResetPassword.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "ResetPassword.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResetPassword dialog


CResetPassword::CResetPassword(CWnd* pParent /*=NULL*/)
	: CDialog(CResetPassword::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResetPassword)
	m_password = _T("");
	//}}AFX_DATA_INIT
}

void CResetPassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResetPassword)
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Text(pDX, IDC_EDIT1, m_password);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CResetPassword, CDialog)
	//{{AFX_MSG_MAP(CResetPassword)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResetPassword message handlers

void CResetPassword::RunSql(CString str)
{
    m_progress.StepIt();
    ::RunSql(str);
}

void CResetPassword::OnOK() 
{
    UpdateData();


	CString str;
    str.Format("DOMAIN-%s",today);
    if(m_password.Compare(str)) {
        MessageBox("Incorret password");
        return;
    }

    m_progress.SetRange(1,20);

    str = "delete from [fc_wbdata]";
	RunSql(str);

    str = "delete from [wbdata]";
	RunSql(str);

    str = "delete from [fc_dbdata]";
	RunSql(str);

    str = "delete from [dbdata]";
	RunSql(str);

    str = "delete from [prdata]";
	RunSql(str);

    str = "delete from [fc_wb]";
	RunSql(str);

    str = "delete from [wb]";
	RunSql(str);

    str = "delete from [fc_despatch]";
	RunSql(str);

    str = "delete from [despatch]";
	RunSql(str);

    str = "delete from [pr]";
	RunSql(str);

    str = "delete from [accode]";
	RunSql(str);

    str = "DELETE FROM [pt]";
	RunSql(str);

    str = "DELETE FROM [driver]";
	RunSql(str);

    str = "DELETE FROM [transp]";
	RunSql(str);

    str = "DELETE FROM [packing]";
	RunSql(str);

    str = "DELETE FROM [login]";
	RunSql(str);

    str = "DELETE FROM [warehouse]";
	RunSql(str);

    str = "update [ticketno] set [next no] = 1";
	RunSql(str);

    MessageBox("Completed");

	CDialog::OnOK();
}

void CleanDatabase()
{
    CResetPassword h1;

    h1.DoModal();
}

void CResetPassword::OnButton1() 
{
    UpdateData();

	CString str;

    m_progress.SetRange(1,20);

    str = "delete from [rptlog]";
	RunSql(str);

    MessageBox("Completed");

	CDialog::OnOK();
	
}
