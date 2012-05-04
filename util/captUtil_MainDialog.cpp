#include "stdafx.h"
#include "captUtil_MainDialog.h"
#include "captUtil_CanvasWnd.h"
#include "captUtil_FontGenerator.h"

//--------------------------------------------------------------------------------------------
CMainDialog::CMainDialog()
{
}

//--------------------------------------------------------------------------------------------
CMainDialog::~CMainDialog()
{
}

//--------------------------------------------------------------------------------------------
LRESULT CMainDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Center window
	CenterWindow(GetParent());

	RECT rectParent;
	GetClientRect(&rectParent);
	m_tabMain.Create(m_hWnd, rectParent, NULL, WS_CHILD|WS_VISIBLE);

	m_testDlg.Create(m_tabMain.m_hWnd, rcDefault);
	m_tabMain.AddTab(_T("Test"), m_testDlg.m_hWnd, TRUE);

	m_generatorDlg.Create(m_tabMain.m_hWnd, rcDefault);
	m_tabMain.AddTab(_T("Generator"), m_generatorDlg.m_hWnd, FALSE);

	return (LRESULT)TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT CMainDialog::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

