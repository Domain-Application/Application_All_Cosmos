// MENUView.h : interface of the CMenuView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUVIEW_H__01A41CA5_3A95_4712_AA5A_118DB73CFB3E__INCLUDED_)
#define AFX_MENUVIEW_H__01A41CA5_3A95_4712_AA5A_118DB73CFB3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "menuDoc.h"
#include "RunButton.h"
#include "Picmap.h"

class CMenuView : public CFormView
{
protected: // create from serialization only
    CMenuView();
    CRunButton m_ed[100];
    DECLARE_DYNCREATE(CMenuView)
    CToolTipCtrl m_ToolTip;
	BOOL m_ToolTipIsActive;
    CPicmap m_imgBg;
    BOOL BackGroundFile;

public:
    //{{AFX_DATA(CMenuView)
	enum { IDD = IDD_MENU_FORM };
	//}}AFX_DATA

// Attributes
public:
    CMenuDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMenuView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnEraseBkgnd(CDC* pDC);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CMenuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    //{{AFX_MSG(CMenuView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MENUView.cpp
inline CMenuDoc* CMenuView::GetDocument()
   { return (CMenuDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MENUVIEW_H__01A41CA5_3A95_4712_AA5A_118DB73CFB3E__INCLUDED_)
