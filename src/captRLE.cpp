/*
	ImgRLE4.cpp - An implementation of the Microsoft RLE compressed 4bpp bitmap format.
	This file is a part of LibRLE.

    Copyright (C) 2009 Andy Ash

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Version 1.0 - 16/6/09 Andy Ash (andy.ash@talk21.com)
*/

#include <stdio.h>

namespace libCapt
{

#define ALLOC_CHUNK		1024

#define CMPS_FLAGS_NONE		0x00
#define CMPS_FLAGS_EOF		0x01
#define CMPS_FLAGS_EOL		0x02

#define RLE_CMD_NULL		0	//A: NULL ~ B: NULL
#define RLE_CMD_EOL			1	//A: NULL ~ B: NULL
#define RLE_CMD_EOF			2	//A: NULL ~ B: NULL
#define RLE_CMD_DELTA		3	//A: XOffs ~ B: YOffs
#define RLE_CMD_ABS			4	//A: Ptr ~ B: Cnt (03-FF)
#define RLE_CMD_ENC			5	//A: Char ~ B: Cnt (01-FF)

#define RLE_CODE_ESCAPE		0x00
#define RLE_CODE_EOL		0x00
#define RLE_CODE_EOF		0x01
#define RLE_CODE_DELTA		0x02

typedef struct
{
	unsigned long XDim;
	unsigned long YDim;
	unsigned long LineSize;

	unsigned char* pInBuf;
	unsigned long InSize;
	unsigned long InPtr;

	unsigned char* pOutBuf;
	unsigned long OutSize;
	unsigned long OutPtr;

	unsigned long NextLine;
	unsigned long EndOfLine;
	unsigned long EndOfFile;
	unsigned char Flags;
	unsigned char Cmd;
	unsigned long aParam;
	unsigned long bParam;
}CmpsState;

typedef struct
{
	unsigned long XDim;
	unsigned long YDim;
	unsigned long LineSize;

	unsigned char* pInBuf;
	unsigned long InSize;
	unsigned long InPtr;

	unsigned char* pOutBuf;
	unsigned long OutSize;

	unsigned long XPos;
	unsigned long YPos;
	unsigned char Cmd;
	unsigned long aParam;
	unsigned long bParam;
}ExpdState;

unsigned char CmpsRead(CmpsState& aCS,unsigned long Ptr)
{
	unsigned char RetVal = 0;

	unsigned long BytePtr = (Ptr >> 1);
	bool High = ((Ptr & 0x01) == 0);

	RetVal = aCS.pInBuf[BytePtr];

	if(High)
	 RetVal >>= 4;

	RetVal &= 0x0F;

	return RetVal;
}

void CmpsSeek4(CmpsState& aCS)
{
	unsigned long LocalPtr;

	LocalPtr = aCS.InPtr;

	bool Stop = false;
	unsigned char AbsCount = 0;
	while(!Stop)
	{
		unsigned char DataA = CmpsRead(aCS,LocalPtr);
		unsigned char DataB = CmpsRead(aCS,LocalPtr+1);
		unsigned char DataC = CmpsRead(aCS,LocalPtr+2);

		if(DataA == DataB)
		 Stop = true;
		else
		if(DataA == DataC)
		 Stop = true;
		else
		{
			AbsCount++;
			LocalPtr++;
		}

		if((LocalPtr >= aCS.EndOfLine) || (AbsCount == 0xFF))
		 Stop = true;
	}

	if(AbsCount == 0)
	{
		LocalPtr = aCS.InPtr;

		unsigned char EncCount = 0;
		unsigned char DataA,DataB;
		DataA = DataB = 0;

		DataA = CmpsRead(aCS,LocalPtr);
		LocalPtr++;
		EncCount++;

		if(LocalPtr < aCS.EndOfLine)
		{
			DataB = CmpsRead(aCS,LocalPtr);
			LocalPtr++;
			EncCount++;

			bool Done;
			do
			{
				Done = true;

				if((LocalPtr < aCS.EndOfLine) && (EncCount < 0xFF))
				{
					unsigned char Test = CmpsRead(aCS,LocalPtr);

					if(
					   ((Test == DataA) && ((EncCount & 0x01) == 0)) ||
					   ((Test == DataB) && ((EncCount & 0x01) == 1))
					  )
					{
						LocalPtr++;
						EncCount++;

						Done = false;
					}
				}

			}while(!Done);
		}

		aCS.Cmd = RLE_CMD_ENC;

		unsigned char Data;
		Data = ((DataA << 4) & 0xF0);
		Data |= DataB;

		aCS.aParam = (((unsigned long) Data) & 0x000000FF);
		aCS.bParam = (((unsigned long) EncCount) & 0x000000FF);
		aCS.InPtr = LocalPtr;
	}
	else
	{
		switch(AbsCount)
		{
			case 1:
			{
				aCS.Cmd = RLE_CMD_ENC;

				unsigned char Data = CmpsRead(aCS,aCS.InPtr);
				Data = ((Data << 4) & 0xF0);

				aCS.aParam = (((unsigned long) Data) & 0x000000FF);
				aCS.bParam = 1;
				aCS.InPtr = LocalPtr;
			}
			break;

			case 2:
			{
				aCS.Cmd = RLE_CMD_ENC;

				unsigned char Data = CmpsRead(aCS,aCS.InPtr);
				Data = ((Data << 4) & 0xF0);
				Data |= CmpsRead(aCS,aCS.InPtr+1);

				aCS.aParam = (((unsigned long) Data) & 0x000000FF);
				aCS.bParam = 2;
				aCS.InPtr = LocalPtr;
			}
			break;
			
			default:
			{
				aCS.Cmd = RLE_CMD_ABS;
				aCS.aParam = aCS.InPtr;
				aCS.bParam = (((unsigned long) AbsCount) & 0x000000FF);
				aCS.InPtr = LocalPtr;
			}
			break;			
		}
	}
}

bool NextCmpsCmd4(CmpsState& aCS)
{
	bool Action = false;
	aCS.Cmd = RLE_CMD_NULL;

	if(aCS.InPtr < aCS.EndOfFile)
	{
		if((aCS.InPtr < aCS.EndOfLine) || ((aCS.Flags & CMPS_FLAGS_EOL) != 0))
		{
			if((aCS.Flags & CMPS_FLAGS_EOL) != 0)
			{
				aCS.Flags &= ~CMPS_FLAGS_EOL;
				aCS.InPtr = aCS.NextLine;
				aCS.EndOfLine = aCS.NextLine + aCS.XDim;
				aCS.NextLine += aCS.LineSize;
			}

			CmpsSeek4(aCS);

			Action = true;
		}
		else
		if((aCS.Flags & CMPS_FLAGS_EOL) == 0)
		{
			aCS.Flags |= CMPS_FLAGS_EOL;
			aCS.Cmd = RLE_CMD_EOL;
			Action = true;
		}
	}
	else
	if((aCS.Flags & CMPS_FLAGS_EOF) == 0)
	{
		aCS.Flags |= CMPS_FLAGS_EOF;
		aCS.Cmd = RLE_CMD_EOF;
		Action = true;
	}

	return Action;
}

bool CmpsWrite4(CmpsState& aCS, unsigned char Data)
{
	if(aCS.OutPtr >= aCS.OutSize)
	{
		return false;
	}

	aCS.pOutBuf[aCS.OutPtr] = Data;
	aCS.OutPtr++;
	return true;
}

bool CmpsDump4(CmpsState& aCS)
{
	#define TRY_CMPS_WRITE_4(a, b) if(!CmpsWrite4(a, b)) return false

	switch(aCS.Cmd)
	{
		case RLE_CMD_EOL:
		{
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_ESCAPE);
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_EOL);
		}
		break;

