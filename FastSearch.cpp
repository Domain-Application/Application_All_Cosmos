// FastSearch.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "FastSearch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFastSearch dialog


CFastSearch::CFastSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CFastSearch::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFastSearch)
	m_date = COleDateTime::GetCurrentTime();
	m_str = _T("");
    m_unprint = TRUE;
	//}}AFX_DATA_INIT
}


void CFastSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFastSearch)
	DDX_Control(pDX, IDC_COMBO1, m_cb1);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_date);
	DDX_CBString(pDX, IDC_COMBO1, m_str);
	DDX_Check(pDX, IDC_CHECK1, m_unprint);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFastSearch, CDialog)
	//{{AFX_MSG_MAP(CFastSearch)
	ON_BN_CLICKED(IDC_BUTTON1, OnAdvanceSearch)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER1, OnDatetimechangeDatetimepicker1)
	ON_CBN_DBLCLK(IDC_COMBO1, OnDblclkCombo1)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFastSearch message handlers

void CFastSearch::OnAdvanceSearch() 
{
    EndDialog(IDC_BUTTON1);
}

void CFastSearch::OnFetchData()
{
    DisplayTicket();
}

BOOL CFastSearch::OnInitDialog() 
{
	CDialog::OnInitDialog();
    OnFetchData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CFastSearch::DisplayTicket()
{
	m_cb1.ResetContent();
    UpdateData();
    lset(one.dat[1],m_date);
    if(m_unprint) {
        str.Format("select * from [%s] where [date] >= '%s' and ([second]=0 or [second] is null )",
           df->m_strTable, one.dat[1]);
    }
    else {
        str.Format("select * from [%s] where [date] >= '%s'",
           df->m_strTable, one.dat[1]);
    }
    ado(&adoTmp,"",NULL,str);
    long last = read_sumindex(1,&adoTmp);
    for(long rec=1; rec<=last; rec++) {
        adoTmp.read(rec);
        str.Format("%s - %s - %s - %s",
            adoTmp.dat[1],adoTmp.dat[ACCODE],adoTmp.dat[LORRY],adoTmp.dat[PGROSS]);
        m_cb1.AddString(str);
    }
	dclose(&adoTmp);
}

void CFastSearch::OnDatetimechangeDatetimepicker1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
    DisplayTicket();
	*pResult = 0;
}

void CFastSearch::OnDblclkCombo1() 
{
	UpdateData();
	int nitem = m_cb1.GetCurSel();
	if(nitem==-1) return;

	lset(df->dat[1],m_str);
	long rec = qd(df->dat[1],df);
	if(rec>0) {
		CDialog::OnOK();
	}
	else {
		CDialog::OnCancel();
	}
}

void CFastSearch::OnOK()  
{
	OnDblclkCombo1();
}

void CFastSearch::OnCheck1() 
{
	UpdateData();	
	DisplayTicket();
}

void CFastSearch::OnDestroy() 
{
	CDialog::OnDestroy();
	

}
