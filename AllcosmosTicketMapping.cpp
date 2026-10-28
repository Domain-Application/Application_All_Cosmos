// AllcosmosTicketMapping.cpp : which AllcosmosSystem call a ticket needs, and its JSON
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#include "stdafx.h"
#include "menu.h"
#include "AllcosmosTicketMapping.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CString Trimmed(const char *raw)
{
    CString s = raw;
    s.TrimLeft();
    s.TrimRight();
    return s;
}

// The saved TIMEIN / TIMEOUT as "yyyy-MM-dd HH:mm:ss"; dat[] holds it in the PC's display format.
static CString DateTimeText(DB *df, const char *column)
{
    CString sql;
    sql.Format("select convert(varchar(19), [%s], 120) from [%s] where [TICKET] = '%s'",
        column, df->name, Trimmed(df->dat[TICKET]));
    return Trimmed(SqlText(sql));
}

// A column of the ticket's first detail line (fdata, wbdata or dbdata, by the header table).
static CString FirstLineValue(DB *df, const char *column)
{
    const char *sub = !stricmp(df->name, "WB") ? "wbdata" : !stricmp(df->name, "DESPATCH") ? "dbdata" : "fdata";
    CString sql;
    sql.Format("select top 1 [%s] from [%s] where [REFNO] = '%s' order by [SQNO]",
        column, sub, Trimmed(df->dat[TICKET]));
    return Trimmed(SqlText(sql));
}

static const char szNoApiUrl[] = "The AllcosmosSystem API URL is not set in Company Profile.";

static const char *NoKeyMessage(BOOL receiving)
{
    return receiving ? "This ticket has no PO No on its first line." : "This ticket has no DO No on its first line.";
}

SAllcosmosApiResult AllcosmosWeighIn(DB *df, int type)
{
    if(!IsAllcosmosApiConfigured()) return SAllcosmosApiResult(szNoApiUrl);

    CString ticket = Trimmed(df->dat[TICKET]);
    CString body;
    if(type == 1) {
        CString po = FirstLineValue(df, "PO1");
        if(po.IsEmpty()) return SAllcosmosApiResult(NoKeyMessage(TRUE));
        CString site = ProfileText("SITE_CODE");
        if(site.IsEmpty()) {
            return LogAllcosmosNotSent(ACS_RM_TICKET_IN, ticket, "Site (Keluli / Suasa) is not set in Company Profile.");
        }
        AppendJsonString(body, "po_number", po);
        AppendJsonString(body, "item_code", Trimmed(df->dat[PT]));
        AppendJsonString(body, "date_in", DateTimeText(df, "TIMEIN"));
        AppendJsonString(body, "site_code", site);
        AppendJsonString(body, "lorry_no", Trimmed(df->dat[LORRY]));
        AppendJsonString(body, "ticket_no", ticket);
        return SendAllcosmosApi(ACS_RM_TICKET_IN, ticket, "{" + body + "}");
    }

    CString doNo = FirstLineValue(df, "DO");
    if(doNo.IsEmpty()) return SAllcosmosApiResult(NoKeyMessage(FALSE));
    AppendJsonString(body, "do_no", doNo);
    AppendJsonString(body, "date_in", DateTimeText(df, "TIMEIN"));
    return SendAllcosmosApi(ACS_DO_TIME_IN, ticket, "{" + body + "}");
}

SAllcosmosApiResult AllcosmosWeighOut(DB *df)
{
    if(!IsAllcosmosApiConfigured()) return SAllcosmosApiResult(szNoApiUrl);

    BOOL receiving = !stricmp(df->name, "WB");
    CString ticket = Trimmed(df->dat[TICKET]);
    CString key = FirstLineValue(df, receiving ? "PO1" : "DO");
    if(key.IsEmpty()) return SAllcosmosApiResult(NoKeyMessage(receiving));

    if(!HasAllcosmosSuccess(ticket, receiving ? ACS_RM_TICKET_IN : ACS_DO_TIME_IN)) {
        SAllcosmosApiResult in = AllcosmosWeighIn(df, receiving ? 1 : 2);
        if(!in.bSuccess) return in;
    }

    compute_weight(df, NULL);
    long net = (long)(m[11] + 0.5);

    CString body;
    if(receiving) {
        AppendJsonString(body, "ticket_no", ticket);
        AppendJsonNumber(body, "actual_weight", net);
        AppendJsonString(body, "date_out", DateTimeText(df, "TIMEOUT"));
        return SendAllcosmosApi(ACS_RM_TICKET_OUT, ticket, "{" + body + "}");
    }
    AppendJsonString(body, "do_no", key);
    AppendJsonString(body, "date_out", DateTimeText(df, "TIMEOUT"));
    AppendJsonNumber(body, "actual_weight", net);
    return SendAllcosmosApi(ACS_DO_TIME_OUT, ticket, "{" + body + "}");
}