		case RLE_CMD_EOF:
		{
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_ESCAPE);
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_EOF);
		}
		break;

		case RLE_CMD_DELTA:
		{
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_ESCAPE);
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_DELTA);
			TRY_CMPS_WRITE_4(aCS,(unsigned char)aCS.aParam);
			TRY_CMPS_WRITE_4(aCS,(unsigned char)aCS.bParam);
		}
		break;

		case RLE_CMD_ABS:
		{
			TRY_CMPS_WRITE_4(aCS,RLE_CODE_ESCAPE);
			TRY_CMPS_WRITE_4(aCS,(unsigned char)aCS.bParam);

			unsigned char i,Count;
			Count = (unsigned char) aCS.bParam;

			if((Count & 0x01) != 0)
			{
				Count >>= 1;
				Count += 1;
			}
			else
			 Count >>= 1;

			unsigned long LocalPtr = aCS.aParam;

			for(i=0;i<Count;i++)
			{
				unsigned char Data = 0;
				Data |= CmpsRead(aCS,LocalPtr);
				LocalPtr++;
				Data = ((Data << 4) & 0xF0);
				Data |= CmpsRead(aCS,LocalPtr);
				LocalPtr++;

				TRY_CMPS_WRITE_4(aCS,Data);
			}

			if((Count & 0x01) != 0)
			 TRY_CMPS_WRITE_4(aCS,0);
		}
		break;

		case RLE_CMD_ENC:
		{
			TRY_CMPS_WRITE_4(aCS,(unsigned char)aCS.bParam);
			TRY_CMPS_WRITE_4(aCS,(unsigned char) aCS.aParam);
		}
		break;

		default:
		case RLE_CMD_NULL:
		break;
	}
	return true;
}

