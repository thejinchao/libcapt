#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>
#include <captGenerator.h>
#include <captFontFile.h>
#include <captRLE.h>


#pragma pack(push, 1)
struct TGA_HEAD				// TGA图像文件头
{
	unsigned char  IdentSize;			// 标志域长度		00 无标志域
	unsigned char  ColorMapType;		// 色彩表类型		00 无色彩表
	unsigned char  ImageType;			// 图像类型			02 非压缩RGB图像
	unsigned short ColorMapStart;		// 第一个色彩表入口	00 00
	unsigned short ColorMapLength;		// 色彩表长度		00 00
	unsigned char  ColorMapBits;		// 色彩表入口大小	00
	unsigned short XStart;				// 图像X原点		00 00
	unsigned short YStart;				// 图像Y原点		00 00
	unsigned short Width;				// 图像宽度			80 02 图像宽度640 ****
	unsigned short Height;				// 图像高度			E0 01 图像高度480 ****
	unsigned char  Bits;				// 每个象素位数		20 象素位数RGBA 32位 ****
	unsigned char  Descriptor;			// 图像描述符		08 Alpha通道位数8
										// 图像第一个点在左下角
};
#pragma pack(pop)

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("Usage: captGen [FontFile] [OutputFile]\n");
		return 1;
	}
	
	FILE* fp = fopen(argv[1], "rb");
	if(fp==0)
	{
		printf("Error, Open font file %s error!\n", argv[1]);
		return 1;
	}
	
	fseek(fp, 0, SEEK_END);
	unsigned int fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	unsigned char* streamBuf = new unsigned char[fileSize];
	fread(streamBuf, 1, fileSize, fp);
	fclose(fp); fp=0;

	//打开字体文件
	libCapt::FontFile fontFile;
	if(!fontFile.loadFromDataStream(streamBuf, fileSize))
	{
		printf("Error, Load font file error!\n");
		delete[] streamBuf;
		return 0;
	}
	delete[] streamBuf; streamBuf=0;
	
	//产生一个图灵测试问题
	srand((unsigned int)time(0));
	libCapt::Question question;
	libCapt::Generator generator(&fontFile);
	generator.generateQuestion(question);
		
	
	//------------------------------------------------------------


	//将图像内容解压并转换成图片
	unsigned char bitBuf[libCapt::Question::IMAGE_BUF_LENGTH];
	if(question.isCompressed())
	{
		unsigned int rleSize = question.getSize();
		unsigned int bufSize = libCapt::Question::IMAGE_BUF_LENGTH;
		rleDecompress(question.imageBuf, rleSize, libCapt::Question::IMAGE_WIDTH, libCapt::Question::IMAGE_HEIGHT, (unsigned char*)bitBuf, bufSize); 
	}
	else
	{
		memcpy(bitBuf, question.imageBuf, libCapt::Question::IMAGE_BUF_LENGTH);
	}

	//生成PBM图像，4bit深度
	fp = fopen(argv[2], "wb");
	if(fp==0)
	{
		printf("Open output file %s error!\n", argv[2]);
		return 1;
	}

	TGA_HEAD head;
	memset(&head, 0, sizeof(head));
	head.ImageType = 2;
	head.Width = libCapt::Question::IMAGE_WIDTH;
	head.Height = libCapt::Question::IMAGE_HEIGHT;
	head.Bits = 24;
	fwrite(&head, sizeof(head), 1, fp);

	for(int y=0; y<libCapt::Question::IMAGE_HEIGHT; y++)
	{
		for(int x=0; x<libCapt::Question::IMAGE_WIDTH; x++)
		{
			unsigned char c = bitBuf[(libCapt::Question::IMAGE_HEIGHT-1-y)*libCapt::Question::IMAGE_PITCH + (x>>1)];
			c = (x&1) ? ((c&0xf)<<4) : (c&0xf0);
			unsigned char pixel[4]={c,c,c,c};
			fwrite(pixel, 1, 3, fp);
		}
		
	}
	fclose(fp); fp=0;
	
	return 0;
}
