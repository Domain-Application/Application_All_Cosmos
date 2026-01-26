// menu.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "menu.h"
#include "Connecting2PLC.h"
#include "domView.h"
#include "nView.h"
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

DB container(IDD_STATUS), trailer(IDD_STATUS);
DB accode(IDD_ACCODE),prodtype(IDD_PT,2);
DB Status(IDD_STATUS),stype;
DB vehicle(IDD_LORRY),pm1no(IDD_LORRY),pm2no(IDD_LORRY);
DB wbsetup;
DB driver(IDD_DRIVER,3);
DB camera(IDD_CAMERA2);
DB packing(IDD_PACKING,5);
DB warehouse(IDD_WAREHOUSE,5);
DB transp(IDD_TRANSP,3);
DB pmtdat(IDD_PMTDAT,7),pm1(IDD_PMTDAT,7),pm2(IDD_PMTDAT,7);
DB weighbridge(IDD_WEIGHBRIDGE);
DB first(IDD_WB);
DB lorry(IDD_LORRY);

/////////////////////////////////////////////////////////////////////////////
// CMenuApp initialization
DB unit(IDD_UNIT);
DB tt(IDD_TT);
int STCR,program4=0;
DB wb(IDD_WB),wbdata(IDD_WBDATA);
DB despatch(IDD_DESPATCH),dbdata(IDD_DBDATA),fdata(IDD_DBDATA);
DB tolerant(IDD_TOLERANT);
DB despatch_ticket_info, receive_ticket_info;
// LD Green (M) Sdn. Bhd

CMenuApp theApp;
CMenuApp::CMenuApp()
{
    internetFolder = "ALLCOS";

    unsigned char  CompanyNo = getCompanyNo();

    // ALL COSMOS INDUSTRIES SDN BHD
    company = "DA2A2AECBAGAB91AGAB9EC5A0AAA99B9A9C95A9AB9ECB9AA0AECCA6AAA";

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
    program4 = Atoi(strVersionInfo);
    CStart p1;
    p1.InitialFlag = 0x0002 | INIT_ADO_USER | INIT_ADO_PROGRAM_PASSWORD |
                     INIT_DISABLE_AUTO_SPACE_LPNO;
    p1.DefaultUser = m_defaultUser;
    p1.DoModal();

    if(p1.rec==0) return FALSE;
    UserLogin(p1.UserID);

	CSingleDocTemplate* pDocTemplate;
	// pDocTemplate = new CSingleDocTemplate(
    //     IDR_MAINMENU,
	// 	RUNTIME_CLASS(CMenuDoc),
	// 	RUNTIME_CLASS(CMainFrame),       // main SDI frame window
	// 	RUNTIME_CLASS(CMenuView));
    if(program4) {
        pDocTemplate = new CSingleDocTemplate(
            IDR_MAINMENU,
            RUNTIME_CLASS(CMenuDoc),
            RUNTIME_CLASS(CMainFrame),
            RUNTIME_CLASS(CNView));
    }
    else {
        IDD_DOMVIEW = IDD_MENU_FORM0;
        pDocTemplate = new CSingleDocTemplate(
            IDR_MAINMENU,
            RUNTIME_CLASS(CMenuDoc),
            RUNTIME_CLASS(CMainFrame),
            RUNTIME_CLASS(CDomView));
    }
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
    ado(&container,"container");
    ado(&trailer,"trailer");
    ado(&first,"first",init_wb);
    tado(&pmtdat,"pmtdat",init_pmtdat);
    ado(&pm1,"pm1",init_pmtdat);
    ado(&pm2,"pm2",init_pmtdat);
    m_camera = "camera";
    if(program4==2) {
		m_camera = "camera2";
	}
    ado(&camera,m_camera);
    ado(&weighbridge,"weighbridge",init_weighbridge,NULL,cnn__5);
	tado(&despatch_ticket_info, "despatch_ticket_info");
	tado(&receive_ticket_info, "receive_ticket_info");
	first.m_strTicketNo = despatch_ticket_info.m_strTicketNo; 
	first.m_strRootTable = despatch_ticket_info.m_strTicketNo;
	first.m_strTicketNo = receive_ticket_info.m_strTicketNo; 
	first.m_strRootTable = receive_ticket_info.m_strTicketNo;
	despatch.m_strTicketNo = despatch_ticket_info.m_strTicketNo; 
	despatch.m_strRootTable = despatch_ticket_info.m_strTicketNo;
	wb.m_strTicketNo = receive_ticket_info.m_strTicketNo; 
	wb.m_strRootTable = receive_ticket_info.m_strTicketNo; 
    ado(&unit,"unit");
    ado(&tt,"tt");
    tado(&wbdata,"wbdata",init_wbdata);
    tado(&dbdata,"dbdata",init_wbdata);
	tado(&fdata,"fdata",init_wbdata);
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
    ado(&lorry,"lorry",init_lorry);
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
	first.e->chain = &fdata;
}

