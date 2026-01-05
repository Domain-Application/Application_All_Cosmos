// Barrier.cpp : implementation file
//

#include "stdafx.h"
#include "menu.h"
#include "Barrier.h"
#include "comm.h"
//#include "status.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL ArrayIsLeft2Right=TRUE;
#define RetryCounter 12


/////////////////////////////////////////////////////////////////////////////
// CBarrier

CBarrier::CBarrier()
{
    flagDisable = FALSE;
    busy = FALSE;
    InProgress = FALSE;
    TimerEvent = 0;
    iStableWtCounter = 0;
    minwt = 0;
    LastWt = -1;
    MControl = FALSE;
    closeCounter = openCounter = 0;
}

CBarrier::~CBarrier()
{
}

BEGIN_MESSAGE_MAP(CBarrier, CStatic)
	//{{AFX_MSG_MAP(CBarrier)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBarrier message handlers

void CBarrier::ShowIcon()
{
    if(InProgress) {
        m_hIcon = AfxGetApp()->LoadIcon(IDI_BAR_HALF);
        this->SetIcon(m_hIcon);
        return;
    }
    CString s;
    s.Format("BR%1.1iDISABLE",no);

    if( weighbridge.Boolean(s)) {
        m_hIcon = AfxGetApp()->LoadIcon(IDI_BARRIER_DISABLED);
        this->SetIcon(m_hIcon);
		flagDisable = TRUE;
        return;
    }
	flagDisable = FALSE;
	
    BOOL flag = BarrierStatus(no);
    if(flag==BarrierIsClose) {
        m_hIcon = AfxGetApp()->LoadIcon(IDI_BAR_CLOSE);
        this->SetIcon(m_hIcon);
    }
    else {
        m_hIcon = AfxGetApp()->LoadIcon(IDI_BAR_OPEN);
        this->SetIcon(m_hIcon);
    }
}

void CBarrier::OnTimer(UINT nIDEvent) 
{
    if(busy) return;

    busy = TRUE;
    iCount++;
	CStatic::OnTimer(nIDEvent);
    if(nIDEvent==1 + no * 2) {
        if(iCount>RetryCounter) {
            KillTimer(nIDEvent);
            WriteCoil(fldBarrierClose,0);
            WriteCoil(fldBarrierOpen,0);
			
            TimerEvent = 0;
            InProgress = FALSE;
            ShowIcon();
            busy = FALSE;
            return;
        }
        BOOL flag = BarrierStatus(no);
        if(flag==BarrierIsOpen) {

            KillTimer(nIDEvent);
            WriteCoil(fldBarrierClose,0);
            WriteCoil(fldBarrierOpen,0);

            TimerEvent = 0;
            if(ADAMSimulator==1) Sleep(100);
            InProgress = FALSE;
            ShowIcon();
            busy = FALSE;
            return;
        }
        DoOpenBarrier();
    }
    else if(nIDEvent==2 + no * 2) {
        if(iCount>RetryCounter) {
            KillTimer(nIDEvent);
            WriteCoil(fldBarrierClose,0);
            WriteCoil(fldBarrierOpen,0);
            TimerEvent = 0;
            InProgress = FALSE;
            ShowIcon();
            busy = FALSE;
            return;
        }
        BOOL flag = BarrierStatus(no);
        if(flag==BarrierIsClose) {
            KillTimer(nIDEvent);
            WriteCoil(fldBarrierClose,0);
            WriteCoil(fldBarrierOpen,0);
            TimerEvent = 0;
            if(ADAMSimulator==1) Sleep(1000);
            InProgress = FALSE;
            ShowIcon();
            busy = FALSE;
            return;
        }

        DoCloseBarrier();
    }
    busy = FALSE;
}

void CBarrier::OnDestroy() 
{
	CStatic::OnDestroy();
}

void CBarrier::FindIOAddress()
{
    int fld = (no == 1) ? weighbridge_BR1_UP : weighbridge_BR2_UP;

    fldBarrierOpen = weighbridge.lval(fld);

    fld = (no == 1) ? weighbridge_BR1_DOWN : weighbridge_BR2_DOWN;
    fldBarrierClose = weighbridge.lval(fld);

    fld = (no == 1) ? weighbridge_BR1 : weighbridge_BR2;
    fldBarrierStatus = atoi(weighbridge.dat[fld]);

    if(no==1) {
        fld = GetDbFieldNo(&weighbridge,"BR1DISABLE");
    }
    else {
        fld = GetDbFieldNo(&weighbridge,"BR2DISABLE");
    }
    if(fld){
        flagDisable = dBfldLogic(&weighbridge,fld);
    }
}

void CBarrier::DoOpenBarrier()
{
    WriteCoil(fldBarrierClose,0);

    int d = (++closeCounter % 8 ) < 4;
    WriteCoil(fldBarrierOpen, d);
}

void CBarrier::DoCloseBarrier()
{

    WriteCoil(fldBarrierOpen,0);
    int d = (++closeCounter % 8 ) < 4;

    WriteCoil(fldBarrierClose, d);

}

void CBarrier::OnStartOpenTimer()
{
    openCounter = 0;
    TimerEvent = 1 + no * 2;
    UINT flag2 = AfxGetMainWnd()->SetTimer(TimerEvent,500,NULL);
	if(flag2==TimerEvent) {
		InProgress = TRUE;
	}
}

void CBarrier::OpenBarrier()
{
    if(TimerEvent==2) {
		KillTimer(TimerEvent);
		TimerEvent =0;
    }
    if(TimerEvent==1) {
		// Cannot put this message b'cos ontimer will ask open barrier again and again 
		//MessageBox("open again");
		return;
    }
    InProgress = FALSE;
    FindIOAddress();
    if(flagDisable) return;

    BOOL flag = BarrierStatus(no);
    if(flag==BarrierIsOpen){
        adamData[0] = 0;
//        adamtcp_WriteCoil(szIPAddress, 1, fldBarrierOpen, 1, adamData);
//        adamtcp_WriteCoil(szIPAddress, 1, fldBarrierClose, 1, adamData);
        WriteCoil(fldBarrierOpen,0);
        WriteCoil(fldBarrierClose,0);
		return;
	}


    iCount = 0;
    DoOpenBarrier();
    
    OnStartOpenTimer();
	

}

void CBarrier::OnStartCloseTimer()
{
    TimerEvent = 2 + no * 2;
    UINT flag2 = AfxGetMainWnd()->SetTimer(TimerEvent,500,NULL);
	if(flag2==TimerEvent) {
		InProgress = TRUE;
	}
	
}

void CBarrier::CloseBarrier()
{
    if(MControl) return;
    LastWt = GetCurrenctCommWeight(0);            //5.3.2009

    if(TimerEvent==2) {
		//MessageBox("close again");
		return;
    }

    if(TimerEvent==1) {
		KillTimer(TimerEvent);
		TimerEvent =0;
    }

	InProgress = FALSE;
    FindIOAddress();
    if(flagDisable) return;

    BOOL flag = BarrierStatus(no);
    if(flag==BarrierIsClose){
        WriteCoil(fldBarrierOpen,0);
        WriteCoil(fldBarrierClose,0);
		return;
	}


    iCount = 0;
    closeCounter = 0;
    DoCloseBarrier();
    OnStartCloseTimer();
}

