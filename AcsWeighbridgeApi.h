// AcsWeighbridgeApi.h - client for the AllcosmosSystem weighbridge API (POST JSON, HTTP Basic).
//Notes: docs/CPP_PROGRAMMING_NOTES.md.
#pragma once

struct SAcsApiResult
{
    BOOL    bSent;          // request reached the server
    BOOL    bSuccess;       // response body "isSuccess": true
    int     nStatusCode;    // HTTP status, 0 if no answer
    CString sMessage;       // website "message", or the transport error
};

// TRUE when profile.API_URL exists and is not blank.
BOOL IsAcsApiConfigured();

// Append one "key": value pair to a JSON object body (no braces).
void AcsJsonString(CString& sBody, const char* szKey, const CString& sValue);
void AcsJsonNumber(CString& sBody, const char* szKey, double dValue);

// POST {sJsonFields} to <profile.API_URL>/api/<szEndpoint>; logs request and response rows.
SAcsApiResult AcsPost(const char* szEndpoint, const CString& sTicketNo, const CString& sJsonFields);
