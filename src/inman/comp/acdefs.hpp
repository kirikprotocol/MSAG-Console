#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_APPCTXDEFS__
#define __SMSC_INMAN_COMP_APPCTXDEFS__

typedef unsigned char      UCHAR_T;
#include "inman/common/types.hpp"
#include "inman/comp/operfactory.hpp"

using smsc::inman::comp::OperationFactoryInstanceT;

namespace smsc {
namespace ac {

//NOTE: zero index is reserved for internal usage !!!
#define id_ac_cap3_sms_AC			1U
#define id_ac_map_networkUnstructuredSs_v2	2U

typedef OperationFactoryInstanceT <id_ac_cap3_sms_AC> 
    CAP3SMSFactory;
typedef OperationFactoryInstanceT <id_ac_map_networkUnstructuredSs_v2>
    MAPUSS2Factory;


struct ACOID {
    //returns TRUE if OIDs are equal
    static int equal(const APP_CONTEXT_T* ac1, const APP_CONTEXT_T* ac2);
    //returns OID in encoded form
    static const APP_CONTEXT_T * OIDbyIdx(unsigned ac_idx);
    //returns index for given OID, zero - for not found
    static unsigned Idx4OID(const APP_CONTEXT_T *);
};

} //ac
} //smsc

#endif /* __SMSC_INMAN_COMP_APPCTXDEFS__ */
