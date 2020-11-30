
// LegAnglesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LegAngles.h"
#include "LegAnglesDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PI 3.141592856
#define ANGLETORAD (PI/180.0)
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CLegAnglesDlg dialog



CLegAnglesDlg::CLegAnglesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLegAnglesDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLegAnglesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLegAnglesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_FRONT_HIP, &CLegAnglesDlg::OnEnChangeFrontHip)
	ON_EN_KILLFOCUS(IDC_FRONT_HIP, &CLegAnglesDlg::OnEnKillfocusFrontHip)
	ON_BN_CLICKED(IDC_SAVE_FRONT, &CLegAnglesDlg::OnBnClickedSaveFront)
	ON_EN_CHANGE(IDC_FRONT_KNEE, &CLegAnglesDlg::OnEnChangeFrontKnee)
	ON_EN_KILLFOCUS(IDC_FRONT_KNEE, &CLegAnglesDlg::OnEnKillfocusFrontKnee)
	ON_EN_CHANGE(IDC_REAR_HIP, &CLegAnglesDlg::OnEnChangeRearHip)
	ON_EN_KILLFOCUS(IDC_REAR_HIP, &CLegAnglesDlg::OnEnKillfocusRearHip)
	ON_EN_CHANGE(IDC_REAR_KNEE, &CLegAnglesDlg::OnEnChangeRearKnee)
	ON_EN_KILLFOCUS(IDC_REAR_KNEE, &CLegAnglesDlg::OnEnKillfocusRearKnee)
	ON_BN_CLICKED(IDC_SAVE_REAR, &CLegAnglesDlg::OnBnClickedSaveRear)
	ON_BN_CLICKED(IDC_CALC_ANGLES, &CLegAnglesDlg::OnBnClickedCalcAngles)
	ON_BN_CLICKED(IDC_SEARCH_INPUT, &CLegAnglesDlg::OnBnClickedSearchInput)
	ON_WM_CHAR()
	ON_WM_CLOSE()
	ON_WM_SYSCHAR()
	ON_BN_CLICKED(IDOK, &CLegAnglesDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_FRONT_TABLE, &CLegAnglesDlg::OnBnClickedFrontTable)
	ON_BN_CLICKED(IDC_SEARCH_VERTHORZ_FILE, &CLegAnglesDlg::OnBnClickedSearchVerthorzFile)
	ON_BN_CLICKED(IDC_CALC_LEG_ANGLES, &CLegAnglesDlg::OnBnClickedCalcLegAngles)
END_MESSAGE_MAP()


// CLegAnglesDlg message handlers

