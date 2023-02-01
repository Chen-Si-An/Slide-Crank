
// SlideCrank1Dlg.cpp : ��@��
//


#include "stdafx.h"
#include "SlideCrank1.h"
#include "SlideCrank1Dlg.h"
#include "afxdialogex.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

//define�����u�@�Ϫ��`��
#define GAP	10				//�u�@�ϻP��ܮ���ɤ�����
#define BOTTOM	302			//�u�@��bottom��m

//define�������c�ؤo���`��
#define GROUND_WIDTH 210	//���x�e��
#define GROUND_HEIGHT 25	//���x����
#define RADIUS 55			//��b�b�|
#define SLIDE_WIDTH 60		//�ƶ��e��
#define SLIDE_HEIGHT 30		//�ƶ�����
#define OVERLAP 10			//��b�򩳻P��b���|���� (���e�򩳦A�e��b)
#define ROD_TIP_RADIUS 5	//�����I�b�|
#define H_LIMIT 200			//���x�W������

#define BLACK RGB (0, 0, 0)
#define WHITE RGB (255, 255, 255)
#define GRAY  RGB (240, 240, 240)
#define BLUE  RGB (135, 206, 235)
#define BROWN RGB (128, 42, 42)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void ThetaTransform (double &);

// �� App About �ϥ� CAboutDlg ��ܤ��

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg ();

// ��ܤ�����
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV �䴩

// �{���X��@
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


// CSlideCrank1Dlg ��ܤ��



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


// CSlideCrank1Dlg �T���B�z�`��

