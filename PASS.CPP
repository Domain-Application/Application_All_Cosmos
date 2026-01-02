// pass.cpp : implementation file
//

#include "stdafx.h"
#include "Logerr.h"

int UserPasswordLoginAccepted(DB *df,int level,CString str)
{

    CString strLevel;
    strLevel.Format(" Level : %i",level);
    CLogerr p1;
    p1.level = level;
    p1.m_purpose = str+strLevel;
    p1.df = df;
    int wflag = p1.DoModal();
    if(wflag==IDOK) {
       return 1;
    }
    return 0;
}