BOOL CLegAnglesDlg::OnInitDialog()
{
	FrontHip = FrontKnee = RearHip = RearKnee = 0;
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Set the lengths of upper and lower leg
	m_UpperLen = 1.0;
	m_LowerLen = 1.36;
	m_TotalLen = m_LowerLen + m_UpperLen;

	// Create the angle look up table	FILE *fp = fopen("FrontLegs.txt", "w");
	m_hipMin = -90;
	m_hipMax = 90;
	m_FrontKneeMin = -90;
	m_FrontKneeMax = 90;
	m_RearKneeMin = -90;
	m_RearKneeMax = 90;
	int nHip = m_hipMax - m_hipMin;
	int nKneeFront = m_FrontKneeMax - m_FrontKneeMin;
	int nKneeRear = m_RearKneeMax - m_RearKneeMin;
	m_nFrontTbl = nKneeFront*nHip;
	m_nRearTbl = nKneeRear*nHip;
	m_FrontTbl[0] = new double[m_nFrontTbl];
	m_FrontTbl[1] = new double[m_nFrontTbl];
	m_RearTbl[0] = new double[m_nRearTbl];
	m_RearTbl[1] = new double[m_nRearTbl];
	for (int knee = m_FrontKneeMin; knee < m_FrontKneeMax; knee++) {
		for (int hip = m_hipMin; hip < m_hipMax; hip++) {
			double hipRad = hip * ANGLETORAD;
			double kneeRad = knee * ANGLETORAD;
			double disth = m_UpperLen* cos(hipRad);
			double distk = m_LowerLen*sin(hipRad + kneeRad);
			double vdist = m_TotalLen - (disth + distk);
			double hdist = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);
			int idx = hip - m_hipMin + (knee - m_FrontKneeMin)*nHip;

			m_FrontTbl[0][idx] = vdist;
			m_FrontTbl[1][idx] = hdist;
		}
	}
	for (int knee = m_RearKneeMin; knee < m_RearKneeMax; knee++) {
		for (int hip = m_hipMin; hip < m_hipMax; hip++) {
			double hipRad = hip * ANGLETORAD;
			double kneeRad = knee * ANGLETORAD;
			double disth = m_UpperLen*cos(hipRad);
			double distk = -m_LowerLen*sin(hipRad + kneeRad);
			double vdist = m_TotalLen - (disth + distk);
			double hdist = m_UpperLen*sin(hipRad) + m_LowerLen*cos(hipRad + kneeRad);
			int idx = hip - m_hipMin + (knee - m_RearKneeMin)*nHip;
			m_RearTbl[0][idx] = vdist;
			m_RearTbl[1][idx] = hdist;
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CLegAnglesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLegAnglesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLegAnglesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool GetInt(CString cStr, int &val)
{
	char *pCh = cStr.GetBuffer();
	int sign = 1;
	while (*pCh == ' ') pCh++;
	if (*pCh == '-') sign = -1, pCh++;
	val = sign * atoi(pCh);
	while (isdigit(*pCh)) pCh++;
	while (*pCh == ' ') pCh++;
	if (*pCh) {
		AfxMessageBox("edit box may contain only characters 0-9 and '-'", MB_OK);
		val = 0;
		return false;
	}
	return true;
}


void CLegAnglesDlg::OnEnChangeFrontHip()
{
	CString cStr;
	GetDlgItem(IDC_FRONT_HIP)->GetWindowText(cStr);
	char *pCh = cStr.GetBuffer();
	if (!GetInt(cStr, FrontHip)) return;
}


void CLegAnglesDlg::OnEnKillfocusFrontHip()
{
	char buf[100];
	double hipRad = FrontHip * ANGLETORAD;
	double kneeRad = FrontKnee * ANGLETORAD;
	double disth = m_UpperLen*cos(hipRad);
	double distk = m_LowerLen*sin(hipRad + kneeRad);
	double vdist = m_TotalLen - (disth + distk);
	double hdist = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);
	sprintf(buf, "hip: %d  knee: %d  vert: %5.3lf  horz: %5.3lf", FrontHip, FrontKnee, vdist, hdist);
	GetDlgItem(IDC_FRONT_RESULTS)->SetWindowTextA(buf);
}


void CLegAnglesDlg::OnBnClickedSaveFront()
{
	char buf[100];
	double hipRad = FrontHip * ANGLETORAD;
	double kneeRad = FrontKnee * ANGLETORAD;
	double disth = m_UpperLen*cos(hipRad);
	double distk = m_LowerLen*sin(hipRad + kneeRad);
	double vdist = m_TotalLen - (disth + distk);
	double hdist = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);
	sprintf(buf, "hip: %d  knee: %d  vert: %5.3lf  horz: %5.3lf", FrontHip, FrontKnee, vdist, hdist);
	CListBox *pList = (CListBox*)GetDlgItem(IDC_FRONT_LIST);

	pList->AddString(buf);
}

void CLegAnglesDlg::OnEnChangeFrontKnee()
{
	CString cStr;
	GetDlgItem(IDC_FRONT_KNEE)->GetWindowText(cStr);
	char *pCh = cStr.GetBuffer();
	if (!GetInt(cStr, FrontKnee)) return;
}


