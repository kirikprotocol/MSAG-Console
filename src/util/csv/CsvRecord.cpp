#include "CsvRecord.h"

namespace {
const char* quote = "\"\"";
const char* backslash = "\\\\";
const char* lf = "\\n";
}

namespace smsc {
namespace util {
namespace csv {

void CsvRecord::addSimpleField( std::string& line, const char* str, size_t sz )
{
    line.reserve( line.size() + sz + 50 );
    if ( ptr_ != buf_ ) { line.push_back(delim_); }
    line.append(str,sz);
    ++ptr_;
}

void CsvRecord::addField( std::string& line, const char* str, size_t sz )
{
    line.reserve( line.size() + sz + 50 );
    if ( ptr_ != buf_ ) { line.push_back(delim_); }
    line.push_back('"');
    const char* eptr = str + sz;
    const char* ptr;
    char esc, c;
    for ( ptr = str; ptr < eptr; ++ptr ) {
        c = *ptr;
        if ( c == '"' ) {
            esc = '"';
        } else if ( !escapeChars_.empty() && escapeChars_.find(c) != std::string::npos ) {
            esc = '\\';
            switch (c) {
            case '\n' : c = 'n' ; break;
            case '\r' : c = 'r' ; break;
            case '\b' : c = 'b' ; break;
            case '\t' : c = 't' ; break;
            case '\0' : c = '0' ; break;
            };
        } else {
            continue;
        }
        if ( ptr != str ) { line.append(str,ptr); }
        line.push_back(esc); // copy escape seq
        line.push_back(c);   // copy escape seq
        str = ptr+1;         // skip this symbol
    }
    if ( str != ptr ) {
        line.append(str,ptr);
    }
    line.push_back('"');
    ++ptr_;
}


bool CsvRecord::isNextFieldSimple() const
{
    if ( ptr_ >= eptr_ ) return false;
    if ( ptr_ == buf_ ) {
        if ( *ptr_ != '"' ) return true;
    } else {
        const char* ptr = ptr_;
        ++ptr;
        if ( ptr >= eptr_ ) return true; // empty field at the end
        if ( *ptr != '"' ) return true;
    }
    return false;
}


const char* CsvRecord::parseSimpleField( size_t& fieldLen )
{
    moveToNextField();
    const char* ptr;
    for ( ptr = ptr_; ptr != eptr_; ++ptr ) {
        if ( *ptr == ',' ) break;
    }
    fieldLen = ptr - ptr_;
    const char* str = ptr_;
    ptr_ += fieldLen;
    return str;
}


void CsvRecord::parseField( std::string& line )
{
    moveToNextField();
    line.clear();
    if ( ptr_ >= eptr_ || *ptr_ == delim_ ) {
        // field is empty
        return;
    }
    if ( *ptr_ != '"' ) {
        throw CsvRecordException( "field at %u is not quoted", unsigned(ptr_-buf_) );
    }
    ++ptr_;
    const char* str = ptr_;
    const char* ptr;
    char c;
    for ( ptr = str; ptr != eptr_; ++ptr ) {
        c = *ptr;
        if ( c == '"' ) {
            if ( ptr != str ) line.append(str,ptr);
            ++ptr;
            if ( ptr == eptr_ ) {
                --ptr; // end of the field
                break;
            } else if ( *ptr == '"' ) {
                // escaped
                line.push_back('"');
                str = ptr+1; // skip this symbol
            } else if ( *ptr == delim_ ) {
                // end of the field
                --ptr;
                break;
            } else {
                // end of the field, but not delimited
                throw CsvRecordException("field ended with not delim %c",delim_);
            }
        } else if ( c == '\\' && !escapeChars_.empty() ) {
            if ( ptr != str ) line.append(str,ptr);
            ++ptr;
            if ( ptr == eptr_ ) {
                // broken field
                throw CsvRecordException("record is ended prematurely");
            }
            c = *ptr;
            switch (c) {
            case 'n' : c = '\n'; break;
            case 'r' : c = '\r'; break;
            case 'b' : c = '\b'; break;
            case 't' : c = '\t'; break;
            case '0' : c = '\0'; break;
            };
            line.push_back(c);
            str = ptr+1;
        }
    }
    if ( ptr == eptr_ || *ptr != '"' ) {
        throw CsvRecordException("record ended prematurely");
    }
    ptr_ = ++ptr; // skip ending quote
}


void CsvRecord::moveToNextField()
{
    if ( ptr_ >= eptr_ ) throw CsvRecordException( "field at %u is at end of record",
                                                   unsigned(ptr_-buf_));
    if ( ptr_ != buf_ ) {
        if ( *ptr_ != delim_ ) {
            throw CsvRecordException("delim at %u is not %c",unsigned(ptr_-buf_),delim_);
        }
        ++ptr_;
    }
}

} // namespace csv
} // namespace util
} // namespace smsc