BOOL CSlideCrank1Dlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();

	// �N [����...] �\���[�J�t�Υ\���C

	// IDM_ABOUTBOX �����b�t�ΩR�O�d�򤧤��C
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

	// �]�w����ܤ�����ϥܡC�����ε{�����D�������O��ܤ���ɡA
	// �ج[�|�۰ʱq�Ʀ��@�~
	SetIcon (m_hIcon, TRUE);		// �]�w�j�ϥ�
	SetIcon (m_hIcon, FALSE);		// �]�w�p�ϥ�


	UpdateData (FALSE);

	//��l����ܬ�"�[�t�ɶ�"
	( (CEdit*) GetDlgItem (IDC_STATIC_ACCTIME))->SetWindowTextA ("Acceleration time");

	//�]�w���current speed and acceleration time��Edit��read only
	( (CEdit* ) GetDlgItem (IDC_EDIT_CURSPEED))->SetReadOnly (TRUE);
	( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetReadOnly (TRUE);

	CString strCurSpeed, strAccTime;

	//��l�����current speed��Edit
	strCurSpeed.Format (_T ("%lf"), 0.0);
	( (CEdit* ) GetDlgItem (IDC_EDIT_CURSPEED))->SetWindowTextA (strCurSpeed);

	//��l�����acceleration time��Edit
	strAccTime.Format (_T ("%lf"), 0.0);
	( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);

	//�]�w�L�İ� (��ܮا��Шt�U)
	GetClientRect (&m_rectWorkspace);
	m_rectWorkspace.top += GAP;
	m_rectWorkspace.left += GAP;
	m_rectWorkspace.bottom = BOTTOM + GAP;
	m_rectWorkspace.right -= GAP;

	return TRUE;  // �Ǧ^ TRUE�A���D�z�ﱱ��]�w�J�I
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

// �p�G�N�̤p�ƫ��s�[�J�z����ܤ���A�z�ݭn�U�C���{���X�A
// �H�Kø�s�ϥܡC���ϥΤ��/�˵��Ҧ��� MFC ���ε{���A
// �ج[�|�۰ʧ������@�~�C

void CSlideCrank1Dlg::OnPaint ()
{
	CPaintDC dcDlg (this);
	CRect rectWorkspace;

	//�w�q�u�@�ϰ�
	GetClientRect (&rectWorkspace);
	rectWorkspace.bottom = BOTTOM;
	rectWorkspace.right -= 2 * GAP;

	//�إ߻P��ܮجۮe���O������ܸ˸m�H��{���w��
	CDC dcMem;
	CBitmap BitmapTemp, *pOldBitmapTemp;

	dcMem.CreateCompatibleDC (&dcDlg);
	BitmapTemp.CreateCompatibleBitmap (&dcDlg, rectWorkspace.Width (), rectWorkspace.Height ());
	pOldBitmapTemp = dcMem.SelectObject (&BitmapTemp);
	dcMem.FillSolidRect (rectWorkspace, GRAY);

	//�u�@�Ϯy�Шt�U
	int iWorkspaceXLimit = rectWorkspace.right;			//�]�w�u�@��x�y�з���			
	int iWorkspaceYLimit = rectWorkspace.bottom - 2;	//�]�w�u�@��y�y�з���	
	int iLGroundY = iWorkspaceYLimit - m_iLHeight;		//�]�w�����xy�y��
	int iRGroundY = iWorkspaceYLimit - m_iRHeight;		//�]�w�k���xy�y��
	int iShaftCenterX = int (iWorkspaceXLimit / 2);		//�]�w��b�b��x�y��		
	int iShaftCenterY = int (iWorkspaceYLimit / 2);		//�]�w��b�b��y�y��	

	//�]�w��s�����
	double iLRod = iShaftCenterX - SLIDE_WIDTH / 2 - RADIUS;	
	double iRRod = iWorkspaceXLimit - iShaftCenterX - SLIDE_WIDTH / 2 - RADIUS;

	//�إߵe���P�e�ꪫ��
	CPen penGround (PS_SOLID, 1, BLACK);
	CBrush brushGround (WHITE);
	CPen penMech (PS_SOLID, 1, GRAY);
	CBrush brushMech (BLUE);
	CPen penRod (PS_SOLID, 3, BROWN);
	CBrush brushRod (BROWN);
	CPen* pOldPen;
	CBrush* pOldBrush;

	//ø�sground
	pOldPen = dcMem.SelectObject (&penGround);
	pOldBrush = dcMem.SelectObject (&brushGround);

	POINT ptShaftGround[4] = {{iShaftCenterX - RADIUS, iWorkspaceYLimit},								
							  {iShaftCenterX + RADIUS, iWorkspaceYLimit},
							  {iShaftCenterX + int (RADIUS / 2), iShaftCenterY + RADIUS - OVERLAP},
							  {iShaftCenterX - int (RADIUS / 2), iShaftCenterY + RADIUS - OVERLAP}}; 

	dcMem.Polygon (ptShaftGround, 4);																			//ø�s��b��
	dcMem.Rectangle (0, iLGroundY - GROUND_HEIGHT, GROUND_WIDTH, iLGroundY);									//ø�s���ƶ����x
	dcMem.Rectangle (iWorkspaceXLimit - GROUND_WIDTH, iRGroundY - GROUND_HEIGHT, iWorkspaceXLimit, iRGroundY);	//ø�s�k�ƶ����x

	double dCosTheta = cos (m_dAng);	//�p��cos
	double dSinTheta = sin (m_dAng);	//�p��sin

	//ø�s��b�P�ƶ�
	dcMem.SelectObject (&penMech);
	dcMem.SelectObject (&brushMech);

	dcMem.Ellipse (iShaftCenterX - RADIUS, iShaftCenterY - RADIUS, iShaftCenterX + RADIUS, iShaftCenterY + RADIUS);												//ø�s��b

	double iLSlideX = iShaftCenterX - RADIUS * dCosTheta - sqrt (pow (iLRod, 2) - pow (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - iShaftCenterY - RADIUS * dSinTheta, 2));	//�p��ƶ�1�����I��x�y��
	double iRSlideX = iShaftCenterX + RADIUS * dCosTheta + sqrt (pow (iRRod, 2) - pow (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - iShaftCenterY + RADIUS * dSinTheta, 2));	//�p��ƶ�2�����I��x�y��

	dcMem.Rectangle (int (iLSlideX - SLIDE_WIDTH / 2), iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT, int (iLSlideX + SLIDE_WIDTH/2), iLGroundY - GROUND_HEIGHT);	//ø�s���ƶ�
	dcMem.Rectangle (int (iRSlideX - SLIDE_WIDTH / 2), iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT, int (iRSlideX + SLIDE_WIDTH/2), iRGroundY - GROUND_HEIGHT);	//ø�s�k�ƶ�

	//ø�s��s��
	dcMem.SelectObject (&penRod);
	dcMem.SelectObject (&brushRod);

	dcMem.MoveTo  (int (iShaftCenterX - RADIUS * dCosTheta), int (iShaftCenterY + RADIUS * dSinTheta));										//���s�����b�W����m
	dcMem.LineTo  (int (iLSlideX), int (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2));														//���s���ƶ��W����m
	dcMem.Ellipse (int (iShaftCenterX - RADIUS * dCosTheta - ROD_TIP_RADIUS), int (iShaftCenterY + RADIUS * dSinTheta - ROD_TIP_RADIUS),	
				   int (iShaftCenterX - RADIUS * dCosTheta + ROD_TIP_RADIUS), int (iShaftCenterY + RADIUS * dSinTheta + ROD_TIP_RADIUS));	//ø�s���s�����b�W���I		
	dcMem.Ellipse (int (iLSlideX - ROD_TIP_RADIUS), int (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - ROD_TIP_RADIUS),						
				   int (iLSlideX + ROD_TIP_RADIUS), int (iLGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 + ROD_TIP_RADIUS));					//ø�s���s���ƶ��W���I
	
	dcMem.MoveTo  (int (iShaftCenterX + RADIUS * dCosTheta), int (iShaftCenterY - RADIUS * dSinTheta));										//�k�s�����b�W����m
	dcMem.LineTo  (int (iRSlideX), int (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2));														//�k�s���ƶ��W����m
	dcMem.Ellipse (int (iShaftCenterX + RADIUS * dCosTheta - ROD_TIP_RADIUS), int (iShaftCenterY - RADIUS * dSinTheta - ROD_TIP_RADIUS),	
				   int (iShaftCenterX + RADIUS * dCosTheta + ROD_TIP_RADIUS), int (iShaftCenterY - RADIUS * dSinTheta + ROD_TIP_RADIUS));	//ø�s�k�s�����b�W���I
	dcMem.Ellipse (int (iRSlideX - ROD_TIP_RADIUS), int (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 - ROD_TIP_RADIUS),						
				   int (iRSlideX + ROD_TIP_RADIUS), int (iRGroundY - GROUND_HEIGHT - SLIDE_HEIGHT / 2 + ROD_TIP_RADIUS));					//ø�s�k�s���ƶ��W���I

	dcDlg.BitBlt (GAP, GAP, rectWorkspace.Width (), rectWorkspace.Height (), &dcMem, 0, 0, SRCCOPY);	//�Nø�s��O������ܸ˸m���I�}�ϫ����ܹ�ܮ�
	
	//�M�z
	dcMem.SelectObject (pOldPen);
	dcMem.SelectObject (pOldBrush);
	dcMem.SelectObject (pOldBitmapTemp);
	BitmapTemp.DeleteObject ();	
	dcMem.DeleteDC ();
}

// ��ϥΪ̩즲�̤p�Ƶ����ɡA
// �t�ΩI�s�o�ӥ\����o�����ܡC
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
		static double s_dCurAng = 0., s_dCurAngVelocity = 0., s_dTimeNeed = 0.;			//�ŧi�R�A�ܼ�current angle, current angular velocity, �[��t�һݮɶ�
		CString strCurSpeed, strAccTime;

		m_clkTimeCurrent = clock ();													//����current time
		dTimeAcc  = (m_clkTimeCurrent - m_clkTimeIni) / (double) (CLOCKS_PER_SEC);		//�p��[�tor��t���A�g�L�`�ɶ�
		dTimeStep = (m_clkTimeCurrent - m_clkTimeForward) / (double) (CLOCKS_PER_SEC);	//�p��@��step�g�L�ɶ�
		m_clkTimeForward = clock ();													//�����e�@��step���ɶ�

		//�[�t
		if (m_bIsAccelerating)
		{
			//���A�ഫ�ɶi�J�@�� (��l��)
			if (m_bIni)	
			{
				s_dTimeNeed = (m_dAngVelocity - s_dCurAngVelocity) / m_dAngAcc;	//�p��[�t�һݮɶ�
				m_bIni = FALSE;
			}

			//�P�_�O�_�w�[�t�ܨϥΪ̳]�w��t
			if (dTimeAcc <= s_dTimeNeed)
			{
				m_dAng = s_dCurAng + s_dCurAngVelocity * dTimeStep + m_dAngAcc * pow (dTimeStep, 2) / 2;							//���[�t�׫�ҹF���צ�m
				s_dCurAngVelocity += m_dAngAcc * dTimeStep;	//����current angular velocity
				
				//���accelration time
				strAccTime.Format (_T ("%lf"), dTimeAcc);
				( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
			}
			else
			{
				if (m_bEnterUniform)	//�[�t���A�U�F���t�A���q�p�ⵥ�[�t�פε��t�ұ��ध����
				{
					m_dAng = s_dCurAng + s_dCurAngVelocity * (dTimeStep - dTimeAcc + s_dTimeNeed) +
						m_dAngAcc * pow (dTimeStep - dTimeAcc + s_dTimeNeed, 2) / 2 + m_dAngVelocity * (dTimeAcc - s_dTimeNeed);	//�[�t�ܫ��w��t��A�H���t����
					s_dCurAngVelocity = m_dAngVelocity;		//����current angular velocity
					m_bEnterUniform = FALSE;

					//���accelration time
					strAccTime.Format (_T ("%lf"), dTimeAcc);
					( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
				}
				else
				{
					m_dAng = s_dCurAng + s_dCurAngVelocity * dTimeStep;																//���t����ҹF���צ�m
					s_dCurAngVelocity = m_dAngVelocity;		//����current angular velocity
				}
			}

		}
		//��t
		else
		{
			//���A�ഫ�ɶi�J�@�� (��l��)
			if (m_bIni)
			{
				s_dTimeNeed = s_dCurAngVelocity / m_dAngAcc;	//�p���t�һݮɶ�
				m_bIni = FALSE;
			}

			//�P�_�O�_��t���R��
			if (dTimeAcc <= s_dTimeNeed)
			{
				m_dAng = s_dCurAng + s_dCurAngVelocity * dTimeStep - m_dAngAcc * pow (dTimeStep, 2) / 2;	//����t�׫�ҹF���צ�m
				s_dCurAngVelocity -= m_dAngAcc * dTimeStep;	//����current angular velocity

				//���decelration time
				strAccTime.Format (_T ("%lf"), dTimeAcc);
				( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
			}
			else
			{
				m_dAng = s_dCurAng + s_dCurAngVelocity * (dTimeStep - dTimeAcc + s_dTimeNeed) -				//�R��ɨ��צ�m
					m_dAngAcc * pow (dTimeStep - dTimeAcc + s_dTimeNeed, 2) / 2 ;
				s_dCurAngVelocity = 0;						//����current angular velocity

				m_bStop = TRUE;	//��t���R��ɳ]�wm_bStop��TRUE

				( (CEdit* ) GetDlgItem (IDC_EDIT_ROTSPEED))->SetReadOnly (FALSE);		//�R���Ѱ��s���read only		
				( (CEdit* ) GetDlgItem (IDC_EDIT_SETTLEDTIME))->SetReadOnly (FALSE);	//�R���Ѱ��s���read only		

				//���decelration time
				strAccTime.Format (_T ("%lf"), dTimeAcc);
				( (CEdit* ) GetDlgItem (IDC_EDIT_ACCTIME))->SetWindowTextA (strAccTime);
			}
		}


		ThetaTransform (m_dAng);	//���Ĩ����ഫ
		s_dCurAng = m_dAng;			//����current angle

		//���current speed
		strCurSpeed.Format (_T ("%lf"), s_dCurAngVelocity / 2 / M_PI * 60);
		( (CEdit* ) GetDlgItem (IDC_EDIT_CURSPEED))->SetWindowTextA (strCurSpeed);

		//�i�JOnPaint��ø
		InvalidateRect (m_rectWorkspace, FALSE);
		UpdateWindow ();
    }

    CDialogEx::OnTimer (nIDEvent);

	//m_bStop��TRUE�A�Y�R��ɡA���A����Timer
	if (!m_bStop)
		SetTimer (0, 10, NULL);
}

//�Ұ���
void CSlideCrank1Dlg::OnBnClickedButtonStart ()
{
	//�P�_�ϥΪ̿�J����t�O�_���s
	if (abs (m_dRpm) < pow (10., -7))
	{
		AfxMessageBox (_T ("Please input nonzero angular velocity."));
		m_bCheckRpm = FALSE;
	}
	else
		m_bCheckRpm = TRUE;

	//�P�_�ϥΪ̿�J���ɶ��O�_������
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
		( (CEdit* ) GetDlgItem (IDC_EDIT_ROTSPEED))->SetReadOnly (TRUE);					//�Ұʫ�]�w�s���read only
		( (CEdit* ) GetDlgItem (IDC_EDIT_SETTLEDTIME))->SetReadOnly (TRUE);					//�Ұʫ�]�w�s���read only
		( (CEdit*) GetDlgItem (IDC_STATIC_ACCTIME))->SetWindowTextA ("Acceleration time");	//���U�}�l������"�[�t�ɶ�"
		m_dAngVelocity = m_dRpm * 2 * M_PI / 60;		//�Nrpm�ഫ��rad/s
		m_dAngAcc = m_dAngVelocity / m_dSettledTime;	//�p�⨤�[�t��
		m_bIsAccelerating = TRUE;						//�]�w�[�t���A
		m_bIni = TRUE;									//�i�J��l��
		m_bEnterUniform = TRUE;							//�]�w�[�t���A�U��TRUE
		m_bStop = FALSE;								//�]�wm_bStop��FALSE
		SetTimer (0, 10, NULL);							//�Ұ�Timer
		m_clkTimeIni = clock ();						//�������A�ഫ�ɶ�
		m_clkTimeForward = clock ();					//�����}�l�[�t�ɶ�
	}
}

//������
void CSlideCrank1Dlg::OnBnClickedButtonStop ()
{
	( (CEdit*) GetDlgItem (IDC_STATIC_ACCTIME))->SetWindowTextA ("Deceleration time");		//���U��������"��t�ɶ�"
	m_bIsAccelerating = FALSE;		//�]�w��t���A
	m_bIni = TRUE;					//�i�J��l��
	m_clkTimeIni = clock ();		//�������A�ഫ�ɶ�
	m_clkTimeForward = clock ();	//�����}�l��t�ɶ�
}

//�ϥΪ̳]�w�����x����
void CSlideCrank1Dlg::OnEnChangeEditLheight ()

{
	UpdateData (TRUE);	//���o�ϥΪ̿�J�������x����

	//�P�_�ϥΪ̿�J�������x���׬O�_����]�w�d��0~200
	if (m_iLHeight < 0 || m_iLHeight > H_LIMIT)
	{
		AfxMessageBox (_T ("Please input the positive integer ranging from 0 to 200."));
		m_iLHeight = 0;	
	}

	UpdateData (FALSE);

	//�i�JOnPaint��ø
	InvalidateRect (m_rectWorkspace, FALSE);
	UpdateWindow ();
}

//�ϥΪ̳]�w�k���x����
void CSlideCrank1Dlg::OnEnChangeEditRheight ()	
{
	UpdateData (TRUE);	//���o�ϥΪ̿�J���k���x����

	//�P�_�ϥΪ̿�J���k���x���׬O�_����]�w�d��0~200
	if (m_iRHeight < 0 || m_iRHeight > H_LIMIT)
	{
		AfxMessageBox (_T ("Please input the positive integer ranging from 0 to 200."));
		m_iRHeight = 0;
	}

	UpdateData (FALSE);

	//�i�JOnPaint��ø
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

//��ENTER, ESC���|�h�X�{��
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

//���ⵥ�Ĩ��סA�Ϩ�Ȥ���-2pi~2pi
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