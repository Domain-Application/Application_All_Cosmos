#if !defined(AFX_SENSOR_H__1646377A_07CC_4913_8E79_6292637E3997__INCLUDED_)
#define AFX_SENSOR_H__1646377A_07CC_4913_8E79_6292637E3997__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Sensor.h : header file
//

#include "DynamicLED.h"
/////////////////////////////////////////////////////////////////////////////
// CSensor window

class CSensor : public CDynamicLED
{
// Construction
public:
	CSensor();
    void Show();
    int  no;
    long weight,lastStatus;
    CString s,hitTime;


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSensor)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSensor();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSensor)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENSOR_H__1646377A_07CC_4913_8E79_6292637E3997__INCLUDED_)