void CLegAnglesDlg::OnEnKillfocusFrontKnee()
{
	char buf[100];
	double hipRad = FrontHip * ANGLETORAD;
	double kneeRad = FrontKnee * ANGLETORAD;
	double disth = m_UpperLen*cos(hipRad);
	double distk = m_LowerLen*sin(hipRad + kneeRad);
	double vdist = m_TotalLen - (disth + distk);
	double hdist = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);
	sprintf(buf, "hip: %d  knee: %d  vert: %5.3lf  horz: %5.3lf", FrontHip, FrontKnee, vdist, hdist);
	GetDlgItem(IDC_FRONT_RESULTS)->SetWindowTextA(buf);
}


void CLegAnglesDlg::OnEnChangeRearHip()
{
	CString cStr;
	GetDlgItem(IDC_REAR_HIP)->GetWindowText(cStr);
	char *pCh = cStr.GetBuffer();
	if (!GetInt(cStr, RearHip)) return;
}


void CLegAnglesDlg::OnEnKillfocusRearHip()
{
	char buf[100];
	double hipRad = RearHip * ANGLETORAD;
	double kneeRad = RearKnee * ANGLETORAD;
	double disth = m_UpperLen*cos(hipRad);
	double distk = -m_LowerLen*sin(hipRad + kneeRad);
	double vdist = m_TotalLen - (disth + distk);
	double hdist = m_UpperLen*sin(hipRad) + m_LowerLen*cos(hipRad + kneeRad);
	sprintf(buf, "hip: %d  knee: %d  vert: %5.3lf  horz: %5.3lf", RearHip, RearKnee, vdist, hdist);
	GetDlgItem(IDC_REAR_RESULTS)->SetWindowTextA(buf);
}


void CLegAnglesDlg::OnEnChangeRearKnee()
{
	CString cStr;
	GetDlgItem(IDC_REAR_KNEE)->GetWindowText(cStr);
	char *pCh = cStr.GetBuffer();
	if (!GetInt(cStr, RearKnee)) return;
}


void CLegAnglesDlg::OnEnKillfocusRearKnee()
{
	char buf[100];
	double hipRad = RearHip * ANGLETORAD;
	double kneeRad = RearKnee * ANGLETORAD;
	double disth = m_UpperLen*cos(hipRad);
	double distk = -m_LowerLen*sin(hipRad + kneeRad);
	double vdist = m_TotalLen - (disth + distk);
	double hdist = m_UpperLen*sin(hipRad) + m_LowerLen*cos(hipRad + kneeRad);
	sprintf(buf, "hip: %d  knee: %d  vert: %5.3lf  horz: %5.3lf", RearHip, RearKnee, vdist, hdist);
	GetDlgItem(IDC_REAR_RESULTS)->SetWindowTextA(buf);
}


void CLegAnglesDlg::OnBnClickedSaveRear()
{
	char buf[100];
	double hipRad = RearHip * ANGLETORAD;
	double kneeRad = RearKnee * ANGLETORAD;
	double disth = m_UpperLen*cos(hipRad);
	double distk = -m_LowerLen*sin(hipRad + kneeRad);
	double vdist = m_TotalLen - (disth + distk);
	double hdist = m_UpperLen*sin(hipRad) + m_LowerLen*cos(hipRad + kneeRad);
	sprintf(buf, "hip: %d  knee: %d  vert: %5.3lf  horz: %5.3lf", RearHip, RearKnee, vdist, hdist);
	CListBox *pList = (CListBox*)GetDlgItem(IDC_REAR_LIST);

	pList->AddString(buf);
}