bool ExpdRead4(ExpdState& aES,unsigned char& Data)
{
	bool RetVal = false;

	if(aES.InPtr < aES.InSize)
	{
		Data = aES.pInBuf[aES.InPtr];
		aES.InPtr++;
		RetVal = true;
	}
	else
	 Data = 0x00;

	return RetVal;
}

bool NextExpdCmd4(ExpdState& aES)
{
	bool RetVal = false;
	aES.Cmd = RLE_CMD_NULL;

	unsigned char Data;
	if(ExpdRead4(aES,Data))
	{
		if(Data == 0x00)
		{
			if(ExpdRead4(aES,Data))
			{
				switch(Data)
				{
					case 0:
					{
						aES.Cmd = RLE_CMD_EOL;
						aES.aParam = 0;
						aES.bParam = 0;
						RetVal = true;
					}
					break;

					case 1:
					{
						aES.Cmd = RLE_CMD_EOF;
						aES.aParam = 0;
						aES.bParam = 0;
						RetVal = true;
					}
					break;

					case 2:
					{
						aES.Cmd = RLE_CMD_DELTA;

						if(ExpdRead4(aES,Data))
						{
							aES.aParam = (((unsigned long)Data) & 0x000000FF);

							if(ExpdRead4(aES,Data))
							{
								aES.bParam = (((unsigned long)Data) & 0x000000FF);
								RetVal = true;
							}
						}
					}
					break;

					default:
					{
						aES.Cmd = RLE_CMD_ABS;
						aES.aParam = aES.InPtr;
						aES.bParam = (((unsigned long)Data) & 0x000000FF);

						if((Data & 0x01) != 0)
						{
							Data >>= 1;
							Data += 1;
						}
						else
						 Data >>= 1;

						aES.InPtr += Data;

						if((Data & 0x01) != 0)
						 aES.InPtr++;

						RetVal = true;
					}
					break;
				}
			}
		}
		else
		{
			aES.Cmd = RLE_CMD_ENC;
			aES.bParam = (((unsigned long)Data) & 0x000000FF);
			if(ExpdRead4(aES,Data))
			{
				aES.aParam = (((unsigned long)Data) & 0x000000FF);
				RetVal = true;
			}
		}
	}

	return RetVal;
}

void WritePixel4(ExpdState& aES,unsigned char Data,bool Hi)
{
	if(aES.XPos >= aES.XDim)
	{
		aES.XPos = 0;
		aES.YPos++;
	}

	if(aES.YPos >= aES.YDim)
	{
		aES.XPos = 0;
		aES.YPos = 0;
	}

	if((aES.XPos < aES.XDim) && (aES.YPos < aES.YDim))
	{
		unsigned long Ptr = (aES.YPos * aES.LineSize) + (aES.XPos >> 1);

		bool DestHi = ((aES.XPos & 0x01) == 0);
		unsigned char Temp = aES.pOutBuf[Ptr];

		if(DestHi)
		{
			if(Hi)
			{
				Temp &= 0x0F;
				Temp |= (Data & 0xF0);
			}
			else
			{
				Temp &= 0x0F;
				Temp |= ((Data << 4) & 0xF0);
			}
		}
		else
		{
			if(Hi)
			{
				Temp &= 0xF0;
				Temp |= ((Data >> 4) & 0x0F);
			}
			else
			{
				Temp &= 0xF0;
				Temp |= (Data & 0x0F);
			}
		}

		aES.pOutBuf[Ptr] = Temp;
	}

	aES.XPos++;
}

