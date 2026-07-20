// mycctv.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "mycctv.h"
#include "db_ado.h"
#include "GdiPlus.h"
#include <vector>

#if _MSC_VER > 1200
#else
    #ifndef ULONG_PTR
        #define ULONG_PTR unsigned long*
    #endif
#endif // _MSC_VER > 1200


#pragma comment (lib, "jcsdk.lib")
#pragma comment (lib, "DsSdk.lib")
#pragma comment (lib, "GdiPlus.lib")
#pragma comment (lib, "HCNetSDK.lib")
#pragma comment (lib, "iphlpapi.lib")
#pragma comment (lib, "PlayCtrl.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int jcId[10];
int iErrorCounter = 0;
ULONG_PTR m_gdiplusToken;
HINSTANCE g_hDSSDK; 

void funJCEventCallback(JCLink_t nLinkID, JCEventType etType, DWORD_PTR pData1, DWORD_PTR pData2, LPVOID pUserData)
{
	DWORD dwMsgID = 0;
    SYSTEMTIME gmt;

    GetLocalTime( &gmt );

    char file[1000];
    sprintf(file,"jcet%2.2u%2.2i%2.2i.log", gmt.wYear-2000,gmt.wMonth,gmt.wDay);
    FILE *ptr = fopen(file,"a");


    CString jmsg;
    switch(etType) {
	case JCET_Alarm:
		{
            jmsg = "Alarm";
			if(ptr) {
                jcId[0] = 1;
			}
		}
		break;
	case JCET_GetFileListOK://????????
		{
            jmsg = "GetFileListOK";
		}
		break;
		
	case JCET_GetFileListError://????????
		{
            jmsg = "GetFileListError";
		}
		break;

	case JCET_StreamReset://??????
		{
            jmsg = "StreamReset";
			JCSDK_EnableDecoder(nLinkID, FALSE);
            app->tell_resetsteam(nLinkID);
//            pJc->PostMessage(WM_RESETSTREAM);
		}
		break;
	}

    switch(etType)  {
	case JCET_ConnectOK://????
		dwMsgID = IDS_ConnectOK;
        jmsg = "ConnectOK";

		break;

	case JCET_UserAccessError: //??????
		dwMsgID = IDS_ConnectAccessError;
        jmsg = "ConnectAccessError";
		break;

	case JCET_NoChannel://???????
        dwMsgID = IDS_ConnectNoChannel;
        jmsg = "ConnectNoChannel";
		break;

	case JCET_ConTypeError://??????
		dwMsgID = IDS_ConnectTypeError;
        jmsg = "ConnectTypeError";
		break;

	case JCET_ConCountLimit://?????????
        dwMsgID = IDS_ConnectCountLimit;
        jmsg = "ConnectCountLimit";
		break;

	case JCET_ConTimeout://????
		dwMsgID = IDS_ConnectTimeout;
        jmsg = "ConnectTimeout";
		break;

	case JCET_DisconOK://??????
		dwMsgID = IDS_DisconnectOK;
        jmsg = "DisconnectOK";
		break;

	case JCET_ConAbout://??????
		dwMsgID = IDS_DisconnectError;
        jmsg = "DisconnectError";
		break;

	case JCET_ServiceStop://??????
		dwMsgID = IDS_ServerStop;
        jmsg = "ServerStop";
		break;

	default:
		return;
		break;
	}

    if(ptr) {
        fprintf(ptr,"%2.2i:%2.2i:%2.2i %3.3i id:%i %s %i %i %i\n",
            gmt.wHour,gmt.wMinute,gmt.wSecond,gmt.wMilliseconds,
            dwMsgID,jmsg, nLinkID,etType,dwMsgID);
        fclose(ptr);
    }
}

void funJCDataCallback(JCLink_t nLinkID, PJCStreamFrame pFrame, LPVOID pUserData)
{
/*	char acBuffer[32];
	sprintf(acBuffer, "Type:%d %dc %dt\n", 
		pFrame->sType,m_LastCallBackCounter, m_nTimer9Counter);
	OutputDebugStringA(acBuffer);

    m_LastCallBackCounter = m_nTimer9Counter; */
}

void funJCRawDataCallback(JCLink_t nLinkID, PJCRawFrame pFrame, LPVOID pUserData)
{
}

void funLanSearchCallback(PJCLanDeviceInfo pDevice)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMyCctvApp initialization


CMyCctvApp::CMyCctvApp()
{
    m_camera = "camera";
}


