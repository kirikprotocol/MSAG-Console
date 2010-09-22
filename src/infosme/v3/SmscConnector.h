#ifndef _INFOSME_V3_SMSCCONNECTOR_H
#define _INFOSME_V3_SMSCCONNECTOR_H

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"

namespace smsc {
namespace infosme {

class SmscSender;

class SmscConnector : public smsc::core::threads::ThreadedTask
{
public:
    SmscConnector( SmscSender& sender ) :
    log_(smsc::logger::Logger::getInstance("conn")),
    sender_(&sender) {}

    virtual const char* taskName() { return "conn"; }
    virtual int Execute();

private:
    smsc::logger::Logger* log_;
    SmscSender*           sender_;
};

} // infosme
} // smsc

#endif
