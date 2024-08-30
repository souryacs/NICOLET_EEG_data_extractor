// ExportRawData.cpp : implementation file
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

void CEEG_marking_toolDlg::ExportRawData(void)
{
	CString extract_raw_data_filename;
	CString temp;
	int selected_channel_index;
	CFileStatus status;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node

	FILE *myfile;
	CString record_start_time;
	CString record_end_time;
	char buf[10];
	int i;

	// these variables are required for start and end time computation
	// of the current specified interval for extraction
	int rec_start_time, rec_end_time;

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

	// this variable signifies whether one or multi channel raw data will be extracted
	if (rde.selective_channel_data_extract == false)
		selected_channel_index = 0;		// start from the first channel
	else
		selected_channel_index = rde.selected_channel_index;

	// loop to extract the text data
	// in the previous version of the code, multi channel data extraction yielded separate data files
	// in thid version, it leads to a consolidated file containing all channel data
	// it is required for multi channel pattern analysis

	if (rde.partial_time_duration == false)
	{
		rec_start_time = 0;
		rec_end_time = (int)(total_recording_duration_sec);
	}
	else
	{
		rec_start_time = (int)(rde.start_hr * 3600 + rde.start_min * 60 + rde.start_sec);
		rec_end_time = (int)(rde.end_hr * 3600 + rde.end_min * 60 + rde.end_sec);
	}
	record_start_time.Format("%d", rec_start_time);
	record_end_time.Format("%d", rec_end_time);

	// extract the subdirectory name for easy identification of the patient data
	temp = directory_name;
	temp.SetAt((temp.GetLength() - 1), '_');
	temp = temp.Mid(temp.ReverseFind('\\') + 1);				

	if ((filtering_on == true) && (rde.filtered_data_extract == true))
	{
		if (rde.selective_channel_data_extract == false)	// all channel data
		{
			extract_raw_data_filename = directory_name + temp + "filtered_data_MultiChannnel_" + record_start_time + "_" + record_end_time + ".bindata";
		}
		else	// specified channel data
		{
			extract_raw_data_filename = directory_name + temp + "filtered_data_" + channel_name[selected_channel_index] + "_" + record_start_time + "_" + record_end_time + ".bindata";
		}
	}
	else
	{
		if (rde.selective_channel_data_extract == false)	// all channel data
		{
			extract_raw_data_filename = directory_name + temp + "raw_data_MultiChannnel_" + record_start_time + "_" + record_end_time + ".bindata";
		}
		else	// specified channel data
		{
			extract_raw_data_filename = directory_name + temp + "raw_data_" + channel_name[selected_channel_index] + "_" + record_start_time + "_" + record_end_time + ".bindata";
		}
	}

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
			// start time is mentioned in the RawDataExtractor structure
			tag = 0x00080032;
			strcpy(VR, "TM");
			VL = MEDIUM_STRING_LEN;
			strcpy(bftd.start_time, "");		//initialize with empty string	
			itoa (rde.start_hr, buf, 10);
			if (strlen(buf) == 1)
			{
				strcat(bftd.start_time, "0");		
			}
			strcat(bftd.start_time, buf);		
			itoa (rde.start_min, buf, 10);
			if (strlen(buf) == 1)
			{
				strcat(bftd.start_time, "0");		
			}
			strcat(bftd.start_time, buf);
			itoa (rde.start_sec, buf, 10);
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

			int record_time_total = (rec_end_time - rec_start_time);
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
			
			// channel number
			// insert the no of channels (one or many)
			tag = 0x003A0202;
			strcpy(VR, "US");
			VL = 2;
			// if partial (single) channel data is extracted then put the channels as 1
			// otherwise put the total no of channels
			if (rde.selective_channel_data_extract == true)
				bftd.sel_chan_no = 1;		
			else
				bftd.sel_chan_no = (short)(no_of_channels);		
			fwrite(&tag, sizeof(tag), 1, myfile);
			fwrite(&VR, sizeof(VR), 1, myfile);
			fwrite(&VL, sizeof(VL), 1, myfile);
			fwrite(&bftd.sel_chan_no, sizeof(bftd.sel_chan_no), 1, myfile);

			while (1)
			{
				// channel label
				tag = 0x003A0203;
				strcpy(VR, "SH");
				VL = MEDIUM_STRING_LEN;
				//strcpy(bftd.chan_name, "");
				//strcat(bftd.chan_name, channel_name[selected_channel_index]);		//channel name is already stored
				fwrite(&tag, sizeof(tag), 1, myfile);
				fwrite(&VR, sizeof(VR), 1, myfile);
				fwrite(&VL, sizeof(VL), 1, myfile);
				//fwrite(&bftd.chan_name, sizeof(bftd.chan_name), 1, myfile);
				char temp_strng[MEDIUM_STRING_LEN];
				strcpy(temp_strng, "");
				strncpy(temp_strng, channel_name[selected_channel_index], (MEDIUM_STRING_LEN * sizeof(char)));
				fwrite(&temp_strng, sizeof(temp_strng), 1, myfile);

				// following code is modified
				// initially no data mark was put on the extracted raw data
				// now based on the user's choice, original marking will be kept					
				bool mark_found = false;

				tag = 0x00400035;
				strcpy(VR, "CS");
				VL = VERY_SHORT_STRING_LEN;		// though CS support medium string len
				if (rde.include_original_labels == false)
				{
					// here we don't put any specific marking
					temp.Format("%d", LONG_DATASET_NO_SPECIFIC_MARK_ID);
				}
				else
				{
					// search in the marking database whether there is some mark within current channel data
					// in the specified interval
					// if so then include that original marking

					// initialize the pointer with the header node of the current channel
					temp_data_mark = marking_database[selected_channel_index];

					// traverse through markings
					for (i = 0; ((i < mark_database_count[selected_channel_index]) && 
						(temp_data_mark->mark_end_time <= total_recording_duration_sec)); i++)
					{
						// search for the markings which are completely embedded within the current interval								
						if ((temp_data_mark->mark_start_time >= rec_start_time) && 
							(temp_data_mark->mark_end_time <= rec_end_time))
						{
							// check for following mark types
							if ((temp_data_mark->mark_event == SEIZURE_MARK) || 
								(temp_data_mark->mark_event == BURST_MARK) || 
								(temp_data_mark->mark_event == ARTIFACT_MARK) || 
								(temp_data_mark->mark_event == SLEEP_SPINDLE_MARK) || 
								(temp_data_mark->mark_event == BURST_SUPPRESSION_MARK))
							{
								mark_found = true;
								break;
							}	// search for the mark types
						}	// search for the mark time
						// advance the node pointer
						temp_data_mark = temp_data_mark->next;
					}	// check for existing marks on the current channel

					if (mark_found == true)
					{
						// corresponding mark is put
						temp.Format("%d", temp_data_mark->mark_event);
					}
					else
					{
						// here we don't put any specific marking
						temp.Format("%d", LONG_DATASET_NO_SPECIFIC_MARK_ID);
					}
				}	// original label put check
				fwrite(&tag, sizeof(tag), 1, myfile);
				fwrite(&VR, sizeof(VR), 1, myfile);
				fwrite(&VL, sizeof(VL), 1, myfile);
				//strcpy(bftd.specific_segment_marking, temp);
				//fwrite(&bftd.specific_segment_marking, sizeof(bftd.specific_segment_marking), 1, myfile);
				//fwrite(&temp, VERY_SHORT_STRING_LEN * sizeof(char), 1, myfile);
				char temp_data_mark_strng[VERY_SHORT_STRING_LEN];
				strcpy(temp_data_mark_strng, "");
				strncpy(temp_data_mark_strng, temp, (VERY_SHORT_STRING_LEN * sizeof(char)));
				fwrite(&temp_data_mark_strng, sizeof(temp_data_mark_strng), 1, myfile);

				// for marking information, provide the timing of the event as well
				if ((rde.include_original_labels == true) && (mark_found == true))
				{
					float temp_event_time;

					// in following 2 cases, though the information is in "TM" format
					// but due to presence of floating point data, we put the data as a floating
					// point number	
					// according to input mark, specify start timing 
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
					//VL = sizeof(bftd.event_end_time);	
					VL = sizeof(temp_event_time);
					fwrite(&tag, sizeof(tag), 1, myfile);
					fwrite(&VR, sizeof(VR), 1, myfile);
					fwrite(&VL, sizeof(VL), 1, myfile);
					//fwrite(&bftd.event_end_time, sizeof(bftd.event_end_time), 1, myfile);
					fwrite(&temp_event_time, sizeof(temp_event_time), 1, myfile);
				}

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

				// if filtering is On and filtered data needs to be extracted
				// then temporary pointer will point to the 1st node of the filtered data set
				if ((filtering_on == true) && (rde.filtered_data_extract == true))
				{
					temp1 = head_filtered_data;	//initialization
				}
				else	//temporary pointer will point to the 1st node of the raw data set
				{
					temp1 = head;	//initialization
				}

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
						for (j = start_node_offset; j <= end_node_offset; /* (end_node_offset - 1); */ j++)	//start node offset is 0 or set value			
						{
							sample_data = (float)(temp1->channel_data[selected_channel_index][j]);
							fwrite(&sample_data, sizeof(sample_data), 1, myfile);
						}
						if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
							temp1 = temp1->next;					
					}
					else if (node_count == start_node_num)		// starting node but not end node
					{
						for (j = start_node_offset; j <= (CHAN_DATA_BLOCK_SIZE - 1); j++)
						{
							sample_data = (float)(temp1->channel_data[selected_channel_index][j]);
							fwrite(&sample_data, sizeof(sample_data), 1, myfile);
						}					
						temp1 = temp1->next;
					}
					else
					{
						for (j = start_node_offset; j <= (CHAN_DATA_BLOCK_SIZE - 1); j++)
						{
							sample_data = (float)(temp1->channel_data[selected_channel_index][j]);
							fwrite(&sample_data, sizeof(sample_data), 1, myfile);
						}					
						temp1 = temp1->next;
					}
					start_node_offset = 0;
				}
				/////////////////////////////////

				selected_channel_index++;	//increment the index of selected channels

				// if there is only a selected channel extraction to be done 
				// or if the total no of channels is already extracted
				// then quit from this loop
				if ((rde.selective_channel_data_extract == true) || 
					(selected_channel_index == no_of_channels))
					break;

			}	// end while loop of data extraction
		}	//end if file is open

		// close the file 
		if (myfile != NULL)
			fclose(myfile);		

	}	// end if file already exists


