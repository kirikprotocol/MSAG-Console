/* ************************************************************************* *
 * 
 * ************************************************************************* */
#ifndef __INMAN_INAP_TCCALLBACKS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_INAP_TCCALLBACKS_HPP

/* function to be called when a connection is broken */
extern "C" unsigned short
  onEINSS7CpConnectBroken(unsigned short from_usrID,
                         unsigned short to_usrID, unsigned char inst_id);

#include "core/synchronization/Mutex.hpp"
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "logger/Logger.h"
using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace inap {

class TCAPDispatcher;

class TCCbkLink {
private:
    static Mutex        _TCCbkSync;
    TCAPDispatcher *    _disp;
    Logger *            logger;

protected:
    //forbid destruction and instantiation
    void* operator new(size_t);
    TCCbkLink();
    ~TCCbkLink()
    { }

public:
    static TCCbkLink & get(void)
    {
        MutexGuard grd(_TCCbkSync);
        static TCCbkLink instance;
        return instance;
    }
    void Init(TCAPDispatcher * use_disp, Logger * use_log)
    {
        MutexGuard grd(_TCCbkSync);
        _disp = use_disp;
        if (use_log)
            logger = use_log;
    }
    void DeInit(void)
    {
        MutexGuard grd(_TCCbkSync);
        _disp = NULL;
        logger = NULL;
    }

    TCAPDispatcher * tcapDisp(void)
    {
        MutexGuard grd(_TCCbkSync);
        return _disp;
    }
    Logger * tcapLogger(void)
    {
        MutexGuard grd(_TCCbkSync);
        return logger;
    }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_INAP_TCCALLBACKS_HPP */

