// AcsWeighbridgeApi.cpp - AllcosmosSystem weighbridge API client.
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#include "stdafx.h"
#include "menu.h"
#include "AcsWeighbridgeApi.h"
#include <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static DB AcsApiRequestLog;
static DB AcsApiResponseLog;

static CString ProfileText(const char* szField)
{
    int fld = GetDbFieldNo(&profile, szField);
    CString s = (fld > 0) ? profile.dat[fld] : "";
    s.TrimRight();
    return s;
}

BOOL IsAcsApiConfigured()
{
    return !ProfileText("API_URL").IsEmpty();
}

static CString JsonEscape(const CString& sIn)
{
    CString s = sIn;
    s.Replace("\\", "\\\\");
    s.Replace("\"", "\\\"");
    s.Replace("\r", "");
    s.Replace("\n", "\\n");
    s.Replace("\t", " ");
    return s;
}

static void AppendJsonRaw(CString& sBody, const char* szKey, const CString& sRawValue)
{
    if (!sBody.IsEmpty()) sBody += ",";
    CString s;
    s.Format("\"%s\":%s", szKey, sRawValue);
    sBody += s;
}

void AcsJsonString(CString& sBody, const char* szKey, const CString& sValue)
{
    AppendJsonRaw(sBody, szKey, "\"" + JsonEscape(sValue) + "\"");
}

// At most 3 decimals, no trailing zeros (API rule).
void AcsJsonNumber(CString& sBody, const char* szKey, double dValue)
{
    CString s;
    s.Format("%.3f", dValue);
    s.TrimRight('0');
    s.TrimRight('.');
    AppendJsonRaw(sBody, szKey, s);
}

static CString Base64(const CString& sIn)
{
    static const char szTab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char* p = (const unsigned char*)(LPCTSTR)sIn;
    int n = sIn.GetLength();
    CString sOut;
    for (int i = 0; i < n; i += 3) {
        int b = p[i] << 16;
        if (i + 1 < n) b |= p[i + 1] << 8;
        if (i + 2 < n) b |= p[i + 2];
        sOut += szTab[(b >> 18) & 63];
        sOut += szTab[(b >> 12) & 63];
        sOut += (i + 1 < n) ? szTab[(b >> 6) & 63] : '=';
        sOut += (i + 2 < n) ? szTab[b & 63] : '=';
    }
    return sOut;
}

// Position just after `"key":` and any blanks, or -1.
static int FindJsonValueStart(const CString& sBody, const char* szKey)
{
    CString sNeedle;
    sNeedle.Format("\"%s\"", szKey);
    int nPos = sBody.Find(sNeedle);
    if (nPos < 0) return -1;
    nPos += sNeedle.GetLength();
    int nLen = sBody.GetLength();
    while (nPos < nLen && isspace((unsigned char)sBody[nPos])) nPos++;
    if (nPos >= nLen || sBody[nPos] != ':') return -1;
    nPos++;
    while (nPos < nLen && isspace((unsigned char)sBody[nPos])) nPos++;
    return nPos;
}

static BOOL FindJsonTrue(const CString& sBody, const char* szKey)
{
    int nPos = FindJsonValueStart(sBody, szKey);
    return nPos >= 0 && sBody.Mid(nPos, 4) == "true";
}

static CString FindJsonString(const CString& sBody, const char* szKey)
{
    int nPos = FindJsonValueStart(sBody, szKey);
    if (nPos < 0 || sBody[nPos] != '"') return "";
    CString sOut;
    int nLen = sBody.GetLength();
    for (nPos++; nPos < nLen && sBody[nPos] != '"'; nPos++) {
        if (sBody[nPos] == '\\' && nPos + 1 < nLen) {
            nPos++;
            char c = sBody[nPos];
            sOut += (c == 'n') ? ' ' : c;
        }
        else {
            sOut += sBody[nPos];
        }
    }
    return sOut;
}

// Log tables are opened on first use, and only when sql/003 has created them.
static BOOL OpenLogTables()
{
    static int nState = 0;      // 0 = not tried, 1 = open, -1 = tables missing
    if (nState == 0) {
        CString s = SqlText("select count(*) from sys.tables where name in ('AcsApiRequestLog','AcsApiResponseLog')");
        nState = (atoi(s) == 2 && tado(&AcsApiRequestLog, "AcsApiRequestLog") && tado(&AcsApiResponseLog, "AcsApiResponseLog")) ? 1 : -1;
    }
    return nState == 1;
}

static long LogRequest(const CString& sTicketNo, const char* szEndpoint, const CString& sUrl, const CString& sJson)
{
    if (!OpenLogTables()) return 0;
    lsetblank(&AcsApiRequestLog);
    lset(&AcsApiRequestLog, "TICKET_NO", sTicketNo);
    lset(&AcsApiRequestLog, "ENDPOINT", szEndpoint);
    lset(&AcsApiRequestLog, "API_URL", sUrl);
    lset(&AcsApiRequestLog, "REQUEST_DATA", sJson);
    AcsApiRequestLog.SetCurrentTime(GetDbFieldNo(&AcsApiRequestLog, "REQUEST_TS"));
    AcsApiRequestLog.AddNew();
    if (!AcsApiRequestLog.m_SaveFlag) return 0;
    return AcsApiRequestLog.lval(GetDbFieldNo(&AcsApiRequestLog, "PKKEY"));
}

