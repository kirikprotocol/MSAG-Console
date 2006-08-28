#include <sys/types.h>
#include <sys/stat.h>
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

const int MAX_EVENTS = 50;
const int RECORDS_COUNT = 1000;
const int ADDRESS_LEN = 10;

//	������ ��������� "������� ��� ��������" 
struct event_cell_ver0
{
	uint32_t        date;
	uint8_t         id;
	uint8_t			calling_num[ADDRESS_LEN];
	uint8_t         reserved[5];
};

//	������ ��������� ����� ������� ��� ���������
struct dat_file_cell_ver0
{
	uint8_t         event_count;
	uint8_t			inaccessible_num[ADDRESS_LEN];
	uint8_t         reserved[5];
	event_cell_ver0 events[MAX_EVENTS];
};

//	����� ��������� "������� ��� ��������" 
struct event_cell_ver1
{
	uint64_t		date;
	uint8_t         id;
	uint8_t			calling_num[ADDRESS_LEN];
	uint8_t         reserved[5];
};

//	����� ��������� ����� ������� ��� ���������
struct dat_file_cell_ver1
{
	uint64_t		schedTime;
	uint8_t			event_count;
	uint16_t		last_error;
	uint8_t			inaccessible_num[ADDRESS_LEN];
	uint8_t         reserved[3];
	event_cell_ver1 events[MAX_EVENTS];
};

int convert(const char* src_path, const char* dst_path)
{
	int	src, dst;

	if(-1 == (src = open(src_path, O_RDONLY)))
	{
		perror("Error opening source file");
		return 1;
	}
	if(-1 == (dst = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
	{
		perror("Error opening destination file");
		close(src);
		return 1;
	}

	const uint32_t		src_buf_size = sizeof(dat_file_cell_ver0) * RECORDS_COUNT;
	const uint32_t		dst_buf_size = sizeof(dat_file_cell_ver1) * RECORDS_COUNT;
	uint8_t				src_buf[src_buf_size ];
	uint8_t				dst_buf[dst_buf_size ];
	dat_file_cell_ver0*	dat0;
	dat_file_cell_ver1*	dat1;
	uint32_t			rb, count;
	off_t				src_file_size = lseek(src, 0, SEEK_END);
	
	if(0 == lseek(src, 0, SEEK_END))
	{
		printf("Source file is empty\n");
		close(src);
		close(dst);
		return 0;
	}

	if( src_file_size != ((src_file_size/sizeof(dat_file_cell_ver0))*sizeof(dat_file_cell_ver0)))
	{
		printf("Source file corrupted\n");
		close(src);
		close(dst);
		return 1;
	}

	lseek(src, 0, SEEK_SET);
		
	while(rb = read(src, (void*)src_buf, src_buf_size))
	{
		count = rb / sizeof(dat_file_cell_ver0);
		for(int i = 0; i < count; i++)
		{
			dat0 = (dat_file_cell_ver0*)&src_buf[i*sizeof(dat_file_cell_ver0)];
			dat1 = (dat_file_cell_ver1*)&dst_buf[i*sizeof(dat_file_cell_ver1)];

			dat1->event_count = dat0->event_count;
			dat1->schedTime = -1;
			dat1->event_count = dat0->event_count;
			memcpy((void*)&(dat1->inaccessible_num), (void*)&(dat0->inaccessible_num), ADDRESS_LEN);

			for(int j = 0; j < MAX_EVENTS; j++)
			{
				dat1->events[j].date = dat0->events[j].date;
				dat1->events[j].id = dat0->events[j].id;
				memcpy((void*)&(dat1->events[j].calling_num), (void*)&(dat0->events[j].calling_num), ADDRESS_LEN);
			}
		}
		write(dst, dst_buf, dst_buf_size);
	}
	close(src);
	close(dst);
  return 1;
}


int main(int argc,char* argv[])
{
  if(argc != 3)
  {
    printf("%s converts old 32-bit format data_file to new 64-bit format\n", argv[0]);
	printf("Usage: %s source_file destinationfile\n", argv[0]);
    return -1;
  }
  convert(argv[1], argv[2]);
  return 1;
}