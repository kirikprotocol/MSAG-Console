/* ************************************************************************** *
 * gsmSCF parameters.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_GSMSCF_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_GSMSCF_HPP

#include <stdio.h>

#include "util/TonNpiAddress.hpp"

namespace smsc {
namespace inman {

using smsc::util::TonNpiAddress;

//max number of chars in string representing gsmSCF parameters
const unsigned GSM_SCF_StringLength = sizeof("%s:{%u}") + TonNpiAddress::_strSZ + sizeof(uint32_t)*3;

typedef smsc::core::buffers::FixedLengthString<GSM_SCF_StringLength> GsmSCFString;

struct GsmSCFinfo {
    static const unsigned  _strSZ = GSM_SCF_StringLength;

    uint32_t      serviceKey;   //4 bytes long
    TonNpiAddress scfAddress;   //gsmSCF address always has ISDN international format

    GsmSCFinfo() : serviceKey(0)
    { }
    GsmSCFinfo(const TonNpiAddress & use_scf, uint32_t use_key)
        : serviceKey(use_key), scfAddress(use_scf)
    { }

    bool empty(void) const { return scfAddress.empty(); }

    void Reset(void) { serviceKey = 0; scfAddress.clear(); }

    int toString(char* buf, bool omit_ton_npi = true, unsigned buflen = GsmSCFinfo::_strSZ) const
    {
        int     n = scfAddress.toString(buf, !omit_ton_npi);
        if (n)
            n += snprintf(buf + n, buflen-1-n, ":{%u}", serviceKey);
        buf[n] = 0;
        return n;
    }

    GsmSCFString toString(bool omit_ton_npi = true) const
    {
        GsmSCFString buf;
        if (!scfAddress.length)
          buf = "<none>";
        else
          toString(buf.str, omit_ton_npi);
        return buf;
    }
};

}//inman
}//smsc
#endif /* __SMSC_INMAN_GSMSCF_HPP */

