// Comm.cpp : implementation file
//

#include "stdafx.h"
#include "myapp.h"
#include "comm.h"
#include "math.h"
#include "db_ado.h"

extern UINT ReadTcpData(CString m_ipadd,int m_port);

/***************************************************
*  win.ini                                         *
*                                                  *
* [Indicator]                                      *
* device=a,bbbb,ccc,sXXgXXiXNNNNNNF                *
* device=a,bbbb,ccc,ssXggXiXNNNNNNF                *
* MinAcceptWeight=500                              *
*                                                  *
* a: com port  b: baud rate  c:stx(start of text)  *
* X: SKIP  N: WEIGHT  F:FINISH T:TARE/NET          *
* s: Stable/Unstable  i:+ or -                     *
* g: gross/tare                                    *
* device=1,2400,010,sXXgXXiXNNNNNNF                *
*  ST,NT,+ 000000Kg
*  ST,GS,+ 001140Kg
*  US,NT,+00619.5kg
*  US,GS,+ 001150Kg
*
* https://stackoverflow.com/questions/9226082/device-misdetected-as-serial-mouse
* HKLM\SYSTEM\CurrentControlSet\Services\sermouse\start ==> 4
* sc config "sermouse" start=disabled
*****************************************************************/
#define _HistoryCount_ 20

int  not_allow_dirty_signal = 0,isExeCall=0;
long MinWt,MaxWt,___lastwt,IsZeroWt,MustRecordMaxWt;
long _duration_readcomm,HistoryWt[_HistoryCount_];
long ZeroCount = 0;
double __minVar=1000;
long cMaxWt1;
char cMaxTime1[20],cMaxDate1[20];
char cB4Time[20],cB4Date[20];
long IndicatorMinWt,cweight,cw_count,commLastWeigh;
long GwstWeight,ActualWeight,GwstPercent;
double fweight=0,tweight=-1.99;
HANDLE hComm;
BOOL ZeroWeight1,bReading,bNeverRead,bPlusSign,Indicator1NoSingal;
int  xk3118mode=0,GSE460Mode=0,leadchar,CommFixLength,CommThreadProc = 0,commMode=3;
int  autoCancelDeductWeight = 0,__maxWeight=999990;
char indicator_string[200],szWtCaptureTime[20];
CString WeighbridgeNo = "";
BOOL C1Stable,C1Gross;
BOOL useTcp4demo;

/******************************************************************************
* 0: unknown 1: 1st wt 2: 2nd wt 3: single wt 8: over max wt 9: drive through *
*******************************************************************************/
void AddLog(const char *m_msg)
{
}

int  FindCommFixLength(char *inbuff,int tlen,char leadchar)
{
	int i = 0,j=0;
	for(i=0; i<tlen; i++) {
		if( inbuff[i] == leadchar) {
			j = i;
			break;
		}
	}
	if(i>=tlen)	return CommFixLength;

	i+=6;
	for(; i<tlen; i++) {
		if( inbuff[i] == leadchar) {
			int len = i - j;
			return len;
		}
	}


	return CommFixLength;
}

BOOL IsHistoryWtStable(long var, int checkCount)
{
    if(checkCount>19) checkCount = 19;
	
    for(int j=checkCount; j>0; j--) {
        long v = HistoryWt[j] - HistoryWt[j-1];
        if( labs(v) > var) return FALSE;
    }

	char szTime[20];
    strtime(szTime);
    long t1 = TimeFormatToSeconds(szTime);
    long t2 = TimeFormatToSeconds(szWtCaptureTime);
    if(t1<t2) return FALSE;
    long secs = t1 - t2;
    if(secs > 5) {
        Indicator1NoSingal = TRUE;
        cweight=-1;
    }
    if(secs > 2) {
         return FALSE;
    }
    Indicator1NoSingal = FALSE;


    return TRUE;
}

BOOL put_history_wt(long weight,long var,UINT stable_count)
{
    int ust = 0;
    int ll = min ( stable_count, _HistoryCount_);
    for(int i=0; i<ll ; i++) {
        HistoryWt[i] = HistoryWt[i+1];
        long off = labs(HistoryWt[i] - weight);
        if( off >= var) {
            ust++;
        }
    }

    HistoryWt[ll] = weight;
    return (ust==0);
}

