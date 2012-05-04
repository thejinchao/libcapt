#pragma once


/** 编辑模式下的字体文件
*/
class FontFilePlus : public libCapt::FontFile
{
public:
	/** 创建空文件, 必须提前传入将要放入的字符数量 */
	bool createEmptyFile(unsigned int codeCounts);
	/** 加入一个字符图片(rgb24bit) */
	bool insertGlyph_RGB(unsigned short code, const unsigned char* pCanvasBitbuf, int canvasSize, unsigned int lPitch);
	/** 保存到文件 */
	bool saveToFile(const TCHAR* szFileName);

private:
	/** 调整图片数据内存 */
	void _increseGlyphBuf(unsigned int increseSize);
	/** 指针序列化 */
	void _pointSerial(void);

private:
	/// 图片数据内存不使用的一段内存长度，避免数据偏移出现0
	enum { GLYPHBUF_EMPTY_SIZE = 4 };

	/// 用于编辑状态填充时的code数据尾
	unsigned int m_nCodeTailOffset;
	/// 用于编辑状态填充时的glyph数据尾
	unsigned int m_nGlyphTailOffset;

public:
	FontFilePlus();
	virtual ~FontFilePlus();
};

