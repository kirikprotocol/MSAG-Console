#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_APPCTXDEFS__
#define __SMSC_INMAN_COMP_APPCTXDEFS__

#include "inman/common/types.hpp"

namespace smsc {
namespace ac {

struct ACOID {
    typedef enum {
        id_ac_NOT_AN_OID = 0  //zero element is reserved !!!
        // ...
        ,id_ac_cap3_sms_AC
        ,id_ac_map_networkUnstructuredSs_v2
        ,id_ac_map_anyTimeInfoHandling_v3
        // ...
        //insert additional OIDs here, before id_ac_RESERVED_MAX
        ,id_ac_RESERVED_MAX
    } DefinedOIDidx;

    //returns TRUE if OIDs are equal
    static bool equal(const APP_CONTEXT_T* ac1, const APP_CONTEXT_T* ac2);
    //returns OID in encoded form
    static const APP_CONTEXT_T * OIDbyIdx(DefinedOIDidx ac_idx);
    //returns index for given OID, id_ac_NOT_AN_OID - for not found
    static DefinedOIDidx Idx4OID(const APP_CONTEXT_T *oid);
};

} //ac
} //smsc

#endif /* __SMSC_INMAN_COMP_APPCTXDEFS__ */
