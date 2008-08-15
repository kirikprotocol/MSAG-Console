#ifndef _SCAG_UTIL_COMMANDOPERATIONS_H
#define _SCAG_UTIL_COMMANDOPERATIONS_H

namespace scag2 {
namespace re {

enum CommandOperations
{
    //SMS
    CO_DELIVER = 0,
    CO_SUBMIT = 1,
    CO_RECEIPT = 2,
    CO_DATA_SC_2_SME = 3,
    CO_DATA_SC_2_SC = 4,
    CO_DATA_SME_2_SME = 5,
    CO_DATA_SME_2_SC = 6,

    //USSD
    CO_USSD_DIALOG = 7,
    CO_PULL_USSD_DIALOG = 7,
    CO_PUSH_USSD_DIALOG = 8,

    //HTTP
    CO_HTTP_DELIVERY = 9

};

}
}

#endif /* !_SCAG_UTIL_COMMANDOPERATIONS_H */
