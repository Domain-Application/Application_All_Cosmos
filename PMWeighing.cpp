// PMWeighing.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "PMWeighing.h"
#include "comm.h"
#include "security.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPMWeighing dialog


CPMWeighing::CPMWeighing(UINT IDD,DB *pdf,DB *psub,int Idx,CWnd* pParent /*=NULL*/)
	: CReqitem(IDD, pdf, psub, Idx, pParent)
{
	//{{AFX_DATA_INIT(CPMWeighing)
	m_vehicle = _T("");
	hideShuntingLorryFlag = FALSE;
	hideRoRoFlag= FALSE;
	//}}AFX_DATA_INIT
}


void CPMWeighing::DoDataExchange(CDataExchange* pDX)
{
	CReqitem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPMWeighing)
	DDX_Text(pDX, IDC_EDIT2, m_vehicle);
	DDV_MaxChars(pDX, m_vehicle, 99);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPMWeighing, CReqitem)
	//{{AFX_MSG_MAP(CPMWeighing)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPMWeighing message handlers

BOOL CPMWeighing::OnInitDialog() 
{
	lsetblank(df);
	lset(lorry.dat[1], m_vehicle);
	if(qd(lorry.dat[1], &lorry) > 0) 
	{
		lset(df->dat[GetDbFieldNo(df, "DRIVER1")], lorry.dat[GetDbFieldNo(&lorry, "DRIVER1")]);
		lset(df->dat[GetDbFieldNo(df, "DRIVER2")], lorry.dat[GetDbFieldNo(&lorry, "DRIVER2")]);
		lset(df->dat[GetDbFieldNo(df, "TRANSP")], lorry.dat[GetDbFieldNo(&lorry, "TRANSP")]);
		ltodb(lorry.val(GetDbFieldNo(&lorry, "IS_SHUNTING_LORRY")), df, GetDbFieldNo(df, "IS_SHUNTING_LORRY"));
	}
	if(first.Search(m_vehicle, PM1NO, TRUE) > 0)
	{
		lset(df->dat[GetDbFieldNo(df, "DRIVER1")], first.dat[GetDbFieldNo(&first, "DRIVER")]);
		lset(df->dat[GetDbFieldNo(df, "TRANSP")], first.dat[GetDbFieldNo(&first, "TRANSP")]);
	}
	CReqitem::OnInitDialog();

	lset(df->dat[2],m_vehicle);
	if((!isInDirection && program4 == 1) || (isInDirection && program4 == 2)){
        CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
    }
    else{
        CheckRadioButton( IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
    }
	if(hideShuntingLorryFlag) {
		GetDlgItem(IDC_EDIT15)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_EDIT15)->ShowWindow(SW_SHOW);
	}
	if(hideRoRoFlag) {
		GetDlgItem(IDC_EDIT16)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_EDIT16)->ShowWindow(SW_SHOW);
	}
	SetTimer(1,250,NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPMWeighing::OnDestroy() 
{
	KillTimer(1);
	CReqitem::OnDestroy();
}

void CPMWeighing::OnTimer(UINT nIDEvent) 
{
    df->SetCurrentTime(Pm_TIMEIN);
    ltodb(cweight,df,Pm_WEIGHT);

	SetDlgItemText( CtrlId(Pm_TIMEIN), df->dat[Pm_TIMEIN]);
	SetDlgItemText( CtrlId(Pm_WEIGHT), df->dat[Pm_WEIGHT]);
	
	CReqitem::OnTimer(nIDEvent);
}

void CPMWeighing::OnOK() 
{
	CString message = IsBlacklistedDriver(df,df->dat[Pm_DRIVER1]);  //brian//check blacklist for driver1 and driver2//20251223
	if(!isblank(message) && UserPasswordLoginAccepted(df, 9, message) == 0)
	{
		return;
	}
	message = IsBlacklistedDriver(df,df->dat[Pm_DRIVER2]);
	if(!isblank(message) && UserPasswordLoginAccepted(df, 9, message) == 0)
	{
		return;
	}
    int key = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
	push_plateno2.Format("");
    if(key==IDC_RADIO1) {
		CReqitem::OnOK();
    }
    else{
		CReqitem::EndDialog(IDC_BUTTON1);
    }
}

void CSecurity::UpdatePrimeMover1stWeight() 
{
	lset(first.dat[PM1NO], m_vehicle);
	ltodb(cweight, &first, PM1WT);
	char szIndicator[2];
	szIndicator[0] = 'A' + program4 - 1;
	szIndicator[1] = 0;
	lset(&first,"IT3", szIndicator);
	first.FldDefault(PM1TIME);
	lset(first.dat[USER3], user.dat[1]);
	first.Update();
}

void CSecurity::UpdatePrimeMover2ndWeight() 
{
	DB adoTmp;
	CString strSQL;
	strSQL.Format("select ticket, lorry as container, date from first  WHERE (dbo.fn_IsValidContainerNo(LORRY) = 1)");
	ado(&adoTmp, "", NULL, strSQL);
	long rec = viewidx(&adoTmp, "Please select container");
	if(rec <= 0)
	{
		return;
	}
	rec = first.Search(adoTmp.dat[2], GetDbFieldNo(&first, "LORRY"), TRUE);
	if(rec <= 0)
	{
		return;
	}
	lset(first.dat[PM2NO], m_vehicle);
	ltodb(cweight, &first, PM2WT);
	char szIndicator[2];
	szIndicator[0] = 'A' + program4 - 1;
	szIndicator[1] = 0;
	lset(&first,"IT4", szIndicator);
	first.FldDefault(PM2TIME);
	lset(first.dat[USER4], user.dat[1]);
	first.Update();
	dclose(&adoTmp);
}

void CSecurity::UpdatePrimeMoverWeighingData(BOOL canAssign2ndPrimeMover)
{
	if(canAssign2ndPrimeMover)
	{
		return;
	}
	lsetblank(&first);
	CString strSQL,strSQL2,binticket;
    strSQL.Format("select Final_Count from Container_In_Yard");
	
	strSQL2.Format("SELECT top 1 ticket FROM [first] where lorry='%s' and BIN is not null order by timein desc", m_vehicle); //check first got ticket with same lorry no and got bin no
	binticket=SqlText(strSQL2);
	binticket.TrimRight();
	if(binticket.GetLength()>0)
	{
		lset(first.dat[1], binticket);
		long xrec = first.Search( first.dat[1], 1, TRUE);
		if(xrec==0) return;
		UpdatePrimeMover1stWeight();
	}
	else
	{
		strSQL.Format("select top 1 ticket from [first] where pm1no = '%s' order by timein desc", m_vehicle);
		CString t = SqlText(strSQL);
		
		lset(first.dat[1], t);
		
		long xrec = first.Search( first.dat[1], 1, TRUE);
		if(xrec==0) return;
		
		UpdatePrimeMover1stWeight();
	}
	
	
   
/*
	if(first.Search(m_vehicle,PM1NO, TRUE) > 0 && isblank(first.dat[PM1WT]))
	{
		UpdatePrimeMover1stWeight();
	}*/
}


void CSecurity::OnRadio3() 
{
    if(cweight<=5000) {
        UINT ID = MessageBox("Weight too light\n\n\nDo you want to continue?", "Error", MB_ICONSTOP | MB_YESNO);
        if(ID==IDNO) return;

    }
    if(cweight>10000) {
        UINT ID = MessageBox("Weight too heavy\n\n\nDo you want to continue?", "Error", MB_ICONSTOP | MB_YESNO);
        if(ID==IDNO) return;
    }

    m_vehicle.TrimRight();
    if(m_vehicle.GetLength()==0) {
        MessageBox("Please key in prime mover no", "Error", MB_ICONSTOP);
        return;
    }

	CPMWeighing p1(IDD_PMWeighing, &pmtdat, &pmtdat, 1, this);
	
	p1.m_vehicle = m_vehicle;
	p1.df = &pmtdat;
	p1.isInDirection = TRUE;
	p1.hideShuntingLorryFlag = FALSE;
	p1.hideRoRoFlag = FALSE;
	UINT ID = p1.DoModal();
	if(ID==IDOK || ID==IDC_BUTTON1) {
		lset(pmtdat.dat[Pm_PMNO], m_vehicle);
		lset(pmtdat.dat[Pm_DIRECTION], "I");
		pmtdat.SetCurrentTime(Pm_DATE);
		pmtdat.SetCurrentTime(Pm_TIMEIN);
		ltodb(cweight,&pmtdat,Pm_WEIGHT);
		ltodb(cweight,&pmtdat,Pm_W1);
		ltodb(cweight,&pmtdat,Pm_Z1);
		SaveImages(&pmtdat,"JPGKEY1");

		FindAddInfoTicketNo(&pmtdat,1);
		pmtdat.AddNew();

        OnUpdateLorry();
		UpdatePrimeMoverWeighingData(TRUE);
		IsZeroWt = FALSE;
		OnOpenBarrier(ID);
	
	}

}

void CSecurity::OnRadio4() 
{	
    if(cweight<=5000) {
        UINT ID = MessageBox("Weight too light\n\n\nDo you want to continue?", "Error", MB_ICONSTOP | MB_YESNO);
        if(ID==IDNO) return;
    }
    if(cweight>10000) {
        UINT ID = MessageBox("Weight too heavy\n\n\nDo you want to continue?", "Error", MB_ICONSTOP | MB_YESNO);
        if(ID==IDNO) return;
    }
	
    m_vehicle.TrimRight();
    if(m_vehicle.GetLength()==0) {
        MessageBox("Please key in prime mover no", "Error", MB_ICONSTOP);
        return;
    }

	CPMWeighing p1(IDD_PMWeighing, &pmtdat, &pmtdat, 2, this);

	p1.m_vehicle = m_vehicle;
	p1.df = &pmtdat;
	p1.isInDirection = FALSE;
	p1.hideShuntingLorryFlag = TRUE;
	p1.hideRoRoFlag = TRUE;
	UINT ID = p1.DoModal();
	
	if(ID==IDOK || ID==IDC_BUTTON1) {
		lset(pmtdat.dat[Pm_PMNO], m_vehicle);
		lset(pmtdat.dat[Pm_DIRECTION], "O");
		pmtdat.SetCurrentTime(Pm_DATE);
		pmtdat.SetCurrentTime(Pm_TIMEIN);
		ltodb(cweight,&pmtdat,Pm_WEIGHT);
		ltodb(cweight,&pmtdat,Pm_W1);
		ltodb(cweight,&pmtdat,Pm_Z1);
		SaveImages(&pmtdat,"JPGKEY1");

		FindAddInfoTicketNo(&pmtdat,1);
		pmtdat.AddNew();

        OnUpdateLorry();
		UpdatePrimeMoverWeighingData(FALSE);
		IsZeroWt = FALSE;
		OnOpenBarrier(ID);
	}
}

void CSecurity::OnUpdateLorry()
{
    lset( lorry.dat[1], m_vehicle);
    long xrec = qd( lorry.dat[1], &lorry);
    lset( lorry.dat[1], m_vehicle);
    lset( lorry.dat[Lo_TIME], pmtdat.dat[Pm_TIMEIN]);
    ltodb ( cweight, &lorry, Lo_TARE);
    ltodb ( program4, &lorry, Lo_INDICATOR);
	lset(lorry.dat[GetDbFieldNo(&lorry,"DRIVER1")], pmtdat.dat[GetDbFieldNo(&pmtdat,"DRIVER1")]);
	lset(lorry.dat[GetDbFieldNo(&lorry,"DRIVER2")], pmtdat.dat[GetDbFieldNo(&pmtdat,"DRIVER2")]);
	lset(lorry.dat[GetDbFieldNo(&lorry,"TRANSP")], pmtdat.dat[GetDbFieldNo(&pmtdat,"TRANSP")]);
	ltodb(pmtdat.val(GetDbFieldNo(&pmtdat,"IS_SHUNTING_LORRY")), &lorry, GetDbFieldNo(&lorry, "IS_SHUNTING_LORRY"));
	lset(lorry.dat[GetDbFieldNo(&lorry,"LAST_WEIGHT_BY")], user.dat[1]);

    if(xrec==0) {
        lorry.AddNew();
    }
    else {
        lorry.SqlUpdate();
    }

}

