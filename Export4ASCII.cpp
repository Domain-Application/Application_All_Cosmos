// Export4ASCII.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "db_ado.h"
#include "Export4ASCII.h"
#include "Junction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum wb2 {Wx_PO=1,Wx_WAREHOUSE,Wx_ITEM,Wx_DATE,Wx_DO,Wx_UNIT,
          Wx_NET_WT,Wx_LOTNO,Wx_USERNAME};

enum export {Ep_PO1=1,Ep_PO2,Ep_PQTY,Ep_REFNO,Ep_DATE,Ep_DO,Ep_PT,Ep_LOTNO,
             Ep_QTY,Ep_NOPALLET,Ep_PALLETWT,Ep_USERNAME};
extern DB adoTmp;
DB export;
/////////////////////////////////////////////////////////////////////////////
// CExport4ASCII dialog

void  lsetdatetime(char *szDate,COleDateTime m_date, char *szTime,COleDateTime m_time)
{
    BOOL bResult = AfxIsValidString(szDate);
    if(bResult==FALSE) {
        MinorWarningMessageBox("lset Is InValid String", __FILE__, __LINE__);
        return;
    }
	CString str;
    str.Format("%4.4i%2.2i%2.2i",m_date.GetYear(),m_date.GetMonth(),m_date.GetDay());
    lset(szDate,str);

    str.Format("%2.2i:%2.2i:%2.2i",m_date.GetHour(),m_date.GetMinute(),m_date.GetSecond());
    lset(szTime,str);
}


CExport4ASCII::CExport4ASCII(CWnd* pParent /*=NULL*/)
	: CDialog(CExport4ASCII::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExport4ASCII)
	m_d1 = COleDateTime::GetCurrentTime();
	m_d2 = COleDateTime::GetCurrentTime();
    m_file = profile.dat[Pro_OutputTxtName];
	m_t1 = COleDateTime::GetCurrentTime();
	m_t2 = COleDateTime::GetCurrentTime();
    m_method = TRUE;
	//}}AFX_DATA_INIT
    m_file.TrimRight();
}


void CExport4ASCII::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExport4ASCII)
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_d1);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_d2);
	DDX_Text(pDX, IDC_EDIT1, m_file);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER3, m_t1);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER4, m_t2);
	DDX_Check(pDX, IDC_CHECK1, m_method);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExport4ASCII, CDialog)
	//{{AFX_MSG_MAP(CExport4ASCII)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExport4ASCII message handlers
BOOL CExport4ASCII::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CheckRadioButton( IDC_RADIO1, IDC_RADIO3, IDC_RADIO1 );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExport4ASCII::OnView()
{
    UINT ID = MessageBox("Export Completed\n\n\nDo you want to open it?","Question", MB_ICONQUESTION | MB_YESNO);
    if(ID==IDYES) {
        HINSTANCE flag = ShellExecute(NULL,"OPEN",m_file,NULL,NULL, SW_SHOW);
    }
}

void CExport4ASCII::OnExportTextFile(DB *df)
{
    CFile f;
    CFileException e;
    if( !f.Open(m_file, CFile::modeCreate | CFile::modeWrite, &e ) ) {
    }
    long last = read_sumindex(-1,df);
    uint sumfield = df->sumfield;
    for(long rec=1; rec<=last; rec++) {
        df->read(rec);
        CString msg;
        for(uint i=1; i<sumfield; i++) {
             if(df->f[i].mode=='D') {
                 msg.Format("%s|", fldfmt(i,df));
             }
             else if(df->f[i].mode=='N') {
                 if(df->val(i)==0) {
                     strset(df->dat[i], 0x20);
                 }
                 msg.Format("%s|", df->dat[i]);
             }
             else {
                 msg.Format("%s|", df->dat[i]);
             }
             f.Write(msg,msg.GetLength());
        }
        msg.Format("%s%c%c", df->dat[i], 13, 10);
//        msg.Format("%s\n", fldfmt(i,df));
        f.Write(msg,msg.GetLength());
    }
    f.Close();

    UINT ID = MessageBox("*** Export to text File Completed ***\n\n\nDo you want to view it?","Question",MB_ICONQUESTION | MB_YESNO);
    if(ID==IDYES) {
		char szFile[1024];

		strcpy(szFile, m_file);
        junction(szFile,1);
    }
}

