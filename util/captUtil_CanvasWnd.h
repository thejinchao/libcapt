#pragma once

class CCanvasWindow : public CWindowImpl< CCanvasWindow >
{
	/*************************************************************************
		Public Methods
	*************************************************************************/
public:
	/** redraw canvas
	*/
	void redraw(void);
	/**
	*/
	void setQuestion(libCapt::Question* question);

	/*************************************************************************
			Inherit Methods
	*************************************************************************/
public:
	DECLARE_WND_CLASS(NULL)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(CHelloView)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	/** recreate background canvas
	*/
	void _createCanvas(int width, int height);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	CBitmap			m_bmpCanvas;	//!< Bmp canvas
	SIZE			m_size;			//!< Canvas size
	unsigned short*	m_pCanvasBitbuf;//!< Canvas bit buf
	int				m_nCanvasPitch;	//!< Cavas bit buf pitch(usnigned short)
	libCapt::Question* m_question;
	unsigned char	m_bitBuf[libCapt::Question::IMAGE_BUF_LENGTH];

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	CCanvasWindow();
	~CCanvasWindow();
};