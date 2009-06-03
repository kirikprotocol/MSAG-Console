#include "CommandOperation.h"

namespace scag2 {
namespace transport {

const char* commandOpName( int op )
{
#define COMMANDOPNAME(x) case x : return #x
    switch (op) {
        COMMANDOPNAME(CO_NA);
        COMMANDOPNAME(CO_DELIVER);
        COMMANDOPNAME(CO_SUBMIT);
        // COMMANDOPNAME(CO_RECEIPT);
        COMMANDOPNAME(CO_DATA_SC_2_SME);
        COMMANDOPNAME(CO_DATA_SC_2_SC);
        COMMANDOPNAME(CO_DATA_SME_2_SME);
        COMMANDOPNAME(CO_DATA_SME_2_SC);
        COMMANDOPNAME(CO_USSD_DIALOG);
        // COMMANDOPNAME(CO_PULL_USSD_DIALOG);
        COMMANDOPNAME(CO_PUSH_USSD_DIALOG);
        COMMANDOPNAME(CO_HTTP_DELIVERY);
    default:
        return "???";
    }
#undef COMMANDOPNAME
}

}
}
