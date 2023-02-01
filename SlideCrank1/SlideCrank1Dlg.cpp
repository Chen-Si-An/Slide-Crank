
// SlideCrank1Dlg.cpp : 實作檔
//


#include "stdafx.h"
#include "SlideCrank1.h"
#include "SlideCrank1Dlg.h"
#include "afxdialogex.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

//define相關工作區的常數
#define GAP	10				//工作區與對話框邊界之間隙
#define BOTTOM	302			//工作區bottom位置

//define相關機構尺寸的常數
#define GROUND_WIDTH 210	//平台寬度
#define GROUND_HEIGHT 25	//平台高度
#define RADIUS 55			//轉軸半徑
#define SLIDE_WIDTH 60		//滑塊寬度
#define SLIDE_HEIGHT 30		//滑塊高度
#define OVERLAP 10			//轉軸基底與轉軸重疊部分 (先畫基底再畫轉軸)
#define ROD_TIP_RADIUS 5	//桿件端點半徑
#define H_LIMIT 200			//平台上限高度

#define BLACK RGB (0, 0, 0)
#define WHITE RGB (255, 255, 255)
#define GRAY  RGB (240, 240, 240)
#define BLUE  RGB (135, 206, 235)
#define BROWN RGB (128, 42, 42)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void ThetaTransform (double &);

// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg ();

// 對話方塊資料
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP ()
};

