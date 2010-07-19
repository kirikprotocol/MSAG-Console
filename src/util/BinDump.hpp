/* ************************************************************************** *
 * Helpers: Customizable HEX Dunmping and Debugger-like tracing.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_BINDUMP_HPP__
#define __SMSC_UTIL_BINDUMP_HPP__

#ifndef __GNUC__
#ident "$Id$"
#endif

#include <memory>
#include "util/vformat.hpp"

namespace smsc {
namespace util {

class StreamAppenderITF {
public:
    virtual unsigned append(const char* str) = 0;
};

class StreamFAppenderITF {
public:
    virtual unsigned fappend(const char* fmt, ...) = 0;
};

/* ************************************************************************** *
 * Customizable HEX Dunmping.
 * ************************************************************************** */
typedef struct {
    unsigned    grpOcts;   //bytes per group, 0 - means group all octets,
                           //defaut: 4
    unsigned    lineOcts;  //bytes per line, 0 - print all octets per line,
                           //default: 0
                           //NOTE: if value not less than input buffer length,
                           //no newline char is printed!
    const char * grpSpr;   //symbol(s) to separate octet groups, default: ' '
    const char * octPrf;   //octet prefix string, "" by default
} HexDumpCFG;

extern const HexDumpCFG _HexDump_CVSD; //C Value Space Delimited
extern const HexDumpCFG _HexDump_CVCS; //C Value Comma Separated
extern const HexDumpCFG _HexDump_HG4S; //Hex octet Grouped by 4, Space delimited, all per Line
#define _HexDump_DFLT   _HexDump_HG4S

extern unsigned DumpHex(unsigned long length, const unsigned char * buf,
                        StreamAppenderITF * usr_print,
                        const HexDumpCFG & use_cfg = _HexDump_DFLT);
extern std::string DumpHex(unsigned long length, const unsigned char * buf,
                        const HexDumpCFG & use_cfg = _HexDump_DFLT);
extern std::string& DumpHex(std::string& fstr, unsigned long length, const unsigned char * buf,
                        const HexDumpCFG & use_cfg = _HexDump_DFLT);

/* ************************************************************************** *
 * Debugger-like trace output: 16 bytes per line, 8 bytes grouped.
 * Offset, HEX and ASCII representations are printed. If symbol is not a
 * printable ASCII symbol then '?' is printed instead.
 * Output example:
  # 00000000:  3C 3F 78 6D 6C 20 76 65 ¦ 72 73 69 6F 6E 3D 22 31  <?xml version="1
  # 00000010:  2E 30 22 20 65 6E 63 6F ¦ 64 69 6E 67 3D 22 55 54  .0" encoding="UT
  # 00000020:  46 2D 38 22 3F 3E 3C 54 ¦ 31 3E 3C 6D 61 72 6B 3E  F-8"?><T1><mark>
  # 00000030:  74 72 75 65 3C 6D 61 72 ¦ 6B 3E 3C 66 31 3E 35 3C  true<mark><f1>5<
  # 00000040:  2F 66 31 3E 3C 74 79 70 ¦ 65 3E 4E 55 4C 4C 3C 2F  /f1><type>NULL</
  # 00000050:  74 79 70 65 3E 3C 2F 54 ¦ 31 3E 20 20 0D 0A        type></T1>  ??
 * ************************************************************************** */
extern unsigned DumpDbg(unsigned long length, const unsigned char * buf, StreamAppenderITF * usr_print);
extern std::string DumpDbg(unsigned long length, const unsigned char * buf);
extern std::string& DumpDbg(std::string& fstr, unsigned long length, const unsigned char * buf);


/* ************************************************************************** *
 * StreamAppenderITF for std::string
 * ************************************************************************** */
class StdStrAppender : public std::auto_ptr<std::string>, public StreamAppenderITF {
public:
    StdStrAppender()                        { reset(new std::string()); }
    StdStrAppender(std::string * use_str)   { reset(use_str); }
    virtual ~StdStrAppender() { }

    unsigned append(const char* add_str)
    {
        std::string * p_str = get();
        size_t n_old = p_str->length();
        (*p_str) += add_str;
        return (unsigned)(p_str->length() - n_old);
    }
};

class StdStrFAppender : public StdStrAppender, public StreamFAppenderITF {
public:
    StdStrFAppender() : StdStrAppender() { }
    StdStrFAppender(std::string * use_str) : StdStrAppender(use_str) { }
    virtual ~StdStrFAppender() { }

    unsigned fappend(const char *fmt, ...)
    {
        std::string * p_str = get();
        size_t n_old = p_str->length();
        va_list arg_list;
        va_start(arg_list, fmt);
        vformat(*p_str, fmt, arg_list);
        va_end(arg_list);
        return (unsigned)(p_str->length() - n_old);
    }
};
}//namespace util
}//namespace smsc
#endif /* __SMSC_UTIL_BINDUMP_HPP__ */

