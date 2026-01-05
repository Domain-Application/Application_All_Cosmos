// ChangeWeighingType.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "ChangeWeighingType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeWeighingType dialog


CChangeWeighingType::CChangeWeighingType(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeWeighingType::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeWeighingType)
	m_vehicle = _T("");
	//}}AFX_DATA_INIT
}


void CChangeWeighingType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeWeighingType)
	DDX_Text(pDX, IDC_EDIT1, m_vehicle);
	DDV_MaxChars(pDX, m_vehicle, 20);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeWeighingType, CDialog)
	//{{AFX_MSG_MAP(CChangeWeighingType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeWeighingType message handlers

BOOL CChangeWeighingType::OnInitDialog() 
{
	CDialog::OnInitDialog();

    int fld = GetDbFieldNo(df ,_szPrintCounter);
    int no = df->lval(fld);
    if(no==1) {
        g = "Receiving";
    }
    else if(no==2) {
        g = "Despatch";
    }
    if(no<1) no = 1;
    CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 + no -1);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeWeighingType::OnOK() 
{
    int no = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2) -IDC_RADIO1 +1;
    if(no==1) {
        g = "Receiving";
    }
    else if(no==2) {
        g = "Despatch";
    }

    s.Format("Change weighing to %s?",g);
    UINT ID = MessageBox(s,"Confirm it", MB_ICONQUESTION | MB_YESNO);
    if(ID==IDYES) {
         s.Format("update [%s]  set [%s] = %i where [%s] = '%s'",
             df->name,_szPrintCounter,
             no, df->fieldName[1], df->dat[1]);
         RunSql(s);
	
         CDialog::OnOK();
    }
}