void CExport4ASCII::OnGetDBFile(DB *df)
{
    long last = read_sumindex(1,df);
    uint sumfield = df->sumfield;

    zap(&report);
    memstart(&export);
    memstart(&report);
    fetch_set(&report, df);

    for(long rec=1; rec<=last; rec++) {
        df->read(rec);
        fetch_rec(&report, df);
        lset(report.dat[Wx_USERNAME], df->dat[USER1]);
        lset(report.dat[Wx_ITEM], df->dat[PT]);
        lset(report.dat[Wx_DO], df->dat[1]);
        lset(report.dat[Wx_UNIT], "KG");
        double w1 = fabs(df->val(FIRST) - df->val(PM1WT));
        double w2 = fabs(df->val(SECOND) - df->val(PM2WT));
        double net = fabs(w1 - w2) - df->val(WASTAGE);
        dtodb(net,&report,Wx_NET_WT);
        memwrt(&report);
        GetSubData(df);
    }
    memend(&report);
    memend(&export);
}

void CExport4ASCII::OnExportExcelFile(DB *df)
{
    int flag;
    if(m_method) {
        flag = CopyFile("export.xls", m_file, FALSE);
    }
    else {
        flag = CopyFile("wb.xls", m_file, FALSE);
    }
    if(flag==FALSE) {
        DWORD errcode = GetLastError();
        if(errcode==53) {
            str = "The network path was not found.";
        }
        else if(errcode==123) {
            str = "The filename, directory name, or volume label syntax is incorrect. ";
        }
        else {
            str.Format("Error Code: %i",errcode);
        }
        AfxMessageBox(str);
        return;
    }
    OnView();
}

void CExport4ASCII::OnOK() 
{
    UpdateData();
    m_file.TrimRight();
    lset(profile.dat[Pro_OutputTxtName], m_file);
    profile.Update();

    lset(two.dat[1], m_d1);
    lset(two.dat[2], m_d2);
    str.Format("select * from [%s] where [date] >= %s and [date] <= %s and tt = 'Y'",
        df->name, DbText(&two,1), DbText(&two,2));

    ado(&adoTmp, "", NULL, str);
    dopen(&export, "export.xls");
    dopen(&report, "wb.xls");
    OnGetDBFile(&adoTmp);
    dclose(&report);
    dclose(&export);

    int no = GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO3);
    if(no==IDC_RADIO1) {
        m_file += ".txt";
        if(m_method) {
             dopen(&report, "export.xls");
        }
        else{
             dopen(&report, "wb.xls");
        }
        OnExportTextFile(&report);
        dclose(&report);
    }
    else if(no==IDC_RADIO2) {
		adoTmp.SaveAsXML(m_file);
        MessageBox("*** Export to XML File Completed ***");
    }
    else {
        m_file += ".xls";
        OnExportExcelFile(&adoTmp);
    }
    dclose(&adoTmp);
	
	CDialog::OnOK();
}

void CExport4ASCII::GetSubData(DB *df)
{
    long irec;
    long Srec = interval(&irec,df->dat[1],sub,1);
    if(Srec==0) return;

    lsetblank(&export);
    for(long rec=0; rec<Srec; rec++) {
        sub->read(rec+irec);
        fetch(&export, sub);
        lset(export.dat[Ep_DATE], df->dat[Date]);
        lset(export.dat[Ep_LOTNO], df->dat[LOTNO]);
        lset(export.dat[Ep_USERNAME], df->dat[USER1]);
        memwrt(&export);
    }
}
