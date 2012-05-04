#ifndef __LIB_CAPT_FONT_FILE_INCLUDE_H__
#define __LIB_CAPT_FONT_FILE_INCLUDE_H__

namespace libCapt
{
/** 记录字体文件
*/
class FontFile
{
public:
	// 文件标识 'CAPF'
	enum { MAGIC_CODE = 0x46504143, };
	/** 字体数据文件
	*/
	struct Head
	{
		unsigned int dwMagicCode;		//'CAPF'
		unsigned int nCodeOffset;		//字符数据偏移
		unsigned int nCodeCounts;		//包含的字符数
		unsigned int nGlyphOffset;		//字符图片数据偏移
		unsigned int nGlyphCounts;		//包含的字符图像数
		unsigned int nGlyphSize;		//字符图片数据长度
	};

	/** 一个字符图形数据 */
	struct CodeGlyph
	{
		union
		{
			unsigned long long	nNextOffset;	//!< 下一张图片的偏移(相对于m_pGlyphBuf),考虑到64位系统下数据格式兼容，使用64位长度
			CodeGlyph*			pNext;			//!< 下一张图片的指针(运行时)
		};
		char				data[1];
	};

	/** 一个字符数据 */
	struct Code
	{
		unsigned short		wCode;			//!< UNICODE编码
		unsigned short		nGlyphCounts;	//!< 图片的数量
		union
		{
			unsigned long long	nFirstOffset;	//!< 第一张图片的偏移(相对于m_pGlyphBuf),考虑到64位系统下数据格式兼容，使用64位长度
			CodeGlyph*			pFirst;			//!< 第一张图片的指针(运行时)
		};
	};

	/** 字符数据特殊标记，换行符和结束符 */
	enum { GLPYH_EOL=-128, GLPYH_EOF=-127 };

public:
	/** 释放 */
	void release(void);
	/** 从数据中加载字体内容 */
	bool loadFromDataStream(const unsigned char* pStream, unsigned int streamSize);
	/** 得到所支持的字符数量 */
	unsigned int getCodeCounts(void) const { return m_fileHead.nCodeCounts; }
	/** 得到其中一个字符 */
	unsigned short getCodeFromIndex(unsigned int index) const;
	/** 通过索引获得某个字符的图形数据 */
	char* getCodeGlyphFromIndex(unsigned int index) const;

protected:
	/**  指针运行时化 */
	void _pointRuntime(void);

private:
	/** 内部函数，读取内存流 */
	bool _readStrem(unsigned char* dest, unsigned int size, const unsigned char*& pStream, const unsigned char* pStreamEnd);

protected:
	/// 文件头
	Head m_fileHead;
	/// 字符数据
	Code* m_pCodeBuf;
	/// 图片数据内存
	unsigned char* m_pGlyphBuf;

public:
	FontFile();
	virtual ~FontFile();
};

}

#endif
