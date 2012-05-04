#pragma once

#include "resource.h"
#include "captUtil_FontGenerator.h"

class GeneratorDialog : public CDialogImpl<GeneratorDialog>
{
public:
	enum { IDD = IDD_DIALOG_GENERATOR };

	BEGIN_MSG_MAP(GeneratorDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BUTTON_FONT, OnButtonAddFont)
		COMMAND_ID_HANDLER(IDC_BUTTON_FONT_CLEAN, OnButtonCleanFont)
		COMMAND_ID_HANDLER(IDC_BUTTON_SAVE, OnButtonSave)
		COMMAND_ID_HANDLER(IDC_BUTTON_GENERATE, OnButtonGenerate)
		COMMAND_ID_HANDLER(IDC_BUTTON_CODE, OnButtonCodeFile)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnButtonAddFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonCleanFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonCodeFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return ::IsDialogMessage(m_hWnd, pMsg);
	}

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	FontGenerator m_fontGenerator;

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	GeneratorDialog();
	virtual ~GeneratorDialog();
};
