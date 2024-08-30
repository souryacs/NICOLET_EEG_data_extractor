// RawDataExtractor.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "RawDataExtractor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RawDataExtractor dialog


RawDataExtractor::RawDataExtractor(CWnd* pParent /*=NULL*/)
	: CDialog(RawDataExtractor::IDD, pParent)
{
	//{{AFX_DATA_INIT(RawDataExtractor)
	//}}AFX_DATA_INIT
}


void RawDataExtractor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RawDataExtractor)
	DDX_Control(pDX, IDC_CHECK1, m_CheckIncludeOrigLabel);
	DDX_Control(pDX, IDC_COMBO_SEX, m_patSex);
	DDX_Control(pDX, IDC_COMBO_GESTATION_AGE, m_GestationAge);
	DDX_Control(pDX, IDC_COMBO_AGE_UNIT, m_PatAgeUnit);
	DDX_Control(pDX, IDC_COMBO_AGE_NUM, m_PatAgeNum);
	DDX_Control(pDX, IDC_COMBO_BIRTH_YEAR, m_DOB_yr);
	DDX_Control(pDX, IDC_COMBO_BIRTH_MONTH, m_DOB_month);
	DDX_Control(pDX, IDC_COMBO_BIRTH_DAY, m_DOB_date);
	DDX_Control(pDX, IDC_EDIT_SAMPLING_RATE, m_SamplingRate);
	DDX_Control(pDX, IDC_EDIT_DOCTOR_NAME, m_UnderDoctorName);
	DDX_Control(pDX, IDC_EDIT_PATIENT_NAME, m_PatName);
	DDX_Control(pDX, IDC_EDIT_PATIENT_ID, m_PatID);
	DDX_Control(pDX, IDC_RADIO_TIME_DURATION_FULL, m_RadioTimeDurationFull);
	DDX_Control(pDX, IDC_RADIO_ALL_CHANNEL_DATA_EXTRACT, m_RadioAllChannelDataExtract);
	DDX_Control(pDX, IDC_RADIO_DATA_EXTRACT_RAW, m_RadioDataExtractRaw);
	DDX_Control(pDX, IDC_EDIT_END_SEC, m_EditEndSec);
	DDX_Control(pDX, IDC_EDIT_END_MIN, m_EditEndMin);
	DDX_Control(pDX, IDC_EDIT_END_HR, m_EditEndHr);
	DDX_Control(pDX, IDC_EDIT_START_SEC, m_EditStartSec);
	DDX_Control(pDX, IDC_EDIT_START_MIN, m_EditStartMin);
	DDX_Control(pDX, IDC_EDIT_START_HR, m_EditStartHr);
	DDX_Control(pDX, IDC_COMBO_SELECT_CHANNEL, m_ComboSelectChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RawDataExtractor, CDialog)
	//{{AFX_MSG_MAP(RawDataExtractor)
	ON_BN_CLICKED(IDC_BUTTON_EXTRACT_DATA, OnButtonExtractData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RawDataExtractor message handlers

BOOL RawDataExtractor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	int i;
	CString buf, temp;

	// initialize the radio buttons
	m_RadioDataExtractRaw.SetCheck(1);
	m_RadioAllChannelDataExtract.SetCheck(1);
	m_RadioTimeDurationFull.SetCheck(1);

	//initialize corresponding boolean variables
	filtered_data_extract = false;
	selective_channel_data_extract = false;	
	partial_time_duration = false;		

	// add the combo box initialization
	// combo box recording duration
	for (i = 0; i < no_of_chan; i++)
	{
		m_ComboSelectChannel.AddString(chan_name[i]);	//(channel_name[i]);
	}
	m_ComboSelectChannel.SetCurSel(0);	//show the 1st item

	// limit the no of digits 
	m_EditStartHr.SetLimitText(2);
	m_EditStartMin.SetLimitText(2);
	m_EditStartSec.SetLimitText(2);
	m_EditEndHr.SetLimitText(2);
	m_EditEndMin.SetLimitText(2);
	m_EditEndSec.SetLimitText(2);

	// set default recording extraction time
	// default start time - 00:00:00
	// default end time is the total recording duration
	m_EditStartHr.SetWindowText("00");
	m_EditStartMin.SetWindowText("00");	
	m_EditStartSec.SetWindowText("00");

	int end_hr = total_recording_time_sec / 3600;
	int end_min = (total_recording_time_sec - end_hr * 3600) / 60;
	int end_sec = (total_recording_time_sec - end_hr * 3600 - end_min * 60);
	temp.Format(_T("%d"), end_hr);
	m_EditEndHr.SetWindowText(temp);
	temp.Format(_T("%d"), end_min);
	m_EditEndMin.SetWindowText(temp);
	temp.Format(_T("%d"), end_sec);
	m_EditEndSec.SetWindowText(temp);

	// finally the extract button clicked boolean info - it must be set to false
	button_clicked = false;

	// initialize the report generating variables
	if (report_fields_exist == true)
	{
		m_PatID.SetWindowText(patient_ID);
		m_PatName.SetWindowText(patient_name);
		m_UnderDoctorName.SetWindowText(doctor_name);
		m_SamplingRate.SetWindowText(sampling_rate_buf);
	}

	// combo box gestational age (weeks)
	for (i = 20; i <= 45; i++)
	{
		buf.Format(_T("%d"), i);
		m_GestationAge.AddString(buf);
	}
	if (report_fields_exist == false)
		m_GestationAge.SetCurSel(0);	//show the 1st item
	else
	{
		char gest_age_val[3];
		strcpy(gest_age_val, "");
		gest_age_val[0] = gest_age[0];	gest_age_val[1] = gest_age[1];	gest_age_val[2] = '\0';	
		// select gestational age value index
		m_GestationAge.SetCurSel(m_GestationAge.FindStringExact(0, gest_age_val));
	}

	//combo box patient age (num)
	for (i = 1; i <= 99; i++)
	{
		buf.Format(_T("%d"), i);
		if (strlen(buf) == 1)
		{
			temp = "0" + buf;
			m_PatAgeNum.AddString(temp);
		}
		else
			m_PatAgeNum.AddString(buf);
	}

	//combo box patient age (unit)
	m_PatAgeUnit.AddString("Days");
	m_PatAgeUnit.AddString("Weeks");
	m_PatAgeUnit.AddString("Months");
	m_PatAgeUnit.AddString("Years");

	if (report_fields_exist == false)
	{
		m_PatAgeNum.SetCurSel(0);	//show the 1st item
		m_PatAgeUnit.SetCurSel(0);	//show the 1st item
	}
	else
	{
		char age_val[3];
		char age_unit;
		strcpy(age_val, "");
		age_val[0] = age[0];	age_val[1] = age[1];	age_val[2] = '\0';	
		age_unit = age[2];
		
		// select age value index
		m_PatAgeNum.SetCurSel(m_PatAgeNum.FindStringExact(0, age_val));
	
		// select age unit index
		int age_un;
		if (age_unit == 'D')
			age_un = 0;
		else if (age_unit == 'W')
			age_un = 1;
		else if (age_unit == 'M')
			age_un = 2;
		else //if (age_unit == 'Y')
			age_un = 3;

		m_PatAgeUnit.SetCurSel(age_un);
	}

	// combo box patient sex 
	temp = "Male";
	m_patSex.AddString(temp);
	temp = "Female";
	m_patSex.AddString(temp);
	if (report_fields_exist == false)
		m_patSex.SetCurSel(0);	//show the 1st item
	else
		m_patSex.SetCurSel(m_patSex.FindStringExact(0, sex));

	//combo boxes for mentioning the date of birth
	
	// date
	for (i = 1; i <= 31; i++)
	{
		buf.Format(_T("%d"), i);
		if (strlen(buf) == 1)
		{
			temp = "0" + buf;
			m_DOB_date.AddString(temp);
		}
		else
			m_DOB_date.AddString(buf);
	}
	if (report_fields_exist == false)
		m_DOB_date.SetCurSel(0);	//show the 1st item
	else
	{
		char date[3];
		strcpy(date, "");
		date[0] = birth_date[6];	date[1] = birth_date[7];	date[2] = '\0';
		int x = atoi(date);
		m_DOB_date.SetCurSel(x - 1);
	}
	// month
	m_DOB_month.AddString("January");
	m_DOB_month.AddString("February");
	m_DOB_month.AddString("March");
	m_DOB_month.AddString("April");
	m_DOB_month.AddString("May");
	m_DOB_month.AddString("June");
	m_DOB_month.AddString("July");
	m_DOB_month.AddString("August");
	m_DOB_month.AddString("September");
	m_DOB_month.AddString("October");
	m_DOB_month.AddString("November");
	m_DOB_month.AddString("December");
	if (report_fields_exist == false)
		m_DOB_month.SetCurSel(0);	//show the 1st item
	else
	{
		char mon[3];
		strcpy(mon, "");
		mon[0] = birth_date[4];	mon[1] = birth_date[5];	mon[2] = '\0';
		int x = atoi(mon);
		m_DOB_month.SetCurSel(x - 1);
	}
	
	//year
	for (i = 1900; i <= 2100; i++)
	{
		buf.Format(_T("%d"), i);
		m_DOB_yr.AddString(buf);
	}	
	if (report_fields_exist == false)
		m_DOB_yr.SetCurSel(111);	//show the 1st item
	else
	{
		char yr[5];
		strcpy(yr, "");
		yr[0] = birth_date[0];	yr[1] = birth_date[1];	yr[2] = birth_date[2];	yr[3] =	birth_date[3];	yr[4] = '\0';
		int x = atoi(yr);
		m_DOB_yr.SetCurSel(x - 1900);
	}

	// initialize the check box containing the parameter
	// use the original label
	m_CheckIncludeOrigLabel.SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void RawDataExtractor::OnButtonExtractData() 
{
	// TODO: Add your control notification handler code here
	CString temp;
	char temp_buf[4];
	int x;

	// check the report generation utilities
	// patient ID
	m_PatID.GetWindowText(temp);	
	strcpy(patient_ID, "");
	strcat(patient_ID, temp);

	// patient name
	m_PatName.GetWindowText(temp);	
	strcpy(patient_name, "");
	strcat(patient_name, temp);
	
	// under doctor name
	m_UnderDoctorName.GetWindowText(temp);
	strcpy(doctor_name, "");
	strcat(doctor_name, temp);

	// sampling rate
	m_SamplingRate.GetWindowText(temp);
	strcpy(sampling_rate_buf, "");
	strcat(sampling_rate_buf, temp);
	
	// birth date
	m_DOB_yr.GetWindowText(temp);
	strcpy(birth_date, "");
	strcat(birth_date, temp);
	x = (m_DOB_month.GetCurSel() + 1);
	itoa(x, temp_buf, 10);
	if (strlen(temp_buf) == 1)
	{
		strcat(birth_date, "0");
	}
	strcat(birth_date, temp_buf);
	m_DOB_date.GetWindowText(temp);
	strcat(birth_date, temp);
	
	//sex
	m_patSex.GetWindowText(temp);
	strcpy(sex, "");
	strcat(sex, temp);

	// gestational age
	m_GestationAge.GetWindowText(temp);
	strcpy(gest_age, "");
	strcat(gest_age, temp);
	strcat(gest_age, "W");

	// age
	m_PatAgeNum.GetWindowText(temp);	
	strcpy(age, "");
	strcat(age, temp);
	x = m_PatAgeUnit.GetCurSel();
	if (x == 0)
		strcat(age, "D");
	else if (x == 1)
		strcat(age, "W");
	else if (x == 2)
		strcat(age, "M");
	else
		strcat(age, "Y");

	// check the radio button states

	// raw data / filtered data extract
	if (m_RadioDataExtractRaw.GetCheck() == 1)	//selected
		filtered_data_extract = false;
	else
		filtered_data_extract = true;

	// all channels / selective channel data extract
	if (m_RadioAllChannelDataExtract.GetCheck() == 1)
	{
		selective_channel_data_extract = false;
		selected_channel_index = 0;
	}
	else
	{
		selective_channel_data_extract = true;
		selected_channel_index = m_ComboSelectChannel.GetCurSel();
	}

	// checkbox including original labels
	if (m_CheckIncludeOrigLabel.GetCheck() == 1)
		include_original_labels = true; 
	else
		include_original_labels = false; 

	// all channels / selective channel data extract
	if (m_RadioTimeDurationFull.GetCheck() == 1)
		partial_time_duration = false;
	else
		partial_time_duration = true;

	
	// check for various input errors while specifying the time intervals
	bool error_condition = false;

	if (partial_time_duration == true)
	{
		m_EditStartHr.GetWindowText(temp);
		start_hr = atoi(temp);
		m_EditStartMin.GetWindowText(temp);
		start_min = atoi(temp);
		m_EditStartSec.GetWindowText(temp);
		start_sec = atoi(temp);
		m_EditEndHr.GetWindowText(temp);
		end_hr = atoi(temp);
		m_EditEndMin.GetWindowText(temp);
		end_min = atoi(temp);
		m_EditEndSec.GetWindowText(temp);
		end_sec = atoi(temp);

		if ((start_hr * 3600 + start_min * 60 + start_sec) > total_recording_time_sec)
			error_condition = true;
		else if ((end_hr * 3600 + end_min * 60 + end_sec) > total_recording_time_sec)
			error_condition = true;
	}
	else	// full time duration 
	{	
		start_hr = 0;	start_min = 0;	start_sec = 0;
		end_hr = total_recording_time_sec / 3600;
		end_min = (total_recording_time_sec - end_hr * 3600) / 60;
		end_sec = (total_recording_time_sec - end_hr * 3600 - end_min * 60);
	}	

	if (error_condition == true)
		AfxMessageBox("Raw Data extraction - time set error - check the time intervals !");
	else
	{
		button_clicked = true;
		CDialog::OnCancel();
	}
}


void RawDataExtractor::InitDerivedParam(bool temp_bool, int no_of_channels, CString channel_name[], double total_record_time) 
{
	int i;
	
	// copy the channel information
	no_of_chan = no_of_channels;
	for (i = 0; i < no_of_chan; i++)
		chan_name[i] = channel_name[i];

	total_recording_time_sec = (int)total_record_time;

	// initialize static boolean
	report_fields_exist = temp_bool;

	return;
}


void RawDataExtractor::FillReportFields(char pID[], char pName[], char dName[], char srb[], char bdate[], char sx[], char gest_ag[], char ag[])
{
	report_fields_exist = true;
	strcpy(patient_ID, pID);
	strcpy(patient_name, pName);
	strcpy(doctor_name, dName);
	strcpy(sampling_rate_buf, srb);
	strcpy(birth_date, bdate);
	strcpy(sex, sx);
	strcpy(gest_age, gest_ag);
	strcpy(age, ag);	
}
