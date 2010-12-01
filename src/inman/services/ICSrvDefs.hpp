/* ************************************************************************** *
 * INMan configurable services facility interfaces and classes definition. 
 * ************************************************************************** */
#ifndef __INMAN_CONFIGURABLE_SERVICES_DEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_CONFIGURABLE_SERVICES_DEFS_HPP__

#include "inman/services/ICSrvIDs.hpp"

#include "logger/Logger.h"
using smsc::logger::Logger;

namespace smsc  {
namespace inman {

//Inman Configurable Services Host interface
class ICServicesHostITF;

//Inman Configurable Service core.
class ICServiceAC {
public:
    enum State { icsStIdle = 0
        , icsStConfig   //service is configured
        , icsStInited   //service is initialized
        , icsStStarted  //service is started
    };

    enum RCode { icsRcOk = 0      //
        , icsRcInsuff = 1   //service is not configured/inited completely,
                            //further initialization attempts possible
        , icsRcError = -1   //fatal error preventing service to proceed 
    };

protected:
    const ICSUId                _icsUid;
    State                       _icsState;
    ICSIdsSet                   _icsDeps;  //ICServices this one depends on
    const ICServicesHostITF *   _icsHost;
    Logger *                    logger;


    RCode   checkDeps(State tgt_state);

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    virtual Mutex & _icsSync(void) const = 0;
    //Initializes service verifying that all dependent services are inited
    virtual RCode _icsInit(void) = 0;
    //Starts service verifying that all dependent services are started
    virtual RCode _icsStart(void) = 0;
    //Stops service
    virtual void  _icsStop(bool do_wait = false) = 0;

public:
    ICServiceAC(ICSUId use_uid,
                const ICServicesHostITF * use_host, Logger * use_log = NULL)
        : _icsUid(use_uid), _icsState(icsStIdle), _icsHost(use_host)
        , logger(use_log ? use_log : Logger::getInstance("smsc.inman"))
    { }
    virtual ~ICServiceAC()
    { //NOTE: it's recommended that successor's destructor
      //calls ICSStop(true);
    }

    ICSUId icsUId(void) const { return _icsUid; }
    State  icsState(void) const { return _icsState; }
    const ICSIdsSet & icsDeps(void) const { return _icsDeps; }

    //Initializes service verifying that all dependent services are inited
    RCode ICSInit(void);
    //Starts service verifying that all dependent services are started
    RCode ICSStart(void);
    //Stops service
    void  ICSStop(bool do_wait = false);

    virtual void * Interface(void) const = 0;
};
typedef ICServiceAC::State  ICSState;


template <class _CfgTArg>
class ICServiceAC_T : public ICServiceAC {
public:
    ICServiceAC_T(ICSUId use_uid, const ICServicesHostITF * svc_host,
                  std::auto_ptr<_CfgTArg> & use_cfg, Logger * use_log = NULL)
        : ICServiceAC(use_uid, svc_host, use_log)
    { }
    virtual ~ICServiceAC_T()
    { }
};

//Inman Configurable Services Host interface
class ICServicesHostITF {
protected:
    virtual ~ICServicesHostITF() //forbid interface destruction
    { }

public:
    virtual ICServiceAC * getICService(ICSUId srv_id) const = 0;
    virtual void *   getInterface(ICSUId srv_id) const = 0;
    virtual ICSState getICSrvState(ICSUId srv_id) const = 0;
};


//Generic ICService producer.
class ICSProducerAC {
protected:
    const ICSUId icsId;
    ICSIdsSet    icsDeps;

    explicit ICSProducerAC(ICSUId ics_uid) : icsId(ics_uid)
    { }

public:
    virtual ~ICSProducerAC()
    { }

    ICSUId icsUId(void) const { return icsId; }

    //Returns service dependencies
    virtual const ICSIdsSet *  Deps(void)
    {
        return icsDeps.empty() ? NULL : &icsDeps;
    }
    //Returns true if service supports configuration by XML file
    virtual bool isConfigurable(void) const = 0;
    //Creates service basing on Config parsed by ICSrvCfgReaderAC
    virtual ICServiceAC *  newService(ICServicesHostITF * svc_host,
                                      Logger * use_log = NULL) = 0;
};

//Generic ICService loading function must have following declaration:
//
// extern ICSProducerAC * ICSLoader_func(void);
//
typedef ICSProducerAC * (*ICSLoader_fp)(void);

} //inman
} //smsc
#endif /* __INMAN_CONFIGURABLE_SERVICES_DEFS_HPP__ */

