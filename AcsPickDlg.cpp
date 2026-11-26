// AcsPickDlg.cpp - list of rows from a SQL query; the operator picks one (AllCosmos Ref No / DO No).
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#include "stdafx.h"
#include "menu.h"
#include "openrs.h"
#include "AcsPickDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAcsPickDlg::CAcsPickDlg(CWnd* pParent)
    : CDialog(CAcsPickDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CAcsPickDlg, CDialog)
    ON_NOTIFY(NM_DBLCLK, IDC_ACS_LIST, OnDblclkList)
END_MESSAGE_MAP()

BOOL CAcsPickDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetWindowText(m_title);
    m_list.SubclassDlgItem(IDC_ACS_LIST, this);
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    OpenRs rs(m_sql);
    if (rs.bOpen) {
        long nCols = rs.rs->Fields->Count;
        for (long c = 0; c < nCols; c++) {
            m_list.InsertColumn(c, (LPCTSTR)rs.rs->Fields->Item[c]->Name, LVCFMT_LEFT, c == 0 ? 190 : 110);
        }
        for (int row = 0; !rs.rs->EndOfFile; row++, rs.rs->MoveNext()) {
            for (long c = 0; c < nCols; c++) {
                CString s = rs.dbText(c + 1);
                s.TrimRight();
                if (c == 0) m_list.InsertItem(row, s); else m_list.SetItemText(row, c, s);
            }
        }
    }
    if (m_list.GetItemCount() > 0) {
        m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }
    m_list.SetFocus();
    return FALSE;
}

void CAcsPickDlg::OnOK()
{
    int row = m_list.GetNextItem(-1, LVNI_SELECTED);
    if (row < 0) return;
    m_pick1 = m_list.GetItemText(row, 0);
    m_pick2 = m_list.GetItemText(row, 1);
    CDialog::OnOK();
}

void CAcsPickDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
    OnOK();
    *pResult = 0;
}
