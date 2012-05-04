#include "stdafx.h"
#include "captUtil_CanvasWnd.h"

//--------------------------------------------------------------------------------------------
CCanvasWindow::CCanvasWindow()
	: m_nCanvasPitch(0)
	, m_question(0)
{
}

//--------------------------------------------------------------------------------------------
CCanvasWindow::~CCanvasWindow()
{

}

//--------------------------------------------------------------------------------------------
LRESULT CCanvasWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);

	if(!m_bmpCanvas.IsNull())
	{
		CDC dcMem;
		dcMem.CreateCompatibleDC(dc);
		HBITMAP hBmpOld = dcMem.SelectBitmap(m_bmpCanvas);
		dc.BitBlt(0, 0, m_size.cx, m_size.cy, dcMem, 0, 0, SRCCOPY);
		dcMem.SelectBitmap(hBmpOld);
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT CCanvasWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return TRUE;
}

//--------------------------------------------------------------------------------------------
void CCanvasWindow::setQuestion(libCapt::Question* question)
{
	m_question = question;

	//½âÑ¹Ëõ
	if(m_question->isCompressed())
	{
		unsigned int rleSize = m_question->getSize();
		unsigned int bufSize = libCapt::Question::IMAGE_BUF_LENGTH;
		rleDecompress(m_question->imageBuf, rleSize, libCapt::Question::IMAGE_WIDTH, libCapt::Question::IMAGE_HEIGHT, (unsigned char*)m_bitBuf, bufSize); 
	}

	redraw();
}

//--------------------------------------------------------------------------------------------
void CCanvasWindow::_createCanvas(int width, int height)
{
	//free old canvas
	if(!m_bmpCanvas.IsNull())
	{
		m_bmpCanvas.DeleteObject();
	}

	m_size.cx = width;
	m_size.cy = height;

	//cal pitch
	m_nCanvasPitch=width*2;
	while(m_nCanvasPitch%4!=0) m_nCanvasPitch++;
	m_nCanvasPitch=m_nCanvasPitch>>1;

	// create canvas dc
	CDC dcCanvas;
	dcCanvas.CreateCompatibleDC(CClientDC(m_hWnd));

	//create DIB Section
	BITMAPINFO bmpInfo; 
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize		= sizeof (BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth		= width; 
	bmpInfo.bmiHeader.biHeight		= -height;
	bmpInfo.bmiHeader.biPlanes		= 1;
	bmpInfo.bmiHeader.biBitCount	= 16;
	bmpInfo.bmiHeader.biSizeImage	= m_nCanvasPitch * height * 2;
	m_bmpCanvas.Attach(CreateDIBSection(CClientDC(m_hWnd), &bmpInfo, DIB_RGB_COLORS, (VOID**)&(m_pCanvasBitbuf), 0, 0));

	//fill with white color
	memset(m_pCanvasBitbuf, 0xFF, m_nCanvasPitch * height*2);
}

//--------------------------------------------------------------------------------------------
LRESULT CCanvasWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int cx = GET_X_LPARAM(lParam);
	int cy = GET_Y_LPARAM(lParam);
	bHandled = FALSE;

	if(cx==0 || cy==0) return 0;

	//reset canvas
	_createCanvas(cx, cy);

	// redraw
	redraw();

	return 1;
}

//--------------------------------------------------------------------------------------------
void CCanvasWindow::redraw(void)
{
	if(m_bmpCanvas.IsNull() || m_question==0) return;

	//fill back
	for(int i=0; i<m_size.cy; i++)
	{
		unsigned short* pCanvasBit = m_pCanvasBitbuf+i*m_nCanvasPitch;
		ZeroMemory(pCanvasBit, m_nCanvasPitch*sizeof(unsigned short)); 
	}

	unsigned char *i=(m_question->nFlags & libCapt::Question::FLAG_RLE) ? (unsigned char *)m_bitBuf : m_question->imageBuf;
	unsigned short* pCanvasBit = m_pCanvasBitbuf;

	for(int y=0; y<libCapt::Question::IMAGE_HEIGHT; y++)
	{
		pCanvasBit = m_pCanvasBitbuf + m_nCanvasPitch*y;
		for(int x=0; x<libCapt::Question::IMAGE_WIDTH; x++)
		{
			unsigned char c = i[y*libCapt::Question::IMAGE_PITCH + (x>>1)];
			if(x&1) 
			{
				c = c&0x0F;
			}
			else
			{
				c = (c>>4)&0x0F;
			}

			*pCanvasBit++ = (c<<11) | (c<<6) | (c<<1);
		}
	}

	//send redraw request
	::InvalidateRect(m_hWnd, 0, TRUE);
}