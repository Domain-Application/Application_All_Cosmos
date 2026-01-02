// ExpData.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "db_ado.h"
#include "ExpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExpData dialog


CExpData::CExpData(CWnd* pParent /*=NULL*/)
	: CDialog(CExpData::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExpData)
	m_d1 = COleDateTime::GetCurrentTime();
	m_d2 = COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
}


void CExpData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpData)
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
	DDX_Control(pDX, IDC_BUTTON2, m_btn2);
	DDX_Control(pDX, IDC_BUTTON1, m_btn1);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_d1);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_d2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExpData, CDialog)
	//{{AFX_MSG_MAP(CExpData)
	ON_BN_CLICKED(IDC_BUTTON1, OnToXLS)
	ON_BN_CLICKED(IDC_BUTTON2, OnToDbase)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExpData message handlers

void CExpData::OnToXLS() 
{
    OnExport("wbdata.xls");
}

void CExpData::OnToDbase() 
{
    OnExport("wbdata.dbf");
}

void CExpData::OnExport(const char *file)
{
    UpdateData();

    lset(two.dat[1],m_d1);
    lset(two.dat[2],m_d2);
    str.Format("select * from [%s] where [date]>=%s and [date]<=%s",
       df->name ,DbText(&two,1),DbText(&two,2));

    ado(&Exp, "", NULL, str);
    dopen(&report, file);
    memstart(&report);

    long last = Exp.t->sumrecord;
    m_progress.SetRange32(1,last);
    fetch_set(&report,&Exp);
    for(long rec=1; rec<=last; rec++) {
        readrec(rec,&Exp);
        m_progress.SetPos(rec);
        fetch_rec(&report,&Exp);
        memwrt(&report);
    }
    memend(&report);
    dclose(&report);

    dclose(&Exp);

    UINT ID = MessageBox("Data export completed\n\n\nDo you want to view it?","Question", MB_ICONQUESTION | MB_YESNO);
    if(ID==IDNO) {
        OnOK();
        return;
    }

    HINSTANCE flag = ShellExecute(
        NULL,
        "OPEN",
        file,
        NULL,
        NULL,
        SW_SHOW
        );
    OnOK();
}

BOOL CExpData::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	short	shBtnColor = 30;
  	m_btn1.SetIcon(IDI_XLS,BS_LEFT);
    m_btn1.SetShade(SHS_VSHADE,8,20,5,RGB(55,55,255));
	
	m_btn2.SetIcon(IDI_DBF,BS_LEFT);
    m_btn2.SetShade(SHS_VSHADE,8,20,5,RGB(55,55,255));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