BOOL SaveWt(char *table, long cweight,int type, int fldViolation)
{
    if(!app->m_RecordWtStatus) return FALSE;
    if(cnn == NULL) return FALSE;

    CString strValue;

    if(type==0) {
        strValue.Format("INSERT INTO [%s] (WEIGHT,WBNO) VALUES (%i,'%s') ",
            table , cweight,WeighbridgeNo);
    }
    else if(type>=1) {
        int key = 0;
        if(fldViolation) {
            key = SqlInt("select max(pkkey) from logalarm");
        }
        strValue.Format("INSERT INTO [%s] ([DATE],[B4TIME],[WEIGHT],WBNO,NEWUSER,TYPE,LOGALARM) VALUES ('%s %s','%s %s', %i,'%s','%s', %i, %i) ",
            table , cMaxDate1, cMaxTime1,  cB4Date, cB4Time,
            cweight, WeighbridgeNo,app->m_UserCode, type ,key);
    }
    runsql(strValue,FALSE,FALSE,cnn__2);
    return TRUE;
}

long GetProfileValue(const char *key)
{
    char szAllDevices[1024];

    memset( szAllDevices,0, 1024);
    ::GetProfileString(IndicatorProfile, key, "",szAllDevices,1024);

    long v = -1;
    if(szAllDevices[0]!=0) {
         v = atol(szAllDevices);
    }
    return v;
}

CString GetProfileText(const char *key)
{
    char szAllDevices[1024];
	
    memset( szAllDevices,0, 1024);
    ::GetProfileString(IndicatorProfile, key, "",szAllDevices,1024);
	
	CString v = szAllDevices;
    return v;
}


long GetProfileIndicatorMinWeight()
{
    return GetProfileValue("MinWt");
}

long GetProfileIndicatorDataSize()
{
    long v = GetProfileValue("ByteSize");

    return (v <=5 || v>8) ? 7 : v;
}

long GetProfileIndicatorDataBits()
{
    long v = GetProfileValue("DataBits");

    return v;
}

long GetProfileIndicatorParity()
{
	int v = GetProfileValue("Parity"); 

    return (v <0 || v>4) ? 2 : v;
}

long GetProfileIndicatorThreadMode()
{
    int v = GetProfileValue("ThreadMode");

    return v;
}

long GetProfileIndicatorCommMode()
{
    int v = GetProfileValue("CommMode");

    return v;
}

long GetProfileIndicatorStopBit()
{
    int v =  GetProfileValue("StopBit");   // 0,1,2 = 1, 1.5, 2

    return (v <=0 || v>2) ? 0 : v;
}

long GetProfileOutputCom()
{
    return GetProfileValue("output");
}

BOOL GetProfileIndicatorSetting(int &port,int &baudrate,int &leadchar,char *indicator_string)
{
    char szAllDevices[1024];
    memset(szAllDevices,0,sizeof(szAllDevices));
    ::GetProfileString(IndicatorProfile,"device",",,,,",szAllDevices,sizeof(szAllDevices));

    char *s1 = strtok(szAllDevices,",");
    if(s1==NULL) return FALSE;
    char *s2 = strtok(NULL,", ");
    if(s2==NULL) return FALSE;
    char *s3 = strtok(NULL,", ");
    if(s3==NULL) return FALSE;
    char *s4 = strtok(NULL,", ");
    if(s4==NULL) return FALSE;

    port      = atol(s1);
    baudrate  = atol(s2);
    leadchar  = atol(s3);
    if(leadchar>255 || (baudrate % 30) ) {
        CString s;
        s.Format("wi.ini error\n\nCom port:%i baud rate:%i token:%i ",
           port, baudrate, leadchar);
        AfxMessageBox( s );
    }

    strcpy(indicator_string,s4);
    return TRUE;
}

