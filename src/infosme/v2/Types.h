#ifndef _SMSC_INFOSME2_TYPES_H
#define _SMSC_INFOSME2_TYPES_H

namespace smsc {
namespace infosme2 {

struct MessageState {
enum {
        NEW = 0,
        WAIT = 1,
        ENROUTE = 2,
        DELIVERED = 3,
        EXPIRED = 4,
        FAILED = 5,
        DELETED = 6,
        RETRY = 7
};
private:
    MessageState();
};

}
}

#endif /* _SMSC_INFOSME2_TYPES_H */
