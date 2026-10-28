// AllcosmosWbApi.cpp : HTTP client for the AllcosmosSystem weighbridge APIs
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#include "stdafx.h"
#include "menu.h"
#include "AllcosmosWbApi.h"
#include <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DB AllcosmosApiRequestLog;
DB AllcosmosApiResponseLog;

CString ProfileText(const char *fieldName)
{
    int fld = GetDbFieldNo(&profile, fieldName);
    CString s = (fld > 0) ? profile.dat[fld] : "";
    s.TrimLeft();
    s.TrimRight();
    return s;
}

BOOL IsAllcosmosApiConfigured()
{
    return !ProfileText("API_URL").IsEmpty();
}

static CString JsonEscape(const CString &in)
{
    CString out = in;
    out.Replace("\\", "\\\\");
    out.Replace("\"", "\\\"");
    out.Replace("\r", "");
    out.Replace("\n", "\\n");
    return out;
}

static void AppendJsonPair(CString &body, const char *key, const CString &json)
{
    if(!body.IsEmpty()) body += ",";
    CString s;
    s.Format("\"%s\":%s", key, json);
    body += s;
}

void AppendJsonString(CString &body, const char *key, const CString &value)
{
    AppendJsonPair(body, key, "\"" + JsonEscape(value) + "\"");
}

void AppendJsonNumber(CString &body, const char *key, long value)
{
    CString s;
    s.Format("%ld", value);
    AppendJsonPair(body, key, s);
}

static CString Base64(const CString &in)
{
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const unsigned char *p = (const unsigned char *)(LPCTSTR)in;
    int len = in.GetLength();
    CString out;
    for(int i = 0; i < len; i += 3) {
        unsigned long n = (unsigned long)p[i] << 16;
        if(i + 1 < len) n |= (unsigned long)p[i+1] << 8;
        if(i + 2 < len) n |= p[i+2];
        out += table[(n >> 18) & 63];
        out += table[(n >> 12) & 63];
        out += (i + 1 < len) ? table[(n >> 6) & 63] : '=';
        out += (i + 2 < len) ? table[n & 63] : '=';
    }
    return out;
}

// Value of "key" in a flat JSON object: the text of a string, or the bare word of true/false/number.
static CString JsonValue(const CString &body, const char *key)
{
    CString needle;
    needle.Format("\"%s\"", key);
    int pos = body.Find(needle);
    if(pos < 0) return "";
    pos = body.Find(':', pos + needle.GetLength());
    if(pos < 0) return "";
    pos++;
    int len = body.GetLength();
    while(pos < len && isspace((unsigned char)body[pos])) pos++;
    if(pos >= len) return "";

    if(body[pos] == '"') {
        CString value;
        for(pos++; pos < len && body[pos] != '"'; pos++) {
            if(body[pos] == '\\' && pos + 1 < len) pos++;
            value += body[pos];
        }
        return value;
    }
    int end = pos;
    while(end < len && body[end] != ',' && body[end] != '}' && !isspace((unsigned char)body[end])) end++;
    return body.Mid(pos, end - pos);
}

static long LogRequest(const char *apiName, const CString &ticketNo, const CString &url, const CString &body)
{
    lsetblank(&AllcosmosApiRequestLog);
    lset(&AllcosmosApiRequestLog, "TICKET_NO", ticketNo);
    lset(&AllcosmosApiRequestLog, "API_NAME", apiName);
    lset(&AllcosmosApiRequestLog, "API_URL", url);
    lset(&AllcosmosApiRequestLog, "REQUEST_DATA", body);
    // AddNew sends blank columns as NULL, so the column DEFAULT never applies.
    AllcosmosApiRequestLog.SetCurrentTime(GetDbFieldNo(&AllcosmosApiRequestLog, "REQUEST_TS"));
    AllcosmosApiRequestLog.AddNew();
    if(AllcosmosApiRequestLog.m_SaveFlag == FALSE) return 0;
    return AllcosmosApiRequestLog.lval(GetDbFieldNo(&AllcosmosApiRequestLog, "PKKEY"));
}

static void LogResponse(long requestKey, const SAllcosmosApiResult &r, const CString &responseBody)
{
    lsetblank(&AllcosmosApiResponseLog);
    ltodb(requestKey, &AllcosmosApiResponseLog, "REQUEST_PKKEY");
    ltodb(r.nStatusCode, &AllcosmosApiResponseLog, "STATUS_CODE");
    lset(&AllcosmosApiResponseLog, "IS_SUCCESS", r.bSuccess ? "1" : "0");
    lset(&AllcosmosApiResponseLog, "RESPONSE_DATA", responseBody);
    lset(&AllcosmosApiResponseLog, "ERROR_MESSAGE", r.sMessage.Left(1000));
    AllcosmosApiResponseLog.SetCurrentTime(GetDbFieldNo(&AllcosmosApiResponseLog, "RESPONSE_TS"));
    AllcosmosApiResponseLog.AddNew();
}