BOOL OpenCommPort(int comport,int baudrate)
{
	CString s;

    s.Format("Open com port:%i", comport);
    app->Footmark(s);
    if(comport==99) {
        useTcp4demo = TRUE;
        app->Footmark(s);
        return TRUE;
    }
    useTcp4demo = FALSE;
    int ByteSize = (unsigned char) GetProfileIndicatorDataBits();
    if(ByteSize<5 || ByteSize>8) {
        ByteSize = (unsigned char) GetProfileIndicatorDataSize();
    }

    int Parity   = (unsigned char) GetProfileIndicatorParity();
    int StopBits = (unsigned char) GetProfileIndicatorStopBit();

    hComm = OpenCom(comport,baudrate,ByteSize,Parity,StopBits);

    if ( hComm == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    IndicatorMinWt = GetProfileIndicatorMinWeight();
    ZeroWeight1 = FALSE;

    return TRUE;
}

void CheckZeroWeight(const long wt)
{
	CString str;
    if(wt<= app->w_zeroWt && C1Gross==TRUE) {
		ZeroCount++;
		if (ZeroCount>=5){
            IsZeroWt = 1;
		}
		
	}	
	else{
		ZeroCount=0;
	}
    if(!app->m_RecordWtStatus) return;

    if( IsZeroWt && wt<= app->w_zeroWt) {
        if(cMaxWt1 >app->w_driverThroughWt && MustRecordMaxWt) {
            int key1 = SqlInt("select max(pkkey) from logalarm");
            if(WbTicketSaved == 0) {
                app->OnLogAlram4DriveThrough(cMaxWt1,cMaxDate1,cMaxTime1,WeighbridgeNo);
            }
            else {
                app->OnLogAlram4OverMaxWt(cMaxWt1,TicketWt,cMaxDate1,cMaxTime1,WeighbridgeNo);
            }
            int key2 = SqlInt("select max(pkkey) from logalarm");
            if(WbTicketSaved == 0) {
                 SaveWt("WEIGHT_MAX_DATA", cMaxWt1, 9, key2-key1);
            }
            else {
                 SaveWt("WEIGHT_MAX_DATA", cMaxWt1, 8, key2-key1);
            }
            TicketWt = 0;
            cMaxWt1 = 0;
        }
        MustRecordMaxWt = 1;
        WbTicketSaved = 0;
        TicketVehicleNo = "";
        space_str(cB4Time,8);
        space_str(cB4Date,8);

        BOOL bflag = IsAdoTableExisted("TIMEFRM");
        if(bflag) {
            if(IsSqlFile == TRUE) {
                str = "truncate table timefrm";
            }
            else {
                str = "delete from timefrm";
            }
            runsql(str,FALSE,FALSE,cnn__2);
        }
        return;
    }
    if(cweight > 200) {
        if(isblank(cB4Time)) {
            _strtime(cB4Time);
            _strdate(cB4Date);
        }
    }
    if(cweight > cMaxWt1 && cweight > MaxWt) {
        _strtime(cMaxTime1);
        _strdate(cMaxDate1);
        cMaxWt1 = cweight;
    }
    if(!app->m_SaveWeighbridgeData) return;

    SaveWt("WEIGHT_BRIDGE_DATA", cweight, 0, 0);
}

void Send2Display()
{
    if(cweight<=IndicatorMinWt)  cweight = 0;

	CheckZeroWeight(cweight);
	
    ___lastwt = cweight;
    if( cweight <=30 ) {					//need to think
        if(autoCancelDeductWeight) {
            __maxWeight = 999990;
            autoCancelDeductWeight = 0;
			IsZeroWt = 0;		//reset zero flag 2025-10-10
            GwstWeight = 0;
            GwstPercent = 0;
            app->ResetDeductWeight();
        }
    }
	
    if(app->bComOutput==FALSE) return;
    cw_count++;
    if( GetProfileValue("DisableComOutput") > 0) return;

    app->ToDisplay(hComm,cweight, GwstWeight);
}


void locProcessCommError(HANDLE hComm)
{
    DWORD dwError = GetLastError();
    if(dwError==0) return;
    unsigned long lrc;
    COMSTAT cs;

    if(commMode==3) {
        TRACE("Comm error %ld\n",dwError);
    }
	/* clear error */
	ClearCommError (hComm, &lrc, &cs);

} /* end function (locProcessCommError) */


double oldww = 0;
void read_wt_end(char m_char,char *wt, long &stx_wt, long &stx)
{
     wt[stx_wt] = 0;
     trim(wt);

	 int ScrollKeyStatus = GetKeyState(VK_SCROLL);
	 if(ScrollKeyStatus) {
		  app->Footmark(wt);		//because data too many
	 }

     if(m_char=='R') {
         CString s = wt;
         s.MakeReverse();
         strcpy(wt, s);
     }

	 double ww = atof(wt);

	 double var = fabs(oldww-ww);
	 oldww = ww;

     if( var> __minVar) {
		 return;		
	 }
	 

     if(bPlusSign == FALSE) {
         fweight = ww * -1;
		 if(xk3118mode) {
			 fweight /= 10;
		 }
     }
     else {
         fweight = ww;
     }



     cweight = long(fweight);
     
	 
	 strtime(szWtCaptureTime);
//	 TRACE("read_wt_end %s %ld\n", nowMs(),cweight);

     if(cweight <= IndicatorMinWt){
         ZeroWeight1 = TRUE;
     }
     if(tweight>=0) {
         C1Gross = fabs(fweight-tweight) < 0.0001;
     }
     stx = -1;
     cweight = app->GetWt(cweight);

     put_history_wt(cweight,10,19);
     Send2Display();
}

void save_trace(char m_char, DWORD dwBytes, char *buf)
{
     if(commMode==3) {
//         TRACE("Wt: %9.3f %i %i len=%i %s\n", fweight,C1Stable,C1Gross,dwBytes, buf);
     }
     else if(commMode==4) {
         CString file;
         file.Format("disp%4s.log", &today[4]);
         FILE *ptr = fopen(file,"a");
         if(ptr) {
             fprintf(ptr,"%c Wt: %9.3f %i %i len=%i\n%s\n\n",
                 m_char, fweight,C1Stable,C1Gross, int(dwBytes), buf);
             fclose(ptr);
         }
     }
}

/*
date: 2021-06-24 gse460 unstable weight format change

S Stat 0   8370 kg    Gross

  8390
  8450
  8510S Stat 0   8510 kg    Gross
  
*/

void locProcessBytes (char *buf, DWORD dwBytes)
{
	int ScrollKeyStatus = GetKeyState(VK_SCROLL);
	if(ScrollKeyStatus) {
		app->Footmark(buf);
	}
    

	char wt[200];
    long stx    = -1;
    long stx_wt = 0;

	//2022-08-29 command mode no need to find leadchar
	//special ad4401 command mode
	if(CommFixLength==dwBytes && dwBytes==18 && leadchar==10) {		
		stx = 0;
	}

    memset( wt, 0, sizeof(wt) );
    int ss = 0;
    for(DWORD i=0; i<dwBytes; i++) {
        if( bReading == FALSE) break;

        char CommChar = buf[i];
        if(GSE460Mode==1) {
            if((int(CommChar) == 3  && i >= 7)) {
                stx_wt = 6;
                strcpy( wt , &buf[i-stx_wt]);
                read_wt_end(0, wt, stx_wt, stx);
                save_trace('*',dwBytes, buf);
                continue;
            }
        }

        if( int(CommChar) == leadchar ) {
            ss = i;
        }

        if((int(CommChar) == leadchar || int(CommChar) == 2)  && stx== -1) {
            stx = stx_wt = 0;
            bPlusSign = TRUE;

            continue;
        }
        else if( int(CommChar) == 45 && leadchar == 43 ) {
            stx = stx_wt = 0;
            bPlusSign = FALSE;


            continue;
        }
        else if( int(CommChar) == 44 && leadchar == 44) {
            stx = stx_wt = 0;
            bPlusSign = TRUE;
            continue;
        }
        if( stx<0 ) continue;

		int sq = stx;
        char m_char = indicator_string[stx++];
        if(m_char=='N'){
            if ((CommChar>='0' && CommChar<='9')    //ignore dirty signal
                 || (CommChar==' ') || (CommChar=='.')) {
                wt[stx_wt++] = CommChar;
            }
            else if( int(CommChar) == 43  ) {
                bPlusSign = TRUE;
            }
            else if( int(CommChar) == 45  ) {
                bPlusSign = FALSE;
            }
            else if( int(CommChar) == 'k' || int(CommChar) == 'g' ) {
            }
            else {
                if(not_allow_dirty_signal) {
                    CString file;
                    file.Format("dirty%4s.log", &today[4]);
                    FILE *ptr = fopen(file,"a");
                    if(ptr) {
                        fprintf(ptr,"%i %c %s\n", i, CommChar, buf);
                        fclose(ptr);
                    }
                    return;         //dirty signal
                }
            }

        }
        else if(m_char=='T'){
            if (CommChar=='G') {
                C1Gross  = TRUE;
            }
            else if (CommChar=='N') {
                C1Gross  = FALSE;
            }
        }
        else if(m_char=='i') {           //07-10-2011
            if(CommChar=='+') {
                bPlusSign = TRUE;
            }
            else if(CommChar=='-') {
                bPlusSign = FALSE;
            }
        }
        else if (strncmp(&indicator_string[sq],"ssXggX",6)==0 ||
                 strncmp(&indicator_string[sq],"sXXgXX",6)==0) {
            if( strncmp (&buf[i],"ST,GS,",6)==0) {
                C1Stable = TRUE;
                C1Gross  = TRUE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"US,GS,",6)==0) {
                C1Stable = FALSE;
                C1Gross  = TRUE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"UL,GS,",6)==0) {
                C1Stable = FALSE;
                C1Gross  = TRUE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"OL,GS,",6)==0) {
                C1Stable = FALSE;
                C1Gross  = TRUE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"ST,NT,",6)==0) {
                C1Stable = TRUE;
                C1Gross  = FALSE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"US,NT,",6)==0) {
                C1Stable = FALSE;
                C1Gross  = FALSE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"UL,NT,",6)==0) {
                C1Stable = FALSE;
                C1Gross  = FALSE;
				i+=5; 
				stx+=5;
            }
            else if( strncmp (&buf[i],"OL,NT,",6)==0) {
                C1Stable = FALSE;
                C1Gross  = FALSE;
				i+=5; 
				stx+=5;
            }
        }

        else if(m_char=='F' || m_char=='R' || m_char=='E'){

//            if(wt[5] != '0') return;

            if(m_char=='E' && i>5  ) {
				buf[i] = 0;
                CString s = &buf[i-5];
                s.MakeLower();
                int n= s.Find("gross");
                C1Gross = (n>=0);
            }

            read_wt_end(m_char, wt, stx_wt, stx);
            save_trace(m_char,dwBytes, buf);

            continue;
        }
        else if(m_char=='f' || m_char=='r'){
            wt[stx_wt] = 0;
            trim(wt);
            if(m_char=='r') {
				CString s = wt;
				s.MakeReverse();
				strcpy(wt, s);
            }
            if(bPlusSign == FALSE) {
                tweight = atof(wt) * -1;
            }
            else {
                tweight = atof(wt);
            }
            stx_wt = 0;
            memset(wt, 0 , sizeof(wt));
            continue;
        }
    }
}