BOOL CMyCctvApp::OnInitializeJCSDK()
{
    if(JCSDK_InitSDK_Count) return FALSE;

    if(!JCSDK_InitSDK(-1)) {
        AfxMessageBox(IDS_InitSDKError);
		CString log;

        log.Format("Failed to initialize SDK!  JCSDK_InitSDK_Count %i\n",
            JCSDK_InitSDK_Count);

        OnLogsnapMsg(NULL,0,0, log);
        return FALSE;
		
	}
    else {
       JCSDK_RegisterCallback(
	   &funJCEventCallback,
       &funJCDataCallback,
       &funJCRawDataCallback,
       &funLanSearchCallback);
    }
    JCSDK_InitSDK_Count++;

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CMyCctvApp::OnInitializeHIKVISION()
{
    if(HIKVISION_InitSDK_Count) return FALSE;

    HIKVISION_InitSDK_Count++;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	g_hDSSDK = LoadLibrary("DsSdk.dll");
	NET_DVR_Init(); 
	NET_DVR_SDKLOCAL_CFG struSdkLocalCfg = {0};
	struSdkLocalCfg.byEnableAbilityParse = 1; //
	struSdkLocalCfg.byVoiceComMode = 0;   //0 use windows Api , 1 Use liabrary
    struSdkLocalCfg.byLoginWithSimXml = 1;
	NET_DVR_SetSDKLocalConfig(&struSdkLocalCfg);

    return TRUE;
}

FILE *CMyCctvApp::OpenCCTV_log()
{
    CString log;

    dbasedate(today);
    log.Format("c_%s.log", &today[4]);
    return fopen(log,"a");
}

BOOL CMyCctvApp::OnLogin(int no)
{
    if(no<0 || no>99) {
        ASSERT( FALSE );
        return FALSE;   //out of range
    }

    CString s;
	char szIp[1000];

	int i = no;
    Jd[i].m_no = no;
    s.Format("SELECT [IPADD] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].ip = SqlText(s);
	
    s.Format("SELECT [PORT] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].m_nPort = SqlInt(s);
    if(Jd[i].m_nPort==0) return 0;       //may be inactive
	
    s.Format("SELECT [NO] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].m_nChannel = SqlInt(s);
    if(Jd[i].m_nChannel==0) return 0;       //may be inactive
	
    s.Format("SELECT [video_clip] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].m_video_clip = SqlInt(s);
	if(AdoErrorNo==0x80040e14) {
        s.Format("Alter Table [%s] add [video_clip] int null",m_camera);
        runsql(s);
	}

    s.Format("SELECT [lpr] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].m_lpr = SqlInt(s);
	if(AdoErrorNo==0x80040e14) {
        s.Format("Alter Table [%s] add [lpr] bit null",m_camera);
        runsql(s);
	}
	
    s.Format("SELECT [srno] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].srno = SqlInt(s);
	if(AdoErrorNo==0x80040e14) {
        s.Format("Alter Table [%s] add [srno] int null",m_camera);
        runsql(s);
	}
	
	char txtVideoUser[200],txtVideoPassword[200];
    s.Format("SELECT [USER] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].user = SqlText(s);
    strcpy(txtVideoUser, Jd[i].user);// admin

    s.Format("SELECT [PASSWORD] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].pw = SqlText(s);
    strcpy(txtVideoPassword, Jd[i].pw);
	
    strcpy(szIp, Jd[i].ip);

    s.Format("SELECT [direction] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].m_direction  = SqlText(s);
	if(AdoErrorNo==0x80040e14) {
        s.Format("Alter Table [%s] add [direction] varchar(20)", m_camera);
        runsql(s);
	}

    s.Format("SELECT [inactive] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
    Jd[i].m_inactive  = SqlInt(s);
	if(AdoErrorNo==0x80040e14) {
        s.Format("Alter Table [%s] add [inactive] bit null",m_camera);
        runsql(s);
	}

    if(Jd[i].m_nPort == 8000) {
        Jd[i].brand = "hikvision";
    }
    else if(Jd[i].m_nPort == 9101) {
    }
    else {
        s.Format("SELECT [BRAND] FROM %s WHERE CAMERA = '%2.2i'",m_camera, no);
        Jd[i].brand = SqlText(s);
    }

	
    if(Jd[i].brand.CompareNoCase("hikvision")==0) {
        OnInitializeHIKVISION();

        NET_DVR_DEVICEINFO_V30 struDeviceInfo;
        memset(&struDeviceInfo, 0, sizeof(NET_DVR_DEVICEINFO_V30));

        NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
        NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
        struLoginInfo.bUseAsynLogin = false;
        struLoginInfo.wPort = Jd[i].m_nPort;

        memcpy(struLoginInfo.sDeviceAddress, Jd[i].ip, NET_DVR_DEV_ADDRESS_MAX_LEN);
        memcpy(struLoginInfo.sUserName, txtVideoUser, NAME_LEN);
        memcpy(struLoginInfo.sPassword, txtVideoPassword, NAME_LEN);

        Jd[i].m_nLinkID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
        AddCCTVLog(no,1,Jd[i].ID,Jd[i].m_nLinkID);
        if(Jd[i].m_nLinkID < 0){
			DWORD err_no = NET_DVR_GetLastError();

            CString log;
            log.Format("NET_DVR_Login_V40 camera %i(%s) fail, error code : %i", no,Jd[i].ip, err_no);
#ifdef _DEBUG
			 ErrorBox(log);
#endif //_DEBUG

            OnLogsnapMsg(NULL,0, no, log);
			return FALSE;
		}
        //2022-08-01 add HIK callback function
        int flag1 = OnHIK_callback(no);
        int flag2 = SetAlarmChannel(no);

        Jd[i].m_status = 1;
        return TRUE;

        NET_DVR_NETCFG_V30* pStruNetCfg = new  NET_DVR_NETCFG_V30;
        NET_DVR_DEVICECFG_V40* pStruDevCfg = new NET_DVR_DEVICECFG_V40;

        if (pStruNetCfg == NULL || pStruDevCfg == NULL)
        {
            return FALSE;
        }

        DWORD dwReturned = 0,err_code = 0;
        if (!NET_DVR_GetDVRConfig(Jd[i].m_nLinkID, NET_DVR_GET_NETCFG_V30, 0,pStruNetCfg, sizeof(NET_DVR_NETCFG_V30), &dwReturned))
        {
            err_code = NET_DVR_GetLastError();
        }

        if (!NET_DVR_GetDVRConfig(Jd[i].m_nLinkID,NET_DVR_GET_DEVICECFG_V40,0,pStruDevCfg,sizeof(NET_DVR_DEVICECFG_V40),&dwReturned))
		{
            err_code = NET_DVR_GetLastError();
		}


        return TRUE;
    }

    OnInitializeJCSDK();
    if(szIp[0]>='A' && szIp[0]<='Z') {
        Jd[i].m_nPort = 0;
    }
    Jd[i].m_nLinkID = JCSDK_Connect( szIp, Jd[i].m_nPort, Jd[i].m_nChannel, txtVideoUser, txtVideoPassword, FALSE, NULL);
    AddCCTVLog(no,1,Jd[i].ID,Jd[i].m_nLinkID);
	
    TRACE("ID:%i\n", Jd[i].m_nLinkID);
    if(Jd[i].m_nLinkID == -1)
    {
        CString log;
        log.Format("JCSDK_Connect, error code : %i", 1);
        OnLogsnapMsg(NULL,0, no, log);
        return FALSE;
    }
    else
    {
    }
    Jd[i].m_status = 1;
    return TRUE;
}

BOOL CMyCctvApp::OnDisconnect(int i,int Initialize,int restart,int firsttime)
{
	BOOL flag = FALSE;

    //2025-07-23 first time no need record
    if(firsttime==0) {
        FILE *ptr = log_file();
        if(ptr) {
            fprintf(ptr,"%s %s %i id:%i handle:%i\n",mysql_now_local() ,
               "disconnect", i ,Jd[i].m_nLinkID,Jd[i].m_lPlayHandle);
            fclose(ptr);
        }
    }

    if(i<0 || i>99) {
        ASSERT( FALSE );
        return flag;   //out of range
    }

    if(Jd[i].m_nLinkID>=0) {
        if( Jd[i].m_recording) {        //2024-01-21 Stop recording first
            OnRecord( i );
        }

        if(Jd[i].brand.CompareNoCase("hikvision")==0){
			//2022-08-01 add close HIK alarm handle
			//2024-01-24 restart no need set alarm again
            if(Jd[i].AlarmHandle>=0) {
				flag = NET_DVR_CloseAlarmChan(Jd[i].AlarmHandle);
			}
            if(Jd[i].m_bDecoding) {
				flag = NET_DVR_StopRealPlay(Jd[i].m_lPlayHandle);
            }
            flag = NET_DVR_Logout(Jd[i].m_nLinkID);
        }
        else {
            if(Jd[i].m_bPreviewing) {
				flag = JCSDK_SetVideoPreview(Jd[i].m_nLinkID, NULL, NULL);
			}
            if(Jd[i].m_bDecoding) {
                OnDisabledecode(i);
            }
            flag = JCSDK_Disconnect(Jd[i].m_nLinkID);
        }
        AddCCTVLog(i,9,Jd[i].ID,0);
    }
    if(!Initialize) return flag;

	Jd[i].ID = 0;
    Jd[i].m_nLinkID = -1;
    Jd[i].m_lPlayHandle = -1;
	Jd[i].m_oldHandle = -1;
    Jd[i].AlarmHandle = -1;
    Jd[i].m_no = 0;
    Jd[i].m_lpr = 0;
    Jd[i].m_recording  = FALSE;
    Jd[i].m_video_clip = FALSE;
    Jd[i].m_bPreviewing = FALSE;
    Jd[i].m_bDecoding = FALSE;
    Jd[i].m_GetPicComplete = FALSE;
    Jd[i].m_exitNeedRefresh = FALSE;
    Jd[i].m_status = 0;
    Jd[i].m_retryCounter = 0;
    Jd[i].m_previewOkCounter = 0;
    Jd[i].srno = 0;
    Jd[i].ip = "";
    Jd[i].brand = "";
    Jd[i].m_direction  = "";

	return flag;
}

void CMyCctvApp::OnResetPreview(CWnd *pdlg)
{
    for(int i=0; i<99; i++) {
        if(Jd[i].m_nLinkID == -1) continue;
        if(!Jd[i].m_bPreview) continue;
        OnDisconnect(i,1,0);

        OnPreview(i,pdlg);
	}
}

void CALLBACK fRealDataCallBack_HD(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize,void *pUser) 
{  
	return;

	long lChan = ((NET_DVR_PREVIEWINFO*)pBuffer)->lChannel;
	HWND playHWnd = ((NET_DVR_PREVIEWINFO*)pBuffer)->hPlayWnd;
	DWORD dRet;  

    switch (dwDataType)
	{  
	case NET_DVR_SYSHEAD:    //???  
		if (!PlayM4_GetPort(&lChan)) //????????????  
		{  
			break;  
		}  
		if(dwBufSize > 0)  
		{  
			if (!PlayM4_OpenStream(lChan,pBuffer,dwBufSize,1024*1024))  
			{  
				dRet=PlayM4_GetLastError(lChan);  
				break;  
			}  
			//???????? ??????  
/*			if (!PlayM4_SetDecCallBack(lChan,DecCBFun_HD))  
			{  
				dRet=PlayM4_GetLastError(lChan);  
				break;  
			}  */
			
			//???????? ?????  
			//if (!PlayM4_SetDecCallBackEx(lChan,DecCBFun,NULL,NULL))  
			//{  
			//  dRet=PlayM4_GetLastError(lChan);  
			//  break;  
			//}  
			
			//??????  
			if (!PlayM4_Play(lChan,playHWnd))  
			{  
				dRet=PlayM4_GetLastError(lChan);  
				break;  
			}  
			
			//??????, ????????  
			if (!PlayM4_PlaySound(lChan))  
			{  
				dRet=PlayM4_GetLastError(lChan);  
				break;  
			}         
		}  
		break;  
		
	case NET_DVR_STREAMDATA:   //????  
		if (dwBufSize > 0 && lChan != -1)  
		{  
			if (!PlayM4_InputData(lChan, pBuffer, dwBufSize))
			{
				break;
			} 
			
			//BOOL inData=PlayM4_InputData(lChan,pBuffer,dwBufSize);  
			//while (!inData)  
			//{  
			//        Sleep(10);  
			//        inData=PlayM4_InputData(lChan,pBuffer,dwBufSize);  
			//        //OutputDebugString(L"PlayM4_InputData failed \n");     
			//}  
		}  
		break;  
	default:
		if (dwBufSize > 0 && lChan != -1)  
		{  
			if (!PlayM4_InputData(lChan, pBuffer, dwBufSize))
			{
				break;
			} 
			
		}  
		break;
		
	}         
}  

void CMyCctvApp::OnPreview(int i,CWnd *pdlg,UINT ID,int exitNeedRefresh,int m_streamType)
{
    if(i<0 || i>99){
        ASSERT( FALSE );
        return;   //out of range
    }

    if(ID>0 && !exitNeedRefresh && ID!=999999) {
        Jd[i].ID = ID;
	}

    
	if(Jd[i].ip.GetLength()==0) {
         app->OnLogin(i);
    }

    if(Jd[i].m_inactive) {          //2023-03-02 ooi
        return;
    }
    if(Jd[i].m_bDecoding==0) {
         app->OnDecoder(i);
    }

    if(Jd[i].m_nLinkID == -1) return;


    UINT DISPLAY_ID = (ID == 0 ) ? Jd[i].ID : ID;

    Jd[i].m_exitNeedRefresh = exitNeedRefresh;
    if(!pdlg) {
        EndPreview(i);
		return;

    }
    CWnd *wnd;
    if(DISPLAY_ID==999999) {
        wnd = pdlg;
    }
    else {
        wnd = pdlg->GetDlgItem( DISPLAY_ID );
    }
    if(!wnd) return;

	HWND hw = wnd->GetSafeHwnd();
	CString s;

    if(Jd[i].brand.CompareNoCase("hikvision")==0){

        char szLan[128] = {0};
        char szTemp[256] = {0};
        NET_DVR_PREVIEWINFO struPreviewInfo = {0};

        struPreviewInfo.hPlayWnd =  hw;
        struPreviewInfo.lChannel = Jd[i].m_nChannel;
        struPreviewInfo.dwLinkMode = 0;

		//2020-07-30 sub stream reduct cpu usage
        //struPreviewInfo.dwStreamType = 1;  //stream type:0- main stream,1- sub stream,2- third stream

        //stream type:0- main stream,1- sub stream,2- third stream
        struPreviewInfo.dwStreamType = m_streamType;
		if (ID==999999){
			struPreviewInfo.dwStreamType = 0;  //stream type:0- main stream,1- sub stream,2- third stream
		}

        //start soft decode
        struPreviewInfo.byPreviewMode = 0;
        struPreviewInfo.bBlocked = 0;
        struPreviewInfo.bPassbackRecord = 0;
        struPreviewInfo.byProtoType = 0;
        long m_iNetDelayBufNum = 15;		//1 real 3 more real 6 balance 15 smooth
        struPreviewInfo.dwDisplayBufNum = m_iNetDelayBufNum;
        Jd[i].m_lPlayHandle = NET_DVR_RealPlay_V40(Jd[i].m_nLinkID, &struPreviewInfo, fRealDataCallBack_HD, NULL);
        AddCCTVLog(i,4,Jd[i].ID,Jd[i].m_lPlayHandle);
		s.Format("%i - %ld",Jd[i].m_no, Jd[i].m_lPlayHandle);
		pdlg->SetDlgItemText(1330+i,s);

		//https://open.hikvision.com/hardware/definitions/NET_DVR_RealPlay_V40.html
        if (Jd[i].m_lPlayHandle < 0)  {
			iErrorCounter++;
#ifdef _DEBUG
 	     	 DWORD errcode = NET_DVR_GetLastError();
             CString msg;
    	     msg.Format("NET_DVR_RealPlay_V40 Error code : %i", errcode);
			 ErrorBox(msg);
#endif //_DEBUG

        }
        else {
			Jd[i].m_bPreview = true;
			Jd[i].m_bPreviewing = true;
            Jd[i].m_previewOkCounter++;
        }
        return;
    }

	
	UINT dwMsgID = 0;
	RECT rtRect = {0};
    wnd->GetClientRect(&rtRect);
    wnd->ClientToScreen(&rtRect);

    if(JCSDK_SetVideoPreview(Jd[i].m_nLinkID, hw, &rtRect)) {
        dwMsgID = IDS_EnablePreviewOK;
        Jd[i].m_bPreview = true;
        Jd[i].m_bPreviewing = true;
        Jd[i].m_previewOkCounter++;
        AddCCTVLog(i,4,Jd[i].ID,0);
		s.Format("%i - %ld",Jd[i].m_no, Jd[i].m_nLinkID);
		pdlg->SetDlgItemText(1330+i,s);

    }
    else {
        FILE *ptr = OpenCCTV_log();
        if(ptr) {
            fprintf(ptr,"preview: %s c%i p%i r%i\n",
                TimeNowMs(1), i,
                Jd[i].m_previewOkCounter,
                Jd[i].m_retryCounter);
            fclose(ptr);
        }

        dwMsgID = IDS_EnablePreviewError;
        Jd[i].m_bPreview = false;
        Jd[i].m_bPreviewing = false;
        AddCCTVLog(i,4,Jd[i].ID,-1);
        if(Jd[i].m_previewOkCounter>0) {
            Sleep(50);
            OnDisconnect(i);
            Jd[i].m_retryCounter++;
            Sleep(50);
        }
	}
}

void CMyCctvApp::OnDecoder(int i)
{
    if(i<0 || i>99){
         ASSERT( FALSE );
         return;   //out of range
    }
    if(Jd[i].m_inactive) return;
    if(Jd[i].m_nLinkID == -1) return;
    AddCCTVLog(i,3,Jd[i].ID,0);
    if(Jd[i].brand.CompareNoCase("hikvision")==0) { //no need decoder
        Jd[i].m_bDecoding = true;
        return;
    }
	
	UINT dwMsgID = 0;
    for(int k=0; ; k++) {
        Sleep(10);
		int flag = JCSDK_EnableDecoder(Jd[i].m_nLinkID, TRUE);
        if(flag)
        {
            dwMsgID = IDS_EnableDecodeOK;
            Jd[i].m_bDecoding = true;
            break;
        }
        if(  k <300 ) continue;

        CString str,msg;
        msg.Format("Camera %i ip:%s channel:%i Decoder fail.", i, Jd[i].ip,Jd[i].m_nChannel);
        OnLogsnapMsg(NULL,0,i,msg);


        dwMsgID = IDS_EnableDecodeError;
        Jd[i].m_bDecoding = false;
        Jd[i].m_bPreviewing = false;
        Jd[i].m_nLinkID = -1;
        break;
    }
}

void CMyCctvApp::OnDisabledecode(int i)
{
    if(i<0 || i>99){
         ASSERT( FALSE );
         return;   //out of range
    }

	UINT dwMsgID = 0;
    AddCCTVLog(i,8,Jd[i].ID,0);
	if(Jd[i].brand.CompareNoCase("hikvision")==0){
		if(Jd[i].m_bDecoding) {
			OnDisabledecode(i);
		}
	}
	else {

		if(JCSDK_EnableDecoder(Jd[i].m_nLinkID, FALSE))
		{
			dwMsgID = IDS_DisableDecodeOK;
		}
		else
		{
			dwMsgID = IDS_DisableDecodeError;
		}
	}

    Jd[i].m_bDecoding = false;
    Jd[i].m_bPreviewing = false;
}


void CMyCctvApp::OnResetstream(WPARAM wParam, LPARAM lParam)
{
    for(int i=0; i<99; i++) {
        if(!Jd[i].m_bDecoding) continue;
        JCSDK_EnableDecoder(Jd[i].m_nLinkID, TRUE);
	}
}

//2022-05-20 process 0 all 1 snapshot only 2/3 convert bmp to jpg only 3. no delete bmp
BOOL CMyCctvApp::OnSnapshot(int i,const char *jpg_file,int quality,int PicSize,int process)
{
	CString msg;
	char photo[1000];
	CString bmp = jpg_file;

	if(i>100) {
		i = findIDC(i);
	}
	if(i<0 || i>100){
		msg.Format("Snapshot %s camera: %i out of range.", jpg_file, i);
		OnLogsnapMsg(NULL,0,i, msg);
        ASSERT( FALSE );
		return FALSE;
	}

    if(Jd[i].m_inactive) {      //2023-03-02
		return FALSE;
    }

	//2022-07-20
	if(Jd[i].m_bypassSnapshot) {
		msg.Format("Bypass snapshot %s camera: %i.", jpg_file, i);
		OnLogsnapMsg(NULL,0,i, msg);
		return TRUE;        //not error message may be disable this camera
	}

    if(Jd[i].m_nLinkID==-1) {
		msg.Format("Snapshot %s camera: %i invalid handle.", jpg_file, i);
		OnLogsnapMsg(NULL,0,i, msg);
		Jd[i].m_bypassSnapshot = 1; //no need check again
		Jd[i].m_errorCounter++;
		return FALSE;
	}


	bmp.MakeLower();
	bmp.Replace(".jpg",".bmp");
    if(process<2) {
        BOOL flagDelete = DeleteFile(jpg_file);

        Jd[i].m_GetPicComplete = FALSE;
        if(Jd[i].m_status>1) {   //2019-08-02
            JCSDK_EnableDecoder(Jd[i].m_nLinkID, TRUE);
            Jd[i].m_status = 1;
        }


        if(Jd[i].brand.CompareNoCase("hikvision")==0 && Jd[i].m_lpr==0) {

            char szLan[128] = {0};
            NET_DVR_JPEGPARA JpegPara;

            if(quality>75) {
                JpegPara.wPicQuality = 0; // 0 best, 1 better, 2 Normal
            }
            else if(quality>40 && quality<=75) {
                JpegPara.wPicQuality = 1; // 0 best, 1 better, 2 Normal
            }
            else {
                JpegPara.wPicQuality = 2; // 0 best, 1 better, 2 Normal
            }
            JpegPara.wPicSize = PicSize;  // 0 352x288,1 176x144, 2 704x576, 255 auto
			

            strcpy(photo,jpg_file);
            BOOL flagSnapshot = NET_DVR_CaptureJPEGPicture(Jd[i].m_nLinkID, Jd[i].m_nChannel, &JpegPara, photo);
            if(flagSnapshot) {
                Jd[i].m_GetPicComplete = TRUE;
                wait_file_created( photo , 100, 10);
                return TRUE;
            }
			else {
				//https://open.hikvision.com/hardware/definitions/NET_DVR_GetLastError.html
				DWORD noerr = NET_DVR_GetLastError();
				if(noerr==NET_DVR_NOSUPPORT) {
					msg.Format("HIK LPR camera %i not support JPG direct output.\n\nPlease change camera setting.", i);
					ErrorBox(msg);
				}
				else {
					msg.Format("Snapshot %s fail error no : %i", photo, noerr);
					OnLogsnapMsg(NULL,0,i, msg);
				}
			}
        }

        if (DeleteFile(bmp)==FALSE){
        }

        strcpy(photo,bmp);


        if(Jd[i].m_lpr==1) {

            BOOL flagSnapshot = NET_DVR_CapturePicture(Jd[i].m_lPlayHandle, photo);
            if(flagSnapshot==FALSE) {
                DWORD err_code = NET_DVR_GetLastError();
                msg.Format("Snapshot %s fail error no : %i", photo, err_code);
                OnLogsnapMsg(NULL,0,i, msg);
                Jd[i].m_errorCounter++;
                return FALSE;
            }
        }
        else {
            BOOL flagSnapshot = JCSDK_SaveBitmap(Jd[i].m_nLinkID, photo);
            if(flagSnapshot==FALSE) {
                OnLogsnapMsg(NULL,0,i,bmp);
                Jd[i].m_errorCounter++;
                return FALSE;
            }
        }
    }

    if(process==0 || process==2 || process==3) {

        if(Jd[i].m_GetPicComplete == FALSE) {
            strcpy(photo,bmp);
            BOOL flag = wait_file_created( photo , 100, 10);
            if(flag==FALSE){

				flag = wait_file_created( photo , 200, 10);
				if(flag==FALSE){
					msg.Format("Unable to create the photo %s in 300ms.", photo);
					OnLogsnapMsg(NULL,0,i, msg);
					Jd[i].m_errorCounter++;
					return FALSE;
				}
            }

            for(int ii=0; ii<3; ii++) {
                flag = Dib2Jpg( bmp, jpg_file, quality);
                if (flag==TRUE) break;

                Sleep(150);
                if(ii<2) continue;

                msg.Format("Unable convert %s to %s",bmp, photo);
                OnLogsnapMsg(NULL,0,i, msg);
                Jd[i].m_errorCounter++;
                return FALSE;
            }
            if(process==0 || process==2) {
                if (DeleteFile(bmp)==FALSE){
                }
            }
            Jd[i].m_GetPicComplete = TRUE;
        }
    }


    return TRUE;
}

BOOL CMyCctvApp::OnSwapCamera(CWnd *pdlg,int i,int j)
{
	if(i==j) return FALSE;
    if(Jd[i].m_nLinkID == -1) return FALSE;
    if(Jd[j].m_nLinkID == -1) return FALSE;
    if(Jd[i].m_inactive) return FALSE;
    if(Jd[j].m_inactive) return FALSE;

	BOOL flag;
    if(Jd[i].brand.CompareNoCase("hikvision")==0){

        flag = NET_DVR_StopRealPlay(Jd[i].m_lPlayHandle);
		if(!flag){
			DWORD no = NET_DVR_GetLastError();
		}
    }
    else {
        flag = JCSDK_SetVideoPreview(Jd[i].m_nLinkID, NULL, NULL);
    }

    if(Jd[j].brand.CompareNoCase("hikvision")==0){

        flag = NET_DVR_StopRealPlay(Jd[j].m_lPlayHandle);
    }
    else {
        flag = JCSDK_SetVideoPreview(Jd[j].m_nLinkID, NULL, NULL);
    }

    //OnDisabledecode(i);
    //OnDisabledecode(j);

    OnSwapJd( i, j );

    OnPreview(i,pdlg);
    OnPreview(j,pdlg);

    return TRUE;
}

BOOL CMyCctvApp::OnRecord(int i, const char *filename)
{
    if(i>100) {
        i = findIDC(i);
    }
    if(i<0 || i>100) return FALSE;

    if(Jd[i].m_inactive) return FALSE;
    if(Jd[i].m_nLinkID==-1) return FALSE;
        
	char cFilename[1000];
	strcpy(cFilename, filename);
	int len = strlen(cFilename);

    DeleteFile(cFilename);          //2024-01-23

	if(Jd[i].brand.CompareNoCase("hikvision")==0){
		if(len==0) {
            Jd[i].m_recording = FALSE;
//            if (!NET_DVR_StopSaveRealData(Jd[i].m_nLinkID))
            if (!NET_DVR_StopSaveRealData(Jd[i].m_lPlayHandle))
			{
				return FALSE;
			}
		}
//        if (!NET_DVR_SaveRealData(Jd[i].m_nLinkID, cFilename))

        if (!NET_DVR_SaveRealData(Jd[i].m_lPlayHandle, cFilename))
		{
			return FALSE;
		}

    }
    else {
        if(len==0) {
            Jd[i].m_recording = FALSE;
            if(!JCSDK_StopRec(Jd[i].m_nLinkID)) return FALSE;
        }
        else {
             if(!JCSDK_StartRec(Jd[i].m_nLinkID, cFilename)) return FALSE;
        }
    }
    Jd[i].m_recording = TRUE;
	return TRUE;

}

int CMyCctvApp::ExitInstance()
{
    for(int i=0; i<99; i++) {
        if(Jd[i].m_inactive) continue;
        if(Jd[i].m_nLinkID == -1) continue;
        OnDisconnect(i);
	}
    if(HIKVISION_InitSDK_Count) {
         NET_DVR_Cleanup();
    }
    return CMyApp::ExitInstance();
}

int CMyCctvApp::EndPreview(int i)
{
    if(Jd[i].m_nLinkID<0) return -1;

    int handle;
	BOOL flag = 0;
    if(Jd[i].brand.CompareNoCase("hikvision")==0){
        flag = NET_DVR_StopRealPlay(Jd[i].m_lPlayHandle);
        handle = Jd[i].m_lPlayHandle;

    }
    else {
        flag = JCSDK_SetVideoPreview(Jd[i].m_nLinkID, NULL, NULL);
        app->Jd[i].m_exitNeedRefresh = TRUE;
        handle = Jd[i].m_nLinkID;
    }
    AddCCTVLog(i,6,Jd[i].ID, handle);
    Sleep(100);
	return flag;
}

void CMyCctvApp::AddCCTVLog(int i,int sq,int id, int status)
{
    if(!IsAdoTableExisted("logcctv")) return;

    CString str;

    str.Format("INSERT INTO logcctv ([camera],sq,id,[type],[NO],[status]) VALUES('%2.2i',%ld,%ld,'%3.3s',%ld,%ld)",
        i, sq, id, Jd[i].brand, Jd[i].m_no,status);
	runsql(str);
}

/**********************************************************************************************************
* option bit 0 small plateno 1 full picture   2 mp4
***********************************************************************************************************/
BOOL CMyCctvApp::OnGetPlateNo(int id,CString &plateno,int option,const char *picFolder2)
{
	char szLan[128] = {0};
	CString csTemp;

    char picFolder[1024];
    strcpy(picFolder,picFolder2);
    if(picFolder[0]==0) {
         strcpy(picFolder,app->picFolder);
    }

    m_lpr_plate_picture = m_lpr_vehicle_picture = plateno = "";


	char szFileName[256] = {0};
	char szSubPicFilePath[256] = {0};  
	char szSubPlateFilePath[256] = {0};
	char szPicDataTime[256] = {0};
	char szPlateDataTime[256] = {0};
	char szPlateFileName[256] = {0};

	app->m_lpr_camera_no = 0;
    if(id>100) {
        id = findIDC(id);
    }
    if(id<0 || id>100) return FALSE;
    int lpr = app->Jd[id].m_lpr;
	if(lpr==0) {
		return FALSE;
	}

	NET_DVR_MANUALSNAP struManualSnap = {0};
	struManualSnap.byOSDEnable = TRUE;
	struManualSnap.byChannel = 1;

	NET_DVR_PLATE_RESULT struResult = {0};
	
	std::vector<BYTE> buffer1(2 * 1024 * 1024, 0);
	std::vector<BYTE> buffer2(1024 * 1024, 0);
	
	struResult.pBuffer1 = &buffer1[0];
	struResult.pBuffer2 = &buffer2[0];


    int m_nLinkID = app->Jd[id].m_nLinkID;
	if (!NET_DVR_ManualSnap(m_nLinkID, &struManualSnap, &struResult))
    {
		long pErrorNo = NET_DVR_GetLastError();
        CString msg = NET_DVR_GetErrorMsg(&pErrorNo);
//		g_pMainDlg->AddLog(m_iDevIndex, OPERATION_FAIL_T, "NET_DVR_ManualSnap");
        return FALSE;
    }  
    else
	{
		app->m_lpr_camera_no = id;

		//通过时间
        csTemp.Format("%s", struResult.byAbsTime);
        
		//车辆种类
		if (0 != struResult.byVehicleType)
		{
			switch (struResult.byVehicleType)
			{
			case 0:
				g_StringLanType(szLan, "未知", "UnKnow");		
				csTemp.Format("%s", szLan);
				break;
			case 1:
				g_StringLanType(szLan, "客车", "Passenger Carriage");	
				csTemp.Format("%s", szLan);
				break;
			case 2:
				g_StringLanType(szLan, "货车", "Freight Car");	
				csTemp.Format("%s", szLan);
				break;
			case 3:
				g_StringLanType(szLan, "轿车", "Car");
				csTemp.Format("%s", szLan);
				break;
			case 4:
				g_StringLanType(szLan, "面包车", "Microbus");
				csTemp.Format("%s", szLan);
				break;
			case 5:
				g_StringLanType(szLan, "小货车", "Little Freight Car");
				csTemp.Format("%s", szLan);
				break;
			case 6:
				g_StringLanType(szLan, "行人", "Passer");	
				csTemp.Format("%s", szLan);
				break;
			case 7:
				g_StringLanType(szLan, "二轮车", "Two-wheeler");	
				csTemp.Format("%s", szLan);
				break;
			case 8:
				g_StringLanType(szLan, "三轮车", "Three-wheeler");	
				csTemp.Format("%s", szLan);
				break;
			default:
				g_StringLanType(szLan, "未知", "UnKnow");		
				csTemp.Format("%s", szLan);
				break;
			}
		}
		else
		{
			//车辆类型
			switch (struResult.struVehicleInfo.byVehicleType)
			{
			case 0:
				g_StringLanType(szLan, "其他车辆", "Other Vehicles");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("其他车辆");
				break;
			case 1:
				g_StringLanType(szLan, "小型车", "Small Cars");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("小型车");
				break;
			case 2:
				g_StringLanType(szLan, "大型车", "Oversize Vehicle");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("大型车");
				break;
			case 3:
				g_StringLanType(szLan, "行人", "Passer");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("行人");
				break;
			case 4:
				g_StringLanType(szLan, "二轮车", "Two-wheeler");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("二轮车");
				break;
			case 5:
				g_StringLanType(szLan, "三轮车", "Three-wheeler");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("三轮车");
				break;
			default:
				g_StringLanType(szLan, "未知", "UnKnow");	
				csTemp.Format("%s", szLan);
				//pDatabaseParam->m_csVehicleType.Format("未知");
				break;
			}
		}

		//车牌颜色
		switch(struResult.struPlateInfo.byColor)
		{
		case VCA_BLUE_PLATE:
			g_StringLanType(szLan, "蓝色", "Blue");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("蓝色");
			break;
			
		case VCA_YELLOW_PLATE:
			g_StringLanType(szLan, "黄色", "Yellow");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("黄色");
			break;
			
		case VCA_WHITE_PLATE:
			g_StringLanType(szLan, "白色", "White");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("白色");
			break;
			
		case VCA_BLACK_PLATE:
			g_StringLanType(szLan, "黑色", "Black");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("黑色");
			break;
			
		default:
			g_StringLanType(szLan, "未知", "UnKnow");	
			csTemp.Format("%s", szLan);
			break;
		} 

		//车辆序号
        int m_iVehicleIndex = struResult.struVehicleInfo.dwIndex;

		//车辆颜色

		//牌照信息
        csTemp.Format("%s", struResult.struPlateInfo.sLicense);
		if(csTemp.CompareNoCase("unknown")==0) {
		}
		else {
			char szTmp[200] = {0};
			strcpy(szTmp, csTemp);
			strcat(szTmp, "  ");
			CheckVehicleNo(szTmp);
			trim(szTmp);
			plateno = szTmp;
		}


		//车牌类型
        switch(struResult.struPlateInfo.byPlateType)
        {
        case VCA_STANDARD92_PLATE:
			g_StringLanType(szLan, "标准民用车与军车", "civil and military ");	
			csTemp.Format("%s", szLan);
            //csTemp.Format("标准民用车与军车");
            break;
            
        case VCA_STANDARD02_PLATE:
			g_StringLanType(szLan, "02式民用车牌", "02 type of civil license");	
			csTemp.Format("%s", szLan);
            //csTemp.Format("02式民用车牌");
            break;
            
        case VCA_WJPOLICE_PLATE:
			g_StringLanType(szLan, "武警车", "Police Car");	
			csTemp.Format("%s", szLan);
            //csTemp.Format("武警车");
            break;
            
        case VCA_JINGCHE_PLATE:
			g_StringLanType(szLan, "警车", "Police Wagon");	
			csTemp.Format("%s", szLan);
            //csTemp.Format("警车");
            break;
            
        case STANDARD92_BACK_PLATE:
			g_StringLanType(szLan, "民用车双行尾牌", "Double license");	
			csTemp.Format("%s", szLan);
            //csTemp.Format("民用车双行尾牌");
            break;
			
		case VCA_SHIGUAN_PLATE:
			g_StringLanType(szLan, "使馆车牌", "Embassy license");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("使馆车牌");
			break;
			
		case VCA_NONGYONG_PLATE:
			g_StringLanType(szLan, "农用车牌", "Agricultural license");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("农用车牌");
			break;
		case VCA_MOTO_PLATE:
			g_StringLanType(szLan, "摩托车车牌", "Motorcycle license");	
			csTemp.Format("%s", szLan);
			//csTemp.Format("摩托车车牌");
			break;
        case NEW_ENERGY_PLATE:
            g_StringLanType(szLan, "新能源车车牌", "new energy license");	
            csTemp.Format("%s", szLan);
			break;
            
        default:
			g_StringLanType(szLan, "未知", "UnKnow");	
			csTemp.Format("%s", szLan);
            //csTemp.Format("未知");
            break;
        }       

        //速度
        csTemp.Format("%dkm/h", struResult.struVehicleInfo.wSpeed);

		//车牌亮度
		csTemp.Format("%d", struResult.struPlateInfo.byBright);
		
		//车牌号码
       	csTemp.Format("%s", struResult.struPlateInfo.sLicense);

		//抓拍类型
		CString m_csTrafficLight;
		switch(struResult.byTrafficLight)
		{
		case 0:
			g_StringLanType(szLan, "非红绿灯时抓拍", "Non light Capture");	
			m_csTrafficLight.Format("%s", szLan);
			//pDatabaseParam->m_csSnapMode.Format("非红绿灯时抓拍");
			break;
		case 1:
			g_StringLanType(szLan, "绿灯时抓拍", "Capture in green light");	
			m_csTrafficLight.Format("%s", szLan);
			//pDatabaseParam->m_csSnapMode.Format("绿灯时抓拍");
			break;
		case 2:
			g_StringLanType(szLan, "红灯时抓拍", "Capture in red light");	
			m_csTrafficLight.Format("%s", szLan);
			//pDatabaseParam->m_csSnapMode.Format("红灯时抓拍");
			break;
		default:
			g_StringLanType(szLan, "未知", "UnKnow");	
			m_csTrafficLight.Format("%s", szLan);
			//pDatabaseParam->m_csSnapMode.Format("未知");
			break;
        }

		//车道号
		g_StringLanType(szLan, "车道号", "Lane");	
		csTemp.Format("%s:%d", szLan, struResult.byDriveChan);                  

		//违规类型
		switch(struResult.struVehicleInfo.byIllegalType)
		{
		case 0:
			g_StringLanType(szLan, "正常", "Normal");
			csTemp.Format("%s\n", szLan);
			g_StringLanType(szLan, "卡口", "Intelligent Monitoring and Recording System");
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 1:
			g_StringLanType(szLan, "低速", "Low Speed");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 2:
			g_StringLanType(szLan, "超速", "Over Speed");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 3:
			g_StringLanType(szLan, "逆行", "Go in the Wrong Direction");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 4:
			g_StringLanType(szLan, "违反交通灯指示", "Run the Red Light");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 5:
			g_StringLanType(szLan, "压车道线", "Over Lane Line");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 6:
			g_StringLanType(szLan, "不按导向", "Travel against Guiding");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 7:
			g_StringLanType(szLan, "路口滞留", "Intersection retention");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 8:
			g_StringLanType(szLan, "机占非", "Motor Vehicle Running on Non-Motor Vehicle Lane");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 9:
			g_StringLanType(szLan, "违法变道", "Illegal Lane Change");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 10:
			g_StringLanType(szLan, "占用专用车道", "Take up dedicated lane");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		case 11:
			g_StringLanType(szLan, "闯禁令", "Violation of Prohibition sign");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		default:
			g_StringLanType(szLan, "未知", "Unknown");
			csTemp.Format("%s\n", szLan);
			sprintf(szSubPicFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			sprintf(szSubPlateFilePath, "%s\\%s", g_struLocalParam.chPictureSavePath, szLan);
			break;
		}

		//车标
		//g_VehicleLogo(struResult.struVehicleInfo.byVehicleLogoRecog, szLan);
		csTemp.Format("%s\n", szLan);

		//车辆图片
		if (struResult.dwPicLen != 0 && struResult.byResultType == 1 && option & 2){
			
            sprintf(szFileName, "%s\\lpr%2.2i.jpg",picFolder, id);

			m_lpr_vehicle_picture = szFileName;
			
            HANDLE lHandle = app->SaveLPR(id,1,szFileName,struResult.pBuffer1, struResult.dwPicLen);
            if (lHandle == INVALID_HANDLE_VALUE)
            {
//                TRACE("create file failed! Error code:%d", WSAGetLastError());
                return 2;
            }	

		}

		//车牌图片
		if (struResult.dwPicPlateLen != 0 && struResult.byResultType == 1 && option & 1) {
			g_StringLanType(szLan, "车牌", "License plate");
			sprintf(szPlateFileName, "%s\\%s%s.jpg", szPlateDataTime/*szSubPlateFilePath*/, szLan, struResult.byAbsTime);                    
			if (struResult.dwPicPlateLen != 0 && struResult.byResultType == 1)
			{
                sprintf(szPlateFileName, "%s\\plate%2.2i.jpg",picFolder, id);

                m_lpr_plate_picture = szPlateFileName;

                HANDLE lHandle = app->SaveLPR(id,2,szPlateFileName,struResult.pBuffer2, struResult.dwPicPlateLen);
				if (lHandle == INVALID_HANDLE_VALUE)
				{
//					TRACE("create file failed! Error code:%d", WSAGetLastError());
					return 2;
				}                
			}
		}

		 //视频 file
		if (struResult.byResultType == 0 && option & 4)  {
            //视频 file
            sprintf(szFileName, "%s\\%2.2i.mp4", picFolder, id);
            
            HANDLE lHandle = app->SaveLPR(id,3,szFileName,struResult.pBuffer1, struResult.dwVideoLen);
            if (lHandle == INVALID_HANDLE_VALUE)
            {
//                TRACE("create file failed! Error code:%d", WSAGetLastError());
                return 2;
            }
        }
	}

	plateno.TrimRight();

	//2022-06-22 if plate without digi consider fail
    if( !IsStringContainDigi(plateno) ) {
		plateno = "";
		return FALSE;
    }


	int len = plateno.GetLength();

    if(len>3) {
         app->LprLog(plateno, id , szFileName,0);
    }
    return ( len> 3);
}

void CMyCctvApp::g_StringLanType(char *szLan, char *msg, char *plate)
{
	
}		


BOOL CMyCctvApp::CreateNewTable()
{
	if(DBType!=DBDataType_MSSQL) return TRUE;             //2021-01-26 other database no need create new tables
    BOOL flag = CMyApp::CreateNewTable();

    CString s;

    if( !IsAdoTableExisted("LOG_SNAPSHOT") ) {
         s = "CREATE TABLE [dbo].[LOG_SNAPSHOT]([TIME] [datetime],[TICKET] [varchar](20),  [CMD] [varchar](2500),[user] [varchar](10),[computer] [varchar](50),[program] [varchar](50),[JPGKEY] [int],[PKKEY] [int] IDENTITY(1,1) NOT NULL,";
         s += "CONSTRAINT [PK_LOG_SNAPSHOT] PRIMARY KEY CLUSTERED ([PKKEY] ASC) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]) ON [PRIMARY]";
         RunSql(s);

         s = "ALTER TABLE [dbo].[LOG_SNAPSHOT] ADD CONSTRAINT [DF_LOG_SNAPSHOT_TIME]  DEFAULT (getdate()) FOR [TIME] ";
         RunSql(s);
    }

    if( !IsAdoTableExisted("logJpg2Db") ) {
         s = "CREATE TABLE [dbo].[logJpg2Db]([TIME] [datetime] NULL CONSTRAINT [DF_logJpg2Db_TIME]  DEFAULT (getdate()),";
         s += "[JPGKEY] [int], [NO] [int], [imagesize] [int],[len] [int],";
         s += "[PKKEY] [int] IDENTITY(1,1) NOT NULL,";
         s += "CONSTRAINT [PK_logJpg2Db] PRIMARY KEY CLUSTERED ([PKKEY] ASC) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]) ON [PRIMARY]";

         RunSql(s);
    }

    return flag;
}

int CMyCctvApp::SetAlarmChannel(int iCamera)
{
//	 if(Jd[iCamera].AlarmHandle>=0) return 1;
	 if(Jd[iCamera].m_lpr==0) return 0;

     memset(&m_struSetupAlarmParamV50, 0, sizeof(m_struSetupAlarmParamV50));
     memset(m_pData, 0, sizeof(m_pData));

     //设置报警默认参数
     m_struSetupAlarmParamV50.dwSize = sizeof(m_struSetupAlarmParamV50);
     m_struSetupAlarmParamV50.byAlarmInfoType = 1;
     m_struSetupAlarmParamV50.byFaceAlarmDetection = 1;
     m_struSetupAlarmParamV50.byRetAlarmTypeV40 = TRUE;
     m_struSetupAlarmParamV50.byRetDevInfoVersion = TRUE;  //CVR上传报警信息类型(仅对接CVR时有效)
     m_struSetupAlarmParamV50.byRetVQDAlarmType = TRUE; //Prefer VQD Alarm type of NET_DVR_VQD_ALARM
     m_struSetupAlarmParamV50.bySupport = 0;
     m_struSetupAlarmParamV50.bySupport |= (1 << 2);


     int id = app->Jd[iCamera].m_nLinkID;

	 int lFortifyHandle = -1;
	 for(int i=1; i<=10; i++ ) {
		 lFortifyHandle = NET_DVR_SetupAlarmChan_V50(id, &m_struSetupAlarmParamV50, NULL, strlen(m_pData));
		 if(lFortifyHandle<0) {
			 DWORD errcode = NET_DVR_GetLastError();

	#ifdef _DEBUG
			 CString msg;
			 msg.Format("NET_DVR_SetupAlarmChan_V50 %i(%s) Err:%i (%i/10)\n",
				 iCamera,Jd[iCamera].ip, errcode, i);
			 TRACE(msg);
	#endif //_DEBUG
             //Sleep(2000);
             //continue;        //waste time so removed
		 }
		
		 app->Jd[iCamera].AlarmHandle = lFortifyHandle;
		 break;
	 }

     return lFortifyHandle;
}

/*********************************************************
Function:	g_ExceptionCallBack
Desc:		exception callback function
Input:	dwType: message type;lUserID: log-in device ID;lHandle, handle of connected channel ; pUser, user data;
Output:	
Return:	
**********************************************************/
void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser)
{
	UNREFERENCED_PARAMETER(pUser);
	int i = 0, j=0;
	int iDeviceIndex = 0;
	CString sTemp;
	//	char szTmpBuf[256] = {0};
	
	
	switch (dwType) 
	{
    case EXCEPTION_RELOGIN:
        app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "EXCEPTION_RELOGIN");
        break;
    case RELOGIN_SUCCESS:
        app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "RELOGIN_SUCCESS");
        break;
	case EXCEPTION_EXCHANGE://the device is off line
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_EXCHANGE");
		break;
    case RESUME_EXCHANGE://the device is on line
        app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "RESUME_EXCHANGE");
		break;
	case EXCEPTION_AUDIOEXCHANGE:	//network exception while voice talk
		
		break;
	case EXCEPTION_ALARM:			//network exception while uploading alarm
		break;
	case EXCEPTION_PREVIEW:			// exception while preview
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "PREVIEW dev[%d]exception[%d]", iDeviceIndex, dwType);
		break;
	case EXCEPTION_SERIAL:			//exception while connecting in a transparent channel mode
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_SERIAL dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_RECONNECT:		//reconnect while preview	
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "preview RECONNECT dev[%d]", iDeviceIndex);
		break;
	case PREVIEW_RECONNECTSUCCESS:		//reconnect successfully while preview				
		app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "PREVIEW_RECONNECTSUCCESS dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_ALARMRECONNECT://reconnect alarm guard channel
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "ALARMRECONNECT dev[%d]", iDeviceIndex);
		break;
	case ALARM_RECONNECTSUCCESS://reconnect alarm guard channel successfully
		app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "ALARM_RECONNECTSUCCESS dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_SERIALRECONNECT://reconnect transparent channel
		app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "SERIALRECONNECT dev[%d]exception[0x%x]", iDeviceIndex, dwType);
		break;
	case EXCEPTION_PLAYBACK:
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_PLAYBACK dev[%d] exception[0x%x] error[%d]", iDeviceIndex, dwType, NET_DVR_GetLastError());
		break;
	case EXCEPTION_DISKFMT:
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_DISKFMT dev[%d]DISKFMT exception[0x%x]", iDeviceIndex, dwType);
		break;
    case EXCEPTION_PASSIVEDECODE:
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_PASSIVEDECODE dev[%d]PASSIVEDECODE exception[0x%x]", iDeviceIndex, dwType);		
        //NET_DVR_MatrixStopPassiveDecode(lHandle);
		
		break;
	case SERIAL_RECONNECTSUCCESS:
		app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "SERIAL_RECONNECTSUCCESS dev[%d]", iDeviceIndex);
		break;
	case EXCEPTION_LOST_ALARM:
		app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "EXCEPTION_LOST_ALARM");
		break;
	case EXCEPTION_MAX_ALARM_INFO:
		app->AddLog(iDeviceIndex, OPERATION_SUCC_T, "EXCEPTION_MAX_ALARM_INFO");
		break;
    case EXCEPTION_PASSIVEDECODE_RECONNNECT:
        app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_PASSIVEDECODE_RECONNNECT");
		break;
    case EXCEPTION_VIDEO_DOWNLOAD: // [add] by yangzheng 2019/11/09 ??????????
        app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "EXCEPTION_VIDEO_DOWNLOAD");
        break;
	default:
		app->AddLog(iDeviceIndex, OPERATION_FAIL_T, "else exception dev[%d]exception[0x%x]", iDeviceIndex, dwType);
		break;
	}
	return;
}


extern void CALLBACK MessageCallback2(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser);

BOOL CMyCctvApp::OnHIK_callback(int id)
{
    int m_nLinkID = app->Jd[id].m_nLinkID;

    int flag = NET_DVR_SetLogToFile(g_struLocalParam.nLogLevel, "c:\\backup\\", g_struLocalParam.bCycleWriteLog);
    flag = NET_DVR_SetExceptionCallBack_V30(WM_NULL/*WM_MYCOMMAND*/, NULL/*this->m_hWnd*/, g_ExceptionCallBack, NULL);
    flag = NET_DVR_SetReconnect(5000, g_struLocalParam.bReconnect);
    flag = NET_DVR_SetDVRMessageCallBack_V50(0, MessageCallback2, NULL);

    return flag;
}

/*********************************************************
Function:	AddLog
Desc:		add local log
Input:	iLogType, log type, 0-alam, 1-operate log, 2-debug info; csLogInfo log info
Output:	
Return:	
**********************************************************/
void CMyCctvApp::AddLog(int iDeviceIndex, int iLogType, const char* format, ...)
{	
	if(app->m_SystemExitNow) return;

	int iLen = 0;
	char szTime[64] = {0};
	char szLogType[32] = "FAIL";
	char szLogInfo[1024*5] = {0};
	char szDevInfo[256] = {0};
	char szErrInfo[256] = {0};
	char szLog[1024*5] = {0};
	va_list arglist;
	va_start(arglist,format);
	vsprintf(szLogInfo,format,arglist); 
	va_end(arglist);
	
	
	
	switch (iLogType)
	{
	case OPERATION_SUCC_T:
	case PLAY_SUCC_T:
		sprintf(szErrInfo, "");
		sprintf(szLogType, "SUCC");
		break;
	case PLAY_FAIL_T:
		sprintf(szErrInfo, "PLAY_M4 Eorror!!!");	
		break;
	case OPERATION_FAIL_T:
	default:
        sprintf(szErrInfo, "err[%d:%s]", NET_DVR_GetLastError(), NET_DVR_GetErrorMsg());
		break;
	}

}

BOOL CMyCctvApp::OnHIK_SetTime(int id, int year, int month, int day, int hour, int minute, int second)
{
    int m_HKLoginID = app->Jd[id].m_nLinkID;

	NET_DVR_PICCFG_V40 dvrPicV40 = {0};
	long dwErr,nDvrStartChan=1;
	unsigned long dwReturn;

	if( !NET_DVR_GetDVRConfig(m_HKLoginID, NET_DVR_GET_PICCFG_V40, nDvrStartChan, &dvrPicV40, sizeof(dvrPicV40), &dwReturn) ) {
		dwErr = NET_DVR_GetLastError();
	}

	NET_DVR_TIME dvrTime = {0};
	dvrTime.dwYear = year;
	dvrTime.dwMonth = month;
	dvrTime.dwDay = day;
	dvrTime.dwHour = hour;
	dvrTime.dwMinute = minute;
	dvrTime.dwSecond = second;
	BOOL flag = NET_DVR_SetDVRConfig(m_HKLoginID, NET_DVR_SET_TIMECFG, 0, &dvrTime, sizeof(dvrTime));
	if( !flag ) {
		dwErr = NET_DVR_GetLastError();
	}
	return flag;
}
