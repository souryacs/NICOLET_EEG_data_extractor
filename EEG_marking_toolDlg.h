// EEG_marking_toolDlg.h : header file
//

#if !defined(AFX_EEG_MARKING_TOOLDLG_H__ACC2507A_978C_4222_AE3E_6A3F7D26718C__INCLUDED_)
#define AFX_EEG_MARKING_TOOLDLG_H__ACC2507A_978C_4222_AE3E_6A3F7D26718C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// include windows function
#include <IO.h>
#include <string>
#include <vector>
#include <windows.h>
#include <sys/stat.h>
#include <sys/types.h>	
#include <time.h>

//opencv function use
#include "cv.h"
#include "highgui.h"
#include "ml.h"

/*
this is for enabling the video summarization part
otherwise there will be no motion detection and video summarization
*/
//#define VIDEO_SUMMARY

/*
following define statement enables detection of burst, artifact and suppression segments
from the time series data and display those annotations in the signal display interval 
*/
//#define PATTERN_DETECTION

//sourya - added define statements
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define MAX_LINE_LEN 2048	
//#define NFFT 8192	//32768
#define CHAN_DATA_BLOCK_SIZE 16384	//1024

// SWAP define for FFT
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

#define SCREEN_X_OFFSET 70
#define SCREEN_Y_OFFSET 20

//#define DEBUG_INFO_PRINT
#define LONG_DATASET_NO_SPECIFIC_MARK_ID 10

#define SEIZURE_MARK 0
#define BURST_MARK 1
#define ARTIFACT_MARK 2
#define SLEEP_SPINDLE_MARK 3
#define BURST_SUPPRESSION_MARK 4
#define NORMAL_MARK 5
#define UNKNOWN_MARK 6
#define UNMARK_EXISTING_MARK 7
#define SUPPRESSION_MARK 9

//sourya - define the maximum no of channels allowed
#define MAX_NO_OF_CHANNELS 25
#define LEVEL_OF_DECOMPOSITION 6

// DEFINE for binary file write
#define VR_LEN 2
#define VERY_SHORT_STRING_LEN 4
#define SHORT_STRING_LEN 8
#define MEDIUM_STRING_LEN 16
#define LONG_STRING_LEN 64
#define VERY_LONG_STRING_LEN 128

// these variables are used for the signal processing 
// for burst, artifact detection
    
#define TIME_WINDOW_LENGTH 1	// sliding window length	
#define SLIDING_WINDOW_OFFSET 0.1  // offset of sliding window    
#define MIN_SUPPRESSION_LEN 2	// 2 sec is the minimum suppression duration at a stretch

// for matching with raw EEG display by standard NICOLET EEG viewer, following multiplying factors 
// are used in conjunction with the filtered EEG data and the raw EEG display
# define FILTER_DATA_MULTIPLYING_FACTOR 0.25
# define RAW_EEG_DISPLAY_MULTIPLYING_FACTOR 2.5

/*
	queue size storing average or background EEG feature values
	for sliding window offset of 0.5 second, queue size of 10 is employed
	for sliding window offset of 0.1 second, queue size of 25 is employed    
*/
#define QUEUE_SIZE 25

/*
	for classifier design, we have to decide the size of training data
	for both burst and artifact detection
*/
#define no_of_instance_burst_detection 1541
#define no_of_feature_burst_detection 7

#define no_of_instance_artifact_detection 1775
#define no_of_feature_artifact_detection 9

/*
	for management of the marking database, storing the marking information
	these timing constraints specify the time within which, if same markings are located,
	will be merged during the time of saving the database
*/
#define SUPPRESSION_MARK_MERGE_MAX_TIME_GAP 0.5
#define SAME_MARK_MERGE_MAX_TIME_GAP 0.5

// thresholds used in paper implementations (Zhang et al.)
// for motion detection routine
#define TH_K 1.6
#define TH_ALPHA 0.1	//0.005

// structure defining the motion information for each of the video file (if exists) along with recorded EEG data
struct Motion_Information
{
	double motion_start_time;
	double motion_end_time;
	int motion_start_frame_count;
	int motion_end_frame_count;
	struct Motion_Information *next;	// next node pointer
	struct Motion_Information *prev;	// prev node pointer
};

// structure containing the information for each video file (if exists) along with recorded EEG data
struct Video_Information
{
	double fps;
	double duration;
	double no_of_frames;
	double frame_height;
	double frame_width;
	char video_filename[12];
	int motion_count;
	COleDateTime video_file_create_time;
	double curr_video_file_start_time;	// this is the start time of the current video file with respect to the raw EEG time elapsed
	double curr_video_file_end_time;		// // this is the end time of the current video file with respect to the raw EEG time elapsed
	struct Motion_Information *head_motion_info;
	struct Motion_Information *tail_motion_info;
	struct Video_Information *next;	// next node pointer
	struct Video_Information *prev;	// prev node pointer
};


