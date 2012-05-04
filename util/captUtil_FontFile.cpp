#include "stdafx.h"
#include "captUtil_FontFile.h"


//--------------------------------------------------------------------------
FontFilePlus::FontFilePlus()
{
}

//--------------------------------------------------------------------------
FontFilePlus::~FontFilePlus()
{
}

//--------------------------------------------------------------------------
bool FontFilePlus::createEmptyFile(unsigned int codeCounts)
{
	if(codeCounts<=0 || codeCounts>=0xFFFF) return false;

	//释放旧数据
	release();

	//初始化数据
	m_fileHead.dwMagicCode = MAGIC_CODE;
	m_fileHead.nCodeCounts = codeCounts;

	//分配Code空间
	m_pCodeBuf = new Code[codeCounts];
	memset(m_pCodeBuf, 0, sizeof(Code)*codeCounts);
	m_nCodeTailOffset = 0;

	//预分配1k内存
	m_fileHead.nGlyphSize = 1024;
	m_pGlyphBuf = new unsigned char[m_fileHead.nGlyphSize];
	memset(m_pGlyphBuf, 0, m_fileHead.nGlyphSize);
	m_nGlyphTailOffset = GLYPHBUF_EMPTY_SIZE; //内存前4个字节不使用,避免图形数据偏移出现0

	return true;
}

//--------------------------------------------------------------------------
void FontFilePlus::_increseGlyphBuf(unsigned int increseSize)
{
	//剩余内存是否足够
	unsigned int glyphSize = m_fileHead.nGlyphSize;
	if(glyphSize!=0 && m_nGlyphTailOffset+increseSize<glyphSize) return;

	//按照内存加倍方式分配新的内存
	unsigned int nNewLength = glyphSize*2;
	while(nNewLength<m_nGlyphTailOffset+increseSize) nNewLength*=2;
	unsigned char* pGlyphBuf = new unsigned char[nNewLength];

	//指针数据静态化
	_pointSerial();

	//复制数据
	memset(pGlyphBuf, 0, nNewLength);
	if(glyphSize > 0) memcpy(pGlyphBuf, m_pGlyphBuf, glyphSize);

	//释放旧内存
	if(m_pGlyphBuf) delete[] m_pGlyphBuf;
	m_pGlyphBuf = pGlyphBuf;
	m_fileHead.nGlyphSize = nNewLength;

	//指针数据运行时化
	_pointRuntime();
}


//--------------------------------------------------------------------------
void FontFilePlus::_pointSerial(void)
{
	for(unsigned int i=0; i<m_fileHead.nCodeCounts; i++)
	{
		Code& code = m_pCodeBuf[i];
		if(code.wCode==0 || code.nGlyphCounts==0) continue;  //尚未使用

		CodeGlyph* codeGlyph = code.pFirst;
		while(codeGlyph->pNext !=0)
		{
			CodeGlyph* next = codeGlyph->pNext;
			codeGlyph->nNextOffset = (unsigned long long)((unsigned char*)next - m_pGlyphBuf);
			codeGlyph = next;
		}

		code.nFirstOffset = (unsigned long long)((unsigned char*)code.pFirst - m_pGlyphBuf);
	}
}

