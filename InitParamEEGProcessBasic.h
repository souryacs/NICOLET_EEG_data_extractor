#if !defined(AFX_INITPARAMEEGPROCESSBASIC_H__6959870B_EA36_45C3_B443_5E3D4C6B8BF4__INCLUDED_)
#define AFX_INITPARAMEEGPROCESSBASIC_H__6959870B_EA36_45C3_B443_5E3D4C6B8BF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InitParamEEGProcessBasic.h : header file
//

#include "EEG_marking_toolDlg.h"

/////////////////////////////////////////////////////////////////////////////
// InitParamEEGProcessBasic dialog

class InitParamEEGProcessBasic : public CDialog
{
// Construction
public:
	InitParamEEGProcessBasic(CWnd* pParent = NULL);   // standard constructor

	// sourya - added public variables
	double sampling_rate_buf;
	double Filter_LPF_buf;
	double Filter_HPF_buf;
	bool button_clicked;

// Dialog Data
	//{{AFX_DATA(InitParamEEGProcessBasic)
	enum { IDD = IDD_DIALOG_INITPARAMS };
	CEdit	m_SamplingRate;
	CComboBox	m_Filter_LPF;
	CComboBox	m_Filter_HPF;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(InitParamEEGProcessBasic)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(InitParamEEGProcessBasic)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonClickInitParam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INITPARAMEEGPROCESSBASIC_H__6959870B_EA36_45C3_B443_5E3D4C6B8BF4__INCLUDED_)