// define structure containing the channel data
struct channnel_data_list		// node of a double linked list for storage of channel data
{
	double **channel_data;	// contains channel data
	struct channnel_data_list *next;	// next node pointer
	struct channnel_data_list *prev;	// prev node pointer
};


// structure for binary file write or read - globally declared 
// according to DICOM standard

// this structure is modified according to the modification for incorporating multi channel
// data write and read operation
struct bin_file_format_dicom
{
	char preamble_text[VERY_LONG_STRING_LEN];
	char form[VERY_SHORT_STRING_LEN];
	char modality[MEDIUM_STRING_LEN];
	char patient_name[LONG_STRING_LEN];
	char patient_ID[LONG_STRING_LEN];
	char birth_date[SHORT_STRING_LEN];
	char sex[MEDIUM_STRING_LEN];
	char start_time[MEDIUM_STRING_LEN];
	char record_dur[MEDIUM_STRING_LEN];
	char age[VERY_SHORT_STRING_LEN];
	char gest_age[VERY_SHORT_STRING_LEN];
	char doctor_name[LONG_STRING_LEN];
	short sel_chan_no;
	char** chan_name;	// chan_name[MEDIUM_STRING_LEN];
	long channel_data_size;
	char sampling_rate_buf[MEDIUM_STRING_LEN];
	short bits_per_sample;
	char filter_low_freq_buf[MEDIUM_STRING_LEN];
	char filter_high_freq_buf[MEDIUM_STRING_LEN];
	char** specific_segment_marking; //specific_segment_marking[VERY_SHORT_STRING_LEN];
	float* event_start_time;	// there was no pointer before
	float* event_end_time;		// there was no pointer before
};

// following structure is the definition of the marking that is used to indicate
// various events
struct data_mark_format
{
	double mark_start_time;
	double mark_end_time;
	int mark_event;
	struct data_mark_format *next;
};

/////////////////////////////////////////////////////////////////////////////
// CEEG_marking_toolDlg dialog

class CEEG_marking_toolDlg : public CDialog
{
// Construction
public:
	CEEG_marking_toolDlg(CWnd* pParent = NULL);	// standard constructor

	//*************************
	// sourya - all global variables 

	// channel data processing global variables
	//double **channel_data;		//stores all channel data
	struct channnel_data_list *head;	// head of the linked list	
	struct channnel_data_list *tail;	// tail of the linked list
	struct channnel_data_list *head_filtered_data;	// head of the linked list containing filtered data	
	struct channnel_data_list *tail_filtered_data;	// tail of the linked list containing filtered data	
	int list_nodecount;
	double total_recording_duration_sec;	//total recording duration
	long channel_data_size;
	int no_of_channels;		//no of channels in the raw EEG data
	CString channel_name[MAX_NO_OF_CHANNELS];	//channel name
	CString chan1_1st_channel[MAX_NO_OF_CHANNELS];	//channel name - 1st channel
	CString chan1_2nd_channel[MAX_NO_OF_CHANNELS];	//channel name - 2nd channel
	int chan1_index[MAX_NO_OF_CHANNELS];	//channel index - 1st channel
	int chan2_index[MAX_NO_OF_CHANNELS];	//channel index - 2nd channel
	double sampling_rate;		//sampling rate
	double TIME_LENGTH;			// duration of segment
	double time_interval;		//reciprocal of sampling rate

	// EEG display based variables
	int screen_x_size;			// x resolution
	int screen_y_size;			// y resolution
	double PixelsPerMM;			// pixel per mm
	double sel_raw_eeg_start_time;	// raw EEG display window start time
	double sel_raw_eeg_end_time;	// raw EEG display window end time	
	int raw_eeg_per_page_duration;	// raw EEG duration
	int sensitivity;   // initial setting according to checked value
	int timebase;		// initial setting according to checked value
	bool video_display;	// video display
	bool mark_modified;	// marking is modified	
	int feature_extraction_marking_interval;	//feature extract sliding window length

	// filtering 
	int NFFT;

	// this is the marking database pointer
	// it points to event database of multiple channels
	// marking_database[ch_no] is the data pointer of a single channel
	// double ***marking_database;	//marking database
	struct data_mark_format **marking_database;
	// pointer to the count information
	int *mark_database_count;	
	int mark_on;		// current mark event

	// color code for different marking
	int color_code[7][3];
	CStatic lblPresent[MAX_NO_OF_CHANNELS];		//label text

