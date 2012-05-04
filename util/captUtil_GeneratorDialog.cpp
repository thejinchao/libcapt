#include "stdafx.h"
#include "captUtil_GeneratorDialog.h"

//--------------------------------------------------------------------------------------------
GeneratorDialog::GeneratorDialog()
{
}

//--------------------------------------------------------------------------------------------
GeneratorDialog::~GeneratorDialog()
{
}

//--------------------------------------------------------------------------------------------
LRESULT GeneratorDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hList = GetDlgItem(IDC_LIST_FONT);

	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = -1;

	lvc.pszText = (LPTSTR)_T("FontName");
	lvc.cx = 150;
	ListView_InsertColumn(hList, 0, &lvc);

	lvc.pszText = (LPTSTR)_T("Width");
	lvc.cx = 50;
	ListView_InsertColumn(hList, 1, &lvc);

	return (LRESULT)TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT GeneratorDialog::OnButtonGenerate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow(GetDlgItem(IDC_BUTTON_SAVE), FALSE);
	if(m_fontGenerator.generateFont())
	{
		::EnableWindow(GetDlgItem(IDC_BUTTON_SAVE), TRUE);
	}
	else
	{
		MessageBox(_T("Must set font and code file first!"), _T("CaptchaUtil"), MB_OK|MB_ICONSTOP);
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT GeneratorDialog::OnButtonAddFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	HFONT hDefaultFont = (HFONT)SendMessage(m_hWnd, WM_GETFONT, 0, 0);
	if(hDefaultFont!=0)
	{
		GetObject(hDefaultFont, sizeof(LOGFONT), &lf);
	}

	lf.lfHeight = -35;  //default size

	CFontDialog dlg(&lf);
	if(IDOK != dlg.DoModal()) return 0;

	dlg.GetCurrentFont(&lf);

	String strError;
	if(!m_fontGenerator.addFont(lf, strError))
	{
		MessageBox(strError.c_str(), _T("Captcha Util"), MB_OK|MB_ICONSTOP);
		return 0;
	}

	HWND hList = GetDlgItem(IDC_LIST_FONT);
	int fontCounts = ListView_GetItemCount(hList);

	TCITEM tci = { 0 };

	int item = ListView_InsertItem(hList, &tci);
	ListView_SetItemText(hList, item, 0, lf.lfFaceName);

	TCHAR szFontSize[32]={0};
	_sntprintf(szFontSize, 32, _T("%d"), -lf.lfHeight);
	ListView_SetItemText(hList, item, 1, szFontSize);

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT GeneratorDialog::OnButtonCodeFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CFileDialog fileDlg(TRUE, _T("Character file"), 0, OFN_HIDEREADONLY, 
		_T("Character text file(*.txt)\0*.txt\0All files(*.*)\0*.*\0\0"), m_hWnd);
	if(IDOK != fileDlg.DoModal()) return 0;

	SetDlgItemText(IDC_EDIT_CODE_FILE, fileDlg.m_szFileName);

	String strErrorMsg;
	if(!m_fontGenerator.setValiableCodeFromFile(fileDlg.m_szFileName, strErrorMsg))
	{
		MessageBox(strErrorMsg.c_str(), _T("Captcha Util"), MB_OK|MB_ICONSTOP);
		return 0;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT GeneratorDialog::OnButtonSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CFileDialog fileDlg(FALSE, _T("Captcha Font File"), 0, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
		_T("Captcha Font File(*.cpf)\0*.cpf\0All files(*.*)\0*.*\0\0"), m_hWnd);
	if(IDOK != fileDlg.DoModal()) return 0;

	if(!m_fontGenerator.getFontFile().saveToFile(fileDlg.m_szFileName))
	{
		return 0;
	}

	MessageBox(_T("Done!"), _T("CaptchaUtil"), MB_OK|MB_ICONINFORMATION);

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT GeneratorDialog::OnButtonCleanFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	HWND hList = GetDlgItem(IDC_LIST_FONT);
	ListView_DeleteAllItems(hList);

	m_fontGenerator.clearFont();
	return 0;
}