# ifdef DEBUG_INFO_PRINT

	CString out_sample_text_filename = directory_name + "sample_header.txt";
	FILE *temp_fod = fopen(out_sample_text_filename, "w");

	fprintf(temp_fod, "\n preamble :  \n %s ", bftd.preamble_text);
	fprintf(temp_fod, "\n modality :  %s ", bftd.modality);
	fprintf(temp_fod, "\n form :  %s ", bftd.form);
	fprintf(temp_fod, "\n age :  %s ", bftd.age);
	fprintf(temp_fod, "\n birth date :  %s ", bftd.birth_date);
	fprintf(temp_fod, "\n channel data size :  %d ", bftd.channel_data_size);
	fprintf(temp_fod, "\n doctor name :  %s ", bftd.doctor_name);
	fprintf(temp_fod, "\n gest_age :  %s ", bftd.gest_age);
	fprintf(temp_fod, "\n patient_ID :  %s ", bftd.patient_ID);
	fprintf(temp_fod, "\n patient_name :  %s ", bftd.patient_name);
	fprintf(temp_fod, "\n record_dur :  %s ", bftd.record_dur);
	fprintf(temp_fod, "\n sampling_rate_buf :  %s ", bftd.sampling_rate_buf);
	fprintf(temp_fod, "\n sex :  %s ", bftd.sex);
	fprintf(temp_fod, "\n start_time :  %s ", bftd.start_time);

	if ((filter_low_freq_pass == 0) && (filter_high_freq_pass == 0))
		fprintf(temp_fod, "\n Filtering is off ---- ");
	else
		fprintf(temp_fod, "\n Filtering is on ---- freq range :  %s  -- %s ", bftd.filter_low_freq_buf, bftd.filter_high_freq_buf);

	fclose(temp_fod);

# endif /* DEBUG_INFO_PRINT */

	data_exported_bin_format = 1;	//flag signifying the export done at least once

	AfxMessageBox("Raw Data extraction is done");

}	//end of function