CAboutDlg::CAboutDlg () : CDialogEx (CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange (CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSlideCrank1Dlg 對話方塊



CSlideCrank1Dlg::CSlideCrank1Dlg (CWnd* pParent /*=NULL*/)
	: CDialogEx (CSlideCrank1Dlg::IDD, pParent)
	, m_dAng (0.)
	, m_dRpm (0.)
	, m_dAngVelocity (0.)
	, m_dSettledTime (0.)
	, m_dAngAcc (0.)
	, m_iLHeight (0)
	, m_iRHeight (0)
	, m_bCheckRpm (TRUE)
	, m_bCheckSec (TRUE)
	, m_bIsAccelerating (TRUE)
	, m_bIni (TRUE)
	, m_bEnterUniform (TRUE)
	, m_bStop (FALSE)
	, m_clkTimeIni (0)
	, m_clkTimeForward (0)
	, m_clkTimeCurrent (0)
	, m_rectWorkspace (0, 0, 0, 0)
{
	m_hIcon = AfxGetApp()->LoadIcon (IDR_MAINFRAME);

}

void CSlideCrank1Dlg::DoDataExchange (CDataExchange* pDX)
{
	CDialogEx::DoDataExchange (pDX);
	DDX_Text (pDX, IDC_EDIT_LHEIGHT, m_iLHeight);
	DDX_Text (pDX, IDC_EDIT_RHEIGHT, m_iRHeight);
	DDX_Text (pDX, IDC_EDIT_ROTSPEED, m_dRpm);
	DDX_Text (pDX, IDC_EDIT_SETTLEDTIME, m_dSettledTime);
}

BEGIN_MESSAGE_MAP (CSlideCrank1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND ()
	ON_WM_PAINT ()
	ON_WM_QUERYDRAGICON ()
	ON_WM_TIMER ()
	ON_BN_CLICKED (IDC_BUTTON_START, &CSlideCrank1Dlg::OnBnClickedButtonStart)
	ON_BN_CLICKED (IDC_BUTTON_STOP, &CSlideCrank1Dlg::OnBnClickedButtonStop)
	ON_EN_CHANGE (IDC_EDIT_LHEIGHT, &CSlideCrank1Dlg::OnEnChangeEditLheight)
	ON_EN_CHANGE (IDC_EDIT_RHEIGHT, &CSlideCrank1Dlg::OnEnChangeEditRheight)
	ON_EN_KILLFOCUS (IDC_EDIT_ROTSPEED, &CSlideCrank1Dlg::OnEnKillfocusEditRotspeed)
	ON_EN_KILLFOCUS (IDC_EDIT_SETTLEDTIME, &CSlideCrank1Dlg::OnEnKillfocusEditSettledtime)	
END_MESSAGE_MAP ()


// CSlideCrank1Dlg 訊息處理常式

BOOL CSlideCrank1Dlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT ((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT (IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu (FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString (IDS_ABOUTBOX);
		ASSERT (bNameValid);
		if (!strAboutMenu.IsEmpty ())
		{
			pSysMenu->AppendMenu (MF_SEPARATOR);
			pSysMenu->AppendMenu (MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon (m_hIcon, TRUE);		// 設定大圖示
	SetIcon (m_hIcon, FALSE);		// 設定小圖示


	UpdateData (FALSE);

	//初始化顯示為"加速時間"
	( (CEdit*) GetDlgItem (IDC_STATIC_ACCTIME))->SetWindowTextA ("Acceleration time");

	//設定顯示current speed and acceleration time的Edit為read only
	( (CEdit* ) GetDlgItem (IDC_EDIT_CURSPEED))->SetReadOnly (TRUE);
	( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetReadOnly (TRUE);

	CString strCurSpeed, strAccTime;

	//初始化顯示current speed的Edit
	strCurSpeed.Format (_T ("%lf"), 0.0);
	( (CEdit* ) GetDlgItem (IDC_EDIT_CURSPEED))->SetWindowTextA (strCurSpeed);

	//初始化顯示acceleration time的Edit
	strAccTime.Format (_T ("%lf"), 0.0);
	( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);

	//設定無效區 (對話框坐標系下)
	GetClientRect (&m_rectWorkspace);
	m_rectWorkspace.top += GAP;
	m_rectWorkspace.left += GAP;
	m_rectWorkspace.bottom = BOTTOM + GAP;
	m_rectWorkspace.right -= GAP;

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CSlideCrank1Dlg::OnSysCommand (UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal ();
	}
	else
	{
		CDialogEx::OnSysCommand (nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CSlideCrank1Dlg::OnPaint ()
{
	CPaintDC dcDlg (this);
	CRect rectWorkspace;

	//定義工作區域
	GetClientRect (&rectWorkspace);
	rectWorkspace.bottom = BOTTOM;
	rectWorkspace.right -= 2 * GAP;

	//建立與對話框相容的記憶體顯示裝置以實現雙緩衝
	CDC dcMem;
	CBitmap BitmapTemp, *pOldBitmapTemp;

	dcMem.CreateCompatibleDC (&dcDlg);
	BitmapTemp.CreateCompatibleBitmap (&dcDlg, rectWorkspace.Width (), rectWorkspace.Height ());
	pOldBitmapTemp = dcMem.SelectObject (&BitmapTemp);
	dcMem.FillSolidRect (rectWorkspace, GRAY);

	//工作區座標系下
	int iWorkspaceXLimit = rectWorkspace.right;			//設定工作區x座標極限			
	int iWorkspaceYLimit = rectWorkspace.bottom - 2;	//設定工作區y座標極限	
	int iLGroundY = iWorkspaceYLimit - m_iLHeight;		//設定左平台y座標
	int iRGroundY = iWorkspaceYLimit - m_iRHeight;		//設定右平台y座標
	int iShaftCenterX = int (iWorkspaceXLimit / 2);		//設定轉軸軸心x座標		
	int iShaftCenterY = int (iWorkspaceYLimit / 2);		//設定轉軸軸心y座標	

	//設定兩連桿長度
	double iLRod = iShaftCenterX - SLIDE_WIDTH / 2 - RADIUS;	
	double iRRod = iWorkspaceXLimit - iShaftCenterX - SLIDE_WIDTH / 2 - RADIUS;

	//建立畫筆與畫刷物件
	CPen penGround (PS_SOLID, 1, BLACK);
	CBrush brushGround (WHITE);
	CPen penMech (PS_SOLID, 1, GRAY);
	CBrush brushMech (BLUE);
	CPen penRod (PS_SOLID, 3, BROWN);
	CBrush brushRod (BROWN);
	CPen* pOldPen;
	CBrush* pOldBrush;

	//繪製ground
	pOldPen = dcMem.SelectObject (&penGround);
	pOldBrush = dcMem.SelectObject (&brushGround);

	POINT ptShaftGround[4] = {{iShaftCenterX - RADIUS, iWorkspaceYLimit},								
							  {iShaftCenterX + RADIUS, iWorkspaceYLimit},
							  {iShaftCenterX + int (RADIUS / 2), iShaftCenterY + RADIUS - OVERLAP},
							  {iShaftCenterX - int (RADIUS / 2), iShaftCenterY + RADIUS - OVERLAP}}; 

	dcMem.Polygon (ptShaftGround, 4);																			//繪製轉軸基底
	dcMem.Rectangle (0, iLGroundY - GROUND_HEIGHT, GROUND_WIDTH, iLGroundY);									//繪製左滑塊平台
	dcMem.Rectangle (iWorkspaceXLimit - GROUND_WIDTH, iRGroundY - GROUND_HEIGHT, iWorkspaceXLimit, iRGroundY);	//繪製右滑塊平台

	double dCosTheta = cos (m_dAng);	//計算cos
	double dSinTheta = sin (m_dAng);	//計算sin

	//繪製轉軸與滑塊
	dcMem.SelectObject (&penMech);
	dcMem.SelectObject (&brushMech);

	dcMem.Ellipse (iShaftCenterX - RADIUS, iShaftCenterY - RADIUS, iShaftCenterX + RADIUS, iShaftCenterY + RADIUS);												//繪製轉軸

	double iLSlideX = iShaftCenterX - RADIUS * dCosTheta - sqrt (pow (iLRod, 2) - pow (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - iShaftCenterY - RADIUS * dSinTheta, 2));	//計算滑塊1中心點之x座標
	double iRSlideX = iShaftCenterX + RADIUS * dCosTheta + sqrt (pow (iRRod, 2) - pow (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - iShaftCenterY + RADIUS * dSinTheta, 2));	//計算滑塊2中心點之x座標

	dcMem.Rectangle (int (iLSlideX - SLIDE_WIDTH / 2), iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT, int (iLSlideX + SLIDE_WIDTH/2), iLGroundY - GROUND_HEIGHT);	//繪製左滑塊
	dcMem.Rectangle (int (iRSlideX - SLIDE_WIDTH / 2), iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT, int (iRSlideX + SLIDE_WIDTH/2), iRGroundY - GROUND_HEIGHT);	//繪製右滑塊

	//繪製兩連桿
	dcMem.SelectObject (&penRod);
	dcMem.SelectObject (&brushRod);

	dcMem.MoveTo  (int (iShaftCenterX - RADIUS * dCosTheta), int (iShaftCenterY + RADIUS * dSinTheta));										//左連桿於轉軸上之位置
	dcMem.LineTo  (int (iLSlideX), int (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2));														//左連桿於滑塊上之位置
	dcMem.Ellipse (int (iShaftCenterX - RADIUS * dCosTheta - ROD_TIP_RADIUS), int (iShaftCenterY + RADIUS * dSinTheta - ROD_TIP_RADIUS),	
				   int (iShaftCenterX - RADIUS * dCosTheta + ROD_TIP_RADIUS), int (iShaftCenterY + RADIUS * dSinTheta + ROD_TIP_RADIUS));	//繪製左連桿於轉軸上的點		
	dcMem.Ellipse (int (iLSlideX - ROD_TIP_RADIUS), int (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - ROD_TIP_RADIUS),						
				   int (iLSlideX + ROD_TIP_RADIUS), int (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 + ROD_TIP_RADIUS));					//繪製左連桿於滑塊上的點
	
	dcMem.MoveTo  (int (iShaftCenterX + RADIUS * dCosTheta), int (iShaftCenterY - RADIUS * dSinTheta));										//右連桿於轉軸上之位置
	dcMem.LineTo  (int (iRSlideX), int (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2));														//右連桿於滑塊上之位置
	dcMem.Ellipse (int (iShaftCenterX + RADIUS * dCosTheta - ROD_TIP_RADIUS), int (iShaftCenterY - RADIUS * dSinTheta - ROD_TIP_RADIUS),	
				   int (iShaftCenterX + RADIUS * dCosTheta + ROD_TIP_RADIUS), int (iShaftCenterY - RADIUS * dSinTheta + ROD_TIP_RADIUS));	//繪製右連桿於轉軸上的點
	dcMem.Ellipse (int (iRSlideX - ROD_TIP_RADIUS), int (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - ROD_TIP_RADIUS),						
				   int (iRSlideX + ROD_TIP_RADIUS), int (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 + ROD_TIP_RADIUS));					//繪製右連桿於滑塊上的點

	dcDlg.BitBlt (GAP, GAP, rectWorkspace.Width (), rectWorkspace.Height (), &dcMem, 0, 0, SRCCOPY);	//將繪製於記憶體顯示裝置之點陣圖拷貝至對話框
	
	//清理
	dcMem.SelectObject (pOldPen);
	dcMem.SelectObject (pOldBrush);
	dcMem.SelectObject (pOldBitmapTemp);
	BitmapTemp.DeleteObject ();	
	dcMem.DeleteDC ();
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CSlideCrank1Dlg::OnQueryDragIcon ()
{
	return static_cast<HCURSOR> (m_hIcon);
}

void CSlideCrank1Dlg::OnTimer (UINT_PTR nIDEvent)
{
	KillTimer (0);

	if (nIDEvent == 0)
    {
		double dTimeAcc = 0., dTimeStep = 0.;
		static double s_dCurAng = 0., s_dCurAngVelocity = 0., s_dTimeNeed = 0.;			//宣告靜態變數current angle, current angular velocity, 加減速所需時間
		CString strCurSpeed, strAccTime;

		m_clkTimeCurrent = clock ();													//紀錄current time
		dTimeAcc  = (m_clkTimeCurrent - m_clkTimeIni) / (double) (CLOCKS_PER_SEC);		//計算加速or減速狀態經過總時間
		dTimeStep = (m_clkTimeCurrent - m_clkTimeForward) / (double) (CLOCKS_PER_SEC);	//計算一個step經過時間
		m_clkTimeForward = clock ();													//紀錄前一個step的時間

		//加速
		if (m_bIsAccelerating)
		{
			//狀態轉換時進入一次 (初始化)
			if (m_bIni)	
			{
				s_dTimeNeed = (m_dAngVelocity - s_dCurAngVelocity) / m_dAngAcc;	//計算加速所需時間
				m_bIni = FALSE;
			}

			//判斷是否已加速至使用者設定轉速
			if (dTimeAcc <= s_dTimeNeed)
			{
				m_dAng = s_dCurAng + s_dCurAngVelocity * dTimeStep + m_dAngAcc * pow (dTimeStep, 2) / 2;							//等加速度後所達角度位置
				s_dCurAngVelocity += m_dAngAcc * dTimeStep;	//紀錄current angular velocity
				
				//顯示accelration time
				strAccTime.Format (_T ("%lf"), dTimeAcc);
				( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
			}
			else
			{
				if (m_bEnterUniform)	//加速狀態下達等速，分段計算等加速度及等速所旋轉之角度
				{
					m_dAng = s_dCurAng + s_dCurAngVelocity * (dTimeStep - dTimeAcc + s_dTimeNeed) +
						m_dAngAcc * pow (dTimeStep - dTimeAcc + s_dTimeNeed, 2) / 2 + m_dAngVelocity * (dTimeAcc - s_dTimeNeed);	//加速至指定轉速後，以等速旋轉
					s_dCurAngVelocity = m_dAngVelocity;		//紀錄current angular velocity
					m_bEnterUniform = FALSE;

					//顯示accelration time
					strAccTime.Format (_T ("%lf"), dTimeAcc);
					( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
				}
				else
				{
					m_dAng = s_dCurAng + s_dCurAngVelocity * dTimeStep;																//等速旋轉所達角度位置
					s_dCurAngVelocity = m_dAngVelocity;		//紀錄current angular velocity
				}
			}

		}
		//減速
		else
		{
			//狀態轉換時進入一次 (初始化)
			if (m_bIni)
			{
				s_dTimeNeed = s_dCurAngVelocity / m_dAngAcc;	//計算減速所需時間
				m_bIni = FALSE;
			}

			//判斷是否減速至靜止
			if (dTimeAcc <= s_dTimeNeed)
			{
				m_dAng = s_dCurAng + s_dCurAngVelocity * dTimeStep - m_dAngAcc * pow (dTimeStep, 2) / 2;	//等減速度後所達角度位置
				s_dCurAngVelocity -= m_dAngAcc * dTimeStep;	//紀錄current angular velocity

				//顯示decelration time
				strAccTime.Format (_T ("%lf"), dTimeAcc);
				( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
			}
			else
			{
				m_dAng = s_dCurAng + s_dCurAngVelocity * (dTimeStep - dTimeAcc + s_dTimeNeed) -				//靜止時角度位置
					m_dAngAcc * pow (dTimeStep - dTimeAcc + s_dTimeNeed, 2) / 2 ;
				s_dCurAngVelocity = 0;						//紀錄current angular velocity

				m_bStop = TRUE;	//減速至靜止時設定m_bStop為TRUE

				( (CEdit* ) GetDlgItem (IDC_EDIT_ROTSPEED))->SetReadOnly (FALSE);		//靜止後解除編輯框read only		
				( (CEdit* ) GetDlgItem (IDC_EDIT_SETTLEDTIME))->SetReadOnly (FALSE);	//靜止後解除編輯框read only		

				//顯示decelration time
				strAccTime.Format (_T ("%lf"), dTimeAcc);
				( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
			}
		}


		ThetaTransform (m_dAng);	//等效角度轉換
		s_dCurAng = m_dAng;			//紀錄current angle

		//顯示current speed
		strCurSpeed.Format (_T ("%lf"), s_dCurAngVelocity / 2 / M_PI * 60);
		( (CEdit* ) GetDlgItem (IDC_EDIT_CURSPEED))->SetWindowTextA (strCurSpeed);

		//進入OnPaint重繪
		InvalidateRect (m_rectWorkspace, FALSE);
		UpdateWindow ();
    }

    CDialogEx::OnTimer (nIDEvent);

	//m_bStop為TRUE，即靜止時，不再重啟Timer
	if (!m_bStop)
		SetTimer (0, 10, NULL);
}

//啟動鍵
void CSlideCrank1Dlg::OnBnClickedButtonStart ()
{
	//判斷使用者輸入之轉速是否為零
	if (abs (m_dRpm) < pow (10., -7))
	{
		AfxMessageBox (_T ("Please input nonzero angular velocity."));
		m_bCheckRpm = FALSE;
	}
	else
		m_bCheckRpm = TRUE;

	//判斷使用者輸入之時間是否為正數
	if (m_dSettledTime <= 0)
	{
		AfxMessageBox (_T ("Please input positive acceleration time."));
		m_bCheckSec = FALSE;
		m_dSettledTime = 0;
	}
	else
		m_bCheckSec = TRUE;

	UpdateData (FALSE);

	if (m_bCheckRpm && m_bCheckSec)
	{
		( (CEdit* ) GetDlgItem (IDC_EDIT_ROTSPEED))->SetReadOnly (TRUE);					//啟動後設定編輯框read only
		( (CEdit* ) GetDlgItem (IDC_EDIT_SETTLEDTIME))->SetReadOnly (TRUE);					//啟動後設定編輯框read only
		( (CEdit*) GetDlgItem (IDC_STATIC_ACCTIME))->SetWindowTextA ("Acceleration time");	//按下開始鍵後顯示"加速時間"
		m_dAngVelocity = m_dRpm * 2 * M_PI / 60;		//將rpm轉換為rad/s
		m_dAngAcc = m_dAngVelocity / m_dSettledTime;	//計算角加速度
		m_bIsAccelerating = TRUE;						//設定加速狀態
		m_bIni = TRUE;									//進入初始化
		m_bEnterUniform = TRUE;							//設定加速狀態下為TRUE
		m_bStop = FALSE;								//設定m_bStop為FALSE
		SetTimer (0, 10, NULL);							//啟動Timer
		m_clkTimeIni = clock ();						//紀錄狀態轉換時間
		m_clkTimeForward = clock ();					//紀錄開始加速時間
	}
}

//停止鍵
void CSlideCrank1Dlg::OnBnClickedButtonStop ()
{
	( (CEdit*) GetDlgItem (IDC_STATIC_ACCTIME))->SetWindowTextA ("Deceleration time");		//按下停止後顯示"減速時間"
	m_bIsAccelerating = FALSE;		//設定減速狀態
	m_bIni = TRUE;					//進入初始化
	m_clkTimeIni = clock ();		//紀錄狀態轉換時間
	m_clkTimeForward = clock ();	//紀錄開始減速時間
}

//使用者設定左平台高度
void CSlideCrank1Dlg::OnEnChangeEditLheight ()

{
	UpdateData (TRUE);	//取得使用者輸入之左平台高度

	//判斷使用者輸入之左平台高度是否介於設定範圍0~200
	if (m_iLHeight < 0 || m_iLHeight > H_LIMIT)
	{
		AfxMessageBox (_T ("Please input the positive integer ranging from 0 to 200."));
		m_iLHeight = 0;	
	}

	UpdateData (FALSE);

	//進入OnPaint重繪
	InvalidateRect (m_rectWorkspace, FALSE);
	UpdateWindow ();
}

//使用者設定右平台高度
void CSlideCrank1Dlg::OnEnChangeEditRheight ()	
{
	UpdateData (TRUE);	//取得使用者輸入之右平台高度

	//判斷使用者輸入之右平台高度是否介於設定範圍0~200
	if (m_iRHeight < 0 || m_iRHeight > H_LIMIT)
	{
		AfxMessageBox (_T ("Please input the positive integer ranging from 0 to 200."));
		m_iRHeight = 0;
	}

	UpdateData (FALSE);

	//進入OnPaint重繪
	InvalidateRect (m_rectWorkspace, FALSE);
	UpdateWindow ();
}

void CSlideCrank1Dlg::OnEnKillfocusEditRotspeed ()
{
	UpdateData (TRUE);
}

void CSlideCrank1Dlg::OnEnKillfocusEditSettledtime ()
{
	UpdateData (TRUE);
}

//按ENTER, ESC不會退出程式
BOOL CSlideCrank1Dlg::PreTranslateMessage (MSG* pMsg)
{
	if (WM_KEYDOWN == pMsg->message)
	{
		UINT nKey = (int) pMsg->wParam; 
		if (VK_RETURN == nKey || VK_ESCAPE == nKey)
			return TRUE ;
	}

	return CDialogEx::PreTranslateMessage (pMsg);
}

//換算等效角度，使其值介於-2pi~2pi
void ThetaTransform (double &theta)
{
	if (abs (theta) > 2 * M_PI)
	{
		if (theta > 0)
			theta -= int (theta / 2 / M_PI) * 2 * M_PI;
		else
			theta += int (abs (theta) / 2 / M_PI) * 2 * M_PI;
	}
}