static void LogResponse(long lRequestKey, const SAcsApiResult& r, const CString& sBody)
{
    if (!OpenLogTables()) return;
    lsetblank(&AcsApiResponseLog);
    ltodb(lRequestKey, &AcsApiResponseLog, GetDbFieldNo(&AcsApiResponseLog, "REQUEST_PKKEY"));
    ltodb(r.nStatusCode, &AcsApiResponseLog, GetDbFieldNo(&AcsApiResponseLog, "STATUS_CODE"));
    lset(&AcsApiResponseLog, "IS_SUCCESS", r.bSuccess ? "1" : "0");
    lset(&AcsApiResponseLog, "MESSAGE", r.sMessage.Left(1000));
    lset(&AcsApiResponseLog, "RESPONSE_DATA", sBody);
    AcsApiResponseLog.SetCurrentTime(GetDbFieldNo(&AcsApiResponseLog, "RESPONSE_TS"));
    AcsApiResponseLog.AddNew();
}

// Sends the request; fills r.bSent / nStatusCode and returns the raw body. Never throws.
static CString HttpPostJson(const CString& sUrl, const CString& sJson, SAcsApiResult& r)
{
    DWORD dwService = 0;
    CString sServer, sObject, sBody;
    INTERNET_PORT nPort = 0;
    if (!AfxParseURL(sUrl, dwService, sServer, sObject, nPort)
        || (dwService != AFX_INET_SERVICE_HTTP && dwService != AFX_INET_SERVICE_HTTPS)) {
        r.sMessage = "profile.API_URL is not a valid http(s) URL: " + sUrl;
        return sBody;
    }

    CInternetSession session("AllcosmosWeighbridge");
    session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10000);
    // 60 s: Ticket Out / DO Time Out answer only after the website's CSL call (up to 30 s) is done.
    session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 60000);
    session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 60000);
    CHttpConnection* pConn = NULL;
    CHttpFile* pFile = NULL;
    try {
        pConn = session.GetHttpConnection(sServer, nPort);
        pFile = pConn->OpenRequest(CHttpConnection::HTTP_VERB_POST, sObject, NULL, 1, NULL, NULL,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE
            | (dwService == AFX_INET_SERVICE_HTTPS ? INTERNET_FLAG_SECURE : 0));

        // Basic header sent up front: the site answers a bad login with 200, never a 401 challenge.
        CString sHeaders = "Content-Type: application/json\r\nAuthorization: Basic "
            + Base64(ProfileText("API_USER") + ":" + ProfileText("API_PASSWORD")) + "\r\n";
        pFile->SendRequest(sHeaders, (LPVOID)(LPCTSTR)sJson, sJson.GetLength());
        r.bSent = TRUE;

        DWORD dwStatus = 0;
        pFile->QueryInfoStatusCode(dwStatus);
        r.nStatusCode = (int)dwStatus;

        char szBuf[1024];
        UINT nRead;
        while ((nRead = pFile->Read(szBuf, sizeof(szBuf) - 1)) > 0) {
            szBuf[nRead] = 0;
            sBody += szBuf;
        }
    }
    catch (CInternetException* pEx) {
        TCHAR szError[512];
        pEx->GetErrorMessage(szError, 512);
        r.sMessage = szError;
        r.sMessage.TrimRight();
        pEx->Delete();
    }
    if (pFile) { pFile->Close(); delete pFile; }
    if (pConn) { pConn->Close(); delete pConn; }
    session.Close();
    return sBody;
}

SAcsApiResult AcsPost(const char* szEndpoint, const CString& sTicketNo, const CString& sJsonFields)
{
    SAcsApiResult r;
    r.bSent = FALSE;
    r.bSuccess = FALSE;
    r.nStatusCode = 0;

    CString sUrl = ProfileText("API_URL");
    sUrl.TrimRight('/');
    sUrl += "/api/";
    sUrl += szEndpoint;
    CString sJson = "{" + sJsonFields + "}";

    long lKey = LogRequest(sTicketNo, szEndpoint, sUrl, sJson);
    CString sBody = HttpPostJson(sUrl, sJson, r);

    // Only isSuccess counts; the site answers HTTP 200 even for failures.
    if (r.bSent) {
        r.bSuccess = FindJsonTrue(sBody, "isSuccess");
        r.sMessage = FindJsonString(sBody, "message");
        if (r.sMessage.IsEmpty() && !r.bSuccess) {
            r.sMessage.Format("HTTP %d, no isSuccess/message in reply: %s", r.nStatusCode, sBody.Left(200));
        }
    }
    else if (r.sMessage.IsEmpty()) {
        r.sMessage = "No answer from " + sUrl;
    }
    LogResponse(lKey, r, sBody);
    return r;
}
