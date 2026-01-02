#include "stdafx.h"
#include "menu.h"
#include "Staff.h"
#include "AskPassword.h"
#include "admindb.h"
#include "Profile.h"
#include "Wb.h"

enum menu {M_NORMAL=1,M_CTRL,M_OTHER};
enum kkk1 {M_REPORT = M_NORMAL * 10 +4 };

void AdminTicketDb(DB *df, DB *sub)
{
    if(df->ID==0) {
        AfxMessageBox("Database not open");
        return;
    }
    CWb p1(df->ID,df,df->f[1].idx & 0x01);
    p1.sub = sub;
//    p1.m_WinPrint = TRUE;
    p1.LptNo       = 1;
    p1.DoModal();
}

void AdminProfileDb(DB *df)
{
    if(df->ID==0) {
        AfxMessageBox("Database not open");
        return;
    }
    CProfile p1(df->ID,df,df->f[1].idx & 0x01);
    p1.OneRecordOnly = TRUE;
    p1.DoModal();
}

int ProgramMenuNo,level;
int CMenuApp::SubProgram(int key)
{
    ProgramMenuNo = 0;
    switch(key) {
    case M_NORMAL*10+1:
        STCR = 1;
        AdminTicketDb(&wb, &wbdata);
        break;
    case M_NORMAL*10+2:
        STCR = 2;
        AdminTicketDb(&despatch, &dbdata);
        break;
    case M_NORMAL*10+3:
        STCR = 1;
        OldTableContainer("wbctn");
        break;
    case M_NORMAL*10+4:
        STCR = 2;
        OldTableContainer("wbctn");
        break;
    case M_NORMAL*10+5:
        STCR = 0;
        TableContainer("fcctn");
        break;
    case M_CTRL*10+1:
        AdminDb(&accode);
        break;
    case M_CTRL*10+2:
        AdminDb(&prodtype);
        break;
    case M_CTRL*10+3:
        AdminDb(&driver);
        break;
    case M_CTRL*10+4:
        AdminDb(&transp);
        break;
    case M_CTRL*10+5:
        AdminDb(&packing);
        break;
    case M_CTRL*10+6:
        AdminDb(&tolerant);
        break;
    case M_CTRL*10+7:
        vehicle.Run();
        break;
    case M_CTRL*10+8:
        AdminDb(&warehouse);
        break;
    case M_CTRL*10+9:
        AdminDb(&unit);
        break;
    case M_OTHER*10+2:
        BackupProcess();
        break;
    case M_OTHER*10+3:
		SetUser();
        break;
    case M_OTHER*10+4:
        return OnSystemUpgrade("WB.EXE","ALLCOS2");
        break;
    case M_OTHER*10+5:
        AdminProfileDb(&profile);
        break;
    case M_OTHER*10+6:
        CleanDatabase();
        break;
    default:
         return 0;
    }
    return 1;
}