void CLegAnglesDlg::OnBnClickedCalcAngles()
{
	// check for valid input file name
	CString inStr,outStr;
	GetDlgItem(IDC_INPUT_FILE)->GetWindowText(inStr);
	FILE *fp = fopen(inStr.GetBuffer(), "r");
	if (fp == NULL) {
		AfxMessageBox("cannot open input file", MB_OK);
		return;
	}
	GetDlgItem(IDC_OUTPUT_FILE)->GetWindowText(outStr);

	// Read input file to see how many lines it has
	int lfh, rfh, rrh, lrh, lfk, rfk, rrk, lrk;
	int nLine = 0;
	while (!feof(fp)) {
		int nItem = fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,\n", &lfh, &rfh, &rrh, &lrh, &lfk, &rfk, &rrk, &lrk);
		nLine++;
	}
	fclose(fp);

	if (nLine == 0) {
		AfxMessageBox("input file is empty", MB_OK);
		return;
	}


	// Allocate arrays
	double *vLeftFront = new double[nLine];
	double *hLeftFront = new double[nLine];
	double *vRiteFront = new double[nLine];
	double *hRiteFront = new double[nLine];
	double *vRiteRear = new double[nLine];
	double *hRiteRear = new double[nLine];
	double *vLeftRear = new double[nLine];
	double *hLeftRear = new double[nLine];

	double *gapLeftFront = new double[nLine];
	double *gapRiteFront = new double[nLine];
	double *gapRiteRear = new double[nLine];
	double *gapLeftRear = new double[nLine];

	// Read input, calculate distances
	fp = fopen(inStr.GetBuffer(), "r");
	for (int i = 0; i < nLine; i++) {
		fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,\n", &lfh, &rfh, &rrh, &lrh, &lfk, &rfk, &rrk, &lrk);
		double hipRad, kneeRad, disth, distk;

		hipRad = lfh * ANGLETORAD;
		kneeRad = lfk * ANGLETORAD;
		disth = m_UpperLen*cos(hipRad);
		distk = m_LowerLen*sin(hipRad + kneeRad);
		vLeftFront[i] = m_TotalLen - (disth + distk);
		hLeftFront[i] = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);

		hipRad = rfh * ANGLETORAD;
		kneeRad = rfk * ANGLETORAD;
		disth = m_UpperLen*cos(hipRad);
		distk = m_LowerLen*sin(hipRad + kneeRad);
		vRiteFront[i] = m_TotalLen - (disth + distk);
		hRiteFront[i] = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);

		hipRad = rrh * ANGLETORAD;
		kneeRad = rrk * ANGLETORAD;
		disth = m_UpperLen*cos(hipRad);
		distk = -m_LowerLen*sin(hipRad + kneeRad);
		vRiteRear[i] = m_TotalLen - (disth + distk);
		hRiteRear[i] = m_UpperLen*sin(hipRad) + m_LowerLen*cos(hipRad + kneeRad);

		hipRad = lrh * ANGLETORAD;
		kneeRad = lrk * ANGLETORAD;
		disth = m_UpperLen*cos(hipRad);
		distk = -m_LowerLen*sin(hipRad + kneeRad);
		vLeftRear[i] = m_TotalLen - (disth + distk);
		hLeftRear[i] = m_UpperLen*sin(hipRad) + m_LowerLen*cos(hipRad + kneeRad);

		if (i > 0) {
			gapLeftFront[i] = hLeftFront[i] - hLeftFront[i - 1];
			gapRiteFront[i] = hRiteFront[i] - hRiteFront[i - 1];
			gapRiteRear[i] = hRiteRear[i] - hRiteRear[i - 1];
			gapLeftRear[i] = hLeftRear[i] - hLeftRear[i - 1];
		}
	}
	gapLeftFront[0] = hLeftFront[0] - hLeftFront[nLine - 1];
	gapRiteFront[0] = hRiteFront[0] - hRiteFront[nLine - 1];
	gapRiteRear[0] = hRiteRear[0] - hRiteRear[nLine - 1];
	gapLeftRear[0] = hLeftRear[0] - hLeftRear[nLine - 1];
	fclose(fp);

	// Write Output file
	fp = fopen(outStr.GetBuffer(), "w");
	if (fp == NULL) {
		AfxMessageBox("cannot open output file", MB_OK);
		return;
	}

	for (int i = 0; i < nLine; i++) {
		fprintf(fp, "%2.2d: %5.3lf %5.3lf %5.3lf %5.3lf   %5.3lf %5.3lf %5.3lf %5.3lf   %5.3lf %5.3lf %5.3lf %5.3lf\n",i,
			vLeftFront[i], vRiteFront[i], vRiteRear[i], vLeftRear[i],
			hLeftFront[i], hRiteFront[i], hRiteRear[i], hLeftRear[i],
			gapLeftFront[i], gapRiteFront[i], gapRiteRear[i], gapLeftRear[i]);
	}
	fclose(fp);
	delete gapLeftFront;
	delete gapRiteFront;
	delete gapRiteRear;
	delete gapLeftRear;
	delete vLeftFront;
	delete vRiteFront;
	delete vRiteRear;
	delete vLeftRear;
	delete hLeftFront;
	delete hRiteFront;
	delete hRiteRear;
	delete hLeftRear;
}

