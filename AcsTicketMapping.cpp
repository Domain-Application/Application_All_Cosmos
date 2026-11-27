// AcsTicketMapping.cpp - AllcosmosSystem Ticket In / Ticket Out / DO Time In / DO Time Out.
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#include "stdafx.h"
#include "menu.h"
#include "AcsWeighbridgeApi.h"
#include "AcsTicketMapping.h"
#include "AcsPickDlg.h"
#include "admindb.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CString Trimmed(const char* p)
{
    CString s = p;
    s.TrimLeft();
    s.TrimRight();
    return s;
}

static CString TicketText(DB* df, const char* szField)
{
    int fld = GetDbFieldNo(df, szField);
    return (fld > 0) ? Trimmed(df->dat[fld]) : CString("");
}

static CString SqlQuote(const CString& s)
{
    CString q = s;
    q.Replace("'", "''");
    return "'" + q + "'";
}

static CString Ticket(DB* df)
{
    return Trimmed(df->dat[TICKET]);
}

// Writes the value to the buffer and straight to the saved row.
static void SetTicketField(DB* df, const char* szField, const CString& sValue)
{
    lset(df, szField, sValue);
    CString s;
    s.Format("update [%s] set [%s] = %s where TICKET = %s", df->name, szField,
        sValue.IsEmpty() ? "NULL" : (LPCTSTR)SqlQuote(sValue), (LPCTSTR)SqlQuote(Ticket(df)));
    RunSql(s);
}

// yyyy-MM-dd HH:mm:ss straight from SQL, whatever format dat[] holds.
static CString SqlDateTime(DB* df, const char* szField)
{
    CString s;
    s.Format("select convert(varchar(19), [%s], 120) from [%s] where TICKET = %s",
        szField, df->name, (LPCTSTR)SqlQuote(Ticket(df)));
    return Trimmed(SqlText(s));
}

static BOOL ObjectExists(const char* szSysTable, const char* szName)
{
    CString s;
    s.Format("select count(*) from sys.%s where name = '%s'", szSysTable, szName);
    return atoi(SqlText(s)) > 0;
}

BOOL AcsIsActive(DB* df)
{
    return IsAcsApiConfigured() && GetDbFieldNo(df, "API_REF_NO") > 0;
}

BOOL AcsIsDespatch(DB* df)
{
    if (!_stricmp(df->name, "despatch")) return TRUE;
    if (!_stricmp(df->name, "wb")) return FALSE;
    return df->lval(GetDbFieldNo(df, _szPrintCounter)) == 2;
}

CString AcsTicketText(DB* df, const char* szField)
{
    return TicketText(df, szField);
}

void AcsShowCtrls(CWnd* pDlg, BOOL bShow, UINT id1, UINT id2, UINT id3)
{
    UINT ids[3] = { id1, id2, id3 };
    for (int i = 0; i < 3; i++) {
        CWnd* w = ids[i] ? pDlg->GetDlgItem(ids[i]) : NULL;
        if (w) w->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
    }
}

static CString s_sLastReply;   // website message of the last call, shown after a resend

BOOL AcsHasBin(DB* df)
{
    return AcsIsActive(df) && GetDbFieldNo(df, "API_BIN") > 0;
}

// Records the outcome on the ticket; the operator sees every failure once.
static BOOL Outcome(DB* df, const char* szFlag, const char* szWhat, BOOL bOk, const CString& sMessage, BOOL bTell = TRUE)
{
    s_sLastReply = sMessage;
    SetTicketField(df, szFlag, bOk ? "Y" : "N");
    CString msg;
    msg.Format("AllCosmos %s, ticket %s: %s", szWhat, (LPCTSTR)Ticket(df), (LPCTSTR)sMessage);
    log_proceeding(msg);
    if (!bOk && bTell) {
        AfxMessageBox(msg + "\n\nThe weighing is saved. Fix it and use Utility > AllCosmos Resend Pending Tickets.", MB_ICONWARNING);
    }
    return bOk;
}

static BOOL Post(DB* df, const char* szFlag, const char* szWhat, const char* szEndpoint, const CString& sFields)
{
    SAcsApiResult r = AcsPost(szEndpoint, Ticket(df), sFields);
    return Outcome(df, szFlag, szWhat, r.bSuccess, r.sMessage);
}

