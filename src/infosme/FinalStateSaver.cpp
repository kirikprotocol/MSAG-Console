#include <time.h>
#include <vector>

#include "TaskTypes.hpp"
#include "FinalStateSaver.h"
#include "util/vformat.hpp"

namespace {
const std::string workingExtension(".workcsv");
const std::string rollingExtension(".csv");
}

namespace smsc {
namespace infosme {

using namespace smsc::core::synchronization;

FinalStateSaver::FinalStateSaver( const std::string& path,
                                  unsigned           rollingInterval ) :
log_(smsc::logger::Logger::getInstance("fs2csv")),
path_(path),
rollingInterval_(rollingInterval),
nextOpen_(time(0))
{
    if ( path_.empty() ) { path_ = "./"; }
    else if ( path_[path_.size()-1] != '/' ) path_ += '/';
    smsc_log_debug(log_,"started at %s",path_.c_str());
    rollOrphans();
}


FinalStateSaver::~FinalStateSaver()
{
    checkRoll(time(0)+rollingInterval_); // making sure that file is rolled
}


void FinalStateSaver::save( time_t          now,
                            const TaskInfo& info,
                            const Message&  msg,
                            uint8_t         state,
                            int             smppStatus,
                            bool            noMoreMessages )
{
    smsc_log_debug(log_,"save task=%u msg=%llu",info.uid,msg.id);
    // prepare the buffer to save
    char timestr[30];
    struct tm tnow;
    localtime_r(&now,&tnow);
    sprintf(timestr,"%04u%02u%02u%02u%02u%02u",
            tnow.tm_year + 1900,
            tnow.tm_mon + 1,
            tnow.tm_mday,
            tnow.tm_hour,
            tnow.tm_min,
            tnow.tm_sec );
    std::string buf;
    smsc::util::format(buf,"%s,%u,%u,%llu,%u,%s,%s,%s\n",timestr,state,info.uid,msg.id,
                       smppStatus,
                       msg.abonent.c_str(),
                       msg.userData.c_str(),
                       info.name.c_str());

    core::synchronization::MutexGuard mg(mtx_);
    checkRollUnsync(now);
    if ( ! file_.isOpened() ) {
        std::string fname;
        fname.reserve(path_.size() + ::strlen(timestr) + ::workingExtension.size() + 1);
        fname.append(path_);
        fname.append(timestr);
        fname.append(::workingExtension);
        file_.WOpen( fname.c_str() );
        nextOpen_ = now + rollingInterval_;
        const char* head = "TIME,STATE,TASKID,MSGID,SMPPSTATUS,ADDRESS,USERDATA,TASKNAME\n";
        file_.Write(head,::strlen(head));
    }
    file_.Write(buf.c_str(),buf.size());
    if ( noMoreMessages ) {
        buf.clear();
        smsc::util::format(buf,"%s,%u,%u,%llu,%u,%s,%s,%s\n",
                           timestr,0,info.uid,0ULL,
                           0,"","",info.name.c_str());
        file_.Write(buf.c_str(),buf.size());
    }
}


void FinalStateSaver::checkRoll( time_t now )
{
    if ( now < nextOpen_ ) return;
    MutexGuard mg(mtx_);
    checkRollUnsync(now);
}


void FinalStateSaver::checkRollUnsync( time_t now )
{
    if ( now < nextOpen_ ) return;
    nextOpen_ = now + rollingInterval_;
    if ( ! file_.isOpened() ) return;
    file_.Close();
    rollFile( file_.getFileName() );
}


void FinalStateSaver::rollOrphans()
{
    std::vector< std::string > entries;
    try {
        smsc::core::buffers::File::ReadDir(path_.c_str(),entries);
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"exc: %s",e.what());
        return;
    }

    // renaming
    for ( std::vector< std::string >::const_iterator i = entries.begin();
          i != entries.end();
          ++i ) {
        if ( i->size() > ::workingExtension.size() &&
             0 == strcmp(i->c_str() + i->size() - ::workingExtension.size(),::workingExtension.c_str()) ) {
            rollFile( path_ + *i );
        }
    }
}


void FinalStateSaver::rollFile( const std::string& fname )
{
    smsc_log_info(log_,"rolling file %s", fname.c_str());
    std::string newFile;
    newFile.reserve(fname.size() - ::workingExtension.size() + ::rollingExtension.size() + 10 );
    newFile.append(fname.c_str(),fname.size()-::workingExtension.size());
    newFile.append(::rollingExtension);
    ::rename( fname.c_str(), newFile.c_str() );
}

} // namespace infosme
} // namespace smsc