UINT ReadCommThreadProc(LPVOID pParam)
{
    char  inbuff[10240];
    DWORD nBytesRead, dwEvent, dwError;
    COMSTAT cs;
    int len1=0,len2=0;

    CommThreadProc = 1;
	app->Footmark("ReadCommThreadProc");

	/* generate event whenever a byte arives */
	SetCommMask (hComm, EV_RXCHAR);

    //2023-05-09
    not_allow_dirty_signal = GetProfileValue("NoDirtySignal");
    if(not_allow_dirty_signal<=0) not_allow_dirty_signal = 0;

    szWtCaptureTime[0] = 0;
    CommFixLength = GetProfileValue("SignalLen");
    if(CommFixLength <= 5) CommFixLength = 25;

    /* read a byte whenever one is received */
    bReading = TRUE;
	while (bReading) {
		/* wait for event */
		if (WaitCommEvent (hComm, &dwEvent, NULL)){
			/* read all available bytes */
            if( bReading == FALSE) break;
			ClearCommError (hComm, &dwError, &cs);
			if ((dwEvent & EV_RXCHAR) && cs.cbInQue) {
                if( bReading == FALSE) break;
                int len = cs.cbInQue;
                if(len>=sizeof(inbuff)) {
                     if(commMode==3) {
                         TRACE("Comm buffers full\n");
                     }
                     PurgeComm(hComm, PURGE_RXCLEAR);
                     continue;
                }
                if(len < CommFixLength * 2){
                    continue;
                }

				if(len>50) {
					int tlen = len - 50;
					ReadFile(hComm, inbuff, tlen, &nBytesRead, NULL);
					if(CommFixLength==25) {
						CommFixLength = FindCommFixLength(inbuff,tlen,leadchar);
					}
					len = 50;
				}

				memset( inbuff, 0, sizeof(inbuff));
                BOOL bResult = ReadFile(hComm, inbuff, len, &nBytesRead, NULL);
                if (bResult==FALSE){
                    locProcessCommError(hComm);
                }
				else {
                    bNeverRead = FALSE;
                    if (nBytesRead) locProcessBytes(inbuff, nBytesRead);
				}
			}
		} else {
            locProcessCommError(hComm);
		}
    }
	/* clean out any pending bytes in the receive buffer */
	PurgeComm(hComm, PURGE_RXCLEAR);
    CommThreadProc = 0;
    bReading = FALSE;
    if(commMode==3) {
        TRACE("Comm Thread Completed\n");
    }
    return 0;
}

