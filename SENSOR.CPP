// Sensor.cpp : implementation file
//

#include "stdafx.h"
#include "Sensor.h"
#include "menu.h"
#include "comm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSensor

CSensor::CSensor()
{
    no = weight = 0;
    lastStatus = 0;

    hitTime = "";
}

CSensor::~CSensor()
{
}


BEGIN_MESSAGE_MAP(CSensor, CDynamicLED)
	//{{AFX_MSG_MAP(CSensor)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSensor message handlers

void CSensor::Show()
{
     int status = SensorStatus(no);
     UINT SW = status==FALSE ? SW_SHOW : SW_HIDE;

     this->ShowWindow(SW);

     if(cweight>500) {
         SYSTEMTIME time;
         GetLocalTime( &time );
         if(status && lastStatus==0) {
             hitTime.Format("%2.2i:%2.2i:%2.2i", time.wHour,time.wMinute,time.wSecond);
         }
         lastStatus = status;
    }
    else {
         hitTime = "";
    }
}
