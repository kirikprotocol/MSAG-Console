static char const ident[] = "$Id$";
/* ************************************************************************* *
 * APPICATION CONTEXTs utility functions, encoded forms.
 * ************************************************************************* */

#include "inman/comp/acdefs.hpp"

namespace smsc {
namespace ac {

static APP_CONTEXT_T const  _OIDS[] = {
     {0, {0}}
//id-ac-cap3-sms-AC:
//    { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) umts-network(1)
//      cap3OE(21) ac(3) 61 }
    ,{7, {0x04, 0x00, 0x00, 0x01, 0x15, 0x03, 0x3D}}
//id_ac_map_networkUnstructuredSs_version2:
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) networkUnstructuredSs(19) version2(2) }
    ,{7, {0x04, 0x00, 0x00, 0x01, 0x00, 0x13, 0x02}}
//id_ac_map_anyTimeInfoHandling_v3:
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) anyTimeInfoHandling(43) version3(3) }
    ,{7, {0x04, 0x00, 0x00, 0x01, 0x00, 0x2B, 0x03}}
};
#define MAX_AC_IDX  (sizeof(_OIDS)/sizeof(APP_CONTEXT_T))

/* ************************************************************************** *
 * class ACOID implementation:
 * ************************************************************************** */

const APP_CONTEXT_T * ACOID::OIDbyIdx(DefinedOIDidx ac_idx)
{
    return (ac_idx && (ac_idx < MAX_AC_IDX)) ? &_OIDS[ac_idx] : 0;
}

bool ACOID::equal(const APP_CONTEXT_T* ac1, const APP_CONTEXT_T* ac2)
{
    if (ac1->acLen && (ac1->acLen == ac2->acLen)) {
        int i = ac1->acLen - 1;
        while ((i >= 0) && (ac1->ac[i] == ac2->ac[i]))
            i--;
        return (i >= 0) ? false : true;
    }
    return false;
}


ACOID::DefinedOIDidx ACOID::Idx4OID(const APP_CONTEXT_T *oid)
{
    for (unsigned i = 1; i < MAX_AC_IDX; i++) {
        if ((oid == &_OIDS[i]) || equal(oid, &_OIDS[i])) {
            return (i < id_ac_RESERVED_MAX) ?
                static_cast<ACOID::DefinedOIDidx>(i) : id_ac_NOT_AN_OID;
        }
    }
    return id_ac_NOT_AN_OID;
}

} //ac
} //smsc
