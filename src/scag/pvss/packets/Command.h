#ifndef _SCAG_PVSS_BASE_COMMAND_H
#define _SCAG_PVSS_BASE_COMMAND_H

#include <string>
#include "util/int.h"

namespace scag2 {
namespace pvss {

class Command
{
public:
    virtual ~Command() {}
    virtual uint32_t getSeqNum() const = 0;
    virtual void setSeqNum( uint32_t seqNum ) = 0;

    /// return true if the command content is valid.
    virtual bool isValid() const = 0;

    /// dump command content.
    /// NOTE: this method should work even if isValid() == false
    virtual std::string toString() const = 0;

    /// clear all fields except seqnum
    virtual void clear() = 0;

protected:
    virtual const char* typeToString() const = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_COMMAND_H */
