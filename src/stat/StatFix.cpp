#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#else
#  if _FILE_OFFSET_BITS!=64
#  error _FILE_OFFSET_BITS must be 64!!!
#  endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <unistd.h>

typedef int64_t offset_type;

offset_type FileSize(int fd)
{
#ifdef __GNUC__
    struct stat st;
#else
    struct ::stat st;
#endif
    fstat(fd, &st);
    return st.st_size;
}

int SetPos(int fd, offset_type off)
{
    if (lseek(fd, off, SEEK_SET) == -1) {
	printf("SetPos failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    return 0;
}
int ReadInt32(int fd, uint32_t& ret)
{
    uint32_t val=0;
    if (read(fd, &val, sizeof(val)) < sizeof(val))
    {
	printf("ReadInt32 failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    ret = ntohl(val);
    return 0;
}
int WriteInt32(int fd, uint32_t ret)
{
    uint32_t val=htonl(ret);
    if (write(fd, &val, sizeof(val)) < sizeof(val))
    {
	printf("WriteInt32 failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    return 0;
}
int ReadInt16(int fd, uint16_t& ret)
{
    uint16_t val=0;
    if (read(fd, &val, sizeof(val)) < sizeof(val))
    {
	printf("ReadInt16 failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    ret = ntohs(val);
    return 0;
}
int WriteInt16(int fd, uint16_t ret)
{
    uint16_t val=htons(ret);
    if (write(fd, &val, sizeof(val)) < sizeof(val))
    {
	printf("WriteInt16 failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    return 0;
}
int ReadBuff(int fd, char* buff, int sz)
{
    if (read(fd, buff, sz) < sz)
    {
	printf("ReadBuff failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    return 0;
}
int WriteBuff(int fd, char* buff, int sz)
{
    if (write(fd, buff, sz) < sz)
    {
	printf("WriteBuff failed. Cause=%s\n", strerror(errno));
	return errno;
    }
    return 0;
}

const int MAX_FILEREC_SIZE = 1024*1000; // 1Mb
char buff[MAX_FILEREC_SIZE+1];
char skip[MAX_FILEREC_SIZE+1];
int bytesSkipped = 0;

void dumpSkipped(bool showall)
{
    if (bytesSkipped >= MAX_FILEREC_SIZE || (showall && bytesSkipped)) {
	printf("Skipped %d bytes:\n", bytesSkipped);
	for (int i=0; i<bytesSkipped; i++) printf("%02X ", (uint8_t)skip[i]); printf("(");
	for (int i=0; i<bytesSkipped; i++) printf("%c", skip[i]); printf(")\n");
	bytesSkipped = 0;
    }
}

int parse(int fdR, int fdW)
{
    int status = 0; bytesSkipped = 0;
    uint16_t int16 = 0; uint32_t int32 = 0; offset_type offR = 0;

    if (status = SetPos(fdR, 0)) return status;
    if (status = SetPos(fdW, 0)) return status;

    // Processing header
    if (status = ReadBuff(fdR, buff, 9)) return status;
    if (status = ReadInt16(fdR, int16)) return status;

    /*if (strcmp(buff, "SMSC.STAT") || int16 != 1) {
	printf("File is not SMSC statistics file (1.0 version)\n");
	return -5;
    }*/

    if (status = WriteBuff(fdW, buff, 9)) return status;
    if (status = WriteInt16(fdW, int16)) return status;

    int recordsRestored = 0;
    uint32_t sz1, sz2; offR += 11;
    offset_type maxOffR = FileSize(fdR);
    printf("File size %lld\n", maxOffR);

    // Processing data
    while (offR+8 < maxOffR)
    {
	//printf("offR=%lld\n", offR);
	if (status = ReadInt32(fdR, sz1)) return status;
	if (!sz1 || sz1 < 1200 || sz1 > 18192 || sz1 >= maxOffR-offR || sz1 >= MAX_FILEREC_SIZE)
	{ // skip 1 byte
	    printf("sz1=%d isn't seems to be valid, off=%lld\n", sz1, offR);
	    dumpSkipped(false);
	    skip[bytesSkipped++] = (uint8_t)((htonl(sz1)>>24)&0x00000000000000ff);
	    if (status = SetPos(fdR, ++offR)) return status;
	    continue;
	}
	if (status = ReadBuff(fdR, buff, sz1)) return status;
	if (status = ReadInt32(fdR, sz2)) return status;
	if (sz1 != sz2) { // skip 1 byte
	    printf("sz1=%d, sz2=%d isn't valid, off=%lld\n", sz1, sz2, offR);
	    dumpSkipped(false);
	    skip[bytesSkipped++] = (uint8_t)((htonl(sz1)>>24)&0x00000000000000ff);
	    if (status = SetPos(fdR, ++offR)) return status;
	    continue;
	}
	dumpSkipped(true);
	if (status = WriteInt32(fdW, sz1)) return status;
	if (status = WriteBuff(fdW, buff, sz1)) return status;
	if (status = WriteInt32(fdW, sz2)) return status;
	offR += (8 + sz1); recordsRestored++;
	printf("Record restored, count=%d (size=%d)\n", recordsRestored, sz1);
    }

    dumpSkipped(true);
    return status;
}

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

int main(int argc, char* argv[])
{
    if (argc < 2) {
	printf("Usage: StatFix file1 (... fileN)\n");
	return -1;
    }

    char wfname[256];
    for (int fcount=1; fcount<argc; fcount++)
    {
	const char* fname = argv[fcount];
	int fdR = open(fname, O_RDONLY|O_LARGEFILE, 0644);
	if (fdR == -1) {
	    printf("Failed to open file: %s\n", fname);
	    continue;
	}

	sprintf(wfname, "%s.fix", fname);
	int fdW = open(wfname, O_WRONLY|O_LARGEFILE|O_CREAT, 0644);
	if (fdW == -1) {
	    close(fdR);
	    printf("Failed to create file: %s\n", wfname);
	    continue;
	}

	printf("Scanning file: %s\n", fname);
	int st = parse(fdR, fdW);
	printf("File %s scan %s (status=%d)\n", fname, (st == 0) ? "ok":"failed", st);
	close(fdR); close(fdW);
    }
    return 0;
}
