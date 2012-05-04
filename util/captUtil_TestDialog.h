#pragma once

#include "resource.h"

class CCanvasWindow;

class TestDialog : public CDialogImpl<TestDialog>
{
public:
	enum { IDD = IDD_DIALOG_TEST };

	BEGIN_MSG_MAP(TestDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BUTTON_RAND, OnButtonRand)
		COMMAND_ID_HANDLER(IDC_BUTTON_LOAD, OnButtonLoadFile)
		COMMAND_RANGE_HANDLER(IDC_RADIO_ANSWER_1, IDC_RADIO_ANSWER_4, OnButtonAnswer)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnButtonRand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonLoadFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonAnswer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return ::IsDialogMessage(m_hWnd, pMsg);
	}

	/*************************************************************************
			Implementation Methods
	*************************************************************************/
private:
	static LRESULT CALLBACK _resultWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	WNDPROC m_resultWndProc;
	CCanvasWindow* m_canvasWindow;			//!< Canvas window 
	libCapt::FontFile m_fontFile;
	libCapt::Question m_question;
	HICON m_hCheckMarkIcon;
	HICON m_hCrossMarkIcon;
	int m_resultState;  //0: none, 1:check 2:cross

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	TestDialog();
	virtual ~TestDialog();
};
