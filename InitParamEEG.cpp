// InitParamEEG.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "EEG_marking_toolDlg.h"
#include "RawDataExtractor.h"
#include "math.h"
#include "InitParamEEGProcessBasic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void CEEG_marking_toolDlg::Init_Param_EEG(void)
{
	// so far no data is exported
	data_exported_bin_format = 0;

	// this file stores the event mark information
	output_text_filename = "marked_burst_suppression_050212_sourya.emdb";	//.emdb stands for eeg marked database

	// decide the output text file which will store the summarized video motion information
	video_motion_text_filename = "video_motion_info_new.midb";	//.midb stands for motion info database

	// burst detection training file name  - initialize
	burst_detection_training_filename = exec_directory + "\\burst_detection_training_file.xls";		

	// artifact detection training file name  - initialize
	artifact_detection_training_filename =	exec_directory + "\\artifact_training_features_max_wss_alpha_05.xls";		

	// raw data segment extraction (burst, artifact or seizure segments etc)
	burst_segment_extraction_directory = exec_directory + "\\BURST\\";
	artifact_segment_extraction_directory = exec_directory + "\\ARTIFACT\\";
	seizure_segment_extraction_directory = exec_directory + "\\SEIZURE\\";
	sleep_spindle_segment_extraction_directory = exec_directory + "\\SLEEP_SPINDLE\\";
	normal_segment_extraction_directory = exec_directory + "\\NORMAL_EEG\\";
	burst_suppression_segment_extraction_directory = exec_directory + "\\BURST_SUPPRESSION\\";
  no_specific_mark_segment_extraction_directory = exec_directory + "\\NO_SPECIFIC_MARK\\";

	// call the dialog box which will initialize the values of sampling rate etc...
	InitParamEEGProcessBasic ipb;

	// call this function to open the dialog
	ipb.DoModal();	

	// this condition waits for closing of the extraction parameter setting dialog
	while(ipb.button_clicked == false);

	// set the parameters
	sampling_rate = ipb.sampling_rate_buf;
	
	// TIME_LENGTH is 80 if sampling rate is 125, 5 if 2000 etc.
	TIME_LENGTH = (10000.0 / sampling_rate);
	
	// filtering paremeters
	filter_low_freq_pass = ipb.Filter_LPF_buf;
	filter_high_freq_pass = ipb.Filter_HPF_buf;
	
} //end function


/*
	this function initializes the linked list 
	it is required for reading data both in text as well as in binary format
*/
void CEEG_marking_toolDlg::Init_Common_Param_EEG(void)
{
	sensitivity = 5;   
	timebase = 15;		
	mark_on = 1;
	video_display = false;
	mark_modified = 0;
	feature_extraction_marking_interval = 1;

	total_recording_duration_sec = 0;
	filtering_on = true;	//set the filtering procedure

	// list initializations
	list_nodecount = 0;
	head = NULL;
	tail = NULL;

	head_filtered_data = NULL;
 	tail_filtered_data = NULL;

	repaint_on = true;	//repaint operation is permitted

}