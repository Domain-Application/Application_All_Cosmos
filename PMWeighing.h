#include "reqitem.h"
class CPMWeighing : public CReqitem
{
public:
	CPMWeighing(UINT XIDD,DB *pdf,DB *psub,int Idx,CWnd* pParent = NULL);
	DB *df;
	BOOL isInDirection;
	BOOL hideShuntingLorryFlag;
	BOOL hideRoRoFlag;	
	enum { IDD = IDD_PMWeighing };
	CString	m_vehicle;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);  
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};