
// LegAnglesDlg.h : header file
//

#pragma once


// CLegAnglesDlg dialog
class CLegAnglesDlg : public CDialogEx
{
// Construction
public:
	CLegAnglesDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LEGANGLES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	int m_hipMin, m_hipMax;
	int m_FrontKneeMin, m_FrontKneeMax, m_RearKneeMin, m_RearKneeMax;
	double *m_FrontTbl[2], *m_RearTbl[2];
	int m_nFrontTbl, m_nRearTbl;
// Implementation
protected:
	HICON m_hIcon;
	double m_UpperLen, m_LowerLen, m_TotalLen;
	int FrontHip, FrontKnee, RearHip, RearKnee;
	char m_InputFileName[400];
	char m_VertHorzFileName[400];

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeFrontHip();
	afx_msg void OnEnKillfocusFrontHip();
	afx_msg void OnBnClickedSaveFront();
	afx_msg void OnEnChangeFrontKnee();
	afx_msg void OnEnKillfocusFrontKnee();
	afx_msg void OnEnChangeRearHip();
	afx_msg void OnEnKillfocusRearHip();
	afx_msg void OnEnChangeRearKnee();
	afx_msg void OnEnKillfocusRearKnee();
	afx_msg void OnBnClickedSaveRear();
	afx_msg void OnBnClickedCalcAngles();
	afx_msg void OnBnClickedSearchInput();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedFrontTable();
	afx_msg void OnBnClickedSearchVerthorzFile();
	afx_msg void OnBnClickedCalcLegAngles();
};
