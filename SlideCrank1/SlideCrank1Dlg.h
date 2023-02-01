
// SlideCrank1Dlg.h : ���Y��
//

#pragma once


// CSlideCrank1Dlg ��ܤ��
class CSlideCrank1Dlg : public CDialogEx
{
// �غc
public:
	CSlideCrank1Dlg (CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
	enum { IDD = IDD_SLIDECRANK1_DIALOG };

	protected:
	virtual void DoDataExchange (CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog ();
	afx_msg void OnSysCommand (UINT nID, LPARAM lParam);
	afx_msg void OnPaint ();
	afx_msg HCURSOR OnQueryDragIcon ();
	afx_msg void OnTimer (UINT_PTR nIDEvent); 
	DECLARE_MESSAGE_MAP ()

public:
	afx_msg void OnBnClickedButtonStart ();
	afx_msg void OnBnClickedButtonStop ();
	afx_msg void OnEnChangeEditLheight ();
	afx_msg void OnEnChangeEditRheight ();
	virtual BOOL PreTranslateMessage (MSG* pMsg);
	afx_msg void OnEnKillfocusEditRotspeed ();
	afx_msg void OnEnKillfocusEditSettledtime ();

private:
	int m_iLHeight;
	int m_iRHeight;
	double m_dRpm;
	double m_dSettledTime;
	double m_dAng, m_dAngVelocity, m_dAngAcc;
	clock_t m_clkTimeIni, m_clkTimeForward, m_clkTimeCurrent;						
	BOOL m_bCheckRpm, m_bCheckSec, m_bIsAccelerating, m_bIni, m_bEnterUniform, m_bStop;
	CRect m_rectWorkspace;
};
