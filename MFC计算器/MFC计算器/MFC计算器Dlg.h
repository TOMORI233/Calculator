
// MFC������Dlg.h : ͷ�ļ�
//

#pragma once


// CMFC������Dlg �Ի���
class CMFC������Dlg : public CDialogEx
{
// ����
public:
	double calculate(CString str_input);
	CMFC������Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString str_input;
	CString str_output;
	afx_msg void OnBnClicked0();
	afx_msg void OnBnClickedDot();
	afx_msg void OnBnClicked1();
	afx_msg void OnBnClicked2();
	afx_msg void OnBnClicked3();
	afx_msg void OnBnClicked4();
	afx_msg void OnBnClicked5();
	afx_msg void OnBnClicked6();
	afx_msg void OnBnClicked7();
	afx_msg void OnBnClicked8();
	afx_msg void OnBnClicked9();
	afx_msg void OnBnClickedPlus();
	afx_msg void OnBnClickedMinus();
	afx_msg void OnBnClickedMultiply();
	afx_msg void OnBnClickedDivide();
	afx_msg void OnBnClickedEqual();
	afx_msg void OnBnClickedAllclear();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedBracketLeft();
	afx_msg void OnBnClickedBracketRight();
	afx_msg void OnBnClickedDec();
	afx_msg void OnBnClickedAns();
	afx_msg void OnBnClickedHex();
	afx_msg void OnBnClickedOct();
	afx_msg void OnBnClickedBin();
	afx_msg void OnBnClickedExp();
	afx_msg void OnBnClickedLn();
	afx_msg void OnBnClickedSin();
	afx_msg void OnBnClickedCos();
	afx_msg void OnBnClickedTan();
	afx_msg void OnBnClickedRad();
	afx_msg void OnBnClickedE();
	afx_msg void OnBnClickedPi();
	afx_msg void OnBnClickedDeg();
	afx_msg void OnBnClickedPow();
	afx_msg void OnBnClickedLogn();
};
