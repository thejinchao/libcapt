#ifndef __LIB_CAPT_GENERATOR_INCLUDE_H__
#define __LIB_CAPT_GENERATOR_INCLUDE_H__

namespace libCapt
{
//预定义
class FontFile;

/** 一份图灵测试数据
*/
struct Question
{
	/// 备选答案长度
	enum { ANASWER_LENGTH = 4 };			
	/// 图片尺寸
	enum { IMAGE_WIDTH=256, IMAGE_HEIGHT=64, IMAGE_PITCH=IMAGE_WIDTH>>1 };
	/// 图片需要的内存字节(未压缩状态)
	enum { IMAGE_BUF_LENGTH = IMAGE_PITCH*IMAGE_HEIGHT };
	/// 图片经过压缩标记
	enum { FLAG_RLE = 1<<16, };

	/// 是否是经过rle压缩
	inline bool isCompressed(void) const { return (nFlags & FLAG_RLE) !=0; }
	/// 获得在数据的长度，如果未经过压缩，大小是IMAGE_BUF_LENGTH
	inline unsigned int getSize(void) const { return nFlags&0xFFFF; }

	/// 数据flag, 低四位是图片内存长度，高四位是图片格式描述(是否压缩)
	unsigned int nFlags;
	/// 备选的四个答案
	unsigned short	wAnswer0[ANASWER_LENGTH];		//备选答案0
	unsigned short	wAnswer1[ANASWER_LENGTH];		//备选答案1
	unsigned short	wAnswer2[ANASWER_LENGTH];		//备选答案2
	unsigned short	wAnswer3[ANASWER_LENGTH];		//备选答案3
	/// 图片数据
	unsigned char imageBuf[IMAGE_BUF_LENGTH];

	//---------------------------
	//正确的答案编号[0, 1, 2, 3]
	int	nCorrectAnswer;
};

/** 生成一份图灵测试数据
*/
class Generator
{
public:
	/** 产生一个问题
	*/
	void generateQuestion(Question& question);

	/*****************************
		内部数据
	******************************/
private:
	//噪点数量
	enum { NOISE_POINT_COUNTS = 100, NOISE_CURVES_COUNTS = 10};
	//一次性产生足够的随机数，加快速度
	typedef unsigned char RandomBuf[Question::IMAGE_WIDTH];
	//图像缓冲区,为简化代码，每个像素1字节
	typedef unsigned char IMAGE_BUF[Question::IMAGE_WIDTH*Question::IMAGE_HEIGHT];
	//随机曲线数据
	enum { CURVES_LENGTH = 256 };
	static const char CURVES_DATA[CURVES_LENGTH];

	int _drawCharacter(int codeIndex, int x_pos, int y_pos, IMAGE_BUF& imageBuf, int _sk1, int _sk2);
	void _drawLine(IMAGE_BUF& im);
	void _drawDots(IMAGE_BUF& im);
	void _blur(IMAGE_BUF& im);
	void _fillRandCurves(IMAGE_BUF& imageBuf);
	void _generatorAnswer(Question& question, unsigned int* answerIndex);
	

private:
	FontFile* m_fontFile;	//!< 字体文件

public:
	Generator(FontFile* fontFile);
	~Generator();
};

}

#endif
