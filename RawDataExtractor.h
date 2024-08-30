#if !defined(AFX_RAWDATAEXTRACTOR_H__26A04CB0_94BD_4DDA_A8DC_D50B71C624CD__INCLUDED_)
#define AFX_RAWDATAEXTRACTOR_H__26A04CB0_94BD_4DDA_A8DC_D50B71C624CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RawDataExtractor.h : header file
//

#include "EEG_marking_toolDlg.h"

/////////////////////////////////////////////////////////////////////////////
// RawDataExtractor dialog

class RawDataExtractor : public CDialog
{
// Construction
public:
	RawDataExtractor(CWnd* pParent = NULL);   // standard constructor

	//sourya - added public variables
	bool filtered_data_extract;	//0 - raw data extract, 1 - filtered data extract, 
	bool selective_channel_data_extract;	//0 - all channel data extract, 1 - selective channel data extract
	bool partial_time_duration;		// 0 - full time duration, 1 - partial time duration	
	bool button_clicked;
	int no_of_chan;
	int selected_channel_index;
	CString chan_name[25];
	int total_recording_time_sec;
	int start_hr, start_min, start_sec, end_hr, end_min, end_sec;	// time interval entered by selective tine extraction
	bool report_fields_exist;
	bool include_original_labels;	// during raw data extraction, preserves the original labels


	// sourya - added patient report variables
	char patient_name[LONG_STRING_LEN];
	char patient_ID[LONG_STRING_LEN];
	char birth_date[SHORT_STRING_LEN];
	char sex[MEDIUM_STRING_LEN];
	char age[VERY_SHORT_STRING_LEN];
	char gest_age[VERY_SHORT_STRING_LEN];
	char doctor_name[LONG_STRING_LEN];
	char sampling_rate_buf[MEDIUM_STRING_LEN];


	// custom function declaration
	void InitDerivedParam(bool temp_bool, int no_of_channels, CString channel_name[], double total_record_time);
	void FillReportFields(char pID[], char pName[], char dName[], char srb[], char bdate[], char sx[], char gest_ag[], char ag[]);

// Dialog Data
	//{{AFX_DATA(RawDataExtractor)
	enum { IDD = IDD_DIALOG_RAWDATAEXTRACTOR };
	CButton	m_CheckIncludeOrigLabel;
	CComboBox	m_patSex;
	CComboBox	m_GestationAge;
	CComboBox	m_PatAgeUnit;
	CComboBox	m_PatAgeNum;
	CComboBox	m_DOB_yr;
	CComboBox	m_DOB_month;
	CComboBox	m_DOB_date;
	CEdit	m_SamplingRate;
	CEdit	m_UnderDoctorName;
	CEdit	m_PatName;
	CEdit	m_PatID;
	CButton	m_RadioTimeDurationFull;
	CButton	m_RadioAllChannelDataExtract;
	CButton	m_RadioDataExtractRaw;
	CEdit	m_EditEndSec;
	CEdit	m_EditEndMin;
	CEdit	m_EditEndHr;
	CEdit	m_EditStartSec;
	CEdit	m_EditStartMin;
	CEdit	m_EditStartHr;
	CComboBox	m_ComboSelectChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RawDataExtractor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation


protected:

	// Generated message map functions
	//{{AFX_MSG(RawDataExtractor)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonExtractData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RAWDATAEXTRACTOR_H__26A04CB0_94BD_4DDA_A8DC_D50B71C624CD__INCLUDED_)
