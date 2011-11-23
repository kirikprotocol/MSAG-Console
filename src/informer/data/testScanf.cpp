#include <stdio.h>
#include "informer/io/Typedefs.h"
#include "informer/io/InfosmeException.h"
#include "core/synchronization/MutexReportContentionRealization.h"

using namespace eyeline::informer;

void readRecord( const char* line, size_t fpos )
{
    int tm_sec;
    char cstate;
    unsigned regId;
    ulonglong msgId;
    unsigned retryCount;
    unsigned planTime;
    char caddr[40];
    int timeLeft;
    int smppStatus;
    char hexflags[200];
    char userdata[1040];
    int pos = 0;
    sscanf(line,"%02u,%c,%n",
           // "%[+.0-9],%d,%d,%[0-9a-f],%[^,],%n",
           &tm_sec, &cstate, &pos );
    if ( pos == 0 ) {
        throw FileReadException("",0,fpos,"cannot read start of '%s'\n",line);
    }
    const char* ptr = line + pos;
    if (cstate == 'B') {
        // B-record
        sscanf(ptr,",%llu,,,,,,,%n",&msgId,&pos);
        if (pos == 0) {
            throw FileReadException("",0,fpos,"cannot read the b-record in '%s'\n",line);
        }
        printf("b-record for msg=%llu\n",msgId);
        return;
    }

    sscanf(ptr,"%u,%llu,%u,%u,%n",
           &regId, &msgId, &retryCount, &planTime, &pos );
    if ( pos == 0 ) {
        throw FileReadException("",0,fpos,"cannot read middle of '%s'\n",line);
    }
    ptr += pos;

    if ( *ptr == ',' ) {
        // empty caddr
        ++ptr;
        caddr[0] = '\0';
    } else {
        sscanf(ptr,"%30[+.0-9],%n",caddr,&pos);
        if ( pos == 0 ) {
            throw FileReadException("",0,fpos,"cannot read addr in '%s'\n",line);
        }
        ptr += pos;
    }
    sscanf(ptr,"%d,%d,%n",&timeLeft,&smppStatus,&pos);
    if ( pos == 0 ) {
        throw FileReadException("",0,fpos,"cannot read timeleft in '%s'\n",line);
    }
    ptr += pos;
    if ( *ptr == ',' ) {
        // empty hexflags
        hexflags[0] = '\0';
        ++ptr;
    } else {
        sscanf(ptr,"%100[0-9a-f],%n",hexflags,&pos);
        if ( pos == 0 ) {
            fprintf(stderr,"cannot read hexflags in '%s'\n",line);
        }
        ptr += pos;
    }
    if ( *ptr == ',' ) {
        // empty udata
        userdata[0] = '\0';
        ++ptr;
    } else {
        sscanf(ptr,"%1024[^,],%n",userdata,&pos);
        if ( pos == 0 ) {
            throw FileReadException("",0,fpos,"cannot read userdata in '%s'\n",line);
        }
        ptr += pos;
    }

    size_t left = strlen(ptr);
    if ( left < 2 || *ptr != '"' || *(ptr+left-1) != '"' ) {
        throw FileReadException("",0,fpos,"cannot read text in '%s'\n",line);
    }

    printf("line: '%s'\n",line);
    printf("sec   = %u\n",tm_sec);
    printf("state = %c\n",cstate);
    printf("regId = %u\n",regId);
    printf("msgId = %llu\n",msgId);
    printf("retry = %u\n",retryCount);
    printf("plan  = %u\n",planTime);
    printf("addr  = %s\n",caddr);
    printf("left  = %d\n",timeLeft);
    printf("smpp  = %d\n",smppStatus);
    printf("flags = %s\n",hexflags);
    printf("udata = %s\n",userdata);
}

int main()
{
    uint8_t state = MSGSTATE_INPUT;
    const int tm_sec = 34;
    const char cstate = msgStateToString(MsgState(state))[0];
    const regionid_type regId = 43;
    const msgid_type msgId = 12346;
    int retryCount = 1;
    unsigned planTime = 12;
    const char* caddr = "+79137654079";
    const timediff_type timeLeft = 345;
    const int smppStatus = 0;
    const char* hexflags = "";
    const char* userData = "user=12";
    const char* text = "hello, world";
    
    char line1[2048];
    sprintf(line1,"%02u,%c,%u,%llu,%u,%u,%s,%d,%d,%s,%s,\"%s\"",
            tm_sec, cstate, regId,
            msgId, retryCount, planTime,
            caddr,
            timeLeft, smppStatus,
            hexflags, userData, text );

    char line2[2048];
    sprintf(line2,"%02u,B,,%llu,,,,,,,",tm_sec,msgId);

    printf("%s\n",line1);
    printf("%s\n",line2);

    // int pos = 0;
    // sscanf(line1,"");
    // int pos = 0;
    // sscanf(line2,"");

    readRecord(line1,0);
    readRecord(line2,0);
    return 0;
}
