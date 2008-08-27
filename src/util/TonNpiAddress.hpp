#ident "@(#)$Id$"
/* ************************************************************************** *
 * TonNpiAddress and GsmSCFinfo helper classes.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_TONNPIADDR_HPP__
#define __SMSC_UTIL_TONNPIADDR_HPP__

#include <inttypes.h>

#include <string.h>
#include <stdio.h>

#include <string>

#define MAP_MAX_IMSI_AddressLength      8
#define MAP_MAX_IMSI_AddressValueLength (MAP_MAX_IMSI_AddressLength*2)
#define MAP_MAX_ISDN_AddressLength      8

#define CAP_MAX_SMS_AddressStringLength 10 //CAP-datatypes.maxSMS-AddressStringLength - 1
#define CAP_MAX_SMS_AddressValueLength  (CAP_MAX_SMS_AddressStringLength*2)

#define NUMBERING_ISDN                  1
#define ToN_INTERNATIONAL               1
#define ToN_ALPHANUM                    5
#define ToN_UNKNOWN                     0

namespace smsc {
namespace util {
                                 
//This is slightly more intelligent analog of sms::sms::Address,
//it also accepts alpha-numeric addresses.
//In opposite to sms::Address, all methods return error code instead
//of throwing exception.
struct TonNpiAddress {              // '.' + 3 + '.' + 3 + '.' + 20
    static const unsigned  _strSZ = (12 + CAP_MAX_SMS_AddressValueLength + 2);

    unsigned char   length, typeOfNumber, numPlanInd;
                     //signals is always zero-terminated
    char            signals[CAP_MAX_SMS_AddressValueLength + 1];

    inline void clear(void) { length = typeOfNumber = numPlanInd = signals[0] = 0; }
    inline const char * getSignals(void) const { return (const char*)&signals[0]; }
    inline bool empty(void) const { return (bool)!length; }

    TonNpiAddress() { clear(); }

    bool fromText(const char* text)
    {
        if (!text || !*text)
            return false;

        char    buff[CAP_MAX_SMS_AddressValueLength + 1];
        char *  addr_value = buff;
        int     iplan = 0, itype = 0;
        int     max_scan = 1, scanned = 0;

        memset(buff, 0, sizeof(buff));
        switch (text[0]) {
        case '.': {  //ton.npi.adr
            scanned = sscanf(text, ".%d.%d.%20s", &itype, &iplan, addr_value);
            max_scan = 3;
        } break;
        case '+': {  //isdn international adr
            scanned = sscanf(text, "+%20[0123456789]s", addr_value);
            iplan = itype = 1;
        } break;
        default:    //isdn unknown or alpha-numeric adr
            size_t txt_len = strlen(text);
            if (txt_len > 0xFF)
                return false;
            length = (unsigned char)txt_len;

            scanned = sscanf(text, "%20[0123456789]s", addr_value);
            if (scanned == 1 && (length == strlen(addr_value))) {
                iplan = 1; /*itype = 0;*/   // isdn unknown
            } else {
                if (length <= CAP_MAX_SMS_AddressValueLength) {
                    /*iplan = 0;*/ itype = 5;   //alpha-numeric adr
                    addr_value = (char*)text;
                    scanned = 1;
                }
            }
        }
        if ((scanned < max_scan) || (iplan > 0xFF) || (itype > 0xFF))
            return false;

        numPlanInd = (unsigned char)iplan;
        typeOfNumber = (unsigned char)itype;
        memcpy(signals, addr_value, length = (unsigned char)strlen((const char*)addr_value));
        signals[length] = 0;
        return true;
    }

    //checks if address is ISDN International or Unknown and sets it to International
    //returns false if address is not an ISDN International
    bool fixISDN(void)
    {
        if ((numPlanInd != NUMBERING_ISDN) || (typeOfNumber > ToN_INTERNATIONAL))
            return false;
        typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown
        return true;
    }

    //Returns true if address numbering is ISDN international
    inline bool interISDN(void) const
    {
        return (bool)((numPlanInd == typeOfNumber) && (numPlanInd == 0x01));
    }
                
    //use at least TonNpiAddress::_strSZ chars buffer
    int toString(char* buf, bool ton_npi = true, unsigned buflen = TonNpiAddress::_strSZ) const
    {
        int n = 0;
        if (length) {
            if (ton_npi)
                n = snprintf(buf, buflen - 1, ".%u.%u.", (unsigned)typeOfNumber,
                                                            (unsigned)numPlanInd);
            else if (interISDN())
                buf[n++] = '+';
            strcpy(buf + n, getSignals()); //inserts ending zero
            n += length;
        } else
            buf[0] = 0;
        return n;
    }

    inline std::string toString(bool ton_npi = true) const
    {
        char buf[TonNpiAddress::_strSZ];
        toString(buf, ton_npi);
        return buf;
    }

    inline bool operator== (const TonNpiAddress &adr2) const
    {
        if ((typeOfNumber == adr2.typeOfNumber)
            && (numPlanInd == adr2.numPlanInd)
            && (length == adr2.length)) {
            return !strcmp(signals, adr2.signals) ? true : false;
        }
        return false;
    }
};

struct GsmSCFinfo { //gsmSCF paramaters
    static const unsigned  _strSZ = sizeof("%s:{%u}") + TonNpiAddress::_strSZ + sizeof(uint32_t)*3;

    uint32_t      serviceKey;   //4 bytes long
    TonNpiAddress scfAddress;   //gsmSCF address always has ISDN international format

    GsmSCFinfo() : serviceKey(0)
    { }
    GsmSCFinfo(const TonNpiAddress & use_scf, uint32_t use_key)
        : scfAddress(use_scf), serviceKey(use_key)
    { }

    void Reset(void) { serviceKey = 0; scfAddress.clear(); }

    int toString(char* buf, bool omit_ton_npi = true, unsigned buflen = GsmSCFinfo::_strSZ) const
    {
        int     n = scfAddress.toString(buf, !omit_ton_npi);
        if (n)
            n += snprintf(buf + n, buflen-1-n, ":{%u}", serviceKey);
        buf[n] = 0;
        return n;
    }

    std::string toString(bool omit_ton_npi = true) const
    {
        if (!scfAddress.length)
            return "<none>";

        char    buf[GsmSCFinfo::_strSZ];
        toString(buf, omit_ton_npi);
        return buf;
    }
};

}//namespace util
}//namespace smsc
#endif /* __SMSC_UTIL_TONNPIADDR_HPP__ */

