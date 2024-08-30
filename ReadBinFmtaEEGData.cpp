// ReadBinFmtaEEGData.cpp : implementation file
//

// this code needs to be modified for incorporating multi channel data

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

// function to initialize the configuration for generalized binary format data read
void CEEG_marking_toolDlg::Read_Bin_Fmt_aEEG_Data(void)
{
	FILE* fid;
	int data_count = 0, i;
	struct channnel_data_list *temp = NULL;	//pointer to the main data
	struct channnel_data_list *temp_filtered_data = NULL;	//pointer to the filtered data
	//int ch_no;
	CString outfile = directory_name + output_text_filename; 	
	CFileStatus status;
	char t[10];
	float temp_val;
	int selected_channel_index = 0;
	
	// initialization of global variables
	strcpy(VR, "");
	strcpy(bftd.modality, "");
	strcpy(bftd.patient_name, "");
	strcpy(bftd.patient_ID, "");
	strcpy(bftd.birth_date, "");
	strcpy(bftd.sex, "");
	strcpy(bftd.start_time, "");
	strcpy(bftd.record_dur, "");
	strcpy(bftd.age, "");
	//strcpy(bftd.chan_name, "");
	strcpy(bftd.sampling_rate_buf, "");
	strcpy(bftd.filter_low_freq_buf, "");
	strcpy(bftd.filter_high_freq_buf, "");

	//we read one channel data, thus initialize no of channels as 1
	no_of_channels = 1;	

	//initialize total recording duration 
	total_recording_duration_sec = 0;

	fid = fopen(fileName, "rb+");	//file open
	if (fid != NULL)
	{
		fseek(fid, 0, SEEK_SET);	// go to beginning of file

		// now read the file's preamble		
		fread(&bftd.preamble_text, sizeof(bftd.preamble_text), 1, fid);

		// read the DICM form string
		fread(&bftd.form, sizeof(bftd.form), 1, fid);
		if (strcmpi(bftd.form, "DICM"))
		{
			AfxMessageBox("Not a DICOM compatible binary file input!");	
			return;
		}

		// now loop through 
		while (!feof (fid))
		{
			// read the tag
			fread(&tag, sizeof(tag), 1, fid);
			fread(&VR, sizeof(VR), 1, fid);
			fread(&VL, sizeof(VL), 1, fid);

			//check for different values of tag and store data accordingly
			if (tag == 0x00080060)		//modality
			{
				fread(&bftd.modality, sizeof(bftd.modality), 1, fid);
				if (strcmpi(bftd.modality, "EEG"))
				{
					AfxMessageBox("Not a EEG data set!");	
					return;
				}
			}
			else if (tag == 0x00100010)		//patient name
			{	
				fread(&bftd.patient_name, sizeof(bftd.patient_name), 1, fid);
			}
			else if (tag == 0x00100020)		//patient ID
			{	
				fread(&bftd.patient_ID, sizeof(bftd.patient_ID), 1, fid);
			}
			else if (tag == 0x00100030)		//birth date
			{	
				fread(&bftd.birth_date, sizeof(bftd.birth_date), 1, fid);
			}
			else if (tag == 0x00100040)		//patient sex
			{	
				fread(&bftd.sex, sizeof(bftd.sex), 1, fid);
			}
			else if (tag == 0x00080032)		//start time (w.r.t original text extracted data)
			{	
				fread(&bftd.start_time, sizeof(bftd.start_time), 1, fid);

				// hr
				strcpy(t, "");
				t[0] = bftd.start_time[0];	t[1] = bftd.start_time[1];	t[2] = '\0';
				binary_fmt_data_read_start_time_offset = (atoi(t) * 3600);

				// min
				strcpy(t, "");
				t[0] = bftd.start_time[2];	t[1] = bftd.start_time[3];	t[2] = '\0';
				binary_fmt_data_read_start_time_offset += (atoi(t) * 60);

				//sec
				strcpy(t, "");
				t[0] = bftd.start_time[4];	t[1] = bftd.start_time[5];	t[2] = '\0';
				binary_fmt_data_read_start_time_offset += atoi(t);
			}
			else if (tag == 0x00080033)		//recording duration (extracted data length)
			{	
				fread(&bftd.record_dur, sizeof(bftd.record_dur), 1, fid);

				// hr
				strcpy(t, "");
				t[0] = bftd.record_dur[0];	t[1] = bftd.record_dur[1];	t[2] = '\0';
				total_recording_duration_sec += (atoi(t) * 3600);

				// min
				strcpy(t, "");
				t[0] = bftd.record_dur[2];	t[1] = bftd.record_dur[3];	t[2] = '\0';
				total_recording_duration_sec += (atoi(t) * 60);

				//sec
				strcpy(t, "");
				t[0] = bftd.record_dur[4];	t[1] = bftd.record_dur[5];	t[2] = '\0';
				total_recording_duration_sec += atoi(t);
			}
			else if (tag == 0x00081050)		// doctor name performing EEG
			{	
				fread(&bftd.doctor_name, sizeof(bftd.doctor_name), 1, fid);
			}
			else if (tag == 0x00101010)		//patient age
			{	
				fread(&bftd.age, sizeof(bftd.age), 1, fid);
			}
			else if (tag == 0x00101011)		//patient gestational age
			{	
				fread(&bftd.gest_age, sizeof(bftd.gest_age), 1, fid);
			}
			else if (tag == 0x003A0010)		//no of samples
			{	
				fread(&bftd.channel_data_size, sizeof(bftd.channel_data_size), 1, fid);
				channel_data_size = bftd.channel_data_size;
			}
			else if (tag == 0x003A001A)		//sampling rate
			{	
				fread(&bftd.sampling_rate_buf, sizeof(bftd.sampling_rate_buf), 1, fid);
				sampling_rate = atof(bftd.sampling_rate_buf);

				// set this parameter required for filtering
				TIME_LENGTH = (10000.0 / sampling_rate);
			}
			else if (tag == 0x003A0202)		//channel number
			{
				// this variable is slightly modified
				// it now stores the no of channels (one or all) for a single file
				fread(&bftd.sel_chan_no, sizeof(bftd.sel_chan_no), 1, fid);
				
				// accordingly we update the variable containing the total no of channels
				no_of_channels = (int)bftd.sel_chan_no;

				/////////////////////////////////////////////////
				// now we have to allocate the memory corresponding to the information
				// contained in the multi channel data
				bftd.chan_name = (char **)malloc(no_of_channels * sizeof(char *));
				for (i = 0; i < no_of_channels; i++)	
				{
					// size of a channel name string
					bftd.chan_name[i] = (char *)malloc(MEDIUM_STRING_LEN * sizeof(char));	
				}


				bftd.specific_segment_marking = (char **)malloc(no_of_channels * sizeof(char *));
				for (i = 0; i < no_of_channels; i++)	
				{
					// size of a channel name string
					bftd.specific_segment_marking[i] = (char *)malloc(VERY_SHORT_STRING_LEN * sizeof(char));	
				}


				bftd.event_start_time = (float *)malloc(no_of_channels * sizeof(float));
				bftd.event_end_time = (float *)malloc(no_of_channels * sizeof(float));

				/////////////////////////////////////////////////
			}
			else if (tag == 0x003A0203)		//channel label
			{	
				selected_channel_index++;	// increment the selected channel index
				// read the channel label in a temporary string
				char temp_strng[MEDIUM_STRING_LEN];
				strcpy(temp_strng, "");
				fread(&temp_strng, sizeof(temp_strng), 1, fid);	
				// now copy the string in the channel name structure
				strncpy(bftd.chan_name[selected_channel_index - 1], temp_strng, (MEDIUM_STRING_LEN * sizeof(char)));
				//fread(&bftd.chan_name[selected_channel_index - 1], (sizeof(char) * MEDIUM_STRING_LEN), 1, fid);
			}
			else if (tag == 0x003A021A)		//bits per sample
			{	
				fread(&bftd.bits_per_sample, sizeof(bftd.bits_per_sample), 1, fid);
			}
			else if (tag == 0x003A0220)		//filter low frequency
			{	
				fread(&bftd.filter_low_freq_buf, sizeof(bftd.filter_low_freq_buf), 1, fid);
				filter_low_freq_pass = atof(bftd.filter_low_freq_buf);
			}
			else if (tag == 0x003A0221)		//filter high frequency
			{	
				fread(&bftd.filter_high_freq_buf, sizeof(bftd.filter_high_freq_buf), 1, fid);
				filter_high_freq_pass = atof(bftd.filter_high_freq_buf);
			}
			else if (tag == 0x00400035)		//specific mark label
			{
				char temp_data_mark_strng[VERY_SHORT_STRING_LEN];
				strcpy(temp_data_mark_strng, "");
				fread(&temp_data_mark_strng, sizeof(temp_data_mark_strng), 1, fid);	
				// now copy the string in the channel name structure
				strncpy(bftd.specific_segment_marking[selected_channel_index - 1], 
					temp_data_mark_strng, (VERY_SHORT_STRING_LEN * sizeof(char)));
				//fread(&bftd.specific_segment_marking[selected_channel_index - 1], 
					//sizeof(bftd.specific_segment_marking[selected_channel_index - 1]), 1, fid);
			}
			else if (tag == 0x00400245)		//event start time
			{
				fread(&bftd.event_start_time[selected_channel_index - 1], 
					sizeof(bftd.event_start_time[selected_channel_index - 1]), 1, fid);
			}
			else if (tag == 0x00400251)		//event end time
			{
				fread(&bftd.event_end_time[selected_channel_index - 1], 
					sizeof(bftd.event_end_time[selected_channel_index - 1]), 1, fid);
			}
			else if (tag == 0x0018602A)		//sample values
			{
				// for the very first channel, we have to allocate the nodes
				// and also fill the data for the channel
				if (selected_channel_index == 1)
				{
					// here read the data - total data size is already read
					for (data_count = 0; data_count < channel_data_size; data_count++)
					{
						if ((data_count % CHAN_DATA_BLOCK_SIZE) == 0) 
						{
							// here we have to allocate one node of list 
							// which will contain channel data 
							// however we allocate space for all the channels simultaneously
							// so this allocation is performed when selected_channel_index is 1

							///////////////////////////////////////////////
							// allocate one node of list which will contain channel data 
							temp = (struct channnel_data_list *)malloc(sizeof(struct channnel_data_list));
							if (temp == NULL)
							{
								AfxMessageBox("Insufficient memory - no new node alloc");	
								return;
							}
							list_nodecount++;	// increment the node count
							temp->next = NULL;
							temp->prev = NULL;
							if (head == NULL && tail == NULL)	//  this is the first node
							{
								head = tail = temp;		// assign pointer
							}
							else
							{
								temp->prev = tail;
								tail->next = temp;
								tail = temp;			// assign pointer
							}
							// for each channel, allocate one chunk of memory
							tail->channel_data = (double**) calloc(no_of_channels, sizeof(double*));
							if (tail->channel_data == NULL)
							{
								AfxMessageBox("Insufficient memory - basic channel data alloc");	
								return;
							}
							for (i = 0; i < no_of_channels; i++)
							{
								tail->channel_data[i] = (double*) calloc(CHAN_DATA_BLOCK_SIZE, sizeof(double));
								if (tail->channel_data[i] == NULL)
								{
									AfxMessageBox("Insufficient memory - no new channel data alloc");	
									return;
								}
							}

							///////////////////////////////////////////////
							// allocate one node of list which will contain filtered channel data
							// this portion is executed only if filtering on is true
							if (filtering_on == true)
							{ 
								temp_filtered_data = (struct channnel_data_list *)malloc(sizeof(struct channnel_data_list));
								if (temp_filtered_data == NULL)
								{
									AfxMessageBox("Insufficient memory - no new node alloc");	
									return;
								}
								temp_filtered_data->next = NULL;
								temp_filtered_data->prev = NULL;
								if (head_filtered_data == NULL && tail_filtered_data == NULL)	//  this is the first node
								{
									head_filtered_data = tail_filtered_data = temp_filtered_data;		// assign pointer
								}
								else
								{
									temp_filtered_data->prev = tail_filtered_data;
									tail_filtered_data->next = temp_filtered_data;
									tail_filtered_data = temp_filtered_data;			// assign pointer
								}
								
								// for each channel, allocate one chunk of memory
								tail_filtered_data->channel_data = (double**) calloc(no_of_channels, sizeof(double*));
								if (tail_filtered_data->channel_data == NULL)
								{
									AfxMessageBox("Insufficient memory - basic filtered channel data alloc");	
									return;
								}
								for (i = 0; i < no_of_channels; i++)
								{
									tail_filtered_data->channel_data[i] = (double*) calloc(CHAN_DATA_BLOCK_SIZE, sizeof(double));
									if (tail_filtered_data->channel_data[i] == NULL)
									{
										AfxMessageBox("Insufficient memory - no new filtered channel data alloc");	
										return;
									}
								}
							}	// filtered data allocation
						}	// data count is interger multiple of block size and channel index is 1

						// after allocation, store the data
						// data is stored in float type - we typecast it into target storage
						fread(&temp_val, sizeof(temp_val), 1, fid);
						tail->channel_data[selected_channel_index - 1][data_count % CHAN_DATA_BLOCK_SIZE] = (double)(temp_val);
						
						if (filtering_on == true)
						{
							// copy the raw data to the filtered data as well
							tail_filtered_data->channel_data[selected_channel_index - 1][data_count % CHAN_DATA_BLOCK_SIZE] = temp_val;	//tail->channel_data[selected_channel_index - 1][data_count % CHAN_DATA_BLOCK_SIZE];
						}
					}	// end of data store loop
				}	// end first channel data read	
				else	// selected channel is not the first channel
				{
					for (data_count = 0; data_count < channel_data_size; data_count++)
					{
						if (data_count == 0)
						{
							// at first inirialize the node pointers which are needed to traverse
							// the linked lists
							temp = head;
							temp_filtered_data = head_filtered_data;
						}
						else if ((data_count % CHAN_DATA_BLOCK_SIZE) == 0) 
						{
							// otherwise we have to move to the next node of the list
							temp = temp->next;
							temp_filtered_data = temp_filtered_data->next;
						}
						fread(&temp_val, sizeof(temp_val), 1, fid);
						temp->channel_data[selected_channel_index - 1][data_count % CHAN_DATA_BLOCK_SIZE] = (double)(temp_val);
						if (filtering_on == true)
						{
							// copy the raw data to the filtered data as well
							temp_filtered_data->channel_data[selected_channel_index - 1][data_count % CHAN_DATA_BLOCK_SIZE] = temp_val;	//tail->channel_data[selected_channel_index - 1][data_count % CHAN_DATA_BLOCK_SIZE];
						}		
					}	// end of data store loop

					// forcibly quit the reading if all channel data is read
					// feof function is causing some problem
					if (selected_channel_index == no_of_channels)
						break;

				}	// end subsequent channel data
			}	// tag for raw data
		}	// file pointer EOF check		
	}	// file open check
	else
	{
		AfxMessageBox("Could Not Open Binary input file - return !");
	}

	//close the input file
	if (fid != NULL)
		fclose(fid);	

# if 1

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
  for (int c = 0; c < no_of_channels; c++)
  {
    fprintf(temp_fod, "\n channel no : %d  label : %s  mark : %f  event start : %f  event end : %f ", 
      c, bftd.chan_name[c], atof(bftd.specific_segment_marking[c]), bftd.event_start_time[c], bftd.event_end_time[c]);
  }

	if ((filter_low_freq_pass == 0) && (filter_high_freq_pass == 0))
		fprintf(temp_fod, "\n Filtering is off ---- ");
	else
		fprintf(temp_fod, "\n Filtering is on ---- freq range :  %s  -- %s ", bftd.filter_low_freq_buf, bftd.filter_high_freq_buf);

	fclose(temp_fod);

# endif /* DEBUG_INFO_PRINT */





	// based on input filtering frequency, determine whether filtering is allowed or not
	if ((filter_low_freq_pass == 0) && (filter_high_freq_pass == 0))
	{
		// set the filtering frequencies
		filter_low_freq_pass = 1.6;	
		filter_high_freq_pass = 15;	
		//filtering_on = false;
	}

}
