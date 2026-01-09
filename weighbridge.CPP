// weighbridge.cpp : implementation file
//

#include "stdafx.h"
#include "db_ado.h"
#include "menu.h"
#include "openrs.h"
#include "weighbridge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ENDPROGRAM IDD_EXITPROGRAM-1

/////////////////////////////////////////////////////////////////////////////
// CWeighbridge dialog

CWeighbridge::CWeighbridge(UINT IDD, DB *pdf, int Idx, CWnd *pParent /*=NULL*/)
    : CStaff(IDD, pdf, Idx, pParent)
{
  //{{AFX_DATA_INIT(CWeighbridge)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}

void CWeighbridge::DoDataExchange(CDataExchange *pDX)
{
  CStaff::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CWeighbridge)
  // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWeighbridge, CStaff)
//{{AFX_MSG_MAP(CWeighbridge)
ON_WM_DESTROY()
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BUTTON1, OnChangeWeighbridgeMode)
ON_BN_CLICKED(IDC_BUTTON2, OnSetManualWeighing)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWeighbridge message handlers
BOOL CWeighbridge::OnInitDialog()
{
  CStaff::OnInitDialog();

  // TODO: Add extra initialization here

  return TRUE; // return TRUE unless you set the focus to a control
               // EXCEPTION: OCX Property Pages should return FALSE
}
void CWeighbridge::OnSetManualWeighing()
{
	int fld_Manual_Setting= GetDbFieldNo(df, "SET_MANUAL_FLAG");
	BOOL manualsetting = df->Boolean(fld_Manual_Setting);
	CString msg;
	msg.Format("Change to %s setting.", manualsetting ? "AUTO" : "MANUAL");
	if (UserPasswordLoginAccepted(df, 9, msg) == 0)
	{
		return;
	}
	if(AfxMessageBox("Restart Domain system to take effect.\n Please yes to restart, no to cancel.", MB_OKCANCEL) == IDCANCEL)
	{
		return;
	}
	if (manualsetting)
	{
		ltodb(0, df, fld_Manual_Setting);
	}
	else
	{
		ltodb(1, df, fld_Manual_Setting);
	}
	df->Update();
	ShowInformation();
	app->m_SystemExitNow = TRUE;
	if(AfxGetMainWnd()) {
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ENDPROGRAM);
		return;
	}
	if(AfxGetApp()) {
		AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
    }
}
void CWeighbridge::OnChangeWeighbridgeMode() 
{
  int fld_WB_Mode = GetDbFieldNo(df, "MANUAL_MODE");
  BOOL manualMode = df->Boolean(fld_WB_Mode);
  CString msg;
  msg.Format("Change to %s mode.", manualMode ? "NORMAL" : "DUAL");
  if (UserPasswordLoginAccepted(df, 9, msg) == 0)
  {
	  return;
  }
  if(AfxMessageBox("Restart Domain system to take effect.\n Please yes to restart, no to cancel.", MB_OKCANCEL) == IDCANCEL)
  {
    return;
  }
  if (manualMode)
  {
    ltodb(0, df, fld_WB_Mode);
  }
  else
  {
    ltodb(1, df, fld_WB_Mode);
  }
  df->Update();
  ShowInformation();
  app->m_SystemExitNow = TRUE;
  if(AfxGetMainWnd()) {
    AfxGetMainWnd()->PostMessage(WM_COMMAND, ENDPROGRAM);
    return;
  }
  if(AfxGetApp()) {
    AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
    }
}

void CWeighbridge::SetAttribute(int flag, int fld)
{
  // if(flag)
  // {
  //   GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_SHOW);
	// GetDlgItem(IDC_BUTTON2)->ShowWindow(SW_SHOW);
  // }
  // else
  // {
  //   GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_HIDE);
	// GetDlgItem(IDC_BUTTON2)->ShowWindow(SW_HIDE);
  // }
  CStaff::ShowInformation();
}

void CWeighbridge::ShowInformation()
{
  int fld_WB_Mode = GetDbFieldNo(df, "MANUAL_MODE");
  int fld_Set_Manual = GetDbFieldNo(df, "SET_MANUAL_FLAG");
  BOOL WBMode = df->Boolean(fld_WB_Mode);
  BOOL manualsetting = df->Boolean(fld_Set_Manual);
  CString msg;
  msg.Format("Change to %s mode.", WBMode ? "Normal" : "Dual");
  SetDlgItemText(IDC_BUTTON1, msg);
  msg.Format("Change to %s setting.", manualsetting ? "Auto" : "Manual");
  SetDlgItemText(IDC_BUTTON2, msg);
  CStaff::ShowInformation();
}

void CWeighbridge::OnDestroy()
{
  CStaff::OnDestroy();

  // TODO: Add your message handler code here
}

void CWeighbridge::OnTimer(UINT nIDEvent)
{
  // TODO: Add your message handler code here and/or call default

  CStaff::OnTimer(nIDEvent);
}

void AdminDbweighbridge(DB *df)
{
  if (df->ID == 0)
  {
    AfxMessageBox("Database not open");
    return;
  }
  CWeighbridge p1(df->ID, df, df->f[1].idx & 0x01);
  p1.DoModal();
}

int weighbridge_AdminDb(DB *df, int flag)
{
  AdminDbweighbridge(df);
  return 1;
}

int tell_weighbridge(UINT nChar, int &id, int &fld, DB *df, int sq)
{
  return 1;
}

int start_weighbridge(DB *df) { return 1; }

int initial_weighbridge(DB *df, int *fld) 
{ 
  int i = 0;
  fld[i++] = GetDbFieldNo(df, "MANUAL_MODE");
  fld[i++] = GetDbFieldNo(df, "SET_MANUAL_FLAG");
  fld[i++] = 0; 
  return 1; 
}

int save_weighbridge(DB *df, int flag) { return 1; }

void Relationships_weighbridge(DB *df) {}

void init_weighbridge(DB *df)
{
  df->e->AdminDb = weighbridge_AdminDb;
  df->e->tell = tell_weighbridge;
  df->e->start = start_weighbridge;
  df->e->initial = initial_weighbridge;
  df->e->save = save_weighbridge;
  df->e->Relationships = Relationships_weighbridge;
}