BOOL AcsSendIn(DB* df)
{
    if (!AcsIsActive(df)) return TRUE;
    BOOL bDespatch = AcsIsDespatch(df);
    const char* szWhat = bDespatch ? "DO Time In" : "Ticket In";
    CString sRef = TicketText(df, "API_REF_NO");
    if (sRef.IsEmpty()) {
        return Outcome(df, "API_IN_OK", szWhat, FALSE, bDespatch ? "DO No is blank." : "Ref No is blank.");
    }

    CString f;
    if (bDespatch) {
        AcsJsonString(f, "do_no", sRef);
        AcsJsonString(f, "date_in", SqlDateTime(df, "TIMEIN"));
        return Post(df, "API_IN_OK", szWhat, "dotimein", f);
    }
    AcsJsonString(f, "ref_no", sRef);
    AcsJsonString(f, "ticket_no", Ticket(df));
    AcsJsonString(f, "lorry_no", Trimmed(df->dat[LORRY]));
    AcsJsonString(f, "date_in", SqlDateTime(df, "TIMEIN"));
    return Post(df, "API_IN_OK", szWhat, "ticketin", f);
}

static CString LookupPoUnit(const CString& sRef)
{
    if (!ObjectExists("views", "V_ACS_RECEIVING_REF")) return "";
    return Trimmed(SqlText("select isnull(min(PO_UNIT), '') from V_ACS_RECEIVING_REF where REF_NO = " + SqlQuote(sRef)));
}

// kg per PO unit; 0 = unit this app cannot convert.
static double KgPerUnit(const CString& sUnit)
{
    if (!sUnit.CompareNoCase("KG")) return 1.0;
    if (!sUnit.CompareNoCase("MT")) return 1000.0;
    return 0.0;
}

BOOL AcsSendOut(DB* df)
{
    if (!AcsIsActive(df)) return TRUE;
    BOOL bDespatch = AcsIsDespatch(df);
    const char* szWhat = bDespatch ? "DO Time Out" : "Ticket Out";
    if (TicketText(df, "API_IN_OK") != "Y" && !AcsSendIn(df)) {
        return Outcome(df, "API_OUT_OK", szWhat, FALSE, "not sent because the 1st weigh was not accepted.", FALSE);
    }

    compute_weight(df, df->sub);    // m[2]/m[5] = 1st/2nd weight, m[11] = nett, m[0] = supplier wt (kg)
    CString f;
    if (bDespatch) {
        AcsJsonString(f, "do_no", TicketText(df, "API_REF_NO"));
        AcsJsonString(f, "date_out", SqlDateTime(df, "TIMEOUT"));
        AcsJsonNumber(f, "actual_weight", m[11] / 1000.0);   // MT: the website's Unit master has no KG
        AcsJsonString(f, "actual_unit", "MT");
        return Post(df, "API_OUT_OK", szWhat, "dotimeout", f);
    }

    CString sUnit = TicketText(df, "API_UNIT");
    if (sUnit.IsEmpty()) {
        sUnit = LookupPoUnit(TicketText(df, "API_REF_NO"));
        if (!sUnit.IsEmpty()) SetTicketField(df, "API_UNIT", sUnit);
    }
    double dKg = KgPerUnit(sUnit);
    if (dKg == 0.0) {
        return Outcome(df, "API_OUT_OK", szWhat, FALSE, "PO unit '" + sUnit + "' is not KG or MT; quantities cannot be converted.");
    }
    AcsJsonString(f, "ticket_no", Ticket(df));
    AcsJsonString(f, "date_out", SqlDateTime(df, "TIMEOUT"));
    AcsJsonNumber(f, "received_qty", m[11] / dKg);
    AcsJsonNumber(f, "weighbridge_qty", fabs(m[5] - m[2]) / dKg);
    AcsJsonNumber(f, "supplier_delivery_qty", m[0] / dKg);
    CString sBin = TicketText(df, "API_BIN");
    if (!sBin.IsEmpty()) AcsJsonString(f, "bin_location", sBin);
    return Post(df, "API_OUT_OK", szWhat, "ticketout", f);
}

