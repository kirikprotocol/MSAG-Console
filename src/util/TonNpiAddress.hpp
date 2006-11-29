#ident "$Id$"
/* ************************************************************************** *
 * TonNpiAddress helper class.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_TONNPIADDR_HPP__
#define __SMSC_UTIL_TONNPIADDR_HPP__

#include <string.h>
#include <stdio.h>

#include <string>

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
struct TonNpiAddress {
    unsigned char   length, typeOfNumber, numPlanInd;
                     //signals is always zero-terminated
    char            signals[CAP_MAX_SMS_AddressValueLength + 1];

    inline void clear(void) { length = typeOfNumber = numPlanInd = signals[0] = 0; }
    inline const char * getSignals(void) const { return (const char*)&signals[0]; }

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
            length = strlen(text);
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
        memcpy(signals, addr_value, length = strlen((const char*)addr_value));
        signals[length] = 0;
        return true;
    }

    //use at least 30 chars buffer
    inline int toString(char* buf, int buflen) const
    {
        return snprintf(buf, buflen, ".%u.%u.%s",
                        (unsigned)typeOfNumber, (unsigned)numPlanInd, signals);
    }

    inline std::string toString() const
    {
        char buf[48+2]; // '.' + 3 + '.' + 3 + '.' + 20
        buf[0] = 0;
        snprintf(buf, sizeof(buf)-1, ".%u.%u.%s",
                        (unsigned)typeOfNumber, (unsigned)numPlanInd, signals);
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

}//namespace util
}//namespace smsc
#endif /* __SMSC_UTIL_TONNPIADDR_HPP__ */

