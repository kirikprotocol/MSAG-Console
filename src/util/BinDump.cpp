#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <cstdio>
#include <string.h>
#include "util/BinDump.hpp"

namespace smsc {
namespace util {
/* ************************************************************************** *
 * Customizable HEX Dunmping.
 * ************************************************************************** */
//Hex octet Grouped by 4, Space delimited, all per Line
const HexDumpCFG _HexDump_HG4S = { 4, 0, " ", 0};
const HexDumpCFG _HexDump_CVSD = { 1, 0, " ", "0x"}; //C Value Space Delimited
const HexDumpCFG _HexDump_CVCS = { 1, 0, ",", "0x"}; //C Value Comma Separated

static const char _HexChars[] = "0123456789ABCDEF";

unsigned DumpHex(unsigned long length, const unsigned char * buf,
                StreamAppenderITF * usr_print,
                const HexDumpCFG & use_cfg/* = _HexDump_DFLT*/)
{
    HexDumpCFG      cfg;
    cfg.grpOcts = (unsigned)(!use_cfg.grpOcts ? length : 
                        ((use_cfg.grpOcts < length) ? use_cfg.grpOcts : length));
    cfg.lineOcts = (unsigned)(!use_cfg.lineOcts ? length : 
                        ((use_cfg.lineOcts < length) ? use_cfg.lineOcts : length));
    cfg.grpSpr   = use_cfg.grpSpr ? use_cfg.grpSpr : _HexDump_DFLT.grpSpr;
    cfg.octPrf   = use_cfg.octPrf ? use_cfg.octPrf : _HexDump_DFLT.octPrf;

    unsigned    byteId = 0, printed = 0, octet = cfg.lineOcts;

    while (byteId < length) {
        if (cfg.octPrf && cfg.octPrf[0])
            printed += usr_print->append(cfg.octPrf);
        char tmpbuf[3]; tmpbuf[2] = 0;
        tmpbuf[0] = _HexChars[(buf[byteId] >> 4) & 0x0F];
        tmpbuf[1] = _HexChars[buf[byteId] & 0x0F];
        printed += usr_print->append(tmpbuf);

        if (++byteId != length) { //there are more bytes to print
            if (!--octet) {
            /* Start new line if OCTETS_PER_LINE octets have already been printed */
                printed += usr_print->append("\n");
                octet = use_cfg.lineOcts;;
            } else if (!(byteId % cfg.grpOcts)) { /* separate octets groups */
                if (cfg.grpSpr[0])
                    printed += usr_print->append(cfg.grpSpr);
            }
        }
    }
    /* Finish last line with a newline character if neccessary */
    if (cfg.lineOcts < length)
        printed += usr_print->append("\n");
    return printed;
}

std::string DumpHex(unsigned long length, const unsigned char * buf,
                    const HexDumpCFG & use_cfg/* = _HexDump_DFLT*/)
{
    std::string    fstr;
    StdStrAppender  p_str(&fstr);
    DumpHex(length, buf, &p_str, use_cfg);
    p_str.release();
    return fstr;
}

std::string& DumpHex(std::string& fstr, unsigned long length, const unsigned char * buf,
                    const HexDumpCFG & use_cfg/* = _HexDump_DFLT*/)
{
    StdStrAppender  p_str(&fstr);
    DumpHex(length, buf, &p_str, use_cfg);
    p_str.release();
    return fstr;
}

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
#define OCTETS_PER_LINE         16
#define UNPRINTABLE_CHAR        '?'
#define PAD_SPACES(oct_remains)     (3*oct_remains + 2 + 1)

inline unsigned typeChar(const char & sym, StreamAppenderITF * &usr_print)
{
    char c = sym;
    if (c < ' ' || c > 0x7e)
        c = UNPRINTABLE_CHAR;
    char tmpbuf[10];
    int n = std::snprintf(tmpbuf, sizeof(tmpbuf)-1, "%c", c);
    tmpbuf[(n >= 0) ? n : 0] = 0; 
    return usr_print->append(tmpbuf);
}


unsigned DumpDbg(unsigned long length, const unsigned char * buf, StreamAppenderITF * usr_print)
{
    unsigned    octet = OCTETS_PER_LINE;
    unsigned    offset = 0, byteId = 0, printed = 0;

    if (!length)
        return 0;

    printed += usr_print->append("# 00000000:  ");
    while (byteId < length) {
        char tmpbuf[sizeof("# 00000000:  ") + 2];
        
        tmpbuf[0] = _HexChars[(buf[byteId] >> 4) & 0x0F];
        tmpbuf[1] = _HexChars[buf[byteId] & 0x0F];
        tmpbuf[2] = ' '; tmpbuf[3] = 0;
        printed += usr_print->append(tmpbuf);
        ++byteId;

        if (!--octet) { /* OCTETS_PER_LINE octets have already been        *
                         * printed, print them as chars and start new line */
            printed += usr_print->append(" ");
            int i = byteId - OCTETS_PER_LINE;
            for (; octet < OCTETS_PER_LINE; octet++, i++)
                printed += typeChar(buf[i], usr_print);
            
            if (byteId < length) { //start new line
                int n = std::snprintf(tmpbuf, sizeof(tmpbuf)-1, "\n# %08x:  ",
                         offset += OCTETS_PER_LINE);
                tmpbuf[(n >= 0) ? n : 0] = 0;
                printed += usr_print->append(tmpbuf);
            }
            octet = OCTETS_PER_LINE;
        } else if (!(octet % 8) && (byteId < length))
            printed += usr_print->append("| ");
    }
    if (octet != OCTETS_PER_LINE) { //pad last line with spaces
        char    tmpbuf[PAD_SPACES(OCTETS_PER_LINE) + 6];
        int     i = PAD_SPACES(octet);
        memset(tmpbuf, ' ', i); tmpbuf[i] = 0;
        printed += usr_print->append(tmpbuf);

        i = byteId - OCTETS_PER_LINE + octet;
        for (; octet < OCTETS_PER_LINE; octet++, i++)
            printed += typeChar(buf[i], usr_print);
    }
    printed += usr_print->append("\n");
    return printed;
}

std::string DumpDbg(unsigned long length, const unsigned char * buf)
{
    std::string    fstr;
    StdStrAppender  p_str(&fstr);
    DumpDbg(length, buf, &p_str);
    p_str.release();
    return fstr;
}

std::string& DumpDbg(std::string& fstr, unsigned long length, const unsigned char * buf)
{
    StdStrAppender  p_str(&fstr);
    DumpDbg(length, buf, &p_str);
    p_str.release();
    return fstr;
}

} //namespace util
}//namespace smsc