BOOL AcsPickRefNo(BOOL bDespatch, CString& sRefNo, CString& sUnit)
{
    const char* szView = bDespatch ? "V_ACS_DESPATCH_DO" : "V_ACS_RECEIVING_REF";
    if (!ObjectExists("views", szView)) {
        AfxMessageBox(CString("The AllCosmos list (") + szView + ") is not available yet. Type the "
            + (bDespatch ? "DO No." : "Ref No.") + " instead.", MB_ICONINFORMATION);
        return FALSE;
    }
    // All columns as text (CAcsPickDlg shows them as they are); numbers already on a lorry in the yard are left out.
    CAcsPickDlg dlg;
    if (bDespatch) {
        dlg.m_title = "AllCosmos DO No. (Delivery Date today)";
        dlg.m_sql =
            "select DO_NO [DO No], isnull(ITEM_UNIT_CODE, '') [Unit], DESPATCH_TICKET [Despatch Ticket], isnull(LORRY_PLATE_NO, '') [Lorry],"
            " isnull(CUSTOMER_NAME, '') [Customer], isnull(ITEM_DESCRIPTION, '') [Item], cast(WEIGHT as varchar(20)) [Weight],"
            " cast(LINES as varchar(5)) [Lines] from V_ACS_DESPATCH_DO"
            " where DO_NO not in (select API_REF_NO from [first] where API_REF_NO is not null) order by DO_NO";
    }
    else {
        dlg.m_title = "AllCosmos Ref No. (Raw Material Planning / Import Schedule & Costing)";
        dlg.m_sql =
            "select REF_NO [Ref No], isnull(PO_UNIT, '') [PO Unit], PO_NO [PO No], isnull(VENDOR_NAME, '') [Supplier],"
            " isnull(ITEM_DESCRIPTION, '') [Item], isnull(convert(varchar(10), DELIVERY_DATE, 120), '') [Delivery Date],"
            " isnull(cast(DELIVERY_QUANTITY as varchar(20)), '') [Qty], isnull(CONTAINER_NO, '') [Container], MODULE [Module]"
            " from V_ACS_RECEIVING_REF"
            " where REF_NO not in (select API_REF_NO from [first] where API_REF_NO is not null) order by DELIVERY_DATE, REF_NO";
    }
    if (dlg.DoModal() != IDOK || dlg.m_pick1.IsEmpty()) return FALSE;
    sRefNo = dlg.m_pick1;
    sUnit = bDespatch ? CString("") : dlg.m_pick2;
    return TRUE;
}

static const char szNoBinTable[] = "The AllCosmos bin list (ACS_BIN_LOCATION) is not set up yet: run sql/005.";

void AcsAdminBinLocation()
{
    static DB acsbin(IDD_STATUS);   // opened on first use: the table exists only after sql/005
    static BOOL bOpen = FALSE;
    if (!ObjectExists("tables", "ACS_BIN_LOCATION")) {
        AfxMessageBox(szNoBinTable, MB_ICONINFORMATION);
        return;
    }
    if (!bOpen) bOpen = ado(&acsbin, "ACS_BIN_LOCATION");
    if (bOpen) AdminDb(&acsbin);
}

BOOL AcsPickBin(CString& sBin)
{
    if (!ObjectExists("tables", "ACS_BIN_LOCATION")) {
        AfxMessageBox(szNoBinTable, MB_ICONINFORMATION);
        return FALSE;
    }
    CAcsPickDlg dlg;
    dlg.m_title = "AllCosmos Bin Location";
    dlg.m_sql = "select CODE [Bin Location], isnull(DESCRIPTION, '') [Description] from ACS_BIN_LOCATION order by CODE";
    if (dlg.DoModal() != IDOK || dlg.m_pick1.IsEmpty()) return FALSE;
    sBin = dlg.m_pick1;
    return TRUE;
}

static CString FlagText(DB* df, const char* szFlag)
{
    CString s = TicketText(df, szFlag);
    if (s == "Y") return "Accepted";
    if (s == "N") return "NOT accepted";
    return "Not sent";
}

static BOOL IsFirstTable(DB* df)
{
    return !_stricmp(df->name, "FIRST");
}

CString AcsStatusText(DB* df)
{
    if (Ticket(df).IsEmpty()) return "";
    BOOL bDespatch = AcsIsDespatch(df);
    CString s, sRef = TicketText(df, "API_REF_NO");
    s.Format("%s: %s", bDespatch ? "DO No" : "Ref No", sRef.IsEmpty() ? "(blank)" : (LPCTSTR)sRef);
    if (!bDespatch && AcsHasBin(df)) s += "   Bin: " + TicketText(df, "API_BIN");
    s += "\n1st: " + FlagText(df, "API_IN_OK");
    if (!IsFirstTable(df)) s += "   2nd: " + FlagText(df, "API_OUT_OK");
    return s;
}

static DB* TicketTable(const CString& sTable)
{
    if (!sTable.CompareNoCase("WB")) return &wb;
    if (!sTable.CompareNoCase("DESPATCH")) return &despatch;
    return &first;
}

