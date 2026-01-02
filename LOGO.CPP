// Logo.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"

void CMenuApp::SetStdCompanyLogo()
{
    lset(logo.dat[Logo_NAME],title);
    lset(logo.dat[Logo_CODE],profile.dat[Pro_CODE]);
    lset(logo.dat[Logo_ADD1],profile.dat[Pro_ADD1]);
    lset(logo.dat[Logo_ADD2],profile.dat[Pro_ADD2]);
    lset(logo.dat[Logo_ADD3],profile.dat[Pro_ADD3]);
    lset(logo.dat[Logo_PROGRAM_ID],Pref);
}
