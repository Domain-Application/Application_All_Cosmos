// AcsPickDlg.h - list of rows from a SQL query; the operator picks one (AllCosmos Ref No / DO No).
//Notes: docs/CPP_PROGRAMMING_NOTES.md.
#pragma once

class CAcsPickDlg : public CDialog
{
public:
    CAcsPickDlg(CWnd* pParent = NULL);

    CString m_title;       // window caption
    CString m_sql;         // every column must already be text; column names become the headers
    CString m_pick1;       // picked row, column 1
    CString m_pick2;       // picked row, column 2

    enum { IDD = IDD_ACS_PICK };

protected:
    CListCtrl m_list;
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()
};
