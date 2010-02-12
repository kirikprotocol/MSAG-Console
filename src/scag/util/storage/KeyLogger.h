#ifndef _SCAG_UTIL_STORAGE_KEYLOGGER_H
#define _SCAG_UTIL_STORAGE_KEYLOGGER_H

namespace scag2 {
namespace util {
namespace storage {

class KeyLogger
{
public:
    virtual ~KeyLogger() {}
    virtual const char* toString() const = 0;
};


class DummyKeyLogger : public KeyLogger
{
public:
    virtual const char* toString() const { return "dummy"; }
};


template < class T > class KeyLoggerT : public KeyLogger
{
public:
    KeyLoggerT() : key_(0), done_(false) {}
    inline void setKey( const T& k ) { key_ = &k; done_ = false; }
    virtual const char* toString() const {
        if (!done_) {
            if (key_) { s_ = key_->toString(); done_ = true; }
            else s_.clear();
        }
        return s_.c_str();
    }
private:
    const T*            key_;
    mutable std::string s_;
    mutable bool        done_;
};

}
}
}

#endif