//*****************************************************************************

void CLegAnglesDlg::OnBnClickedSearchInput()
{
	TCHAR szFilters[] =
		_T("text files |*.txt|All Files (*.*)|*.*||");

	// Create an Open dialog; the default file name extension is ".txt".
	CFileDialog CFile(TRUE, _T("txt"), _T(""), OFN_HIDEREADONLY, szFilters, NULL);

	if (CFile.DoModal() != IDOK) return;

	CString fname = CFile.GetPathName();

	strcpy(m_InputFileName, CT2A(fname));
	GetDlgItem(IDC_INPUT_FILE)->SetWindowText(CString(m_InputFileName));
}



void CLegAnglesDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}


void CLegAnglesDlg::OnBnClickedFrontTable()
{
	FILE *fp = fopen("FrontLegs.txt", "w");
	fprintf(fp, "    ");
	for (int hip = 0; hip < 70; hip++) fprintf(fp, "      %2.2d      ", hip);
	fprintf(fp, "\n");
	for (int knee = 0; knee < 50; knee++) {
		fprintf(fp, "%2.2d: ", knee);
		for (int hip = 0; hip < 70; hip++) {
			double hipRad = hip * ANGLETORAD;
			double kneeRad = knee * ANGLETORAD;
			double disth = m_UpperLen*cos(hipRad);
			double distk = m_LowerLen*sin(hipRad + kneeRad);
			double vdist = m_TotalLen -(disth + distk);
			double hdist = m_UpperLen*sin(hipRad) - m_LowerLen*cos(hipRad + kneeRad);
			fprintf(fp, "%5.3lf,%6.3lf  ", vdist, hdist);

		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}


void CLegAnglesDlg::OnBnClickedSearchVerthorzFile()
{
TCHAR szFilters[] =
_T("text files |*.txt|All Files (*.*)|*.*||");

// Create an Open dialog; the default file name extension is ".txt".
CFileDialog CFile(TRUE, _T("txt"), _T(""), OFN_HIDEREADONLY, szFilters, NULL);

if (CFile.DoModal() != IDOK) return;

CString fname = CFile.GetPathName();

strcpy(m_VertHorzFileName, CT2A(fname));
GetDlgItem(IDC_VERTHORZ_FILE)->SetWindowText(CString(m_VertHorzFileName));
}


void CLegAnglesDlg::OnBnClickedCalcLegAngles()
{
	// check for valid input file name
	CString inStr, outStr;
	GetDlgItem(IDC_VERTHORZ_FILE)->GetWindowText(inStr);
	FILE *fp = fopen(inStr.GetBuffer(), "r");
	if (fp == NULL) {
		AfxMessageBox("cannot open input file", MB_OK);
		return;
	}
	GetDlgItem(IDC_LEG_ANGLES)->GetWindowText(outStr);
	FILE* fpOut = fopen(outStr.GetBuffer(), "w");
	if (fpOut == NULL) {
		AfxMessageBox("cannot open output file", MB_OK);
		fclose(fp);
		return;
	}

	// Read input file: each line must have 4 pairs of vert and horz distnaces. LF, RF, RR, LR
	double lfv, lfh, rfv, rfh, rrv, rrh, lrv, lrh;
	int nLine = 0;
	while (!feof(fp)) {
		int nItem = fscanf(fp, "%lf,%lf %lf,%lf %lf,%lf %lf,%lf\n", &lfv, &lfh, &rfv, &rfh, &rrv, &rrh, &lrv, &lrh);
		if (nItem != 8) {
			char msg[70];
			sprintf(msg,"error in line %d",nLine);
			AfxMessageBox(msg, MB_OK);
			fclose(fp);
			fclose(fpOut);
			return;
		}
		nLine++;

		int nHip = m_hipMax - m_hipMin;

		// look up left front.
		int bestIndex = 0;
		double vDist = lfv - m_FrontTbl[0][0];
		double hDist = lfh - m_FrontTbl[1][0];
		double bestDist = vDist*vDist + hDist*hDist;
		for (int i = 0; i < m_nFrontTbl; i++) {
			vDist = lfv - m_FrontTbl[0][i];
			hDist = lfh - m_FrontTbl[1][i];
			double dist = vDist*vDist + hDist*hDist;
			if (dist < bestDist) {
				bestDist = dist;
				bestIndex = i;
			}
		}
		int LFhipAngle = bestIndex % nHip + m_hipMin;
		int LFKneeAngle = bestIndex / nHip + m_FrontKneeMin;

		// look up right front.
		bestIndex = 0;
		vDist = rfv - m_FrontTbl[0][0];
		hDist = rfh - m_FrontTbl[1][0];
		bestDist = vDist*vDist + hDist*hDist;
		for (int i = 0; i < m_nFrontTbl; i++) {
			vDist = rfv - m_FrontTbl[0][i];
			hDist = rfh - m_FrontTbl[1][i];
			double dist = vDist*vDist + hDist*hDist;
			if (dist < bestDist) {
				bestDist = dist;
				bestIndex = i;
			}
		}
		int RFhipAngle = bestIndex % nHip + m_hipMin;
		int RFKneeAngle = bestIndex / nHip + m_FrontKneeMin;

		// look up right rear.
		bestIndex = 0;
		vDist = rrv - m_RearTbl[0][0];
		hDist = rrh - m_RearTbl[1][0];
		bestDist = vDist*vDist + hDist*hDist;
		for (int i = 0; i < m_nRearTbl; i++) {
			vDist = rrv - m_RearTbl[0][i];
			hDist = rrh - m_RearTbl[1][i];
			double dist = vDist*vDist + hDist*hDist;
			if (dist < bestDist) {
				bestDist = dist;
				bestIndex = i;
			}
		}
		int RRhipAngle = bestIndex % nHip + m_hipMin;
		int RRKneeAngle = bestIndex / nHip + m_RearKneeMin;

		// look up left rear.
		bestIndex = 0;
		vDist = lrv - m_RearTbl[0][0];
		hDist = lrh - m_RearTbl[1][0];
		bestDist = vDist*vDist + hDist*hDist;
		for (int i = 0; i < m_nRearTbl; i++) {
			vDist = lrv - m_RearTbl[0][i];
			hDist = lrh - m_RearTbl[1][i];
			double dist = vDist*vDist + hDist*hDist;
			if (dist < bestDist) {
				bestDist = dist;
				bestIndex = i;
			}
		}
		int LRhipAngle = bestIndex % nHip + m_hipMin;
		int LRKneeAngle = bestIndex / nHip + m_RearKneeMin;

		fprintf(fpOut, "%d,%d,%d,%d,%d,%d,%d,%d,\n", 
			LFhipAngle, RFhipAngle, RRhipAngle, LRhipAngle, LFKneeAngle, RFKneeAngle, RRKneeAngle, LRKneeAngle);

	}
	fclose(fp);
	fclose(fpOut);
}
