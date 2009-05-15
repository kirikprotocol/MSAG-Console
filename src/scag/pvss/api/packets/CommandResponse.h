#ifndef _SCAG_PVSS_BASE_COMMANDRESPONSE_H
#define _SCAG_PVSS_BASE_COMMANDRESPONSE_H

#include <string>
#include "util/int.h"
#include "scag/pvss/common/StatusType.h"
#include "ProfileResponseVisitor.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {

class CommandResponse : public StatusType
{
protected:
    static smsc::logger::Logger* log_;
    
protected:
    CommandResponse() : status_(UNKNOWN) {}
    CommandResponse( uint8_t status ) : status_(status) {}

public:
    virtual ~CommandResponse() {}

    /// return true if the command content is valid.
    virtual bool isValid( PvssException* = 0 ) const { return true; }

    /// dump command content.
    /// NOTE: this method should work even if isValid() == false
    virtual std::string toString() const {
        char buf[48];
        snprintf(buf,sizeof(buf), "%s status=%s", typeToString(), statusToString(status_) );
        return buf;
    }

    /// clear all fields
    virtual void clear() { status_ = UNKNOWN; }
    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  = 0;
    virtual CommandResponse* clone() const = 0;

    inline bool isRequest() const { return false; }
    inline uint8_t getStatus() const { return status_; }
    inline void setStatus( uint8_t status ) { status_ = status; }

    // for serialization
    inline uint8_t getStatusValue() const { return getStatus(); }
    inline void setStatusValue( uint8_t status ) { setStatus(status); }

protected:
    virtual const char* typeToString() const = 0;
    void initLog();
    void logDtor();

private:
    uint8_t status_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_COMMANDRESPONSE_H */