UINT LoopReadCommThreadProc(LPVOID pParam)
{
	app->Footmark("LoopReadCommThreadProc");

    
	char  inbuff[10240];
    DWORD nBytesRead, dwError;
    COMSTAT cs;

    CommThreadProc = 1;

    long loopCount = 0;
    CommFixLength = GetProfileValue("SignalLen");
    if(CommFixLength <= 5) CommFixLength = 25;

    /* read a byte whenever one is received */
	while (bReading) {
        if(app->m_SystemExitNow) break;
		Sleep(10);			//reduce cpu usage percentage
        ClearCommError (hComm, &dwError, &cs);
        int len = cs.cbInQue;
        if(len==0){
             continue;
        }
        else if(len>=sizeof(inbuff)) {
             if(commMode==3) {
                 TRACE("Comm buffers full\n");
             }
             PurgeComm(hComm, PURGE_RXCLEAR);
             continue;
        }
        else if(len < CommFixLength * 2) {
             Sleep(40);
             continue;
        }

		if(len>50) {
			int tlen = len - 50;
			ReadFile(hComm, inbuff, tlen, &nBytesRead, NULL);
			len = 50;
			if(CommFixLength==25) {
				CommFixLength = FindCommFixLength(inbuff,tlen,leadchar);
			}
		}

        memset(inbuff, 0, sizeof(inbuff));
        BOOL bResult = ReadFile(hComm, inbuff, len, &nBytesRead, NULL);
        if (bResult==FALSE){
            /* handle error */
            locProcessCommError(hComm);
        } /* end if (error reading bytes) */
        else {
            bNeverRead = FALSE;
            if (nBytesRead)
                locProcessBytes(inbuff, nBytesRead);
        }
    }
	/* clean out any pending bytes in the receive buffer */
	PurgeComm(hComm, PURGE_RXCLEAR);
    CommThreadProc = 0;
    bReading = FALSE;
    if(commMode==3) {
        TRACE("Comm Thread Completed\n");
    }
    return 0;
}

