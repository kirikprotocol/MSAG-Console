#ifndef __SMSC_INMAN_EMULATION__
#define __SMSC_INMAN_EMULATION__

#include "logger/Logger.h"
#include "ss7cp.h"
#include "et94inapcs1plusapi.h"

using smsc::logger::Logger;

static void debug(const char* msg)
{
    static Logger* logger = Logger::getInstance("smsc.inman.emulation");
    assert( logger );
    smsc_log_debug( logger, msg );
}

#define MsgInit MyMsgInit

USHORT_T MyMsgInit(USHORT_T maxEntries)
{
    debug("MsgInit()");
    return 0;
}

#define MsgOpen MyMsgOpen

USHORT_T MyMsgOpen(USHORT_T userID)
{
    debug("MsgOpen()");
    return 0;
}

#define MsgConn MyMsgConn

USHORT_T MyMsgConn(USHORT_T userId, USHORT_T inapId)
{
    debug("MsgConn()");
    return 0;
}

#define MsgRel  MyMsgRel

USHORT_T MyMsgRel(USHORT_T userId, USHORT_T inapId)
{
    debug("MsgRel()");
    return 0;
}

#define MsgClose MyMsgClose

USHORT_T MyMsgClose(USHORT_T userId)
{
    debug("MsgClose()");
    return 0;
}

#define MsgExit MyMsgExit

void MyMsgExit()
{
    debug("MsgExit()");
}

#define MsgRecv MyMsgRecv

USHORT_T  MsgRecv(MSG_T *msg)
{
    debug("MsgRecv()");
    usleep( 1000 * 1000 );
    return 0;
}

#define E94InapBindReq MyE94InapBindReq

USHORT_T  MyE94InapBindReq(UCHAR_T SSN, USHORT_T userId, BOOLEAN_T)
{
    debug("E94InapBindReq()");
    return 0;
}

#define E94InapHandleInd MyE94InapHandleInd

USHORT_T  MyE94InapHandleInd(MSG_T* msg)
{
    debug("E94InapHandleInd()");
    return 0;
}

#endif