void ExpdDump4(ExpdState& aES)
{
	switch(aES.Cmd)
	{
		case RLE_CMD_EOL:
		{
			aES.XPos = 0;
			aES.YPos++;
		}
		break;

		case RLE_CMD_EOF:
		{
			aES.XPos = 0;
			aES.YPos = 0;
		}
		break;

		case RLE_CMD_DELTA:
		{
			aES.XPos += aES.aParam;
			aES.YPos += aES.bParam;
		}
		break;

		case RLE_CMD_ABS:
		{
			unsigned long LocalPtr = aES.aParam;
			unsigned char i,Count;

			Count = (unsigned char) aES.bParam;
			for(i=0;i<Count;i++)
			{
				if((i & 0x01) == 0)
				{
					WritePixel4(aES,aES.pInBuf[LocalPtr],true);
				}
				else
				{
					WritePixel4(aES,aES.pInBuf[LocalPtr],false);
					LocalPtr++;
				}
			}
		}
		break;

		case RLE_CMD_ENC:
		{
			unsigned char Data = (unsigned char) aES.aParam;
			unsigned char i,Count;

			Count = (unsigned char) aES.bParam;
			for(i=0;i<Count;i++)
			{
				if((i & 0x01) == 0)
				 WritePixel4(aES,Data,true);
				else
				 WritePixel4(aES,Data,false);
			}
		}
		break;

		default:
		case RLE_CMD_NULL:
		break;
	}
}

//-------------------------------------------------------------------------------------------------
bool rleCompress(const unsigned char* imageBuf, int width, int height, unsigned char* rleBuf, unsigned int& rleBufSize)
{
	CmpsState aCS;

	aCS.XDim = width;
	aCS.YDim = height;
	aCS.LineSize = width;

	aCS.InSize = (width*height)>>1;
	aCS.pInBuf = (unsigned char*)imageBuf;
	aCS.InPtr = 0;

	aCS.OutSize = rleBufSize;
	aCS.pOutBuf = rleBuf;
	aCS.OutPtr = 0;

	aCS.NextLine = aCS.LineSize;
	aCS.EndOfLine = aCS.XDim;
	aCS.EndOfFile = (((aCS.YDim - 1) * aCS.LineSize) + aCS.XDim);
	aCS.Flags = CMPS_FLAGS_NONE;

	while(NextCmpsCmd4(aCS))
	{
		if(!CmpsDump4(aCS)) return false;
	}

	rleBufSize = aCS.OutPtr;

	return true;
}

//-------------------------------------------------------------------------------------------------
bool rleDecompress(const unsigned char* rleBuf, unsigned int rleBufSize, int width, int height, unsigned char* imageBuf, unsigned int& imageBufSize)
{
	ExpdState aES;

	aES.XDim = width;
	aES.YDim = height;
	aES.LineSize = (width >> 1);

	if((width & 0x01) != 0)
	 aES.LineSize++;
	 
	char Rem = (char) (aES.LineSize % 4);
	if(Rem != 0)
	 aES.LineSize += (4 - Rem);

	aES.pInBuf = (unsigned char*) rleBuf;
	aES.InSize = rleBufSize;
	aES.InPtr = 0;

	aES.OutSize = aES.YDim * aES.LineSize;
	if(aES.OutSize > imageBufSize)
	{
		return false;
	}

	aES.pOutBuf = (unsigned char*) imageBuf;

	aES.XPos = 0;
	aES.YPos = 0;

	while(NextExpdCmd4(aES))
	{
		ExpdDump4(aES);
	}

	imageBufSize = aES.OutSize;

	return true;
}

}

