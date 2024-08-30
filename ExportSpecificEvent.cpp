// ExportSpecificEvent.cpp : implementation file
//

#include "stdafx.h"
#include "EEG_marking_tool.h"
#include "EEG_marking_toolDlg.h"
#include "RawDataExtractor.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CEEG_marking_toolDlg::ExportSpecificEvent(void)
{
	// here we check for distinct burst, artifact, burst-suppression and seizure event cases
	// if found then we create corresponding mark containing small file

	// effectively the file contains the file portion containing a single event
	// it is useful to construct a dataset

	int ch_no;
	int mark_count, i;
	CString extract_raw_data_filename;
	CString temp;
	CFileStatus status;
	FILE *myfile;
	CString record_start_time;
	CString record_end_time;
	int rec_start_time, rec_end_time;
	char buf[10];
  int selected_chan_index;

	struct data_mark_format* temp_data_mark;	// pointer to new allocated node
	struct data_mark_format* temp_data_mark2;	// pointer to new allocated node
	bool overlapping_mark_found;

	int BEFORE_MARK_DURATION_NON_OVERLAP = 3;
	int AFTER_MARK_DURATION_NON_OVERLAP = 2;

	/****************************************/
	// clear the cases for which one mark contains another mark completely within it
	DelOverlappingMark();

	// once we remove all the markings which have inbuild completely overlapping marks (ambiguity)
	// now we merge similar markings if they are maintained within very small distance	
	MergeSimilarMark();

	/****************************************/

	// initialization of global variables
	// if already no binary file is selected or no export operation is done
	if ((fileName.Find(".bindata") == -1) && (data_exported_bin_format == 0))
	{
		strcpy(VR, "");
		strcpy(bftd.modality, "");
		strcpy(bftd.start_time, "");
		strcpy(bftd.record_dur, "");
		//strcpy(bftd.chan_name, "");
		strcpy(bftd.filter_low_freq_buf, "");
		strcpy(bftd.filter_high_freq_buf, "");
	}

	// declare the other dialog class instance
	RawDataExtractor rde;

	// init some parameters
	rde.InitDerivedParam(false, no_of_channels, channel_name, total_recording_duration_sec);

	// if current opened input file is a binary file then pass the binary file parameters to the 
	// raw data extractor structure
	if ((fileName.Find(".bindata") != -1) || (data_exported_bin_format == 1))
	{
		rde.FillReportFields(bftd.patient_ID, bftd.patient_name, bftd.doctor_name, 
			bftd.sampling_rate_buf, bftd.birth_date, bftd.sex, bftd.gest_age, bftd.age);
	}
	else
	{
		strcpy(bftd.patient_ID, "");
		strcpy(bftd.patient_name, "");
		strcpy(bftd.doctor_name, "");
		temp.Format("%f", sampling_rate);
		strcpy(bftd.sampling_rate_buf, temp);
		strcpy(bftd.birth_date, "");
		strcpy(bftd.sex, "");
		strcpy(bftd.gest_age, "");
		strcpy(bftd.age, "");
		rde.FillReportFields(bftd.patient_ID, bftd.patient_name, bftd.doctor_name, 
			bftd.sampling_rate_buf, bftd.birth_date, bftd.sex, bftd.gest_age, bftd.age);
	}

	// call this function to view the dialog
	rde.DoModal();	

	// this condition waits or closing of the extraction parameter setting dialog
	while(rde.button_clicked == false);

  /****************************************/

	// now extract different marking segments
	// here first of all we check for the markings which have any overlapping segment
	// of same or different mark
	// we discard corresponding mark and take rest of the marks

	// traverse through channels
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (mark_count = 0; ((mark_count < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); mark_count++)
		{
			overlapping_mark_found = false;

			//invalid start or end time of the mark - continue
			if ((temp_data_mark->mark_start_time == -1) || (temp_data_mark->mark_end_time == -1))	
				continue;	

      // this condition checks the event marks which needs to be exported
			if (/*(temp_data_mark->mark_event == SEIZURE_MARK) || */
				(temp_data_mark->mark_event == BURST_MARK) || 
				(temp_data_mark->mark_event == ARTIFACT_MARK) || 
				/* (temp_data_mark->mark_event == SLEEP_SPINDLE_MARK) || */
				(temp_data_mark->mark_event == BURST_SUPPRESSION_MARK) || 
				(temp_data_mark->mark_event == NORMAL_MARK) || 
        (temp_data_mark->mark_event == LONG_DATASET_NO_SPECIFIC_MARK_ID))
			{
				// initialize the 2nd pointer with the header node of the current channel
				temp_data_mark2 = marking_database[ch_no];

				// now when we encounter a mark
				// we check whether any other mark has its timing overlapping with current mark time 
				// or within 3 sec duration before and 2 sec duration after
				for (i = 0; ((i < mark_database_count[ch_no]) && 
					(temp_data_mark2->mark_end_time <= total_recording_duration_sec)); i++)
				{
					// for current mark, skip any comparison
					if (i == mark_count)
						continue;

					// invalid start or end time
					if ((temp_data_mark2->mark_start_time == -1) || (temp_data_mark2->mark_end_time == -1))	
						continue;

# if 1
          // if the mark is not either burst or artifact then we don't consider it
          // in other words, for the first burst or artifact marks, if we find
          // overlapping burst or artifact marks then we will not include the mark for export
					if ((temp_data_mark2->mark_event != BURST_MARK) && 
            (temp_data_mark2->mark_event != ARTIFACT_MARK) &&
            (temp_data_mark2->mark_event != BURST_SUPPRESSION_MARK) &&
            (temp_data_mark2->mark_event != NORMAL_MARK) &&
            (temp_data_mark2->mark_event != LONG_DATASET_NO_SPECIFIC_MARK_ID))
						continue;
# endif

					// if any mark starts within current mark interval
					if ((temp_data_mark2->mark_start_time >= temp_data_mark->mark_start_time) && 
						(temp_data_mark2->mark_start_time <= temp_data_mark->mark_end_time))
					{
						overlapping_mark_found = true;
						break;
					}

					// if any mark ends within current mark interval
					if ((temp_data_mark2->mark_end_time >= temp_data_mark->mark_start_time) && 
						(temp_data_mark2->mark_end_time <= temp_data_mark->mark_end_time))
					{
						overlapping_mark_found = true;
						break;
					}

					// if any mark end time is within 3 sec of current mark start time
					if ((temp_data_mark2->mark_end_time < temp_data_mark->mark_start_time) && 
						((temp_data_mark->mark_start_time - temp_data_mark2->mark_end_time) < BEFORE_MARK_DURATION_NON_OVERLAP))
					{
						overlapping_mark_found = true;
						break;
					}

					// if any mark start time is within 2 sec of current mark end time
					if ((temp_data_mark2->mark_start_time > temp_data_mark->mark_end_time) && 
						((temp_data_mark2->mark_start_time - temp_data_mark->mark_end_time) < AFTER_MARK_DURATION_NON_OVERLAP))
					{
						overlapping_mark_found = true;
						break;
					}

					// if any mark start or end time is crosses the boundary
					if ((temp_data_mark->mark_start_time - BEFORE_MARK_DURATION_NON_OVERLAP) < 0)
					{
						overlapping_mark_found = true;
						break;
					}

					if ((temp_data_mark->mark_end_time + AFTER_MARK_DURATION_NON_OVERLAP + 0.5) > total_recording_duration_sec)
					{
						overlapping_mark_found = true;
						break;
					}

					// advance the node pointer 
					temp_data_mark2 = temp_data_mark2->next;

				}	// end other mark checking

				if (overlapping_mark_found == false)
				{
					// there is no overlapping mark
					// so we can write the data
					rec_start_time = (int)(temp_data_mark->mark_start_time - BEFORE_MARK_DURATION_NON_OVERLAP);
					rec_end_time = (int)(temp_data_mark->mark_end_time + AFTER_MARK_DURATION_NON_OVERLAP + 0.5);

					//record_start_time.Format("%d", (rec_start_time + 1));
          record_start_time.Format("%d", rec_start_time);
					record_end_time.Format("%d", rec_end_time);

          // now determine the file name of the extracted data segment
          // depending upon the type of mark and whether the data is raw or filtered
          extract_raw_data_filename = DetermineFileName(ch_no, directory_name, temp_data_mark->mark_event, 
            filtering_on, rde.filtered_data_extract, rde.selective_channel_data_extract,
            record_start_time, record_end_time);

					// we check if the extracted text file already exists
					// if so then we don't write the file
					if (!(CFile::GetStatus(extract_raw_data_filename, status)))
					{	
						myfile = fopen(extract_raw_data_filename, "wb+");	// open the file
						if(myfile != NULL)	// file is opened successfully
						{
							// at first write the 128 bytes preamble (all zero for the moment)
							strcpy(bftd.preamble_text, "");	//initialization
							strcat(bftd.preamble_text, "DICOM EEG viewer");	
							strcat(bftd.preamble_text, "\t TMLab, CSE, IITKGP");	
							strcat(bftd.preamble_text, "\t Sourya Bhattacharyya");	
							strcat(bftd.preamble_text, "\t Supervisors - J.Mukhopadhyay & A.K.Majumdar");	
							fwrite(&bftd.preamble_text, sizeof(bftd.preamble_text), 1, myfile);

							//then write the 4 characters DICM
							strcpy(bftd.form, "");
							strcat(bftd.form, "DICM");
							fwrite(&bftd.form, sizeof(bftd.form), 1, myfile);

							//modality of the data (that is type EEG)
							tag = 0x00080060;
							strcpy(VR, "CS");
							VL = MEDIUM_STRING_LEN;
							strcpy(bftd.modality, "");
							strcat(bftd.modality, "EEG");		//EEG data signature
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.modality, sizeof(bftd.modality), 1, myfile);

							// patient name
							tag = 0x00100010;
							strcpy(VR, "PN");
							VL = LONG_STRING_LEN;
		          strcpy(bftd.patient_name, "");
		          strcat(bftd.patient_name, rde.patient_name);
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.patient_name, sizeof(bftd.patient_name), 1, myfile);

							// patient ID
							tag = 0x00100020;
							strcpy(VR, "LO");
							VL = LONG_STRING_LEN;
			        strcpy(bftd.patient_ID, "");	
			        strcat(bftd.patient_ID, rde.patient_ID);	
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.patient_ID, sizeof(bftd.patient_ID), 1, myfile);

							// birthdate
							tag = 0x00100030;
							strcpy(VR, "DA");
							VL = SHORT_STRING_LEN;
			        strcpy(bftd.birth_date, "");
			        strcat(bftd.birth_date, rde.birth_date);	//YYYYMMDD format
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.birth_date, sizeof(bftd.birth_date), 1, myfile);

							// patient sex
							tag = 0x00100040;
							strcpy(VR, "CS");
							VL = MEDIUM_STRING_LEN;
			        strcpy(bftd.sex, "");	
			        strcat(bftd.sex, rde.sex);			//"Male" / "Female"
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.sex, sizeof(bftd.sex), 1, myfile);

							// start time of the extracted data
							// as compared to the overall original extracted data
							tag = 0x00080032;
							strcpy(VR, "TM");
							VL = MEDIUM_STRING_LEN;
							strcpy(bftd.start_time, "");		//initialize with empty string	
							int start_hr = rec_start_time / 3600;
							itoa (start_hr, buf, 10);
							if (strlen(buf) == 1)
							{
								strcat(bftd.start_time, "0");		
							}
							strcat(bftd.start_time, buf);	
							int start_min = (rec_start_time - start_hr * 3600) / 60;							
							itoa (start_min, buf, 10);
							if (strlen(buf) == 1)
							{
								strcat(bftd.start_time, "0");		
							}
							strcat(bftd.start_time, buf);
							int start_sec = (rec_start_time - start_hr * 3600 - start_min * 60);
							itoa (start_sec, buf, 10);
							if (strlen(buf) == 1)
							{
								strcat(bftd.start_time, "0");		
							}
							strcat(bftd.start_time, buf);
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.start_time, sizeof(bftd.start_time), 1, myfile);

							// record duration
							tag = 0x00080033;
							strcpy(VR, "TM");
							VL = MEDIUM_STRING_LEN;

							int record_time_total = rec_end_time - rec_start_time;
							int record_hr = record_time_total / 3600;
							int record_min = (record_time_total - record_hr * 3600) / 60;
							int record_sec = (record_time_total - record_hr * 3600 - record_min * 60); 

							//"hhmmss" format
							strcpy(bftd.record_dur, "");
							itoa (record_hr, buf, 10);
							if (strlen(buf) == 1)	{
								strcat(bftd.record_dur, "0");		
							}
							strcat(bftd.record_dur, buf);		
							itoa (record_min, buf, 10);
							if (strlen(buf) == 1)	{
								strcat(bftd.record_dur, "0");		
							}
							strcat(bftd.record_dur, buf);
							itoa (record_sec, buf, 10);
							if (strlen(buf) == 1)	{
								strcat(bftd.record_dur, "0");		
							}
							strcat(bftd.record_dur, buf);

							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.record_dur, sizeof(bftd.record_dur), 1, myfile);

							// doctor name performing EEG
							tag = 0x00081050;
							strcpy(VR, "PN");
							VL = LONG_STRING_LEN;
			        strcpy(bftd.doctor_name, "");
			        strcpy(bftd.doctor_name, rde.doctor_name);
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.doctor_name, sizeof(bftd.doctor_name), 1, myfile);

							// patient age
							tag = 0x00101010;
							strcpy(VR, "AS");
							VL = VERY_SHORT_STRING_LEN;
			        strcpy(bftd.age, "");
			        strcat(bftd.age, rde.age);		//"nnD" / "nnW" / "nnM" / "nnY" format
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.age, sizeof(bftd.age), 1, myfile);

							// patient gestational age
							// custom tag definition - not included in DICOM standard
							tag = 0x00101011;
							strcpy(VR, "AS");
							VL = VERY_SHORT_STRING_LEN;
			        strcpy(bftd.gest_age, "");
			        strcat(bftd.gest_age, rde.gest_age);		//"nn" format (in terms of weeks)
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.gest_age, sizeof(bftd.gest_age), 1, myfile);

							//no of samples
							tag = 0x003A0010;
							strcpy(VR, "UL");
							VL = sizeof(channel_data_size);
							bftd.channel_data_size = (int)(record_time_total * sampling_rate);
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.channel_data_size, sizeof(bftd.channel_data_size), 1, myfile);
							
							//sampling rate
							tag = 0x003A001A;
							strcpy(VR, "DS");
							VL = MEDIUM_STRING_LEN;
							//temp.Format("%f", sampling_rate);
							//strcpy(bftd.sampling_rate_buf, temp);
			        strcpy(bftd.sampling_rate_buf, "");
			        strcat(bftd.sampling_rate_buf, rde.sampling_rate_buf);
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.sampling_rate_buf, sizeof(bftd.sampling_rate_buf), 1, myfile);

              // bits per sample
              tag = 0x003A021A;
              strcpy(VR, "US");
              VL = 2;
              bftd.bits_per_sample = 32;		
              fwrite(&tag, sizeof(tag), 1, myfile);
              fwrite(&VR, sizeof(VR), 1, myfile);
              fwrite(&VL, sizeof(VL), 1, myfile);
              fwrite(&bftd.bits_per_sample, sizeof(bftd.bits_per_sample), 1, myfile);

              // filter low frequency
              tag = 0x003A0220;
              strcpy(VR, "DS");
              VL = MEDIUM_STRING_LEN;
			        if ((filtering_on == true) && (rde.filtered_data_extract == true))
				        temp.Format("%f", filter_low_freq_pass);
			        else
				        temp.Format("%f", 0);	// pass 0 as the low pass filter frequency
              strcpy(bftd.filter_low_freq_buf, temp);
              fwrite(&tag, sizeof(tag), 1, myfile);
              fwrite(&VR, sizeof(VR), 1, myfile);
              fwrite(&VL, sizeof(VL), 1, myfile);
              fwrite(&bftd.filter_low_freq_buf, sizeof(bftd.filter_low_freq_buf), 1, myfile);

              // filter high frequency
              tag = 0x003A0221;
              strcpy(VR, "DS");
              VL = MEDIUM_STRING_LEN;
			        if ((filtering_on == true) && (rde.filtered_data_extract == true))
				        temp.Format("%f", filter_high_freq_pass);
			        else
				        temp.Format("%f", 0);	// pass 0 as the high pass filter frequency
              strcpy(bftd.filter_high_freq_buf, temp);
              fwrite(&tag, sizeof(tag), 1, myfile);
              fwrite(&VR, sizeof(VR), 1, myfile);
              fwrite(&VL, sizeof(VL), 1, myfile);
              fwrite(&bftd.filter_high_freq_buf, sizeof(bftd.filter_high_freq_buf), 1, myfile);

							//channel number
							tag = 0x003A0202;
							strcpy(VR, "US");
							VL = 2;
              if (rde.selective_channel_data_extract == true)
              {
							  // here we just put 1 as a total no of channels
							  bftd.sel_chan_no = 1;	//(ch_no);  
              }
              else
              {
                bftd.sel_chan_no = (short)(no_of_channels);			
              }
							fwrite(&tag, sizeof(tag), 1, myfile);
							fwrite(&VR, sizeof(VR), 1, myfile);
							fwrite(&VL, sizeof(VL), 1, myfile);
							fwrite(&bftd.sel_chan_no, sizeof(bftd.sel_chan_no), 1, myfile);

              // when multi channel data export for the current event is enabled
              // then this while loop exports all the channel data (within the event time frame)
              // in a step by step manner
              selected_chan_index = 0;  // init - used for multi channel data extract
              while (1)
              {
							  // channel label
							  tag = 0x003A0203;
							  strcpy(VR, "SH");
							  VL = MEDIUM_STRING_LEN;
							  fwrite(&tag, sizeof(tag), 1, myfile);
							  fwrite(&VR, sizeof(VR), 1, myfile);
							  fwrite(&VL, sizeof(VL), 1, myfile);
				        char temp_strng[MEDIUM_STRING_LEN];
				        strcpy(temp_strng, "");
                if (rde.selective_channel_data_extract == true)  // single channel data extract
                {
							    //fwrite(&channel_name[ch_no], sizeof(channel_name[ch_no]), 1, myfile);
                  strncpy(temp_strng, channel_name[ch_no], (MEDIUM_STRING_LEN * sizeof(char)));
                }
                else
                {
                  //fwrite(&channel_name[selected_chan_index], sizeof(channel_name[selected_chan_index]), 1, myfile);
                  strncpy(temp_strng, channel_name[selected_chan_index], (MEDIUM_STRING_LEN * sizeof(char)));
                }
                fwrite(&temp_strng, sizeof(temp_strng), 1, myfile);

                // following if statement ensures that for the multi channel data export, only the
                // original event associated with the channel indexed hy the "ch_no" will be propagated
                // for single channel data extract, there is no such condition
                // this scheme may be modified later
                if ((rde.selective_channel_data_extract == true) /* single channel data extract */ || 
                    (selected_chan_index == ch_no))  /* multi channel data extract */
                {
							    // here we put any specific marking according to the input mark
							    tag = 0x00400035;
							    strcpy(VR, "CS");
							    VL = VERY_SHORT_STRING_LEN;		// though CS support medium string len
							    temp.Format("%d", temp_data_mark->mark_event);	//pass current mark
							    fwrite(&tag, sizeof(tag), 1, myfile);
							    fwrite(&VR, sizeof(VR), 1, myfile);
							    fwrite(&VL, sizeof(VL), 1, myfile);							  
							    //fwrite(&temp, sizeof(temp), 1, myfile);
				          char temp_data_mark_strng[VERY_SHORT_STRING_LEN];
				          strcpy(temp_data_mark_strng, "");
				          strncpy(temp_data_mark_strng, temp, (VERY_SHORT_STRING_LEN * sizeof(char)));
				          fwrite(&temp_data_mark_strng, sizeof(temp_data_mark_strng), 1, myfile);

							    // in following 2 cases, though the information is in "TM" format
							    // but due to presence of floating point data, we put the data as a floating
							    // point number	
							    // according to input mark, specify start timing 
							    float temp_event_time;

							    tag = 0x00400245;
							    strcpy(VR, "TM");
							    // here we put the marking time with respect to the start time
							    //bftd.event_start_time = (float)(temp_data_mark->mark_start_time - rec_start_time);
							    temp_event_time = (float)(temp_data_mark->mark_start_time - rec_start_time);
							    //VL = sizeof(bftd.event_start_time);	
                  VL = sizeof(temp_event_time);
							    fwrite(&tag, sizeof(tag), 1, myfile);
							    fwrite(&VR, sizeof(VR), 1, myfile);
							    fwrite(&VL, sizeof(VL), 1, myfile);
							    //fwrite(&bftd.event_start_time, sizeof(bftd.event_start_time), 1, myfile);
							    fwrite(&temp_event_time, sizeof(temp_event_time), 1, myfile);

							    // according to input mark, specify end timing
							    tag = 0x00400251;
							    strcpy(VR, "TM");
							    // here we put the marking time with respect to the start time
							    //bftd.event_end_time = (float)(temp_data_mark->mark_end_time - rec_start_time);
							    temp_event_time = (float)(temp_data_mark->mark_end_time - rec_start_time);
							    //VL = sizeof(bftd.event_end_time);	//float 
                  VL = sizeof(temp_event_time);
							    fwrite(&tag, sizeof(tag), 1, myfile);
							    fwrite(&VR, sizeof(VR), 1, myfile);
							    fwrite(&VL, sizeof(VL), 1, myfile);
							    //fwrite(&bftd.event_end_time, sizeof(bftd.event_end_time), 1, myfile);
							    fwrite(&temp_event_time, sizeof(temp_event_time), 1, myfile);
                } // end if - single or multi channel data extract

							  // sample values
							  // here also TAG value is not standardized as per DICOM standard
							  tag = 0x0018602A;
							  strcpy(VR, "FL");	//"FD"	
							  VL = (bftd.bits_per_sample * bftd.channel_data_size) / 8;
							  fwrite(&tag, sizeof(tag), 1, myfile);
							  fwrite(&VR, sizeof(VR), 1, myfile);
							  fwrite(&VL, sizeof(VL), 1, myfile);

							  /////////////////////////////////
							  // now write the sample values

							  int y_start_index = (int)(rec_start_time * sampling_rate);
							  int y_end_index = (int)(rec_end_time * sampling_rate) - 1;
							  int node_count;
							  int start_node_offset, end_node_offset, start_node_num, end_node_num;
							  struct channnel_data_list *temp1;
							  int j;
							  float sample_data;	//double sample_data;

							  start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;
							  end_node_offset = y_end_index % CHAN_DATA_BLOCK_SIZE;
							  start_node_num = y_start_index / CHAN_DATA_BLOCK_SIZE;
							  end_node_num = y_end_index / CHAN_DATA_BLOCK_SIZE;

							  temp1 = head;	//initialization to the beginning of raw data

							  // temp1 should reach to the starting node
							  for (j = 0; j < start_node_num; j++)
							  {
								  temp1 = temp1->next;
							  }

							  start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;
							  for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
							  {
								  if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
								  {
									  for (j = start_node_offset; j <= end_node_offset;	/*(end_node_offset - 1); */	j++)	//start node offset is 0 or set value			
									  {
                      if (rde.selective_channel_data_extract == true)  // single channel data extract
										    sample_data = (float)(temp1->channel_data[ch_no][j]);
                      else
                        sample_data = (float)(temp1->channel_data[selected_chan_index][j]);
										  fwrite(&sample_data, sizeof(sample_data), 1, myfile);
									  }
									  if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
										  temp1 = temp1->next;					
								  }
								  else if (node_count == start_node_num)		// starting node but not end node
								  {
									  for (j = start_node_offset; j <= (CHAN_DATA_BLOCK_SIZE - 1); j++)
									  {
                      if (rde.selective_channel_data_extract == true)  // single channel data extract
										    sample_data = (float)(temp1->channel_data[ch_no][j]);
                      else
                        sample_data = (float)(temp1->channel_data[selected_chan_index][j]);
										  fwrite(&sample_data, sizeof(sample_data), 1, myfile);
									  }					
									  temp1 = temp1->next;
								  }
								  else
								  {
									  for (j = start_node_offset; j <= (CHAN_DATA_BLOCK_SIZE - 1); j++)
									  {
                      if (rde.selective_channel_data_extract == true)  // single channel data extract
										    sample_data = (float)(temp1->channel_data[ch_no][j]);
                      else
                        sample_data = (float)(temp1->channel_data[selected_chan_index][j]);
										  fwrite(&sample_data, sizeof(sample_data), 1, myfile);
									  }					
									  temp1 = temp1->next;
								  }
								  start_node_offset = 0;
							  }
							  /////////////////////////////////

                // now check for siongle or multi channel data extract condition
                // based on that, exit from loop
                selected_chan_index++;
                if ((rde.selective_channel_data_extract == true) /* single channel data extract */ || 
                    (selected_chan_index == no_of_channels))  /* multi channel data extract */
                  break;
               }  // end for loop - multi channel data export
							fclose(myfile);		// close the file
						}	//end if file is open
					}	// end if file already exists
				}	// end if one valid mark found for writing
			}	// end export burst or artifact data

			// advance the node pointer 
			temp_data_mark = temp_data_mark->next;
			
		}	// end marking traverse for 1 channel
	}	// end channel loop

	/****************************************/
  data_exported_bin_format = 1;	//flag signifying the export done at least once

	AfxMessageBox("Exported specific event based data");
}

