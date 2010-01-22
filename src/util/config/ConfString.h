#ifndef _SMSC_INFOSME2_CONFSTRING_H
#define _SMSC_INFOSME2_CONFSTRING_H

namespace smsc {
namespace util {
namespace config {

// the only use of the class is to wrap char* strings returned from ConfigView.
class ConfString
{
public:
    ConfString() : v_() {}
    ConfString( char* v ) {
        if ( !v ) { v_ = ""; return; }
        static const char* whitespaces = " \t\v\r\n";
        v_ = v; delete [] v;
        size_t b = v_.find_first_not_of(whitespaces);
        if ( b != std::string::npos ) {
            v_.erase(v_.begin(),v_.begin()+b);
        }
        b = v_.find_last_not_of(whitespaces);
        if ( b != std::string::npos ) {
            ++b;
            v_.erase(v_.begin()+b,v_.end());
        }
    }
    ~ConfString() {}

    const std::string& str() const { return v_; }
    const char* c_str() const { return v_.c_str(); }
    
private:
    ConfString( const ConfString& x );
    ConfString& operator = ( const ConfString& x );
private:
    std::string v_;
};

}
}
}

#endif
