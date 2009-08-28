#ifndef UTIL_CSV_CSVRECORD_H
#define UTIL_CSV_CSVRECORD_H

#include <string>
#include "util/Exception.hpp"

namespace smsc {
namespace util {
namespace csv {

class CsvRecord
{
public:
    class CsvRecordException : public smsc::util::Exception {

    public:
        CsvRecordException( const char* fmt, ... ) {
            SMSC_UTIL_EX_FILL(fmt);
        }
    };

    // -- methods for csv record creation
    // NOTE: ptr_ will be used as a written field counter
    CsvRecord( const char* escapeChars = 0, char delim = ',' ) :
    buf_(0), eptr_(0), ptr_(0), delim_(delim) {
        if ( escapeChars ) {
            escapeChars_.reserve(10);
            escapeChars_ = escapeChars;
            if ( escapeChars_.find('\\') == std::string::npos ) escapeChars_.push_back('\\');
            escapeChars_.push_back('\0');
        }
    }

    /// add simple field to the line
    void addSimpleField( std::string& line, const char* str, size_t sz );

    /// NOTE: escaping \, \n and ", field is taken into quotes
    void addField( std::string& line, const char* str, size_t sz );


    // -- methods for csv record parsing

    CsvRecord( const char* buf, size_t bufsize,
               bool escapeChars = false, char delim = ',' ) :
    buf_(buf), eptr_(buf+bufsize), ptr_(buf), delim_(delim) {
        if (escapeChars) escapeChars_ = "\\\n\r\0";
    }

    /// if no more fields are in the input
    bool isEOL() const {
        return ( ptr_ >= eptr_ );
    }

    bool isNextFieldSimple() const;

    /// retrieve a field from the input (move iterator to the next field)
    /// return the starting position of the field and fill the length of the field
    const char* parseSimpleField( size_t& fieldLen ); // throw exception
    void parseSimpleField( std::string& field ) {
        field.clear();
        size_t n;
        const char* p = parseSimpleField(n);
        field.append(p,n);
    }
    void parseField( std::string& field );            // throw exception

private:
    void moveToNextField();

private:
    const char* buf_;
    const char* eptr_;
    const char* ptr_;
    std::string escapeChars_;
    char        delim_;
};

} // namespace csv
} // namespace util
} // namespace smsc

#endif /* !UTIL_CSV_CSVRECORD_H */
