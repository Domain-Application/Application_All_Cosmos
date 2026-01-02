// MENUView.cpp : implementation of the CMenuView class
//

#include "stdafx.h"
#include "MENU.h"

#include "MENUDoc.h"
#include "MENUView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMenuView

IMPLEMENT_DYNCREATE(CMenuView, CFormView)

BEGIN_MESSAGE_MAP(CMenuView, CFormView)
    //{{AFX_MSG_MAP(CMenuView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMenuView construction/destruction

CMenuView::CMenuView()
    : CFormView(CMenuView::IDD)
{
    //{{AFX_DATA_INIT(CMenuView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// TODO: add construction code here
	m_ToolTipIsActive = FALSE;
}

CMenuView::~CMenuView()
{
}

void CMenuView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMenuView)
	//}}AFX_DATA_MAP
}

BOOL CMenuView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CMenuView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	int i,j,k;
    BackGroundFile = IsFileExisted("bg.jpg");
    if(BackGroundFile) m_imgBg.Load("bg.jpg");
	m_ToolTip.Create(this);
    ado(&report,"runbutton");
    for(i=0; i<=99; i++) {
        HWND hWnd = ::GetDlgItem(this->GetSafeHwnd(), ID00000+i);
        if(!hWnd) continue;
        m_ed[i].SubclassDlgItem(ID00000+i,this);
        CString str;
        str.Format("Program ID: %5.5ld", i);
        if(!m_ToolTip.AddTool(GetDlgItem(IDC_BUTTON1 + i - 1), str)) {
            TRACE("Unable To Create ToolTip\n");
        }
        str.Format("%5.5ld", i);
        long xrec = report.Search(str,1,TRUE);
        if(xrec>0) {
            if(!isblank(report.dat[2])) {
                 SetDlgItemText(IDC_BUTTON1 + i - 1, report.dat[2]);
            }
        }
        m_ed[i].SetIcon(IDI_ICON000 + i , BS_RIGHT);
        if((i%3)==0) {
            m_ed[i].SetShade(SHS_VSHADE,8,20,5,RGB(55,55,255));
        }
        else if((i%3)==1) {
            m_ed[i].SetShade(SHS_HARDBUMP,8,20,5,RGB(55,55,255));
        }
        else if((i%3)==2) {
            m_ed[i].SetShade(SHS_SOFTBUMP,8,20,5,RGB(55,55,255));
        }
    }
    dclose(&report);
    for(j=0,i=UR_N01; i<=UR_N20; i++) {
        int node = user.lval(i);
        if(node<=0) continue;

        HWND hWnd = ::GetDlgItem(this->GetSafeHwnd(), ID00001+node-1);
        if(hWnd) GetDlgItem(ID00001+node-1)->ShowWindow(SW_HIDE);
        for(k=0; k<10; k++) {
             int z = ID00001+ (node*10) + k;
             hWnd = ::GetDlgItem(this->GetSafeHwnd(), z);
             if(hWnd) GetDlgItem(z)->ShowWindow(SW_HIDE);
        }
    }
    for(i=UR_N101; i<=UR_N110; i++) {
        int node = user.lval(i);
        if(node<=0) continue;

        HWND hWnd = ::GetDlgItem(this->GetSafeHwnd(), ID00001+node-1);
        if(hWnd) GetDlgItem(ID00001+node-1)->ShowWindow(SW_HIDE);
        for(k=0; k<10; k++) {
             int z = ID00001+ (node*10) + k;
             hWnd = ::GetDlgItem(this->GetSafeHwnd(), z);
             if(hWnd) GetDlgItem(z)->ShowWindow(SW_HIDE);
		}
    }
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
    m_ToolTipIsActive = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMenuView printing

BOOL CMenuView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMenuView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMenuView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CMenuView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

/////////////////////////////////////////////////////////////////////////////
// CMenuView diagnostics

#ifdef _DEBUG
void CMenuView::AssertValid() const
{
	CFormView::AssertValid();
}

void CMenuView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CMenuDoc* CMenuView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMenuDoc)));
    return (CMenuDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMenuView message handlers


BOOL CMenuView::PreTranslateMessage(MSG* pMsg) 
{
    if( m_ToolTipIsActive) {
		m_ToolTip.RelayEvent(pMsg);
	}
	
	return CFormView::PreTranslateMessage(pMsg);
}

BOOL CMenuView::OnEraseBkgnd(CDC* pDC)
{	
    if(!BackGroundFile) return CFormView::OnEraseBkgnd(pDC);

    CRect rtClient;
	GetClientRect(rtClient);
    m_imgBg.Show(pDC,rtClient);

	return TRUE;
}
