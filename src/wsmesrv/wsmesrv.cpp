#include <malloc.h>
#include <signal.h>
#include <thread.h>
#include <unistd.h>
#include "portss7.h"
#include "ss7tmc.h"
#include "ss7log.h"
#include "ss7msg.h"
#include "i97tcapapi.h"

#define MAXENTRIES 50
#define SSN 6
#define USER USER02_ID
#define PROVIDER TCAP_ID

#define RETURN_VALUE MSG_OK

int going=0;

void Deinit()
{
  printf( "Exiting program\n" );
  going = 0;
  EINSS7_I97TUnBindReq(SSN,USER);
  sleep(2);
  MsgRel(USER,PROVIDER);
  MsgClose(USER);
  MsgExit();
  exit(0);
}

static void sighandler( int signal ) {
  Deinit();
}

void Dump(char* name,UCHAR_T* buf,int len)
{
  fprintf(stderr,"%s, len=%d, packet:",name,len);
  for(int i=0;i<len;i++)
  {
    fprintf(stderr," %02X",buf[i]);
  }
  fprintf(stderr,"\n");
}

USHORT_T EINSS7_I97TBeginInd(   UCHAR_T ssn,
                                USHORT_T userId,
                                USHORT_T dialogueId,
                                UCHAR_T priOrder,
                                UCHAR_T qualityOfService,
                                UCHAR_T destAdrLength,
                                UCHAR_T *destAdr_p,
                                UCHAR_T orgAdrLength,
                                UCHAR_T *orgAdr_p,
                                UCHAR_T compPresent,
                                UCHAR_T appContextLength,
                                UCHAR_T *appContext_p,
                                USHORT_T userInfoLength,
                                UCHAR_T *userInfo_p)
{
  fprintf(stderr,"BeginInd ===>>>\n");
  fprintf(stderr,"ssn=%d, userId=%d, dId=%d\n",
    (int)ssn,(int)userId,(int)dialogueId);
  Dump("ac",appContext_p,appContextLength);
  Dump("ui",userInfo_p,userInfoLength);
  Dump("orgAddr",orgAdr_p,orgAdrLength);
  Dump("destAddr",destAdr_p,destAdrLength);

  fprintf(stderr,"BeginInd <<<===\n");
  return RETURN_VALUE;
}


USHORT_T EINSS7_I97TInvokeInd(  UCHAR_T ssn,
                                USHORT_T userId,
                                USHORT_T dialogueId,
                                UCHAR_T invokeId,
                                UCHAR_T lastComponent,
                                UCHAR_T linkedIdUsed,
                                UCHAR_T linkedId,
                                UCHAR_T operationTag,
                                USHORT_T operationLength,
                                UCHAR_T *operationCode_p,
                                USHORT_T paramLength,
                                UCHAR_T *parameters_p)
{
  fprintf(stderr,"InvokeInd ===>>>\n");

  fprintf(stderr,"ssn=%d, userId=%d, dId=%d, invokeId=%d\n",
    (int)ssn,(int)userId,(int)dialogueId,(int)invokeId);

  Dump("opcode",operationCode_p,operationLength);
  Dump("param",parameters_p,paramLength);

  fprintf(stderr,"InvokeInd <<<===\n");
  return RETURN_VALUE;
}

int main(int argc, char **argv)
{
  MSG_T message;
  USHORT_T result;
  thread_t threadId;

  sigset( SIGINT, sighandler );

  //thr_create( NULL, 0, cmdThread, NULL, 0, &threadId );

  result = MsgInit(MAXENTRIES);

  if(result != 0) {
    printf("\nMsgInit Failed with code %d\n",result);
    exit(1);
  }
  printf( "MsgInit\n");  result = MsgOpen(USER);

  if(result != 0) {
    printf("\nMsgOpen failed with code %d\n",result);
    exit(2);
  }
  printf( "MsgOpen\n" );

  result = MsgConn(USER,PROVIDER);

  if(result != 0) {
    printf("\nMsgConn TCAP failed with code %d\n",result);
    exit(3);
  }
  printf( "MsgConn TCAP\n");
/*  result = MsgConn(USER,USER);
  if(result != 0) {
    printf("\nMsgConn to myself failed with code %d\n",result);
    exit(3);
  }*/

  result = EINSS7_I97TBindReq(SSN,USER, EINSS7_I97TCAP_WHITE_USER);

  if (result != 0) {
    printf("Bind failed! with code %d\n",result);
    exit(3);
  }

  printf( "EINSS7_I97TBindReq\n");
  going = 1;
  message.receiver = USER;

  while(going)
  {
//    result = MsgRecv(&message);
    result = EINSS7CpMsgRecv_r( &message, 1000 );

    if(result==MSG_TIMEOUT)continue;

    printf( "MsgRecv result=%d\n",result);
    if( result != MSG_OK ) going = 0;
    else {
        result = EINSS7_I97THandleInd(&message);
        EINSS7CpReleaseMsgBuffer(&message);
        printf( "MsgHandle result=%d\n",result);        if(result != MSG_OK) going = 0;
    }

  }
  Deinit();
  return result;
}
