#pragma once

#include "captUtil_FontFile.h"

/** 用来生成字体文件
*/
class FontGenerator
{
public:
	/** 设置需要生成的字符, 返回有效字符个数
	*/
	int setValiableCodeFromFile(const TCHAR* szFileName, String& strErrMsg);
	/** 添加所使用的字体, 如果该字体已经有定义，返回false
	*/
	bool addFont(const LOGFONT& lf, String& strErrorMsg);
	/** 清空字体
	*/
	void clearFont(void);
	/** 开始生成数据
	*/
	bool generateFont(void);
	/** 获得文件
	*/
	FontFilePlus& getFontFile(void) { return m_fontFile; }

private:
	/// 画布的大小
	enum { CANVAS_SIZE = 48, };
	/// 使用的字体
	std::vector< std::pair< LOGFONT, HFONT > > m_allFont;
	/// 16位UNICODE总共的字符数量
	enum { CODE_COUNTS = 0x10000, };
	/// 需要产生的字符
	std::vector< unsigned short > m_allCode;
	/// 字体文件
	FontFilePlus m_fontFile;

public:
	FontGenerator();
	~FontGenerator();
};


