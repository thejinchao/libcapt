#include "stdafx.h"
#include "captUtil_FontGenerator.h"

//--------------------------------------------------------------------------
FontGenerator::FontGenerator()
{
}

//--------------------------------------------------------------------------
FontGenerator::~FontGenerator()
{

}


//--------------------------------------------------------------------------
int FontGenerator::setValiableCodeFromFile(const TCHAR* szFileName, String& strErrMsg)
{
	const unsigned short INVALID_CODE[] = 
	{
		L'\0', L'\r', L'\n', L'\t', L' ',
	};
	const unsigned short* INVALID_CODE_END = INVALID_CODE+(sizeof(INVALID_CODE)/sizeof(unsigned short));

	strErrMsg.clear();
	m_allCode.clear();

	FILE* fp = _tfopen(szFileName, _T("rb"));
	if(!fp)
	{
		strErrMsg = _T("Can't open file!");
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//必须是偶数
	if((fileSize&1) != 0) fileSize++;

	unsigned char* pBuf = new unsigned char[fileSize+2];
	memset(pBuf, 0, fileSize+2);
	fread(pBuf, 1, fileSize, fp);
	fclose(fp); fp=0;

	const unsigned short* pwchar_buf = (const unsigned short*)pBuf;

	if(pwchar_buf[0] != 0xFEFF)
	{
		strErrMsg = _T("Must UNICODE(16bits) file!");
		delete[] pBuf; return 0;
	}

	std::vector< bool > tempCodeBuf;
	tempCodeBuf.resize(CODE_COUNTS, false);
	for(int i=1; i<fileSize/2; i++)
	{
		unsigned short code = pwchar_buf[i];

		//如果是非法字符，忽略
		if(INVALID_CODE_END != std::find(INVALID_CODE, INVALID_CODE_END, code)) continue;

		tempCodeBuf[code]=true;
	}

	//统计字数
	for(int i=0; i<CODE_COUNTS; i++)
	{
		if(tempCodeBuf[i]) m_allCode.push_back((unsigned short)i);
	}

	delete[] pBuf; pBuf = 0;
	return true;
}

//--------------------------------------------------------------------------
bool FontGenerator::addFont(const LOGFONT& lf, String& strErrMsg)
{
	strErrMsg.clear();

	//检查是否已经加入
	for(int i=0; i<(int)m_allFont.size(); i++)
	{
		const LOGFONT& font = m_allFont[i].first;
		if(memcmp(&font, &lf, sizeof(LOGFONT)) == 0)
		{
			strErrMsg = _T("Multi define font!");
			return false;
		}
	}

	//创建字体
	HFONT newFont = ::CreateFontIndirect(&lf);
	if(!newFont)
	{
		TCHAR szTemp[64] = {0};
		_sntprintf(szTemp, 64, _T("Create font error:%08X"), ::GetLastError());
		strErrMsg = szTemp;
		return false;
	}

	m_allFont.push_back(std::make_pair(lf, newFont));

	return true;
}

//--------------------------------------------------------------------------
void FontGenerator::clearFont(void)
{
	m_allFont.clear();
}

//--------------------------------------------------------------------------
bool FontGenerator::generateFont(void)
{
	if(m_allFont.empty() || m_allCode.empty()) return false;

	//生成临时的Bitmap
	const int canvasPitch = CANVAS_SIZE*3;
	const unsigned char* pCanvasBitbuf = 0;

	BITMAPINFO bmpInfo; 
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize		= sizeof (BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth		= CANVAS_SIZE; 
	bmpInfo.bmiHeader.biHeight		= -CANVAS_SIZE;
	bmpInfo.bmiHeader.biPlanes		= 1;
	bmpInfo.bmiHeader.biBitCount	= 24;
	bmpInfo.bmiHeader.biSizeImage	= canvasPitch * CANVAS_SIZE;

	HDC hScreenDC = GetDC(GetDesktopWindow());
	HBITMAP hCanvasBmp = CreateDIBSection(hScreenDC, &bmpInfo, DIB_RGB_COLORS, (VOID**)&(pCanvasBitbuf), 0, 0);
	if(hCanvasBmp==0)
	{
		//Create DIB error
		return false;
	}

	//创建Canvas DC
	HDC hCanvasDC = ::CreateCompatibleDC(hScreenDC);
	HBITMAP hOldBmp = (HBITMAP)::SelectObject(hCanvasDC, hCanvasBmp);

	//释放临时dc
	ReleaseDC(GetDesktopWindow(), hScreenDC);	hScreenDC = 0;

	//设置dc参数
	::SetTextColor(hCanvasDC, RGB(255, 255, 255));
	::SetBkColor(hCanvasDC, RGB(0, 0, 0));

	//Canvas 大小
	RECT rect;
	::SetRect(&rect, 0, 0, CANVAS_SIZE, CANVAS_SIZE);

	//准备字体文件
	m_fontFile.createEmptyFile((unsigned int)m_allCode.size());

	//查询所有字符
	for(unsigned int i=0; i<(unsigned int)m_allCode.size(); i++)
	{
		//查询所有字体
		for(int j=0; j<(int)m_allFont.size(); j++)
		{
			HFONT hFont = m_allFont[j].second;
			HFONT hOldFont = (HFONT)::SelectObject(hCanvasDC, hFont);

			//清空
			::FillRect(hCanvasDC, &rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));

			//画字符
			unsigned short tempBuf[2] = { m_allCode[i], 0 };

			::DrawTextW(hCanvasDC, (LPCWSTR)tempBuf, 1, &rect, DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);

			//恢复旧字体
			::SelectObject(hCanvasDC, hOldFont);

			//保存点阵数据
			m_fontFile.insertGlyph_RGB(m_allCode[i], pCanvasBitbuf, CANVAS_SIZE, canvasPitch);
		}
	}

	//释放DIB对象
	::SelectObject(hCanvasDC, hOldBmp);
	::DeleteDC(hCanvasDC);

	return true;

}