void CMenuApp::SetDBRelations()
{
    wb.m_pdf =  despatch.m_pdf = pmtdat.m_pdf = TRUE;
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

    CString s;
    if(program4) 
    {
        lset(weighbridge.dat[weighbridge_COMPUTER_NAME], app->m_strComputerName);
		s.Format("select count(*) from [%s] where [%s] = '%s'", 
            weighbridge.name, weighbridge.fieldName[weighbridge_COMPUTER_NAME],weighbridge.dat[weighbridge_COMPUTER_NAME]);
		long xrec = Sqlint(s);
        int no =xrec;
		if(xrec==0) 
        {
			s.Format("HMI %s not found", app->m_strComputerName);
			AfxMessageBox(s);
			return;
		}
        lset(weighbridge.dat[weighbridge_COMPUTER_NAME], app->m_strComputerName);
        weighbridge.Search(weighbridge.dat[weighbridge_COMPUTER_NAME],weighbridge_COMPUTER_NAME,1);
		SetADAM_IPAdd();
        if(no > 1) 
        {
            s.Format("HMI %s line %s PLC ip %s", app->m_strComputerName,
                weighbridge.dat[1], weighbridge.dat[weighbridge_IP]);
            AfxMessageBox(s);
        }
	}
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

void CMenuApp::SetTitleColor()
{
    app->DialogIcon = IDI_SCALE;
    if(STCR == 1) {
        app->DialogIcon2 = IDI_R;
        int r = 255;
        int g = 0;
        int b = 120;
        app->clrFrom = r  + g * 256 + b* 256 *256;
        app->clrTo   = RGB(255,255,255);
        app->ChangeDialogCaptionColor = TRUE;
        app->ChangeTabCtnCaptionColor = TRUE;
    }
    else if(STCR == 2) {
        app->DialogIcon2 = IDI_D;
        int r = 0;
        int g = 255;
        int b = 120;
        app->clrFrom = r  + g * 256 + b* 256 *256;
        app->clrTo   = RGB(255,255,255);
        app->ChangeDialogCaptionColor = TRUE;
        app->ChangeTabCtnCaptionColor = TRUE;
    }
    else {
        app->ChangeDialogCaptionColor = FALSE;
        app->ChangeTabCtnCaptionColor = FALSE;
    }
}

void SetWindowLocation(CDialog *p)
{
    RECT rect,rectOld;
	
    p->GetWindowRect(&rectOld);
    p->GetWindowRect(&rect);
	
	p->ClientToScreen(&rect);


    int offset = rect.right - rect.left;
    rect.left = 615;
    rect.right = rect.left + offset;

    offset = rect.bottom - rect.top;
    rect.top = 380;
    rect.bottom = rect.top + offset;


    p->MoveWindow(&rect);

}

void CMenuApp::LprLog(const char *no,int CameraIndex,const char *platePhoto,int mode)
{
    if(!app->m_lpr_trace_type) return;

    CString s,p;

    p.Format("%s\\%s",app->picFolder,no);
	CreateDirectory(p,NULL);

    CString strMode = mode ? "A" : "C";

    SYSTEMTIME gmt;
	
    GetLocalTime( &gmt );

    p.Format("%s\\%s\\%4.4u%2.2i%2.2i %2.2i%2.2i%2.2i%s.jpg",
        app->picFolder,no,
        gmt.wYear,gmt.wMonth,gmt.wDay,
        gmt.wHour,gmt.wMinute,gmt.wSecond,strMode);


    s.Format("insert into [LogTraceLPRPhoto] ([LPRNO],[PATH],[ID],TYPE) values ('%s','%s',%i,%i)",
        no,p,CameraIndex,mode);
    Runsql(s);
    BOOL flagCopy = CopyFile(platePhoto,p,TRUE);

}

void CMenuApp::AddLog(int iDeviceIndex, int iLogType, const char* format, ...)
{	
}


int CMenuApp::SetAlarmChannel(int iCamera)
{
    return 0;
}

int iGetPlateSuccess = 0;
CString ord_lpn_no,ord_iu_no,ai_plate_no;
int CMenuApp::OnGetPlateNo(int id,CString &plateno,int option,const char *picFolder)
{
    int flagGetPlateNo = CMyCctvApp::OnGetPlateNo( id, ord_lpn_no,  option, picFolder);

	ord_lpn_no.TrimRight();

	plateno = ord_lpn_no;

    return flagGetPlateNo;
}

CString push_plateno = "";
CString push_plateno1 = "";
CString push_plateno2 = "";
CString msg_plateno1 = "";
CString msg_plateno2 = "";
CString plateno = "";
CString lprno = "";
CString licence_plate_picture;
CString test_plateno = "WHSU2064802";
int direction = 0;
int lpr_detected_number[2];
void CMenuApp::LprDetected(const char *no,int CameraIndex)
{
	
    push_plateno = no;
    push_plateno.TrimRight();
    if(push_plateno.GetLength()<=0){
		return;
	}
	
    if( !IsStringContainDigi(push_plateno) ) {
		return;
    }
	
#ifdef _DEBUG
	if(!push_plateno.CompareNoCase("unknown")) {
		push_plateno =  test_plateno;        //for testing only
	}
#endif //_DEBUG
	
	
	if(!push_plateno.CompareNoCase("unknown")) {
		push_plateno = "";
		return;
	}
	else {
		int len = strlen(push_plateno);
		if(len<4) {
			push_plateno = "";
			return;
		} 
		
		//singapore plate only
		//        push_plateno = AIPlateno(push_plateno);		
        
		if( !IsPlateNo( push_plateno ) ) {
			push_plateno = "";
			return;
		}
		
		if(isblank(push_plateno)) {
			push_plateno = "";
			return;
		}
		
		CString s;
		
		char szDirection[2] = {0};
		
		if(CameraIndex==1) {
			push_plateno1.Format("%s",  push_plateno);
			msg_plateno1.Format("%s %s LPR", TimeNowMs(), push_plateno);
			direction = 1;
		}
        else if(CameraIndex==2) {
			push_plateno2.Format("%s",  push_plateno);
			msg_plateno2.Format("%s %s CNR", TimeNowMs(), push_plateno);
			direction = 2;
		}
	}
}

int CMenuApp::ExitInstance()
{
    CloseConnection__3();
    CloseConnection__4();
    CloseConnection__5();
    return CMyApp::ExitInstance();
}

BOOL CMenuApp::IsHasSingleTask()
{
    return FALSE;
}

// Global helper to extract XML values
CString GetXmlValueGlobal(const CString& strXml, const CString& strTag)
{
    CString strStartTag = "<" + strTag + ">";
    CString strEndTag = "</" + strTag + ">";
    int nStart = strXml.Find(strStartTag);
    if (nStart == -1) return "";
    nStart += strStartTag.GetLength();
    int nEnd = strXml.Find(strEndTag, nStart);
    if (nEnd == -1) return "";
    return strXml.Mid(nStart, nEnd - nStart);
}

BOOL IsMalaysiaPlate(const CString& str)
{
    CString s = str;
    s.TrimLeft(); s.TrimRight();
    s.Replace(" ", ""); // Remove spaces
    int len = s.GetLength();
    if (len < 2 || len > 10) return FALSE;

    int i = 0;
    // 1-3 Letters
    int letterCount = 0;
    while (i < len && ((s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= 'a' && s[i] <= 'z'))) {
        letterCount++;
        i++;
    }
    if (letterCount == 0 || letterCount > 3) return FALSE; 

    // 1-4 Digits
    int digitCount = 0;
    while (i < len && (s[i] >= '0' && s[i] <= '9')) {
        digitCount++;
        i++;
    }
    if (digitCount == 0 || digitCount > 4) return FALSE;

    // Optional 1 Letter at the end
    if (i < len) {
        if (!((s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= 'a' && s[i] <= 'z'))) return FALSE;
        i++;
    }

    return (i == len);
}

BOOL ValidateISO6346(const CString& strContainer)
{
    if (strContainer.GetLength() != 11)
        return FALSE;

    CString strUpper = strContainer;
    strUpper.MakeUpper();

    int charValues[26] = {
        10, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 34, 35, 36, 37, 38
    };

    int sum = 0;
    for (int i = 0; i < 10; ++i)
    {
        char c = strUpper[i];
        int val = 0;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'A' && c <= 'Z') val = charValues[c - 'A'];
        else return FALSE;
        sum += val * (1 << i);
    }

    int checkDigit = sum % 11;
    if (checkDigit == 10) checkDigit = 0;
    return (checkDigit == (strUpper[10] - '0'));
}

