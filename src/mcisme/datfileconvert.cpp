#include <sys/types.h>
#include <sys/stat.h>
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

const int MAX_EVENTS = 50;

//	старая структура "событие для абонента" 
struct event_cell_ver0
{
	uint32_t        date;
	uint8_t         id;
	uint8_t		calling_num[10];
	uint8_t         reserved[5];
};

//	старая структура файла событий для абонентов
struct dat_file_cell_ver0
{
	uint8_t         event_count;
	uint8_t		inaccessible_num[10];
	uint8_t         reserved[5];
	event_cell_ver0 events[MAX_EVENTS];
};

//	новая структура "событие для абонента" 
struct event_cell_ver1
{
	uint64_t         date;
	uint8_t         id;
	uint8_t		calling_num[10];
	uint8_t         reserved[5];
};

//	новая структура файла событий для абонентов
struct dat_file_cell_ver1
{
	uint64_t	schedTime;
	uint8_t         event_count;
	uint8_t		inaccessible_num[10];
	uint8_t         reserved[5];
	event_cell_ver1 events[MAX_EVENTS];
};

int convert(const char* src_path, const char* dst_path)
{
	int src;
	int dst;

	if(-1 == (src = open(src_path, O_RDONLY)))
	{
		perror("Error opening source file");
		return 1;
	}
	if(-1 == (dst = open(dst_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
	{
		perror("Error opening destination file");
		return 1;
	}
/*	uint32_t	rb, i, count; //, cell_num=0;
	off_t		size;
	dat_file_cell	AbntEvents;

	idx_file.SeekEnd(0);
	if(0 == (size = idx_file.Pos()))
	{
		smsc_log_debug(logger, "FSStorage: FSStorage: Nothing to load - idx_file is empty.\n");
		delete[] buf;
		return 0;
	}
	idx_file.Seek(0);
		
	while(rb = idx_file.Read((void*)buf, buf_size))
	{
//		if( (size - idx_file.Pos())/sizeof(idx_file_cell) < 1000)
		count = rb / sizeof(idx_file_cell);
		if( (count * sizeof(idx_file_cell)) != rb)
		{
			delete[] buf; 
			return 1;
		}
		for(i = 0; i < count; i++)
		{
			if(0 == memcmp((void*)&buf[i*sizeof(idx_file_cell)], (void*)zero_idx_cell, sizeof(idx_file_cell)))
			{
				freeCells.AddCell(cell_num);
			}
			else
			{
				AbntAddr abnt(&buf[i*sizeof(idx_file_cell)]);
				LoadAbntEvents(abnt, &AbntEvents);
				hashAbnt.Insert(abnt, cell_num);
				pDeliveryQueue->Schedule(abnt, false, AbntEvents.schedTime);
			}
			cell_num++;
		}
	}
*/
  return 1;
}


int main(int argc,char* argv[])
{
  if(argc==1)
  {
    printf("%s converts old 32-bit format data_file to new 64-bit format\n",argv[0]);
    return -1;
  }

  printf("00%%\n");
  for(int i=0; i <=100; i++)
  {
	sleep(1);
	printf("%c%c%.2d\n", 8,8, i);
  }


  return 1;
}