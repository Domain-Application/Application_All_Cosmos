// AcsTicketMapping.h - builds the four AllcosmosSystem calls from a FIRST / WB / DESPATCH row.
//Notes: docs/CPP_PROGRAMMING_NOTES.md.
#pragma once

// TRUE when the API is configured and sql/002 has added API_REF_NO to this table.
BOOL AcsIsActive(DB* df);

// Despatch when the table is DESPATCH, or FIRST with [Print Counter] = 2.
BOOL AcsIsDespatch(DB* df);

// 1st weigh: ticketin / dotimein. 2nd weigh: ticketout / dotimeout. The row must be saved.
// Sets API_IN_OK / API_OUT_OK to Y or N; on failure shows the website message once.
BOOL AcsSendIn(DB* df);
BOOL AcsSendOut(DB* df);

// Pick list from V_ACS_RECEIVING_REF / V_ACS_DESPATCH_DO. sUnit = PO unit (receiving only).
BOOL AcsPickRefNo(BOOL bDespatch, CString& sRefNo, CString& sUnit);

// Menu command: pick one ticket flagged N, optionally pick a new Ref No / DO No, resend it.
void AcsResendPending();