SAllcosmosApiResult SendAllcosmosApi(const char *apiName, const CString &ticketNo, const CString &body)
{
    SAllcosmosApiResult r;

    CString url = ProfileText("API_URL");
    if(url.Right(1) == "/") url = url.Left(url.GetLength() - 1);
    url += "/api/";
    url += apiName;

    long requestKey = LogRequest(apiName, ticketNo, url, body);

    DWORD serviceType = 0;
    CString server, object;
    INTERNET_PORT port = 0;
    if(!AfxParseURL(url, serviceType, server, object, port)
        || (serviceType != AFX_INET_SERVICE_HTTP && serviceType != AFX_INET_SERVICE_HTTPS)) {
        r.sMessage = "API URL in Company Profile is not a valid http(s) address: " + url;
        LogResponse(requestKey, r, "");
        return r;
    }

    CString headers = "Content-Type: application/json\r\nAuthorization: Basic "
        + Base64(ProfileText("API_USERNAME") + ":" + ProfileText("API_PASSWORD")) + "\r\n";

    CInternetSession session("AllcosmosWeighbridge");
    session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10000);
    session.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 15000);
    session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 15000);
    CHttpConnection *conn = NULL;
    CHttpFile *file = NULL;
    CString responseBody;

    try {
        conn = session.GetHttpConnection(server, port);
        file = conn->OpenRequest(CHttpConnection::HTTP_VERB_POST, object, NULL, 1, NULL, NULL,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE
            | (serviceType == AFX_INET_SERVICE_HTTPS ? INTERNET_FLAG_SECURE : 0));
        file->SendRequest(headers, (LPVOID)(LPCTSTR)body, body.GetLength());

        DWORD status = 0;
        file->QueryInfoStatusCode(status);
        r.nStatusCode = (int)status;

        CString line;
        while(file->ReadString(line)) responseBody += line;

        r.sMessage = JsonValue(responseBody, "message");
        r.bSuccess = (status == 200 && JsonValue(responseBody, "isSuccess") == "true");
        if(!r.bSuccess && r.sMessage.IsEmpty()) {
            r.sMessage.Format("HTTP status %d", r.nStatusCode);
        }
    }
    catch(CInternetException *e) {
        char text[512];
        e->GetErrorMessage(text, sizeof(text));
        r.sMessage = text;
        r.sMessage.TrimRight();
        e->Delete();
    }

    if(file) { file->Close(); delete file; }
    if(conn) { conn->Close(); delete conn; }
    session.Close();

    LogResponse(requestKey, r, responseBody);
    return r;
}

SAllcosmosApiResult LogAllcosmosNotSent(const char *apiName, const CString &ticketNo, const CString &reason)
{
    SAllcosmosApiResult r(reason);
    LogResponse(LogRequest(apiName, ticketNo, "", ""), r, "");
    return r;
}

BOOL HasAllcosmosSuccess(const CString &ticketNo, const char *apiName)
{
    CString sql;
    sql.Format("select count(*) from [AllcosmosApiResponseLog] r "
        "join [AllcosmosApiRequestLog] q on r.REQUEST_PKKEY = q.PKKEY "
        "where q.TICKET_NO = '%s' and q.API_NAME = '%s' and r.IS_SUCCESS = 1",
        ticketNo, apiName);
    return atoi(SqlText(sql)) > 0;
}

CString GetAllcosmosStatusText(const CString &ticketNo)
{
    if(ticketNo.IsEmpty() || !IsAllcosmosApiConfigured()) return "";

    CString sql;
    sql.Format("select top 1 'AllcosmosSystem ' + q.API_NAME "
        "+ case when r.IS_SUCCESS = 1 then ': OK (' else ': FAILED (' end "
        "+ convert(varchar, r.RESPONSE_TS, 120) + ')' "
        "+ case when isnull(r.ERROR_MESSAGE, '') <> '' then ' - ' + r.ERROR_MESSAGE else '' end "
        "from [AllcosmosApiResponseLog] r "
        "join [AllcosmosApiRequestLog] q on r.REQUEST_PKKEY = q.PKKEY "
        "where q.TICKET_NO = '%s' order by r.PKKEY desc", ticketNo);
    CString status = SqlText(sql);
    status.TrimRight();
    return status.IsEmpty() ? CString("AllcosmosSystem: not sent") : status;
}