/*
this function determines the output filename which will store the extracted data segments
the file name is decided based on the event type, and whether the data is filtered or not
*/
CString CEEG_marking_toolDlg::DetermineFileName(int ch_no, CString directory_name, int mark_event, bool filtering_on, 
                          bool filtered_data_extract, bool selective_channel_data_extract,
                          CString record_start_time, CString record_end_time)
{
  CString temp;
  CString extract_raw_data_filename;
	temp = directory_name;
	temp.SetAt((temp.GetLength() - 1), '_');
	temp = temp.Mid(temp.ReverseFind('\\') + 1);				
  if (selective_channel_data_extract == true)  // single channel data extract
  {
    if ((filtering_on == true) && (filtered_data_extract == true))  // filtered data
    {
      if (mark_event == BURST_MARK)
        extract_raw_data_filename = burst_segment_extraction_directory + temp + "filtered_burst_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SEIZURE_MARK)
        extract_raw_data_filename = seizure_segment_extraction_directory + temp + "filtered_seizure_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == ARTIFACT_MARK)
        extract_raw_data_filename = artifact_segment_extraction_directory + temp + "filtered_artifact_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SLEEP_SPINDLE_MARK)
        extract_raw_data_filename = sleep_spindle_segment_extraction_directory + temp + "filtered_sleep_spindle_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == BURST_SUPPRESSION_MARK)
        extract_raw_data_filename = burst_suppression_segment_extraction_directory + temp + "filtered_BS_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";  
      else if (mark_event == NORMAL_MARK)
        extract_raw_data_filename = normal_segment_extraction_directory + temp + "filtered_normal_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == LONG_DATASET_NO_SPECIFIC_MARK_ID)
        extract_raw_data_filename = no_specific_mark_segment_extraction_directory + temp + "filtered_nospecmark_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
    }
    else  // raw data
    {
      if (mark_event == BURST_MARK)
        extract_raw_data_filename = burst_segment_extraction_directory + temp + "raw_burst_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SEIZURE_MARK)
        extract_raw_data_filename = seizure_segment_extraction_directory + temp + "raw_seizure_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == ARTIFACT_MARK)
        extract_raw_data_filename = artifact_segment_extraction_directory + temp + "raw_artifact_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SLEEP_SPINDLE_MARK)
        extract_raw_data_filename = sleep_spindle_segment_extraction_directory + temp + "raw_sleep_spindle_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == BURST_SUPPRESSION_MARK)
        extract_raw_data_filename = burst_suppression_segment_extraction_directory + temp + "raw_BS_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == NORMAL_MARK)
        extract_raw_data_filename = normal_segment_extraction_directory + temp + "raw_normal_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == LONG_DATASET_NO_SPECIFIC_MARK_ID)
        extract_raw_data_filename = no_specific_mark_segment_extraction_directory + temp + "raw_nospecmark_" + channel_name[ch_no] + "_" + record_start_time + "_" + record_end_time + ".bindata";
    }
  }
  else  // multi channel data extract
  { 
    if ((filtering_on == true) && (filtered_data_extract == true))  // filtered data
    {
      if (mark_event == BURST_MARK)
        extract_raw_data_filename = burst_segment_extraction_directory + temp + "filtered_burst_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SEIZURE_MARK)
        extract_raw_data_filename = seizure_segment_extraction_directory + temp + "filtered_seizure_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == ARTIFACT_MARK)
        extract_raw_data_filename = artifact_segment_extraction_directory + temp + "filtered_artifact_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SLEEP_SPINDLE_MARK)
        extract_raw_data_filename = sleep_spindle_segment_extraction_directory + temp + "filtered_sleep_spindle_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == BURST_SUPPRESSION_MARK)
        extract_raw_data_filename = burst_suppression_segment_extraction_directory + temp + "filtered_BS_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == NORMAL_MARK)
        extract_raw_data_filename = normal_segment_extraction_directory + temp + "filtered_normal_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == LONG_DATASET_NO_SPECIFIC_MARK_ID)
        extract_raw_data_filename = no_specific_mark_segment_extraction_directory + temp + "filtered_nospec_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
    }      
    else  // raw data
    {
      if (mark_event == BURST_MARK)
        extract_raw_data_filename = burst_segment_extraction_directory + temp + "raw_burst_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SEIZURE_MARK)
        extract_raw_data_filename = seizure_segment_extraction_directory + temp + "raw_seizure_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == ARTIFACT_MARK)
        extract_raw_data_filename = artifact_segment_extraction_directory + temp + "raw_artifact_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == SLEEP_SPINDLE_MARK)
        extract_raw_data_filename = sleep_spindle_segment_extraction_directory + temp + "raw_sleep_spindle_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == BURST_SUPPRESSION_MARK)
        extract_raw_data_filename = burst_suppression_segment_extraction_directory + temp + "raw_BS_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == NORMAL_MARK)
        extract_raw_data_filename = normal_segment_extraction_directory + temp + "raw_normal_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
      else if (mark_event == LONG_DATASET_NO_SPECIFIC_MARK_ID)
        extract_raw_data_filename = no_specific_mark_segment_extraction_directory + temp + "raw_nospec_MultiChannel_" + record_start_time + "_" + record_end_time + ".bindata";
    }      
  }
  return extract_raw_data_filename;
} // end of function