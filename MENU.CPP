// menu.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "menu.h"

#include "MainFrm.h"
#include "menuDoc.h"
#include "menuView.h"
#include "password.h"
#include "start.h"
#include <locale.h>
#include "initial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DB accode(IDD_ACCODE),prodtype(IDD_PT,2);
DB Status(IDD_STATUS),stype;
DB vehicle(IDD_LORRY),pm1no(IDD_LORRY),pm2no(IDD_LORRY);
DB wbsetup;
DB driver(IDD_DRIVER,3);
DB  packing(IDD_PACKING,5);
DB  warehouse(IDD_WAREHOUSE,5);
DB  transp(IDD_TRANSP,3);

/////////////////////////////////////////////////////////////////////////////
// CMenuApp initialization
DB  unit(IDD_UNIT);
DB  tt(IDD_TT);
int STCR;
DB  wb(IDD_WB),wbdata(IDD_WBDATA);
DB  despatch(IDD_DESPATCH),dbdata(IDD_DBDATA);
DB  tolerant(IDD_TOLERANT);
// LD Green (M) Sdn. Bhd

CMenuApp theApp;
CMenuApp::CMenuApp()
{
    internetFolder = "ALLCOS";

    unsigned char  CompanyNo = getCompanyNo();

    company = "ë;;ýËÊ+Êýk»ªÊºÚk«ÊýÊ»ýÛ{»";

//    company = "ëÉÉ¨Øé¸©ýËo¸éi©ØýŠ©i‰yi‰";
    IsSqlFile = TRUE;
    GetSystemProviderString();
    ReportSortUserLevel = 9;
}

void CMenuApp::Profile()
{

    SetRegistryKey(_T(title));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
}

BOOL CMenuApp::Domain()
{
    CStart p1;
    p1.InitialFlag = 0x0002 | INIT_ADO_USER | INIT_ADO_PROGRAM_PASSWORD;
    p1.DefaultUser = m_defaultUser;
    p1.DoModal();

    if(p1.rec==0) return FALSE;
    UserLogin(p1.UserID);

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
        IDR_MAINMENU,
		RUNTIME_CLASS(CMenuDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMenuView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

    HideStaffMenu = 9999;
	return TRUE;
}

void CMenuApp::OpenAllDataFile()
{
    ado(&unit,"unit");
    ado(&tt,"tt");
    tado(&wbdata,"wbdata",init_wbdata);
    tado(&dbdata,"dbdata",init_wbdata);
    ado(&tolerant,"tolerant");
    if(ado(&profile,"profile")) {
        readrec(1,&profile);
        profile.ID = IDD_PROFILE;
    }
    tado(&wb,"wb",init_wb);
    tado(&despatch,"despatch",init_wb);
    ado(&accode,"accode",init_accode);
    ado(&prodtype,"pt",init_stdlist);
    ado(&driver,"driver",init_stdlist);
    ado(&vehicle,"lorry",init_lorry);
    ado(&pm1no,"pm1no",init_lorry);
    ado(&pm2no,"pm2no",init_lorry);
    ado(&stype,"stype");
    ado(&Status,"status");

    if(ado(&logo,"logo")) {
       SetStdCompanyLogo();
       writerec(1,&logo);
    }

    ropen(&one,"one.scr");
    ropen(&two,"two.scr");
    dopen(&wbsetup,"wbsetup.scr");

    ado(&packing,"packing",init_packing);
    ado(&warehouse,"warehouse");
    ado(&transp,"transp");

    wb.e->chain = &wbdata;
    despatch.e->chain = &dbdata;
}

void CMenuApp::SetDBRelations()
{
    wb.m_WithoutPrintWithoutFC = TRUE;
    wb.m_PrintOneFormOnly = TRUE;
    wb.m_PreviewCrystalReport = FALSE;
#ifdef _DEBUG
    wb.m_PreviewCrystalReport = TRUE;
#endif //_DEBUG

    despatch.m_WithoutPrintWithoutFC = TRUE;
    despatch.m_PrintOneFormOnly = TRUE;
    despatch.m_PreviewCrystalReport = FALSE;
#ifdef _DEBUG
    despatch.m_PreviewCrystalReport = TRUE;
#endif //_DEBUG
}

BOOL CMenuApp::RptResetFiles(const CString m_CrystalReportName,const CString repname,CString &newname)
{
    if(STCR <= 1) return FALSE;
    if(!repname.CompareNoCase("wb")) {
        newname = "despatch";
        return TRUE;
    }
    if(!repname.CompareNoCase("rep_wb")) {
        newname = "rep_despatch";
        return TRUE;
    }
    if(!repname.CompareNoCase("wbdata")) {
        newname = "dbdata";
        return TRUE;
    }
    return FALSE;
}

CString CMenuApp::ConfirmCrystalTableFile(char *FieldName,CString m_CrystalReportName)
{
    if(STCR <= 1) return FieldName;
    CString repname = FieldName;
    if(!repname.CompareNoCase("wb")) {
        return "despatch";
    }
    if(!repname.CompareNoCase("rep_wb")) {
        return "rep_despatch";
    }
    if(!repname.CompareNoCase("wbdata")) {
        return "dbdata";
    }
    return repname;
}

