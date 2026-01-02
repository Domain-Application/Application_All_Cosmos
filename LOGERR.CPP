// Logerr.cpp : implementation file
//

#include "stdafx.h"
#include "Logerr.h"
#include "menu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogerr dialog


CLogerr::CLogerr(CWnd* pParent /*=NULL*/)
	: CDialog(CLogerr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogerr)
	m_user = _T("");
	m_pass = _T("");
	m_purpose = _T("");
	//}}AFX_DATA_INIT
    level = 0;
	backup(&user);
}


CLogerr::~CLogerr()
{
	restore(&user);
}

void CLogerr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLogerr)
	DDX_Text(pDX, IDC_EDIT2, m_user);
	DDX_Text(pDX, IDC_EDIT3, m_pass);
	DDX_Text(pDX, IDC_EDIT4, m_purpose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLogerr, CDialog)
	//{{AFX_MSG_MAP(CLogerr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogerr message handlers

void CLogerr::OnOK() 
{
	DB logerr;
	tado(&logerr,"logerr");

	UpdateData(TRUE);

	lset(user.dat[1],m_user);
	long xrec = qd(user.dat[1],&user);
	if(xrec==0) {
        xrec = find(user.dat[1], 1, 1, &user, TRUE);
        if(xrec==0) {
            MessageBox("User ID not found");
            return;
        }
    }
	char password[20];
	strcpy(password,user.dat[UR_ACCESS]);
	lset(password,m_pass);
	if(strcmp(user.dat[UR_ACCESS],password)) {
		MessageBox("Password incorrect");
		return;
	}
    if(atoi(user.dat[UR_LEVEL])<level) {
        MessageBox("not authorised");
		return;
    }
    waitflock(&logerr);
	long mrec = read_sumindex(0,&logerr) + 1;                  //net concept

    lset(logerr.dat[LGR_DATE],today);
    lset(logerr.dat[LGR_CASHIER],user.dat[1]);
    _strtime(logerr.dat[LGR_TIME]);
    lset(logerr.dat[LGR_TICKET],df->dat[1]);
    lset(logerr.dat[LGR_PASS_TYPE],m_purpose);
    lset(logerr.dat[LGR_PROGRAM],Pref);
    writerec(mrec,&logerr);
    unflock(&logerr);
	dclose(&logerr);
	CDialog::OnOK();
}

void CLogerr::OnCancel() 
{
	
	CDialog::OnCancel();
}

BOOL CLogerr::OnInitDialog() 
{
	CDialog::OnInitDialog();

    SetDlgItemText(IDC_EDIT1,mdate(today));
    GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
