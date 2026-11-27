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

// Trimmed text of a ticket column read by name; "" when the column is missing.
CString AcsTicketText(DB* df, const char* szField);

// Shows or hides up to three controls of one AllCosmos field (0 = unused).
void AcsShowCtrls(CWnd* pDlg, BOOL bShow, UINT id1, UINT id2, UINT id3 = 0);

// TRUE when sql/005 has added API_BIN to this table.
BOOL AcsHasBin(DB* df);

// Pick list from ACS_BIN_LOCATION.
BOOL AcsPickBin(CString& sBin);

// Menu command: the bin master screen (code + description, shared IDD_STATUS dialog).
void AcsAdminBinLocation();

// Two lines for the ticket screen: Ref No / DO No, bin, and what the website accepted.
CString AcsStatusText(DB* df);

// Resends the current ticket (FIRST: 1st weigh, WB/DESPATCH: 2nd weigh); asks first when it was accepted.
void AcsResendTicket(DB* df);

// Menu command: pick one ticket flagged N, optionally pick a new Ref No / DO No, resend it.
void AcsResendPending();