	// EEG processing parameters
	CString exec_directory;		// executable directory of the code itself
	CString directory_name;	// directory of file
	CString fileName;		// filename
	CString output_text_filename;	//output excel file 
	CString video_motion_text_filename;		// text file storing the video motion information

	CString burst_segment_extraction_directory;
	CString artifact_segment_extraction_directory;
	CString seizure_segment_extraction_directory;
	CString sleep_spindle_segment_extraction_directory;
	CString normal_segment_extraction_directory;
	CString burst_suppression_segment_extraction_directory;
  CString no_specific_mark_segment_extraction_directory;

	double filter_low_freq_pass;	// filter high pass freq
	double filter_high_freq_pass;	// filter low pass freq
	bool filtering_on;

	// video processing global variables
	int no_of_video_files;	// no of video recorded files
	COleDateTime recording_start_time;	// time and date when recording was started
	CString video_files_dir_name;	// directory where the video information is stored
	struct Video_Information *head_video_info;	// head pointer to the video information
	struct Video_Information *tail_video_info;	// tail pointer to the video information
	int init_x, init_y, final_x, final_y;	// bounding rectangle on the video data - to mark the patient

	// we declare arrays and counter for storing the 
	int record_pause_event_count;
	double record_pause_duration[100];
	double record_pause_start_time[100];

	// this boolean variable is set as ON 
	// it signifies repaint function ON
	// when we press the exit button then repaint operation will be stopped
	bool repaint_on;

	// this variable is set when the extracted channel text data has inbuild montage information
	// i.e. bipolar montage information is embedded within text file
	// there is no need to check for external montage information
	bool montage_info_inbuilt;

	//////////////////////////////////
	// classifier design (SVM) for burst and artifact separation
	
	// burst detection 
	CvSVM *svm_classifier_burst_detection;	// svm classifier for burst detection
	CString burst_detection_training_filename;
	CvMat *mat_test_feat_burst;				// matrix for testing data collect (burst) and classify
	
	// artifact detection
	CvSVM *svm_classifier_artifact_detection;	// svm classifier for artifact detection
	CString artifact_detection_training_filename;
	CvMat *mat_test_feat_artifact;				// matrix for testing data collect (artifact) and classify

	/*
	double training_feature[NO_OF_INSTANCE][NO_OF_FEATURE];
	int training_group[NO_OF_INSTANCE];
	double prior[NO_OF_GROUPS];
	double D[NO_OF_GROUPS];
	double gmeans[NO_OF_GROUPS][NO_OF_FEATURE];
	double logDetSigma[NO_OF_GROUPS];
	double standard_dev_classifier[NO_OF_GROUPS][NO_OF_FEATURE];
	double test_sample[NO_OF_FEATURE];
	double test_diff_temp[NO_OF_FEATURE];
	*/

	// slider control used to navigate between the EEG segments
	CSliderCtrl slider_eeg;		//removed the pointer

	//////////////////////////////////

	// if  the extracted binary format data had start time different from zero (that is, it was clipped
	// within the original text file segment) then we note down that offset
	double binary_fmt_data_read_start_time_offset;

	// variables related to the export of marked data 
	// signifies whether binary format data export is done at all or not
	int data_exported_bin_format;	
	// following variables are used for DICOM standard compatibility
	int tag;
	char VR[VR_LEN];
	int VL;
	struct bin_file_format_dicom bftd;

	//*************************
	// sourya - all custom function prototype
	void Read_Bin_Fmt_aEEG_Data(void);
	void Read_Text_Amp_EEG_machine_Config(void);
	void Init_Common_Param_EEG(void);
	void Init_Param_EEG(void);
	void Read_and_Process_EEG_Data(bool binary_data_read);
	void Classifier_Training(CString training_filename, int no_of_feature, int no_of_instance, CvSVM *svm_classifier);
	void Read_Amp_EEG_machine_input_data(void);
	void Filter_Amp_EEG_Data(void);
	double modZeroBessel(double x);
	void plot_raw_EEG_signal(int repaint_on);
	void fftfilt_C_impl(double* filter_fft, double* inp_sig, double* out_sig, int in_sig_len, int out_sig_len, int filt_sig_len, double* x_fft, double* y_mult, int L);
	void FFT_C(double *data, int number_of_complex_samples, int isign);
	double median(double *data, int n);
	double mean(double* x, int n);
	
	void wtr_fwd_db4(double *a, int n, int *len_coeff);
	void wtr_fwd_coif3(double *a, int n, int *len_coeff);
	double calc_rigsure_thrs(double *inp_sig, int inp_sig_len);

	void ROI_Detection(int ch_no);
	void artifact_detection(int ch_no);	
	void Suppression_Detection(int ch_no);
	void burst_followed_by_suppression_detection();
	
	void Init_Display_Marking_parameters(bool binary_data_read);