//--------------------------------------------------------------------------
bool FontFilePlus::insertGlyph_RGB(unsigned short code, const unsigned char* pCanvasBitbuf, int canvasSize, unsigned int lPitch)
{
	//查找Code信息
	Code* codeInfo = 0;
	for(unsigned int i=0; i<m_nCodeTailOffset; i++)
	{
		if(m_pCodeBuf[i].wCode==code) 
		{
			codeInfo = m_pCodeBuf+i;
			break;
		}
	}
	if(codeInfo==0 && m_nCodeTailOffset<m_fileHead.nCodeCounts)
	{
		codeInfo = m_pCodeBuf+(m_nCodeTailOffset++);
		codeInfo->wCode = code;
	}
	if(codeInfo==0)
	{
		//code已经塞满，错误
		return false;
	}

	//字体图像信息临时内存
	char tempMemory[1024*2]={0};
	int tempMemSize = 0;

	//生成数据
	for(int i=0; i<canvasSize; i++)
	{
		int blank_size = 0;
		const unsigned char* p = (const unsigned char*)pCanvasBitbuf + lPitch*i;
		for(int j=0; j<canvasSize; j++)
		{
			unsigned char c = *p;
			if(c==0) blank_size++;
			else
			{
				if(blank_size!=0)
				{
					tempMemory[tempMemSize++] = (char)(-blank_size);
					blank_size=0;
				}
				tempMemory[tempMemSize++] = ((unsigned char)(255-c))>>4;
			}
			p+=3;
		}
		tempMemory[tempMemSize++] = GLPYH_EOL;
	}

	//生成文件结束符(定位最后一个行结束符)
	while(tempMemSize>0 && tempMemory[tempMemSize-1]==GLPYH_EOL) tempMemSize--;
	tempMemory[tempMemSize++]=GLPYH_EOF;

	//内存调整
	_increseGlyphBuf(tempMemSize+sizeof(CodeGlyph));

	//加入
	CodeGlyph codeGlyph;
	memset(&codeGlyph, 0, sizeof(codeGlyph));

	//调整链表数据
	if(codeInfo->nGlyphCounts == 0)
	{
		//第一个字符
		codeGlyph.pNext=0;
		codeInfo->pFirst = (CodeGlyph*)(m_pGlyphBuf+m_nGlyphTailOffset);
	}
	else
	{
		//插入链表
		CodeGlyph* firstNow = codeInfo->pFirst;
		codeGlyph.pNext=firstNow;
		codeInfo->pFirst = (CodeGlyph*)(m_pGlyphBuf+m_nGlyphTailOffset);
	}

	int data_offset = (int)((unsigned char*)&(codeGlyph.data)-(unsigned char*)(&codeGlyph));
	memcpy(m_pGlyphBuf+m_nGlyphTailOffset, &codeGlyph, sizeof(codeGlyph)); m_nGlyphTailOffset+=sizeof(codeGlyph)-data_offset;
	memcpy(m_pGlyphBuf+m_nGlyphTailOffset, tempMemory, tempMemSize); m_nGlyphTailOffset+=tempMemSize;

	//图像计数增加
	codeInfo->nGlyphCounts += 1;
	m_fileHead.nGlyphCounts += 1;

	return true;
}

//--------------------------------------------------------------------------
bool FontFilePlus::saveToFile(const TCHAR* szFileName)
{
	//修改文件头
	m_fileHead.nCodeOffset = sizeof(m_fileHead);
	m_fileHead.nCodeCounts = m_nCodeTailOffset;
	m_fileHead.nGlyphOffset = m_fileHead.nCodeOffset+sizeof(Code)*m_fileHead.nCodeCounts;
	m_fileHead.nGlyphSize = m_nGlyphTailOffset;
/*
	if(debug)
	{
		FILE* fp = fopen(szFileName, "w");
		fprintf(fp, "codeCounts = %d\n", m_fileHead.nCodeCounts);
		fprintf(fp, "glyphCounts = %d\n", m_fileHead.nGlyphCounts);

		for(unsigned int i=0; i<m_fileHead.nCodeCounts; i++)
		{
			Code& code = m_pCodeBuf[i];
			if(code.wCode==0 || code.nGlyphCounts==0) continue;  //尚未使用

			fprintf(fp, "-------%d--------\n", code.wCode);
			CodeGlyph* glyph = code.pFirst;
			while(glyph!=0)
			{
				char* data = (char*)(glyph->data);
				for(;*data!=GLPYH_EOF;data++)
				{
					char c = *data;
					if(c<0) 
					{
						if(c == GLPYH_EOL) 
						{ 
							fprintf(fp, "\n");
						}
						else
						{
							for(char i=0; i<-c; i++) fprintf(fp, "_");
						}
					}
					else
					{
						fprintf(fp, "*");
					}
				};
				fprintf(fp, "\n===\n");
				glyph = glyph->pNext;
			}
		}
		fclose(fp);
	}
	else
//*/
	{
		//输出到文件
		FILE* fp = _tfopen(szFileName, _T("wb"));
		if(!fp) return false;

		//指针序列化
		_pointSerial();

		//保存信息
		fwrite(&m_fileHead, sizeof(m_fileHead), 1, fp);
		fwrite(m_pCodeBuf, sizeof(Code), m_fileHead.nCodeCounts, fp);
		fwrite(m_pGlyphBuf, 1, m_nGlyphTailOffset, fp);
		fclose(fp);

		//恢复
		_pointRuntime();
	}

	return true;
}