int ProcessAnprXml(CString& strBuffer, CString& strPlate, CString& strContainer)
{
    int result = ANPR_RESULT_NONE;
    strPlate.Empty();
    strContainer.Empty();

    // 1. Check for License Plate
    CString strPlateVal = GetXmlValueGlobal(strBuffer, "licensePlate");
    if (!strPlateVal.IsEmpty())
    {
        strPlate = strPlateVal;
        result |= ANPR_RESULT_PLATE;
        
        // Clean up buffer for plate tag
        int nS = strBuffer.Find("<licensePlate>");
        int nE = strBuffer.Find("</licensePlate>", nS);
        if (nS != -1 && nE != -1)
            strBuffer.Delete(nS, (nE + 15) - nS); 
    }

    // 2. Check for Container
    CString strMain = GetXmlValueGlobal(strBuffer, "containerMainNum");
    CString strSub = GetXmlValueGlobal(strBuffer, "containerSubNum");
    if (!strMain.IsEmpty() && !strSub.IsEmpty())
    {
        strContainer = strMain + strSub;
        result |= ANPR_RESULT_CONTAINER;

        // Clean up buffer for container tags
        int nS1 = strBuffer.Find("<containerMainNum>");
        int nE1 = strBuffer.Find("</containerMainNum>", nS1);
        if (nS1 != -1 && nE1 != -1) strBuffer.Delete(nS1, (nE1 + 19) - nS1);

        int nS2 = strBuffer.Find("<containerSubNum>");
        int nE2 = strBuffer.Find("</containerSubNum>", nS2);
        if (nS2 != -1 && nE2 != -1) strBuffer.Delete(nS2, (nE2 + 18) - nS2);
    }

    return result;
}
