#ifndef __XMAP_H_GREEN__
#define __XMAP_H_GREEN__

#define EINSS7_THREADSAFE 1

extern "C" {

#include <portss7.h>
#include <ss7tmc.h>
#include <ss7log.h>
#include <ss7msg.h>
#include <et96map_api_defines.h>
#include <et96map_api_types.h>

#include <et96map_dialogue.h>
#include <et96map_sms.h>
#include <et96map_ussd.h>
}

#include <logger/Logger.h>

#include <stdio.h>
#include <time.h>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <signal.h>
#include <vector>
#include <iostream>
#include <thread.h>
#include <unistd.h>

 

using namespace std;




using namespace smsc::logger;

//#include <stdio.h>
//#include <signal.h>
//#include <pthread.h>
//#include <unistd.h>
//#include <string.h>


#define MDF_TRANSMITT 1
#define MDF_RECIEVE   0
#define MDF_WAIT_FOR_DELETE 2
#define MDS_SEND_LAST  0xffee

/*struct MicroString
{
 unsigned len; 
 char bytes[256];
};

struct MOMessage_t
{
 int status;
 USHORT_T dialogID;
 UCHAR_T  cdata[256];
};
*/


#define USING_DELETION   
#define TEMPLATE_MSG MSG_T*

extern void _mkMapAddress( ET96MAP_ADDRESS_T *addr, const char *saddr, unsigned len);
extern void _mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, const ET96MAP_ADDRESS_T *saddr, ET96MAP_LOCAL_SSN_T ssn);

#endif