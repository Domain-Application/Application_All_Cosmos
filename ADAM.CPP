// adam.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "adamtcp.h"
#include "modbus.h"
#include "comm.h"
#include "menu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment (lib, "adamtcp.lib")
#define DeviceID       0x01  // 0x01 indicate the 5000/TCP

extern int ADAMRequest(const char *server_name,char *command, BYTE *byData);
CString fileCtrlLog="ctrl.log";

int WriteCoilFlag = 0,AI1;

int ADAM_flag=-1;
int ADAMSimulator;
char szIPAddress[99];
#define DEFAULT_PORT   502  // Port for Modbus/TCP
//-------- default timeout ------
int     iConnectionTimeout=2000;
int     iSendTimeout=2000;
int     iReceiveTimeout=2000;
CString WeightServer = "localhost";
BYTE DiData[500];
BYTE DoData[500];
BYTE adamData[1280];

CString last_ms;

int SensorStatus(int no)
{
	if(ADAM_flag) return 0;

	int fld = 0;
    if(no==1 || no==4) fld = weighbridge_Sensor1;
    if(no==2 || no==3) fld = weighbridge_Sensor2;


    int point = weighbridge.lval(fld);
    if(point==0) {
        return FALSE;
    }
	if(no>2) point += 32;
    

    return int( adamData[ point - 1] );
}

void addLog(const char *msg)
{
}

void SetADAM_IPAdd()
{
	if(ADAMSimulator==1) {
		strcpy(szIPAddress, "localhost");
	}
	else {
		strcpy(szIPAddress, weighbridge.dat[weighbridge_IP]);
	}
	trim(szIPAddress);

}


BOOL ADAM_Open()
{
	if(ADAMSimulator==1) {
        ADAM_flag = 0;
        return 0;
	}

    app->Footmark("Adam open");
    ADAM_flag = ADAMTCP_Open();
    app->Footmark(szIPAddress);
    ADAM_flag=ADAMTCP_Connect(szIPAddress,DEFAULT_PORT,
				  iConnectionTimeout, iSendTimeout, iReceiveTimeout);

    app->Footmark("Adam flag");
    return ADAM_flag;
}

void ADAM_Close()
{
	if(ADAMSimulator==1) {
        return;
	}
    if( ADAM_flag<0 ) return;

    ADAMTCP_Disconnect();
}

int adamtcp_WriteCoil(char *szIPAddress, WORD wDeviceID, WORD wStartAddress, WORD wCount, BYTE *byData)
{

    int iRetVal=0;
	if(ADAMSimulator) {
        CString errmsg;
        char s[2000];

        sprintf(s,"ADAMW:%3.3i:%3.3i:",wStartAddress,wCount);
        for(int i=0; i<wCount; i++) {
            if(byData[i]==0) {
                strcat(s,"0");
            }
            else {
                strcat(s,"1");
            }
        }
        int flag = ClientRequest(WeightServer, s,errmsg);
        return 0;
    }

    for(int i=0; i<10; i++) {
        iRetVal=ADAM_Open();
        if(!iRetVal) {
            iRetVal=ADAMTCP_WriteCoil(szIPAddress, wDeviceID, wStartAddress, wCount, byData);
        }
        ADAMTCP_Disconnect();
        if( !iRetVal ) return iRetVal;
        Sleep(100);
    }

    ASSERT(FALSE);
    return iRetVal;
}

int adamtcp_ReadCoil(char *szIPAddress, WORD wDeviceID, WORD wStartAddress, WORD wCount, BYTE *byData)
{
    if( ADAM_flag<0 ) return 0;
	if(ADAMSimulator) {
		CString errmsg;
        char s[2000];

        sprintf(s,"ADAMR:%3.3i:%3.3i:",wStartAddress,wCount);
        int flag = ADAMRequest(WeightServer, s, byData);
        return 0;
    }

    int iRetVal=ADAM_Open();
    if(!iRetVal) {
        int iRetVal=ADAMTCP_ReadCoil(szIPAddress, wDeviceID, wStartAddress, wCount, byData);
    }
    ADAMTCP_Disconnect();
    if( iRetVal )
    {
        CString msg;
        msg.Format("ReadCoil Fail %d\n", iRetVal);
        addLog(msg);
        ASSERT(FALSE);
    }
    return iRetVal;
}

