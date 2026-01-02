// menu.h : main header file for the MENU application
//

#if !defined(AFX_MENU_H__D6EA9606_A1F2_11D0_8F37_945AC87D3839__INCLUDED_)
#define AFX_MENU_H__D6EA9606_A1F2_11D0_8F37_945AC87D3839__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "myapp.h"       // main symbols
#include <Tan.h>       // main symbols
//{{AFX_INCLUDES()
#include "msflexgrid.h"
//}}AFX_INCLUDES
extern DB  unit;
extern DB  tt;
extern DB  wbdata, prdata, dbdata, despatch;
extern DB  tolerant,vehicle,pm1no,pm2no;
extern DB  dscr,packing,transp,warehouse;

extern int ClientRequest(const char *server_name,char *command,CString &errmsg);
extern int current_indicator;
extern long tcpWeight;
extern int STCR;
extern void init_lorry(DB *df);
extern void init_wbdata(DB *df);
extern void init_packing(DB *df);
extern void DetailReport4AC2(DB *df, int flag);
extern int UserPasswordLoginAccepted(DB *df,int level,CString str);
extern void DetailReport4PT(DB *df, int flag=0);
extern void MasterGrossTareNetReport(DB *df);
extern void AdminDbProfile(DB *df);
extern DB *ttt,Status,stype,wbsetup;
extern void CleanDatabase();
extern void ProfileData(DB *df);
extern int   PViewFlag;
extern void init_stdlist(DB *df);
extern void SetWBGridRows(CMSFlexGrid *grid);
extern void PrintLoadingUnloadingForm(long irec,FILE *ptr,DB *df);
extern void DetailGrossTareNetListingReport(DB *df);
extern void DetailReport4AC(DB *df,int flag=0);
extern void lset(char *szDate,COleDateTime m_date, char *szTime,COleDateTime m_time);


extern void init_accode(DB *df);
extern void init_first(DB *df);
extern void init_wb(DB *df);
extern void pslip1(long irec,FILE *ptr,DB *df);
extern int tscr_flag(DB *df);
extern void SetStdCompanyLogo();
extern long GwstWeight,ActualWeight;
extern void WeighingProcess(DB *main,DB *first);
extern void MasterReport(DB *df);
extern void PrintProductCode(long irec,FILE *ptr,DB *df);
extern void PrintDriverCode(long irec,FILE *ptr,DB *df);
extern double m[16];
extern void compute_weight(DB *df, DB *sub);
extern void SalesContractUpdateReport();
extern int repct(char *Menu,int IdxItem,long irec,DB *df);
extern void init_gst(DB *df);
extern void Psmf(long irec,FILE *ptr,DB *df);
extern void DateSummary(DB *df);
extern void VehicleSummary(DB *df);
extern void Pcode(long irec,FILE *ptr,DB *df);
extern int keytscr(void);
extern void ProductSummaryReport(DB *df);
extern void PorlaReport(void);
extern void BuyerTransportFeeSummary(DB *df);
extern void ProfileSetup(void);
extern void DetailListingReport(DB *df);
extern void init_product(DB *df);
extern void init_driver(DB *df);
extern void init_accode(DB *df);
extern void Weighting(DB *wb,DB *first);
extern void DispPublicWeight(DB*);
extern long input_weight(KB key,char *mesure);
extern void Pcust(long ,FILE *,DB *);
extern char *dsavemenu,*company;
extern void prec(long irec,FILE *ptr,DB *df);
extern long weight1,weight2;
extern void DespatchWeighting(void);
extern void DispFirst(DB *);
extern void Posting(void);
extern void PrintLogo(FILE *ptr,char *tl);
extern void WBmodify(DB *);
extern void tscr_report(DB *);

extern DB   accode,first,prodtype,tscr,driver,gst,logo;
extern DB   bal,wb,one,user,login,report,two;
extern DB   pr,fpr,profile,porla;

enum wb {TICKET=1,LORRY,Date,STATUS,TT,CUST,PT,DRIVER,TOLERANT,TRANSP,WAREHOUSE,
               PGROSS,PTARE,WASTAGE,SWT,
               SEALFR,SEALTO,LOTNO,TIMEIN,TIMEOUT,
               REM,PM1NO,PM1TIME,PM1WT,PM2NO,PM2TIME,PM2WT,
               I1,I2,I3,I4,
               W1,W2,Z1,Z2,FWNO,USER1,USER2,USER3,USER4,USER5,USER6};

#define TDATE Date
#define ACCODE CUST
#define FIRST  PGROSS
#define SECOND PTARE

enum wbdata {Wd_DO=3,Wd_PO1,Wd_PO2,Wd_PQTY,Wd_PT,Wd_QTY,Wd_UNIT,Wd_PACKING,
             Wd_UW,Wd_NoPallet,Wd_PalletWt,Wd_TW,Wd_PKKEY};
enum prodtype {PT_CODE=1,PT_DECRIPT,PT_REMARKS,PT_PACKING};
enum accode {CU_CODENO=1,CU_CUSTOMER,CU_SNAME,CU_ADD1,CU_ADD2,CU_ADD3,
            CU_TEL,CU_FAX,CU_POLAR,CU_EXPIRY,CU_CONTACT,CU_REMARKS};
enum two {Tw_D1=1,Tw_D2,Tw_PAGE};
enum scv {Scv_SO1=1,Scv_SO2,Scv_ACCODE,Scv_PT,Scv_STYPE,Scv_PAGE};
enum profile {Pro_CODE=1,Pro_ADD1,Pro_ADD2,Pro_ADD3,Pro_WtServer,
              Pro_OutputTxtName,Pro_STTIME,Pro_MinWt,
              Pro_STABLE,Pro_ZERO,Pro_SW,Pro_DRIVER};
enum logerr {LGR_DATE=1,LGR_CASHIER,LGR_ACCESS,LGR_TICKET,LGR_TIME,LGR_PASS_TYPE,LGR_PROGRAM};
enum lorry {Lo_LORRY=1,Lo_DRIVER1,Lo_DRIVER2,Lo_TRANSP,
            Lo_TIME,Lo_TARE,Lo_REMARKS,Lo_USER};
enum packing {Pk_PACKING=1,Pk_DES,Pk_NETWT,Pk_PW,Pk_WEIGHT,Pk_REMARKS};
enum logo {Logo_CODE=1,Logo_NAME,Logo_ADD1,Logo_ADD2,Logo_ADD3,Logo_ADD4,
           Logo_DATE,Logo_DATE1,Logo_DATE2,Logo_TIME1,Logo_TIME2,
           Logo_SELECT1,Logo_SELECT2,Logo_SELECT3,Logo_RTITLE,Logo_GROUP,
           Logo_USER,Logo_USERNAME,Logo_PROGRAM_ID,Logo_CONDITION,Logo_PRINT_RC};
/////////////////////////////////////////////////////////////////////////////
// CMenuApp:
// See menu.cpp for the implementation of this class
//

class CMenuApp : public CMyApp
{
public:
	CMenuApp();
    virtual BOOL Domain();
    void Profile();
    virtual void OpenAllDataFile();
    virtual void SetStdCompanyLogo();
    virtual void SetDBRelations();
    virtual int SubProgram(int key);
    virtual BOOL RptResetFiles(const CString m_CrystalReportName,const CString repname,CString &newname);
    virtual CString ConfirmCrystalTableFile(char *FieldName,CString m_CrystalReportName);
};
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MENU_H__D6EA9606_A1F2_11D0_8F37_945AC87D3839__INCLUDED_)
