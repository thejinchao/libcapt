/*
  http://brokestream.com/captcha.html

  Copyright (C) 2009 Ivan Tikhonov

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Ivan Tikhonov, kefeer@brokestream.com
*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "captGenerator.h"
#include "captRLE.h"
#include "captFontFile.h"

namespace libCapt
{

//-------------------------------------------------------------------------------------------------
#define MAX(x,y)				((x>y)?(x):(y))
#define RAND					rand
#define RAND_RANGE(_min, _max)	RAND()%(_max-_min) + _min

//-------------------------------------------------------------------------------------------------
const char Generator::CURVES_DATA[Generator::CURVES_LENGTH] = 
{0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,48,51,54,57,59,62,65,67,70,73,75,78,80,82,85,87,89,91,94,96,98,100,102,103,105,107,108,110,112,113,114,116,117,118,119,120,121,122,123,123,124,125,125,126,126,126,126,126,127,126,126,126,126,126,125,125,124,123,123,122,121,120,119,118,117,116,114,113,112,110,108,107,105,103,102,100,98,96,94,91,89,87,85,82,80,78,75,73,70,67,65,62,59,57,54,51,48,45,42,39,36,33,30,27,24,21,18,15,12,9,6,3,0,-3,-6,-9,-12,-15,-18,-21,-24,-27,-30,-33,-36,-39,-42,-45,-48,-51,-54,-57,-59,-62,-65,-67,-70,-73,-75,-78,-80,-82,-85,-87,-89,-91,-94,-96,-98,-100,-102,-103,-105,-107,-108,-110,-112,-113,-114,-116,-117,-118,-119,-120,-121,-122,-123,-123,-124,-125,-125,-126,-126,-126,-126,-126,-127,-126,-126,-126,-126,-126,-125,-125,-124,-123,-123,-122,-121,-120,-119,-118,-117,-116,-114,-113,-112,-110,-108,-107,-105,-103,-102,-100,-98,-96,-94,-91,-89,-87,-85,-82,-80,-78,-75,-73,-70,-67,-65,-62,-59,-57,-54,-51,-48,-45,-42,-39,-36,-33,-30,-27,-24,-21,-18,-15,-12,-9,-6,-3,};

//-------------------------------------------------------------------------------------------------
Generator::Generator(FontFile* fontFile)
	: m_fontFile(fontFile)
{
	assert(m_fontFile);
}

//-------------------------------------------------------------------------------------------------
Generator::~Generator()
{
}

//-------------------------------------------------------------------------------------------------
int Generator::_drawCharacter(int codeIndex, int x_pos, int y_pos, IMAGE_BUF& imageBuf, int _sk1, int _sk2)
{
	unsigned char *line_begin = imageBuf + Question::IMAGE_WIDTH*y_pos;
	unsigned char *line_start = imageBuf + Question::IMAGE_WIDTH*y_pos + x_pos;

	unsigned char *i=line_start;
	int sk1 = _sk1 + x_pos;
	int sk2 = _sk2 + x_pos;
	int mpos = x_pos;
	int row=0;
	for(char *fntGlyph = m_fontFile->getCodeGlyphFromIndex(codeIndex); *fntGlyph!=FontFile::GLPYH_EOF; fntGlyph++) 
	{
		if(*fntGlyph<0) 
		{
			if(*fntGlyph==FontFile::GLPYH_EOL) 
			{ 
				//换行符
				line_begin += Question::IMAGE_WIDTH;
				line_start += Question::IMAGE_WIDTH;
				i=line_start; 
				sk1 = _sk1 + x_pos; 
				row++; 
			}
			else
			{
				//横向偏移
				i+=-*fntGlyph;
			}
			continue;
		}

		if(sk1>=CURVES_LENGTH) sk1=sk1 % CURVES_LENGTH;
		int skew = CURVES_DATA[sk1]>>4;
		sk1+=RAND_RANGE(1, 2);

		if(sk2>=CURVES_LENGTH) sk2=sk2 % CURVES_LENGTH;
		int skewh = CURVES_DATA[sk2]>>6;
		sk2+=RAND()&0x1;

		unsigned char *x= i + skew*Question::IMAGE_WIDTH + skewh;
		mpos=MAX(mpos, x_pos+(int)(i-line_start));
		
		if(	(x-imageBuf)<Question::IMAGE_HEIGHT*Question::IMAGE_WIDTH && (i-line_begin+skewh)<Question::IMAGE_WIDTH)
		{
			*x=(*(unsigned char*)fntGlyph)<<4;
		}
		i++;
	}
	return mpos;
}


//-------------------------------------------------------------------------------------------------
void Generator::_drawLine(IMAGE_BUF& imageBuf) 
{
	int sk = RAND() & 0X7F;

	int line_start_y = RAND_RANGE(20, Question::IMAGE_HEIGHT-20);
	for(int x=0; x<Question::IMAGE_WIDTH-1; x++) 
	{
		if(sk>=CURVES_LENGTH) sk=sk%CURVES_LENGTH;
		int skew=CURVES_DATA[sk]>>4;
		sk+=(RAND()&1) ? 0 : 5;

		unsigned char *i= imageBuf + (Question::IMAGE_WIDTH*(line_start_y+skew)+x);

		i[0]=0; i[1]=0;
		i[Question::IMAGE_WIDTH]=0; i[Question::IMAGE_WIDTH+1]=0;
	}
}

//-------------------------------------------------------------------------------------------------
void Generator::_fillRandCurves(IMAGE_BUF& imageBuf)
{
	for(int n=0; n<NOISE_CURVES_COUNTS; n++)
	{
		int rand_x = RAND_RANGE(5, Question::IMAGE_WIDTH-10);
		int rand_y = RAND_RANGE(5, Question::IMAGE_HEIGHT-10);
		int len = RAND_RANGE(2, 7);
		int sk = RAND()&0x3f;

		for(int ii=0; ii<len; ii++)
		{
			if(sk>=CURVES_LENGTH) sk=sk%CURVES_LENGTH;
			int skew=CURVES_DATA[sk]>>4;
			sk+=(RAND()&1) ? 0 : 5;

			unsigned char *i= imageBuf + (Question::IMAGE_WIDTH*(rand_y+skew)+rand_x+ii);
			if((i-imageBuf)<Question::IMAGE_HEIGHT*Question::IMAGE_WIDTH)
			{
				i[0]=0;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void Generator::_drawDots(IMAGE_BUF& imageBuf) 
{
	for(int n=0; n<NOISE_POINT_COUNTS; n++) 
	{
		unsigned int v = (unsigned int)RAND();
		unsigned char *i = imageBuf + v%(Question::IMAGE_WIDTH*(Question::IMAGE_HEIGHT-3));
		
		i[0]=0xff;
		i[1]=0xff;
		i[2]=0xff;
		i[Question::IMAGE_WIDTH+0]=0xff;
		i[Question::IMAGE_WIDTH+1]=0xff;
		i[Question::IMAGE_WIDTH+2]=0xff;
	}
}

//-------------------------------------------------------------------------------------------------
void Generator::_blur(IMAGE_BUF& imageBuf)
{
	unsigned char *i = imageBuf;
	for(int y=0;y<Question::IMAGE_HEIGHT-2;y++) 
	{
		for(int x=0;x<Question::IMAGE_WIDTH-2;x++) 
		{
			unsigned int c11=*i,c12=i[1];
			unsigned int c21=i[Question::IMAGE_WIDTH],c22=i[Question::IMAGE_WIDTH+1];

			//fetch 2x2
			*i++=((c11+c12+c21+c22)/4);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void Generator::_generatorAnswer(Question& question, unsigned int* answerIndex)
{
	//总共可以使用的字符数量
	int totalFontCounts = m_fontFile->getCodeCounts();
	const int codeCounts = Question::ANASWER_LENGTH*4;
	int codeIndex[codeCounts] = {0};

	for(int i=0; i<codeCounts; i++)
	{
		//产生一个不重复的索引
		int counts=0;
		int answer= 0;
		do
		{
			answer = RAND()%totalFontCounts;
			//检查重复
			bool same=false;
			for(int j=0; j<i; j++)
			{
				if(answer == codeIndex[j])
				{
					same=true;
					break;
				}
			}
			counts++;
			if(!same) break;
		}while(counts<1000); //防止死循环

		codeIndex[i] = answer;
	}

	//随即挑选一个座位， 正确的答案
	question.nCorrectAnswer = RAND()%4;

	//获取，四个答案所代表的字符串
	for(int i=0; i<4; i++)
	{
		question.wAnswer0[i] = m_fontFile->getCodeFromIndex(codeIndex[i*4+0]);
		question.wAnswer1[i] = m_fontFile->getCodeFromIndex(codeIndex[i*4+1]);
		question.wAnswer2[i] = m_fontFile->getCodeFromIndex(codeIndex[i*4+2]);
		question.wAnswer3[i] = m_fontFile->getCodeFromIndex(codeIndex[i*4+3]);

		answerIndex[i] = codeIndex[i*4+question.nCorrectAnswer];
	}
}

//-------------------------------------------------------------------------------------------------
void Generator::generateQuestion(Question& question)
{
	//临时图像缓冲区
	IMAGE_BUF imageBuf;
	//以白色填充背景
	memset(imageBuf, 0xff, sizeof(imageBuf)); 

	//生成字符索引
	unsigned answerIndex[Question::ANASWER_LENGTH];
	_generatorAnswer(question, answerIndex);

	//起始x坐标
	int x_offset=RAND_RANGE(20, Question::IMAGE_WIDTH-180);
	//起始y坐标, 8, 16
	int y_offset = RAND_RANGE(8, Question::IMAGE_HEIGHT-40);
	//用于扭曲效果的随机数
	int sk1 = RAND() & 0x7f; 
	int sk2 = RAND() & 0x3f; 

	for(int i=0; i<Question::ANASWER_LENGTH; i++)
	{
		x_offset = _drawCharacter(answerIndex[i], x_offset, y_offset, imageBuf, sk1, sk2); 
	}

	_drawLine(imageBuf);
	_drawDots(imageBuf); 
	_fillRandCurves(imageBuf);
	_blur(imageBuf);

	//拷贝
	unsigned char* d = question.imageBuf;
	unsigned short* s = (unsigned short*)imageBuf;
	for(int i=0; i<Question::IMAGE_BUF_LENGTH; i++, s++, d++)
	{
		//两个像素一次拷贝
		unsigned short sc = *s;
		*d =  (((sc>>4)&0x0F)<<4) | ((sc>>12)&0x0F);
	}

	//-----------------------------
	//RLE COMPRESS
	unsigned int buf_size = Question::IMAGE_BUF_LENGTH;
	unsigned char rleBuf[Question::IMAGE_BUF_LENGTH];
	if(rleCompress(question.imageBuf, Question::IMAGE_WIDTH, Question::IMAGE_HEIGHT, rleBuf, buf_size))
	{
		//压缩成功
		memcpy(question.imageBuf, rleBuf, buf_size);
		question.nFlags = buf_size&0xFFFF | Question::FLAG_RLE;
	}
	else
	{
		question.nFlags = buf_size&0xFFFF;
	}
}


}