void WriteCoil(int Id,BOOL OnOFF)
{
	if(Id<0) return;

	WriteCoilFlag = 1;

	SetADAM_IPAdd();

    CString s;

    s.Format("Write coil %i %i", Id, OnOFF);
    app->Footmark(s);
    if(ADAMSimulator==2) {

		// Id -= 16;		//PLC TCP

        CString s;
        CString m_ip = szIPAddress;
        UINT m_port = 503;
        int m_slave = 1;
        int function = 5;
        int m_add = Id - 1;

        s.Format("Write coil id:%i value:%i", m_add,OnOFF);
		addLog(s);

        BYTE data[1000] = {0};
        int nb = OnOFF ? 0xff : 0;

		for(int zz=0; zz<5; zz++) {
			int flag2 = OnModbusTCP(m_ip,m_port,m_tid,m_slave, function,  m_add, nb, 0, data, sizeof(data));
            int ret = data[10];
			if(flag2==TRUE) {
				if( data[10] != BYTE(nb)) {
                    s.Format("error Write coil id:%i value:%i return:%i/%i  %s", m_add,OnOFF,nb,ret,last_ms);
					addLog(s);
                    Sleep(100+50*zz);
					continue;
				}
			}
            if(zz) {
                 s.Format("try%i Write coil id:%i value:%i return:%i/%i", zz, m_add,OnOFF,nb,ret);
                 addLog(s);
            }
			break;
		}
		WriteCoilFlag = 0;

        return;
    }
    if(ADAMSimulator==3) {

		Id -= 16;		//PLC RTU
        UINT m_port = 1;
        int m_slave = 1;
        int function = 5;
        int m_add = Id - 1;

        BYTE data[1000] = {0};
        int nb = OnOFF ? 0xff : 0;
        int flag2 = OnModbusRTU(m_port,m_slave, function,  m_add, nb, 0, data, sizeof(data));
		if(flag2==TRUE) {
			DoData[m_add] = OnOFF;			//performance issue
		}

		WriteCoilFlag = 0;
        return;
    }


    BYTE byData[12];

	byData[0] = OnOFF;
	WORD wDeviceID=1;
	WORD wStartAddress=Id;
	WORD wCount = 1;
	int iRetVal=adamtcp_WriteCoil(szIPAddress, wDeviceID, wStartAddress, wCount, byData);
    CheckMemoryError();
	WriteCoilFlag = 0;
	DoData[wStartAddress-17] = OnOFF;			//performance issue

    if( iRetVal ) return;
}

int readADAMFlag = 0;
int fvalue[9];
int readADAMData(BYTE *inbuff,int function,int step)
{
    if(WriteCoilFlag) {
        addLog("read adam busy");
        return -2;
    }

	SetADAM_IPAdd();

    if(ADAMSimulator==2) {    //modbus TCP

        CString m_ip = szIPAddress;
        UINT m_port = 502;
        int m_slave = 1;
        int m_add = 0;

		app->Footmark(m_ip);

        readADAMFlag = 1;
        BYTE data[1000] = {0};
        int flag = OnModbusTCP(m_ip,m_port,m_tid,m_slave, function,  m_add,0, 24, data, sizeof(data));

        readADAMFlag = 0;
		if(_modbus_error_code==0) {		//if error not update
			if(function<=2) {
				bit2Byte(&data[9], inbuff, 3);
				
				fvalue[function] = 0;
				memcpy((void*)&fvalue[function], &data[9], 3);
			}
			else {
				bit2Byte(&data[9], &inbuff[128], 3);
			}
		}
        return flag;
    }
    else if(ADAMSimulator==3) {  //modbus RTU

        UINT m_port = 1;
        int m_slave = 1;
        int m_add = 0;

        BYTE data[1000] = {0};
        BOOL flag = OnModbusRTU(m_port,m_slave,function, m_add, 0, 24, data, sizeof(data));
        if(function<=2) {
            bit2Byte(&data[9], inbuff, 3);
        }
        else {
            bit2Byte(&data[9], &inbuff[128], 3);
        }
        return flag;
    }

	int id = 1;
	if(function==1) {			//read coil
		id += 16;
	}
    int iRetVal=adamtcp_ReadCoil(szIPAddress, 1, id, 32, inbuff);

    int v = Byte2bit(inbuff, 32);
	fvalue[function] = v;

    return iRetVal;
}

BOOL BarrierStatus(int no)
{	
	int fld = weighbridge_BR1 + (no-1) *  5;

	int br1_status = weighbridge.lval(fld);
    if(br1_status) {
		return !DiData[br1_status-1];
	}

	return FALSE;
}

BOOL ILoopStatus(int no)
{
	int fld = weighbridge_LOOP1 + (no-1) *  5;

	int br1_status = weighbridge.lval(fld);
	if(br1_status) {
		return DiData[br1_status-1];
	}

	return FALSE;
}


CString strIndicator(int current_indicator)
{
    char szIndicator[2] = {0};

	if(current_indicator<1 || current_indicator>2) current_indicator = program4;

    szIndicator[0] = 'A' + current_indicator - 1;

	return CString(szIndicator);
}

int cmob[2] = {0};
int cmst[2] = {0};
int CheckManualOpenBarrier(int sq,CListBox  &m_list)
{
	CString s;
	int  flag = 0;

    for(int i=0; i<2; i++) {
        int di_point = weighbridge.lval(weighbridge_MOB1+i) - 1;
        if(di_point<=0) continue;

		int status = DiData[di_point];
		if( status == cmst[i]) continue;
		cmst[i] = status;
        if( status ) {

			if(sq - cmob[i] < 20) continue;

            CString n = mysql_now_local(1);

            s.Format("insert into [LogManualOpenBarrier] (datetime,wb,BarrierNo) values ('%s','%s',%i)",
                n, strIndicator( program4 ), i+1);
            runsql(s);

            s.Format("BR%i ->  %s", i+1, n);
            m_list.AddString(s);
			cmob[i] = sq;
			flag++;
        }
    }
	return flag;
}
