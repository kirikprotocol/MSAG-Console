#ifndef _SCAG_PVSS_BASE_SERIALIZEDCOMMANDRESPONSE_H
#define _SCAG_PVSS_BASE_SERIALIZEDCOMMANDRESPONSE_H

#include <memory>
#include "logger/Logger.h"
#include "CommandResponse.h"
#include "Protocol.h"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace pvss {

class ProfileResponseVisitor;

/// The dummy command which is attached to profile request when processed in pvss proxy.
class SerializedCommandResponse : public CommandResponse
{
public:
    SerializedCommandResponse(const Protocol::Buffer& buf) : size_(buf.GetPos()), data_(new char[buf.GetPos()]) {
        memcpy(data_.get(),buf.get(),size_);
    }

    virtual std::string toString() const {
        util::HexDump hd;
        util::HexDump::string_type rv;
        // std::string rv;
        rv.reserve(10+size_*3);
        hd.addstr(rv,typeToString());
        if ( data_.get() ) {
            hd.addstr(rv," ");
            hd.hexdump(rv,data_.get(),size_);
        }
        return hd.c_str(rv);
        // char buf[32];
        // sprintf( buf, "seqNum=%u %s", seqNum_, typeToString() );
        // return buf;
    }

    void setupBuffer( Protocol::Buffer& buf ) {
        buf.setExtBuf( getData(), getSize() );
        buf.SetPos(getSize()); // necessary for BufferWriter
    }

    /// check if the command is valid (except seqnum field)
    virtual bool isValid( PvssException* = 0 ) const { return true; }

    /// visitor pattern
    virtual bool visit( ProfileResponseVisitor& visitor ) { return false; }

    /// clone the command
    virtual SerializedCommandResponse* clone() const {
        return new SerializedCommandResponse(*this);
    }

    /// clear all fields except seqnum
    virtual void clear() { size_ = 0; }

    virtual const char* typeToString() const { return "serial_resp"; }

    inline char* getData() const { return data_.get(); }
    inline size_t getSize() const { return size_; }
    
protected:
    SerializedCommandResponse( const SerializedCommandResponse& cmd ) : size_(cmd.size_), data_(new char[size_]) {
        memcpy(data_.get(),cmd.data_.get(),size_);
    }

private:
    SerializedCommandResponse& operator = ( const SerializedCommandResponse& );

private:
    size_t              size_;
    std::auto_ptr<char> data_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SERIALIZEDPROFILECOMMAND_H */
