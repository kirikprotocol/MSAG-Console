#include <malloc.h>
#include <signal.h>
#include <thread.h>
#include <unistd.h>
#include "portss7.h"
#include "ss7tmc.h"
#include "ss7log.h"
#include "ss7msg.h"
#include "i97tcapapi.h"
#include "et96map_api_defines.h"
#include "et96map_api_types.h"
#include "et96map_dialogue.h"
#include "hlrtask.h"

#define MAXENTRIES 50
#define SSN 6
#define USER USER02_ID
#define PROVIDER TCAP_ID

int main(int argc, char **argv) {
  MSG_T message;
  USHORT_T result;
  thread_t threadId;
  
//  sigset( SIGINT, sighandler );
  mmsWaitDelimiter.dialogueId = 0;
  hlrResult.dialogueId = 0;
  
  //thr_create( NULL, 0, cmdThread, NULL, 0, &threadId );
  
  result = MsgInit(MAXENTRIES);
  
  if(result != 0) {
    printf("\nMsgInit Failed with code %d\n",result);
    exit(1);
  }
  printf( "MsgInit\n" );
  result = MsgOpen(USER);

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
  printf( "MsgConn TCAP\n" );

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
  
  printf( "EINSS7_I97TBindReq\n" );
  
  going = 1;
  message.receiver = USER;

  while(going) {
//    result = MsgRecv(&message);
    result = EINSS7CpMsgRecv_r( &message, 1000 );

    if( result == MSG_TIMEOUT ) {
      continue;
    }
    printf( "MsgRecv result=%d\n",result );

    if( result != MSG_OK ) going = 0;
    else {
        result = EINSS7_I97THandleInd(&message);
        printf( "MsgHandle result=%d\n",result );
        if(result != MSG_OK) going = 0;
    }
    
  }
  EINSS7_I97TUnBindReq(SSN, USER);  
  sleep(2);
  MsgRel(USER,PROVIDER);
  MsgClose(USER);
  MsgExit();
  return result;
}
