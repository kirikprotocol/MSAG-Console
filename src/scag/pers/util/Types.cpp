#include "Types.h"

namespace scag { namespace pers { namespace util {
namespace perstypes {

const char* persProfileType( ProfileType pt )
{
    switch (pt) {
#define PT(pt,pn) case (pt) : return pn
        PT(PT_UNKNOWN,"PT_UNKNOWN");
        PT(PT_ABONENT,"PT_ABONENT");
        PT(PT_OPERATOR,"PT_OPERATOR");
        PT(PT_PROVIDER,"PT_PROVIDER");
        PT(PT_SERVICE,"PT_SERVICE");
#undef PT
    default :
        return "???";
    }
}

const char* persCmdName( PersCmd c )
{
    switch (c) {
#define CT(ct,cn) case (ct) : return cn
        CT(PC_UNKNOWN,"pc_unk");
        CT(PC_DEL,"pc_del");
        CT(PC_SET,"pc_set");
        CT(PC_GET,"pc_get");
        CT(PC_INC,"pc_inc");
        CT(PC_INC_MOD,"pc_inc_mod");
        CT(PC_PING,"pc_ping");
        CT(PC_BATCH,"pc_batch");
        CT(PC_TRANSACT_BATCH,"pc_trans_batch");
        CT(PC_INC_RESULT,"pc_inc_res");
        CT(PC_MTBATCH,"pc_mtbatch");
#undef CT
    default : return "???";
    }
}

}
}
}
}