UINT ReadTCPThreadProc(LPVOID pParam)
{
	CString m_ipadd = GetProfileText("ipadd");
	m_ipadd.TrimRight();
	long m_port = GetProfileValue("port");
    
	UINT flag = 0;
	while (bReading) {
		flag = ReadTcpData( m_ipadd, m_port);
		if(!bReading) break;

		Sleep(1000);
	}

    return flag;
}

extern BOOL GetProfileIndicatorSetting(int &port,int &baudrate,int &leadchar,char *indicator_string);
BOOL OpenCommEvent()
{
    
	isExeCall = 1;
    bNeverRead = TRUE;
    CString s;
    C1Stable   = C1Gross = TRUE;            //07-10-2011
    hComm = 0;
	cw_count = 0;
    IsZeroWt = 0;
    WbTicketSaved = 0;
    MustRecordMaxWt = 1;
    TicketVehicleNo = "";
    indicator_string[0] = 0;

    int  comport = 0, baudrate = 0;
    BOOL bFlag = FALSE;

    app->Footmark("OpenCommEvent " + IndicatorProfile);
    if(!app->no_win_ini) {
        bFlag = GetProfileIndicatorSetting(comport,baudrate,leadchar,indicator_string);
        if(bFlag) {
            s.Format("profile : %i %i %i %s",comport,baudrate,leadchar,indicator_string);
        }
        else {
            s = "unable read win.ini";
        }
    }
    else {
        s = "This program is no win.ini setting";
    }
    app->Footmark(s);

    if(bFlag==FALSE) {
        CString s1 = "c:\\windows\\comm.ini";        //window folder

        BOOL flag = IsFileExisted( s1 );
        if(!flag) {
            s1 = "\\comm.ini";            //root folder
            flag = IsFileExisted( s1 );
            if(!flag) {
                s1 = "comm.ini";            //program folder
                flag = IsFileExisted( s1 );
                if(!flag) {
                    app->Footmark("comm.ini not found");
                    return FALSE;
                }
            }
        }

        char szText[5026];
        CString s = "";

        FILE *ptr = Fopen(s1,"rt");
        if(!ptr) return FALSE;

        for (int line=1 ;; line++) {
            fgets( szText,4098,ptr);
            clear_text_line_feed(szText);
            if(line==1) {
                comport = atoi(szText);
            }
            else if(line==2) {
                baudrate = atoi(szText);
            }
            else if(line==3) {
                leadchar = atoi(szText);
            }
            else if(line==4 && strlen(szText)< sizeof(indicator_string)) {
                strcpy(indicator_string,szText);
            }
            else if(line==5) {
                IndicatorMinWt = atoi(szText);
            }
            else if(line==6) {
                CommFixLength = atoi(szText);
            }
            if(feof(ptr)) break;
        }
        fclose(ptr);
		s.Format("comm.ini : %i %i %i %s",comport,baudrate,leadchar,indicator_string);
		app->Footmark(s);
	}
    if(comport>0 && baudrate>0 && indicator_string[0]) {
        bReading = FALSE;
        OpenCommPort(comport,baudrate);
        return TRUE;
    }

	CString m_ipadd = GetProfileText("ipadd");
	m_ipadd.TrimRight();
	long m_port = GetProfileValue("port");

	
	if(m_port>0 && m_ipadd.GetLength()>0) {
        bReading = FALSE;
	    app->Footmark("Open TCP event");
        return TRUE;
	}

    app->Footmark("Unable open comm event");
    return FALSE;
}

