// MainFrm.h : interface of the CMainFrame class
//Notes: docs/CPP_PROGRAMMING_NOTES.md.
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__D6EA960A_A1F2_11D0_8F37_945AC87D3839__INCLUDED_)
#define AFX_MAINFRM_H__D6EA960A_A1F2_11D0_8F37_945AC87D3839__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "myfrm.h"
#include "resource.h"

class CMainFrame : public CMyFrame
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
// Operations
public:
    void OnClose();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
    virtual void OnOpenOtherFiles();
    virtual void OnCloseThreads();

protected:  // control bar embedded members
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	//}}AFX_MSG
	afx_msg LRESULT OnSnapLog(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__D6EA960A_A1F2_11D0_8F37_945AC87D3839__INCLUDED_)
