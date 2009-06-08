#ifndef __SCAG_RE_BASE_EVENTHANDLERTYPE_H
#define __SCAG_RE_BASE_EVENTHANDLERTYPE_H

namespace scag2 {
namespace re {

enum EventHandlerType
{
    EH_UNKNOWN = 0,
    EH_SUBMIT_SM = 1,
    EH_SUBMIT_SM_RESP = 2,
    EH_DELIVER_SM = 3,
    EH_DELIVER_SM_RESP = 4,
    // EH_RECEIPT = 5,

    EH_HTTP_REQUEST = 6,
    EH_HTTP_RESPONSE = 7,
    EH_HTTP_DELIVERY = 8,

    EH_DATA_SM = 9,
    EH_DATA_SM_RESP = 10,

    EH_SESSION_INIT = 11,
    EH_SESSION_DESTROY = 12
};

}
}

#endif /* !__SCAG_RE_BASE_EVENTHANDLERTYPE_H */