BOOL IsCommWinExisted()
{
    int  comport, baudrate;
    BOOL bFlag = GetProfileIndicatorSetting(comport,baudrate,leadchar,indicator_string);
    return bFlag;
}

BOOL SetCommThread(HWND hWnd,BOOL Check)
{
    if( CommThreadProc == 1) {
        if(commMode==3) {
            TRACE("Thread still active\n");
        }
        return FALSE;
    }

    bReading = FALSE;
    memset(HistoryWt , 0, sizeof(HistoryWt));

    __minVar = (double) GetProfileValue("Var");
    if(__minVar<=0) __minVar = 1000;

    bReading = TRUE;
    CWinThread *pThread;

	CString m_ipadd = GetProfileText("ipadd");
	m_ipadd.TrimRight();
	long m_port = GetProfileValue("port");

	
	if(m_port>0 && m_ipadd.GetLength()>0) {
        pThread = AfxBeginThread(ReadTCPThreadProc, hWnd, THREAD_PRIORITY_BELOW_NORMAL);
		if(!pThread) {
			 bReading = FALSE;
			 return FALSE;
		}
		else {
			Sleep(100);
		}
		return TRUE;
	}

    if(!hComm) return FALSE;

    if(Check) {
         BYTE  inbuff[100];
         DWORD nBytesRead;
         if (ReadFile(hComm, inbuff, 1, &nBytesRead, NULL)==0) {
             nBytesRead = 0;
         }
         if(nBytesRead==0) {
             ErrorBox(IDS_ERROR_INDICATOR, 1311);
             CommThreadProc = 0;
             return FALSE;
         }
    }

    
	long mode = GetProfileIndicatorThreadMode();
    CommThreadProc = 1;         //claim before AfxBeginThread so a second call cannot start another reader
    if(mode==0) {
        pThread = AfxBeginThread(LoopReadCommThreadProc, hWnd, THREAD_PRIORITY_BELOW_NORMAL);
    }
    else {
        pThread = AfxBeginThread(ReadCommThreadProc, hWnd, THREAD_PRIORITY_BELOW_NORMAL);
    }


    GSE460Mode = GetProfileValue("GSE460");
    xk3118mode = GetProfileValue("XK3118");
    commMode = GetProfileIndicatorCommMode();
    if(commMode<0) commMode = 0;
    if(!pThread) {
         CommThreadProc = 0;
         bReading = FALSE;
         return FALSE;
    }
	else {
		Sleep(100);
	}
    return TRUE;
}

void CloseCommThread()
{
    bReading = FALSE;
}

static void WaitCommThreadExit(int ms)
{
    for(int waited=0; CommThreadProc==1 && waited<ms; waited+=20) Sleep(20);
}

//CancelIoEx is Vista+ and not in the VC6 SDK, so it is looked up at run time
static void CancelPendingCommIo(HANDLE h)
{
    typedef BOOL (WINAPI *PFN_CANCELIOEX)(HANDLE, LPOVERLAPPED);
    PFN_CANCELIOEX pCancelIoEx = (PFN_CANCELIOEX) GetProcAddress(GetModuleHandle("kernel32.dll"), "CancelIoEx");
    if(pCancelIoEx) pCancelIoEx(h, NULL);
}

void CloseCommEvent()
{
    if(hComm) {
        bReading = FALSE;

        //port is non-overlapped: SetCommMask/CloseHandle would block behind the reader's pending WaitCommEvent
        WaitCommThreadExit(1000);
        if( CommThreadProc == 1) {
            CancelPendingCommIo(hComm);
            WaitCommThreadExit(1000);
        }
        if( CommThreadProc == 1) {
            app->Footmark("Com port not closed: reader thread still active");
            if(commMode==3) {
                TRACE("\nCom port no close.");
            }
			return;
		}

        /* clear all events and force WaitCommEvent to return */
        BOOL flag = SetCommMask (hComm, 0L);

        /* clean out any pending bytes in the receive buffer */
        PurgeComm(hComm, PURGE_RXCLEAR);
		
        TryCloseHandle(hComm);
        if(commMode==3) {
            TRACE("\nCom port close.");
        }
    }
}

