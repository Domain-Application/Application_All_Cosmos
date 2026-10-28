// AllcosmosWbApi.h : HTTP client for the AllcosmosSystem weighbridge APIs
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#if !defined(ALLCOSMOSWBAPI_H_INCLUDED)
#define ALLCOSMOSWBAPI_H_INCLUDED

extern DB AllcosmosApiRequestLog;
extern DB AllcosmosApiResponseLog;

struct SAllcosmosApiResult
{
    BOOL    bSuccess;       // HTTP 200 and "isSuccess": true
    int     nStatusCode;    // 0 when no answer came back
    CString sMessage;       // the API's message, or why the call failed

    SAllcosmosApiResult(const char *message = "") : bSuccess(FALSE), nStatusCode(0), sMessage(message) {}
};

// Endpoint names, appended to profile.API_URL as /api/<name>.
#define ACS_RM_TICKET_IN   "rawmaterialticketin"
#define ACS_RM_TICKET_OUT  "rawmaterialticketout"
#define ACS_DO_TIME_IN     "dotimein"
#define ACS_DO_TIME_OUT    "dotimeout"

BOOL IsAllcosmosApiConfigured();
CString ProfileText(const char *fieldName);

void AppendJsonString(CString &body, const char *key, const CString &value);
void AppendJsonNumber(CString &body, const char *key, long value);

// POSTs {body} to one endpoint with Basic auth; logs the request and the response. Never throws.
SAllcosmosApiResult SendAllcosmosApi(const char *apiName, const CString &ticketNo, const CString &body);
// Logs a call that was not sent (e.g. a missing setting), so the ticket's status line shows why.
SAllcosmosApiResult LogAllcosmosNotSent(const char *apiName, const CString &ticketNo, const CString &reason);

BOOL HasAllcosmosSuccess(const CString &ticketNo, const char *apiName);
CString GetAllcosmosStatusText(const CString &ticketNo);

#endif
