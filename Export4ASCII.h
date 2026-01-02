#if !defined(AFX_EXPORT4ASCII_H__1A35189E_1C13_4ACC_9C44_9432D785FE85__INCLUDED_)
#define AFX_EXPORT4ASCII_H__1A35189E_1C13_4ACC_9C44_9432D785FE85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Export4ASCII.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExport4ASCII dialog

class CExport4ASCII : public CDialog
{
// Construction
public:
	CExport4ASCII(CWnd* pParent = NULL);   // standard constructor
    void OnExportTextFile(DB *df);
    void OnExportExcelFile(DB *df);
    void OnGetDBFile(DB *df);
    void GetSubData(DB *df);
    void OnView();
    CString str;
    DB *df,*sub;

// Dialog Data
	//{{AFX_DATA(CExport4ASCII)
	enum { IDD = IDD_Export4ASCII };
	COleDateTime	m_d1;
	COleDateTime	m_d2;
	CString	m_file;
	COleDateTime	m_t1;
	COleDateTime	m_t2;
	BOOL	m_method;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExport4ASCII)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExport4ASCII)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORT4ASCII_H__1A35189E_1C13_4ACC_9C44_9432D785FE85__INCLUDED_)
