#ifndef _SCAG_UTIL_STORAGE_TESTLOGGER1_H
#define _SCAG_UTIL_STORAGE_TESTLOGGER1_H

#include <iostream>

/// a test of logger with template code
template < class T >
    class Testlogger
{
public:
    Testlogger() : log_(NULL), holder_("") {
        this->log_ = smsc::logger::Logger::getInstance("disk");
    }

    void prepare( const T& t ) {
        this->holder_ = t.getKey();
    }

    bool show( unsigned int i ) {
        std::cout << "show:" << holder_.c_str() << std::endl;
        smsc_log_debug( this->log_, "show: index=%d val=%s", i, this->holder_.c_str() );
        return true;
    }

private:
    smsc::logger::Logger* log_;
    mutable std::string holder_;
};

#endif /* _SCAG_UTIL_STORAGE_TESTLOGGER1_H */
