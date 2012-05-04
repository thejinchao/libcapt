#pragma once

#include "Resource.h"
#include "captUtil_GeneratorDialog.h"
#include "captUtil_TestDialog.h"
#include "WTLTabViewCtrl.h"

//pre-define class
class CCanvasWindow;

/** Main Function Dialog
*/
class CMainDialog : public CDialogImpl< CMainDialog >
{
	/*************************************************************************
			Public Methods
	*************************************************************************/
public:

	/*************************************************************************
			Inherit Methods
	*************************************************************************/
public:
	enum { IDD = IDD_DIALOG_MAIN };

	BEGIN_MSG_MAP(CMainDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	/*************************************************************************
			Implementation Methods
	*************************************************************************/
private:
	static LRESULT CALLBACK _resultWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	WNDPROC m_resultWndProc;
	CWTLTabViewCtrl m_tabMain;
	GeneratorDialog m_generatorDlg;
	TestDialog m_testDlg;

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	CMainDialog();
	~CMainDialog();
};
