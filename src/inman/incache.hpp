#ident "$Id$"
#ifndef SMSC_INMAN_CACHE_HPP
#define SMSC_INMAN_CACHE_HPP

#include <time.h>

namespace smsc {
namespace inman {
namespace cache {

typedef const char * AbonentId;

typedef enum { btUnknown = 0, btPostpaid, btPrepaid } AbonentBillType;

class InAbonentQueryListenerITF {
public:
    virtual void abonentQueryCB(AbonentId ab_number, AbonentBillType ab_type) = 0;
};

class InAbonentQueryManagerITF {
public:
    //Returns true if query for fiven abonent is already initiated
    virtual bool isQuering(AbonentId ab_number) = 0;
    //Returns true if query succesfully started, false otherwise
    virtual bool startQuery(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
    //Returns zero if query cancelled and finished,
    //positive errcode if query just cancelled, negative - if query is unknown
    virtual int  cancelQuery(AbonentId ab_number, bool wait = false) = 0;
    //Returns zero if query finished, positive errcode if query execution timeout expired, 
    //neagtive errcode - if query is not started.
    virtual int  execQuery(AbonentId ab_number, AbonentBillType & result,
                                     unsigned short timeout_secs = 0) = 0;
};

class AbonentCacheITF {
public:
    virtual AbonentBillType getAbonentInfo(AbonentId ab_number) = 0;
    virtual void setAbonentInfo(AbonentId ab_number, AbonentBillType ab_type,
                                                            time_t expired = 0) = 0;
    //starts thread updating AbonentInfo and waits for its completion
    virtual AbonentBillType waitAbonentInfoUpdate(AbonentId ab_number,
                                                  unsigned short timeout_secs = 0);
    //starts thread updating AbonentInfo, that will call the callback upon completion
    virtual bool queryAbonentInfo(AbonentId ab_number, InAbonentQueryListenerITF * pf_cb) = 0;
};

} //cache
} //inman
} //smsc

#endif /* SMSC_INMAN_CACHE_HPP */

