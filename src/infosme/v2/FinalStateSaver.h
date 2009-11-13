#ifndef _INFOSME_FINALSTATESAVER_H
#define _INFOSME_FINALSTATESAVER_H

#include <time.h>
#include <string>
#include "util/int.h"
#include "core/buffers/File.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace smsc {
namespace infosme {

struct TaskInfo;

class FinalStateSaver
{
public:
    FinalStateSaver( const std::string& path,
                     unsigned rollingInterval = 10 /* in seconds */ );

    ~FinalStateSaver();

    // add a new final state for a message, check for rolling
    void save( time_t          now,
               const TaskInfo& info,
               const Message&  msg,
               uint8_t         state,
               int             smppStatus,
               bool            noMoreMessages );

    // check if the file should be rolled (and roll if needed)
    void checkRoll( time_t now );

private:
    void checkRollUnsync( time_t now );
    void rollOrphans();
    void rollFile( const std::string& fname );

private:
    smsc::logger::Logger*              log_;
    std::string                        path_;
    unsigned                           rollingInterval_;
    time_t                             nextOpen_;
    smsc::core::buffers::File          file_;
    smsc::core::synchronization::Mutex mtx_;
};

} // namespace infosme
} // namespace smsc

#endif /* !_INFOSME_FINALSTATESAVER_H */
