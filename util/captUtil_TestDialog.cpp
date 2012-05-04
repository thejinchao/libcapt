#include "stdafx.h"
#include "captUtil_TestDialog.h"
#include "captUtil_CanvasWnd.h"

//--------------------------------------------------------------------------------------------
TestDialog::TestDialog()
	: m_canvasWindow(0)
	, m_hCheckMarkIcon(0)
	, m_hCrossMarkIcon(0)
	, m_resultState(0)
	, m_resultWndProc(0)
{
	m_question.nCorrectAnswer = -1;
}

//--------------------------------------------------------------------------------------------
TestDialog::~TestDialog()
{
}

//--------------------------------------------------------------------------------------------
LRESULT TestDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Create canvas
	HWND hCanvasParent = ::GetDlgItem(m_hWnd, IDC_CANVAS);

	RECT rect;
	::GetClientRect(hCanvasParent, &rect);

	m_canvasWindow = new CCanvasWindow;
	m_canvasWindow->Create(hCanvasParent, rect, _T("Canvas"), WS_CHILD);
	m_canvasWindow->ShowWindow(SW_SHOW);

	::SetWindowLong(GetDlgItem(IDC_STATIC_RESULT), GWL_USERDATA, (LONG)(LONG_PTR)(this));
	m_resultWndProc = (WNDPROC)(LONG_PTR)::SetWindowLong(GetDlgItem(IDC_STATIC_RESULT), GWL_WNDPROC, (LONG)(LONG_PTR)_resultWndProc);

	HINSTANCE hInstance = GetModuleHandle(0);
	m_hCheckMarkIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_CHECKMARK), IMAGE_ICON, 
				48, 48, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
	m_hCrossMarkIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_CROSSMARK), IMAGE_ICON, 
				48, 48, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);

	return (LRESULT)TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT CALLBACK TestDialog::_resultWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TestDialog *pDialog = (TestDialog *)(LONG_PTR)::GetWindowLong(hWnd, GWL_USERDATA);
	switch(uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hWnd, &ps);

			RECT rect;
			::GetClientRect(hWnd, &rect);

			HBRUSH hSolidBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			::FillRect(hdc, &rect, hSolidBrush);

			switch(pDialog->m_resultState)
			{
			case 1:
				DrawIconEx(hdc, 0, 0, pDialog->m_hCheckMarkIcon, 48, 48, 0, NULL, DI_NORMAL);
				break;
			case 2:
				DrawIconEx(hdc, 0, 0, pDialog->m_hCrossMarkIcon, 48, 48, 0, NULL, DI_NORMAL);
				break;

			default: break;
			}

			DeleteObject(hdc);
			::EndPaint(hWnd, &ps);
			return TRUE;
		}
		break;
	default: break;
	}
	return pDialog->m_resultWndProc(hWnd, uMsg, wParam, lParam);
}

//--------------------------------------------------------------------------------------------
LRESULT TestDialog::OnButtonRand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	libCapt::Generator gen(&m_fontFile);
	gen.generateQuestion(m_question);

	//显示备选答案
	unsigned short* answerTxt[4] = {m_question.wAnswer0, m_question.wAnswer1, m_question.wAnswer2, m_question.wAnswer3};
	unsigned int idAnswer[4] = {IDC_RADIO_ANSWER_1, IDC_RADIO_ANSWER_2, IDC_RADIO_ANSWER_3, IDC_RADIO_ANSWER_4};
	
	for(int i=0; i<4; i++)
	{
		wchar_t wszTemp[32]={0};
		memcpy(wszTemp, answerTxt[i], sizeof(unsigned short)*libCapt::Question::ANASWER_LENGTH);

#ifdef UNICODE
		SetDlgItemText(idAnswer[i], wszTemp);
#else
		char szTemp[32]={0};
		WideCharToMultiByte(CP_ACP, 0, wszTemp, libCapt::Question::ANASWER_LENGTH+1, szTemp, 32, 0, 0);
		SetDlgItemText(idAnswer[i], szTemp);
#endif
	}

	CheckRadioButton(idAnswer[0], idAnswer[3], 0);
	m_resultState = 0;

	// 显示大小
	TCHAR temp[32];
	_sntprintf(temp, 32, _T("ImageSize: %.2f kb"), (float)(m_question.nFlags&0xFFFF)/1024.f);
	SetDlgItemText(IDC_STATIC_SIZE, temp);

	//更新图像
	m_canvasWindow->setQuestion(&m_question);
	m_canvasWindow->redraw();

	//更新结果
	::InvalidateRect(GetDlgItem(IDC_STATIC_RESULT), 0, TRUE);
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT TestDialog::OnButtonLoadFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CFileDialog fileDlg(TRUE, _T("Captcha Font File"), 0, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
		_T("Captcha Font File(*.cpf)\0*.cpf\0All files(*.*)\0*.*\0\0"), m_hWnd);
	if(IDOK != fileDlg.DoModal()) return 0;

	FILE* fp = _tfopen(fileDlg.m_szFileName, _T("rb"));
	fseek(fp, 0, SEEK_END);
	unsigned int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	unsigned char* streamBuf = new unsigned char[fileSize];
	fread(streamBuf, 1, fileSize, fp);
	fclose(fp);

	if(!(m_fontFile.loadFromDataStream(streamBuf, fileSize)))
	{
		delete[] streamBuf;
		return 0;
	}

	delete[] streamBuf;
	::EnableWindow(GetDlgItem(IDC_BUTTON_RAND), TRUE);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT TestDialog::OnButtonAnswer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int answerIndex = wID-IDC_RADIO_ANSWER_1;
	if(m_question.nCorrectAnswer<0) return 0;
	if(m_question.nCorrectAnswer==answerIndex)
	{
		m_resultState = 1;
	}
	else
	{
		m_resultState = 2;
	}
	::InvalidateRect(GetDlgItem(IDC_STATIC_RESULT), 0, TRUE);
	return 0;
}