	struct data_mark_format* AddMarkData(int ch_no, double event_start_time, double event_end_time, int mark_info);
	void DelOverlappingMark();
	void MergeSimilarMark();
	void DiscardSmallSuppMark();
	void ResetSpecifiedMark();

	int ComputeNFFT(int inp_sig_len, int filt_sig_len);

	double ComputeHiguchiFD(double* input_signal, int inp_sig_len);
	double ComputeKurt(double* input_signal, int inp_sig_len);
	void Read_Signal_Segment(int ch_no, int start_index, int end_index, double* input_signal, int inp_sig_len);
	double ComputeMNLE(double* input_signal, int inp_sig_len);
	double ComputeMeanAbsVolt(double* input_signal, int inp_sig_len);
	
	void DeriveFFT(double *input_signal, int nfft);
	double ComputePSD(double* input_signal, int nfft, double d);
	double Compute_X_Hz_power(double* input_signal, int nfft, double d, double x);
	double ComputeSEF95(double* input_signal, double sampling_rate, int nfft, double d, double total_psd);

	double ComputeVar(double* input_signal, int inp_sig_len);
	CString convert_time_to_str(int curr_time);

	void Video_Summarize();
# if 0
	void video_motion_detect(struct Video_Information *tail_video_info, CString video_filename);
# else
  void HybridMotionDetect(struct Video_Information *tail_video_info, IplImage* first_frame, 
                          IplImage* middle_frame, IplImage* last_frame, 
                          IplImage* backgrnd_frame, int frame_count, 
                          IplImage* det_for_img);
# endif

	void Write_Motion_Info(struct Video_Information *tail_video_info, int no_of_video_files);
	void Read_Video_Motion_Information();
	void plot_video_data();
	void SetRecordingStartTimeAndDate();
	void ReadEventExportData();

	void ExportEvents(void);
	void ExportRawData(void);
	void ExportSpecificEvent(void);
  CString CEEG_marking_toolDlg::DetermineFileName(int ch_no, CString directory_name, int mark_event, bool filtering_on, 
                          bool filtered_data_extract, bool selective_channel_data_extract,
                          CString record_start_time, CString record_end_time);
	void TimebaseChangeFunc(int timebase); 

	CPen xpen[7];
	CPen pen;
	HWND hWnd;
	HDC	hDC;
	CRect	rect;
	HBRUSH	drawBrush;
	//*************************

// Dialog Data
	//{{AFX_DATA(CEEG_marking_toolDlg)
	enum { IDD = IDD_EEG_MARKING_TOOL_DIALOG };
	CStatic	m_chn4;
	CStatic	m_chn3;
	CStatic	m_chn2;
	CStatic	m_chn1;
	CButton	m_Grid;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEEG_marking_toolDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEEG_marking_toolDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFileOpen();
	afx_msg void OnViewNext(int);
	afx_msg void OnViewPrev(int);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFileSave();
	afx_msg void OnTimebase6();
	afx_msg void OnSensitivity1();
	afx_msg void OnSensitivity2();
	afx_msg void OnSensitivity3();
	afx_msg void OnSensitivity5();
	afx_msg void OnSensitivity7();
	afx_msg void OnSensitivity10();
	afx_msg void OnSensitivity15();
	afx_msg void OnSensitivity20();
	afx_msg void OnSensitivity30();
	afx_msg void OnSensitivity50();
	afx_msg void OnSensitivity70();
	afx_msg void OnSensitivity100();
	afx_msg void OnSensitivity200();
	afx_msg void OnSensitivity500();
	afx_msg void OnTimebase8();
	afx_msg void OnTimebase10();
	afx_msg void OnTimebase15();
	afx_msg void OnTimebase20();
	afx_msg void OnTimebase30();
	afx_msg void OnTimebase60();
	afx_msg void OnTimebase120();
	afx_msg void OnTimebase240();
	afx_msg void OnMarkingSeizure();
	afx_msg void OnMarkingBurst();
	afx_msg void OnMarkingArtifact();
	afx_msg void OnMarkingSleepspindle();
	afx_msg void OnMarkingBurstSuppression();
	afx_msg void OnMarkingUnknownmark();
	afx_msg void OnMarkingUnmark();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClose();
	afx_msg void OnFileExit();
	afx_msg void OnExportRaw();
	afx_msg void OnExportEvents();
	afx_msg void OnExportSpecificEvent();
	afx_msg void OnFileShowvideo();
	afx_msg void OnViewNextEvent();
	afx_msg void OnViewPrevEvent();
	afx_msg void OnMarkingNormalEEG();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EEG_MARKING_TOOLDLG_H__ACC2507A_978C_4222_AE3E_6A3F7D26718C__INCLUDED_)
