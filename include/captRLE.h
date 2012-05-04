#ifndef __LIB_CAPT_RLE_INCLUDE_H__
#define __LIB_CAPT_RLE_INCLUDE_H__

/** 提供简单的图像RLE压缩/解压缩算法,4bit深度
*/
namespace libCapt
{

/** 针对4bit深度图像的RLE压缩算法
@param imageBuf 4bit格式的图像数据
@param width 图像宽度
@param height 图像高度
@param rleBuf 用于存储压缩后图像的缓冲区
@param rleBufSize rleBuf的字节长度
@return 
	如果成功，返回true, rleBufSize存储压缩后的大小
	如果失败,返回false
*/
bool rleCompress(const unsigned char* imageBuf, int width, int height, unsigned char* rleBuf, unsigned int& rleBufSize);

/** 针对4bit深度图像的RLE解压缩算法
@param rleBuf 压缩后的rle图像数据
@param width 图像宽度
@param height 图像高度
@param rleBuf 用于存储解压缩后图像的缓冲区
@param rleBufSize imageBuf的字节长度
@return 
	如果成功，返回true, imageBufSize存储压缩后的大小
	如果失败,返回false
*/
bool rleDecompress(const unsigned char* rleBuf, unsigned int rleBufSize, int width, int height, unsigned char* imageBuf, unsigned int& imageBufSize);

}

#endif

