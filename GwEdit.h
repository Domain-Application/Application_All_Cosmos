#if !defined(AFX_GWEDIT_H__27423541_2725_11D3_B36C_00001CD5F6A7__INCLUDED_)
#define AFX_GWEDIT_H__27423541_2725_11D3_B36C_00001CD5F6A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GwEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGwEdit window

class CGwEdit : public CEdit
{
// Construction
public:
	CGwEdit();
    UINT OldChar;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGwEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGwEdit();

	// Generated message map functions
protected:
    int flag;
	//{{AFX_MSG(CGwEdit)
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GWEDIT_H__27423541_2725_11D3_B36C_00001CD5F6A7__INCLUDED_)
