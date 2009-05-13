#ifndef _SCAG_PVSS_BASE_SERIALIZEDPROFILECOMMAND_H
#define _SCAG_PVSS_BASE_SERIALIZEDPROFILECOMMAND_H

#include <memory>
#include "logger/Logger.h"
#include "ProfileCommand.h"
#include "Protocol.h"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace pvss {

class ProfileCommandVisitor;

/// The dummy command which is attached to profile request when processed in pvss proxy.
class SerializedProfileCommand : public ProfileCommand
{
public:
    SerializedProfileCommand(const Protocol::Buffer& buf) : size_(buf.GetPos()), data_(new char[buf.GetPos()]) {
        memcpy(data_.get(),buf.get(),size_);
    }

    virtual std::string toString() const {
        std::string rv;
        rv.reserve(10+size_*3);
        rv.append(typeToString());
        if ( data_.get() ) {
            rv.push_back(' ');
            util::HexDump hd;
            hd.hexdump(rv,data_.get(),size_);
        }
        return rv;
        // char buf[32];
        // sprintf( buf, "seqNum=%u %s", seqNum_, typeToString() );
        // return buf;
    }

    void setupBuffer( Protocol::Buffer& buf ) {
        buf.setExtBuf( getData(), getSize() );
        buf.SetPos(getSize()); // necessary for BufferWriter
    }

    /// check if the command is valid (except seqnum field)
    virtual bool isValid() const { return true; }

    /// visitor pattern
    virtual bool visit( ProfileCommandVisitor& visitor ) { return false; }

    /// clone the command
    virtual SerializedProfileCommand* clone() const {
        return new SerializedProfileCommand(*this);
    }

    /// clear all fields except seqnum
    virtual void clear() { size_ = 0; }

    virtual const char* typeToString() const { return "serial"; }

    inline char* getData() const { return data_.get(); }
    inline size_t getSize() const { return size_; }
    
protected:
    SerializedProfileCommand( const SerializedProfileCommand& cmd ) : size_(cmd.size_), data_(new char[size_]) {
        memcpy(data_.get(),cmd.data_.get(),size_);
    }

private:
    ProfileCommand& operator = ( const ProfileCommand& );

private:
    size_t              size_;
    std::auto_ptr<char> data_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SERIALIZEDPROFILECOMMAND_H */
