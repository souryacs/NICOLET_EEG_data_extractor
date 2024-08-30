// ExportEvents.cpp : implementation file
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

void CEEG_marking_toolDlg::ExportEvents(void)
{

	int y_start_index, y_end_index;
	int ch_no;
	int i, j;
	struct channnel_data_list *temp1;
	int start_node_offset, end_node_offset, start_node_num, end_node_num;
	int node_count;
	struct data_mark_format* temp_data_mark;	// pointer to new allocated node

	FILE *fod; 
	CString out_mark_text_filename = directory_name + "marking_data_exported.txt";
	fod = fopen(out_mark_text_filename, "w");

	fprintf(fod, "\n\n Sampling rate :  %lf  \n\n", sampling_rate);

	// traverse through channels
	for (ch_no = 0; ch_no < no_of_channels; ch_no++)
	{
		fprintf(fod, "\n\n\n\n CHANNEL NO :  %s  \n\n\n\n", channel_name[ch_no]);

		// initialize the pointer with the header node of the current channel
		temp_data_mark = marking_database[ch_no];

		// traverse through markings
		for (i = 0; ((i < mark_database_count[ch_no]) && (temp_data_mark->mark_end_time <= total_recording_duration_sec)); i++)
		{
			if ((temp_data_mark->mark_event == SEIZURE_MARK) || 
				(temp_data_mark->mark_event == BURST_MARK) || 
				(temp_data_mark->mark_event == ARTIFACT_MARK) || 
				(temp_data_mark->mark_event == SLEEP_SPINDLE_MARK) || 
				(temp_data_mark->mark_event == BURST_SUPPRESSION_MARK))
			{
				if (temp_data_mark->mark_event == SEIZURE_MARK)	
				{
					fprintf(fod, "\n\n SEIZURE ");
				}
				else if (temp_data_mark->mark_event == BURST_MARK) 
				{
					fprintf(fod, "\n\n BURST ");
				}
				else if (temp_data_mark->mark_event == ARTIFACT_MARK) 
				{
					fprintf(fod, "\n\n ARTIFACT ");
				}
				else if (temp_data_mark->mark_event == SLEEP_SPINDLE_MARK) 
				{
					fprintf(fod, "\n\n SLEEP SPINDLE ");
				}
				else if (temp_data_mark->mark_event == BURST_SUPPRESSION_MARK)
				{
					fprintf(fod, "\n\n Burst Suppression ");
				}

				fprintf(fod, "   start time : %lf  end time : %lf  ", temp_data_mark->mark_start_time, temp_data_mark->mark_end_time);

				// define start and end boundary
				y_start_index = (int)(temp_data_mark->mark_start_time * sampling_rate);
				y_end_index = (int)(temp_data_mark->mark_end_time * sampling_rate);
				start_node_offset = y_start_index % CHAN_DATA_BLOCK_SIZE;
				end_node_offset = y_end_index % CHAN_DATA_BLOCK_SIZE;
				start_node_num = y_start_index / CHAN_DATA_BLOCK_SIZE;
				end_node_num = y_end_index / CHAN_DATA_BLOCK_SIZE;

				fprintf(fod, "   no of samples : %d  \n", (y_end_index - y_start_index + 1));

				// if filtering is on then temporary ponter will point to the start of filtered data containing node set
				if (filtering_on == true)
				{
					temp1 = head_filtered_data;	//initialization
				}
				else	// it will contain to the start of raw data containing node set
				{
					temp1 = head;	//initialization
				}

				// temp1 should reach to the starting node
				for (j = 0; j < start_node_num; j++)
				{
					temp1 = temp1->next;
				}

				// extract the data
				for (node_count = start_node_num; node_count <= end_node_num; node_count++) 
				{
					if (node_count == end_node_num)		// this is the last node - may or may not be completely processed
					{
						for (j = start_node_offset; j <= (end_node_offset - 1); j++)	//start node offset is 0 or set value			
							fprintf(fod, "\n %lf", temp1->channel_data[ch_no][j]);
						
						if (end_node_offset == (CHAN_DATA_BLOCK_SIZE - 1))	/// this node is completely processed
							temp1 = temp1->next;					
					}
					else if (node_count == start_node_num)		// starting node but not end node
					{
						for (j = start_node_offset; j < (CHAN_DATA_BLOCK_SIZE - 1); j++)
							fprintf(fod, "\n %lf", temp1->channel_data[ch_no][j]);

						temp1 = temp1->next;
					}
					else
					{
						for (j = start_node_offset; j < (CHAN_DATA_BLOCK_SIZE - 1); j++)
							fprintf(fod, "\n %lf", temp1->channel_data[ch_no][j]);	

						temp1 = temp1->next;
					}
					start_node_offset = 0;
				}
			}	// marking check

			// advance the node pointer
			temp_data_mark = temp_data_mark->next;

		}	// end of for loop for 1 channel marks
	}	// end of channel loop

	fclose(fod);	//close the output file

	AfxMessageBox("Marking Data extraction is done");

}