long GetCurrenctCommWeight(int port)
{
    if(useTcp4demo) {
        CString WeightServer = GetProfileText("WmServer");
		
		WeightServer.TrimRight();
		if(WeightServer.GetLength()<=0) {
			WeightServer = "localhost";
		}

        CString errmsg;
        char szPort[10];
        sprintf(szPort,"W%1.1i", port);
        int flag = MyRequest(WeightServer, szPort,errmsg, 5001);
        if(flag==-1) {

            //2024-07-26 cannot auto change
            //useTcp4demo = FALSE;
            cweight = -1;
            Indicator1NoSingal = TRUE;
            return cweight;

        }
        cweight = atol(errmsg);
        if(cweight <= IndicatorMinWt){
            ZeroWeight1 = TRUE;
        }
        CheckZeroWeight(cweight);
        commLastWeigh = cweight;
        Indicator1NoSingal = FALSE;
        return cweight;
    }

	char szTime[20];
    strtime(szTime);
    long t1 = TimeFormatToSeconds(szTime);
    long t2 = TimeFormatToSeconds(szWtCaptureTime);

	_duration_readcomm = t1 - t2;
    if(_duration_readcomm < 0) return 0;

    if(_duration_readcomm > 5) {
        Indicator1NoSingal = TRUE;
        cweight = -1;
        return -1;
	}
    Indicator1NoSingal = FALSE;
    commLastWeigh = cweight;
    return cweight;
}

double GetCurrentWeight()
{
	int port = 1;
    if(useTcp4demo) {
        CString WeightServer = GetProfileText("WmServer");

		WeightServer.TrimRight();
		if(WeightServer.GetLength()<=0) {
			WeightServer = "localhost";
		}
        CString errmsg;
        char szPort[10];
        sprintf(szPort,"W%1.1i", port);
        int flag = MyRequest(WeightServer,szPort,errmsg, 5001);
        if(flag==-1) {
            useTcp4demo = FALSE;
        }
		fweight = atof(errmsg);
        cweight = long(fweight);
        if(fweight <= IndicatorMinWt){
            ZeroWeight1 = TRUE;
        }
        CheckZeroWeight(cweight);
        commLastWeigh = cweight;
        Indicator1NoSingal = FALSE;
        return fweight;
    }

	char szTime[20];
    strtime(szTime);
    long t1 = TimeFormatToSeconds(szTime);
    long t2 = TimeFormatToSeconds(szWtCaptureTime);
    if(t1<t2) return 0;
    long secs = t1 - t2;
    if(secs > 50) {
        Indicator1NoSingal = TRUE;
        CString s;

        s.Format("%s %s %isec %ldkg %i\n",
           szTime, szWtCaptureTime, secs, cweight, CommFixLength);
        app->Footmark(s);

        fweight = -1;
        return -1;
	}
    Indicator1NoSingal = FALSE;
    commLastWeigh = cweight;
    return fweight;
}


UINT ReadTcpData(CString m_ipadd,int m_port)
{
    CString s,m_msg;

	WSADATA ws;
	WSAStartup(MAKEWORD(2,2),&ws);

    SOCKET sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        //AfxMessageBox("Could not create socket. Error");
        m_msg.Format("Could not create socket. Error, IP %s \r\n",m_ipadd);
        AddLog(m_msg);
		cweight = -2;
        WSACleanup();
        return 1;
    }
	
	struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(m_ipadd);
    server.sin_family = AF_INET;
    server.sin_port = htons( m_port );
	
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        m_msg.Format("Connection failed. Error, IP %s \r\n", m_ipadd);
        AddLog(m_msg);
		cweight = -3;
        closesocket(sock);
        WSACleanup();
        return 2;
    }

    BYTE Buffer[200];       //locProcessBytes copies into a 200-byte wt[] and treats buf as a C string
	memset(Buffer, 0, sizeof(Buffer));
	
    int fd = sock;
	struct timeval timeout = {0, 500 * 1000};  // make select() return once per second
	
	int flag = 0, offset = 0;
	while(bReading ) {

		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(fd, &readSet);
		int nReturn = select(fd+1, &readSet, NULL, NULL, &timeout);
		if (nReturn < 0){
            Sleep(100);
            continue;
		}
		if (!FD_ISSET(fd, &readSet)){
            continue;
		} 
		
        int retval = recv(sock,(char *) Buffer , sizeof(Buffer) - 1,0 );
		if(retval == 0) {
			offset++;
			if(offset>10) break;
		}
		else if(retval < 0) {
			int err = WSAGetLastError();
			TRACE("WSA code : %i", err);
			flag = 9;
			cweight = -1;
			break;
		}
		if(bReading==0) break;
        Buffer[retval] = 0;
        locProcessBytes((char *)Buffer, retval);

    }
	
    shutdown(sock,2);
    closesocket(sock);
    WSACleanup();
    //cweight = -4;

    return flag;
}