// The Ref No / DO No can still change until the 1st weigh is accepted.
static void OfferNewRefNo(DB* df)
{
    BOOL bDespatch = AcsIsDespatch(df);
    CString sMsg;
    sMsg.Format("Ticket %s, lorry %s: %s is '%s'.\n\nPick a different one from the AllCosmos list?",
        (LPCTSTR)Ticket(df), (LPCTSTR)Trimmed(df->dat[LORRY]), bDespatch ? "DO No" : "Ref No", (LPCTSTR)TicketText(df, "API_REF_NO"));
    CString sRef, sUnit;
    if (AfxMessageBox(sMsg, MB_YESNO | MB_ICONQUESTION) == IDYES && AcsPickRefNo(bDespatch, sRef, sUnit)) {
        SetTicketField(df, "API_REF_NO", sRef);
        SetTicketField(df, "API_UNIT", sUnit);
    }
}

static void OfferBin(DB* df)
{
    if (AcsIsDespatch(df) || IsFirstTable(df) || !AcsHasBin(df) || !TicketText(df, "API_BIN").IsEmpty()) return;
    CString sBin;
    if (AfxMessageBox("Ticket " + Ticket(df) + ": Bin Location is blank and CSL needs it for the GRN.\n\nPick one now?",
                      MB_YESNO | MB_ICONQUESTION) == IDYES && AcsPickBin(sBin)) {
        SetTicketField(df, "API_BIN", sBin);
    }
}

void AcsResendTicket(DB* df)
{
    if (!AcsIsActive(df) || Ticket(df).IsEmpty()) return;
    const char* szFlag = IsFirstTable(df) ? "API_IN_OK" : "API_OUT_OK";
    CString sWasFlag = TicketText(df, szFlag);
    if (sWasFlag == "Y" && AfxMessageBox("Ticket " + Ticket(df) + " is already accepted by AllCosmos.\n\nSend it again?",
                                         MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    if (TicketText(df, "API_IN_OK") != "Y") OfferNewRefNo(df);
    OfferBin(df);
    if (IsFirstTable(df) ? AcsSendIn(df) : AcsSendOut(df)) {
        AfxMessageBox("Ticket " + Ticket(df) + " accepted by AllCosmos.\n\n" + s_sLastReply, MB_ICONINFORMATION);
    }
    else if (sWasFlag == "Y") {
        SetTicketField(df, szFlag, "Y");   // the website still holds the earlier accepted data
    }
}

void AcsResendPending()
{
    if (!AcsIsActive(&first)) {
        AfxMessageBox("AllCosmos API is not set up: run sql/001-003 and fill in API URL, user and password in the profile table.", MB_ICONINFORMATION);
        return;
    }
    const char* szPending =
        " from (select TICKET, 'FIRST' TBL, LORRY, API_REF_NO, API_IN_OK, API_OUT_OK, TIMEIN from [first] where API_IN_OK = 'N'"
        " union all select TICKET, 'WB', LORRY, API_REF_NO, API_IN_OK, API_OUT_OK, TIMEIN from [wb] where API_IN_OK = 'N' or API_OUT_OK = 'N'"
        " union all select TICKET, 'DESPATCH', LORRY, API_REF_NO, API_IN_OK, API_OUT_OK, TIMEIN from [despatch] where API_IN_OK = 'N' or API_OUT_OK = 'N') x";
    if (atoi(SqlText(CString("select count(*)") + szPending)) == 0) {
        AfxMessageBox("No AllCosmos tickets are waiting to be resent.", MB_ICONINFORMATION);
        return;
    }
    CAcsPickDlg dlg;
    dlg.m_title = "AllCosmos - tickets not accepted yet (pick one to resend)";
    dlg.m_sql = CString("select TICKET [Ticket], TBL [Table], isnull(LORRY, '') [Lorry], isnull(API_REF_NO, '') [Ref No / DO No],"
        " isnull(API_IN_OK, '') [1st weigh OK], isnull(API_OUT_OK, '') [2nd weigh OK], convert(varchar(19), TIMEIN, 120) [Time In]")
        + szPending + " order by TIMEIN desc";
    if (dlg.DoModal() != IDOK || dlg.m_pick1.IsEmpty()) return;

    DB* df = TicketTable(dlg.m_pick2);
    if (df->Search(dlg.m_pick1, TICKET, TRUE)) AcsResendTicket(df);
}
