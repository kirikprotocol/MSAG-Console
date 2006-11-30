#ident "$Id$"
/*
NOTE: ApplicationContextFactory is not a threadsafe one!!!

USAGE EXAMPLE: see inman/tstfactr.cpp

1) add to acdefs.hpp application context index, for example:

  #define id_ac_sample_v1   	4U
2)  
  add to acdefs.cpp::_OIDS[] encoded form of new AC OID

3) implement the function type of FactoryInitFunc, that registers components in factory,
   by following template:
   
//following operations defined in id_ac_sample_v1
struct SampleAC_OpCode {
    enum {
	processRequest = 99
    };
};

//errors defined for  ProcessRequest OPERATION
struct ERR_ProcessRequest {
    enum {
        callBarred, dataMissing, systemFailure,
        unexpectedDataValue, unknownAlphabet         
    };
}

OperationFactory * initSampleAComponents(OperationFactory * fact)
{
    if (!fact) {
        fact = smsc::inman::comp::OperationFactoryInstanceT<id_ac_sample_v1>::getInstance();
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.comp.SampleACFactory"));

        //Register OPERATIONs: Argument type, Result type, Errors
        fact->registerArg(SampleAC_OpCode::processRequest,
	        new CompFactory::ProducerT<ProcessRequestArg>() );
        fact->registerRes(SampleAC_OpCode::processRequest,
	        new CompFactory::ProducerT<ProcessRequestRes>() );
        fact->bindErrors(SampleAC_OpCode::processRequest, 5, 
                         ERR_ProcessRequest::callBarred,
                         ERR_ProcessRequest::dataMissing,
                         ERR_ProcessRequest::systemFailure,
                         ERR_ProcessRequest::unexpectedDataValue,
                         ERR_ProcessRequest::unknownAlphabet
                         );
        //...
    }
    return fact;
}    

4) The operation factory instance may be created either by executing:

OperationFactoryInstanceT <id_ac_sample_v1>::getInstance()->Init(initSampleAComponents);
    or by
ApplicationContextFactory::Init(id_ac_sample_v1, initSampleAComponents);

5) The OPERATION components (arguments/results) may be created by calling
 either:
    OperationFactoryInstanceT <id_ac_cap4_sms_AC>::getInstance()->createArg(opcode);
    OperationFactoryInstanceT <id_ac_cap4_sms_AC>::getInstance()->createRes(opcode);
 or:
    ApplicationContextFactory::getFactory(id_ac_cap4_sms_AC)->createArg(opcode);
    ApplicationContextFactory::getFactory(id_ac_cap4_sms_AC)->createRes(opcode);
*/


#ifndef __SMSC_INMAN_COMP_OPERFACTORY__
#define __SMSC_INMAN_COMP_OPERFACTORY__

#include "util/Factory.hpp"
#include "inman/comp/compdefs.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;
using smsc::inman::comp::Component;

namespace smsc {
namespace inman {
namespace comp {

typedef smsc::util::FactoryT<unsigned, Component>           CompFactory;
typedef smsc::util::FactoryT<unsigned, Component>::Producer CompProducer;

class OperationFactory {
public:
    //constructor registers factory in AppCtxFactory
    OperationFactory(unsigned ac_idx);
    virtual ~OperationFactory();

    Component * createArg(unsigned opcode);
    Component * createRes(unsigned opcode);
    Component * createErr(unsigned errcode);

    //returns TRUE if OPERATION has RESULT defined
    bool hasResult(unsigned opcode);
    //returns TRUE if ERROR identified by errcode is defined for OPERATION
    bool hasError(unsigned opcode, unsigned errcode);
    //returns number of ERRORS defined for OPERATION
    unsigned hasErrors(unsigned opcode);

    unsigned getACidx(void) { return _ac_idx; }

    //calling rule: registerArg(opcode1, new CompFactory::ProducerT<ArgType1>());
    void registerArg(unsigned opcode, CompProducer* alloc);
    void registerRes(unsigned opcode, CompProducer* alloc);
    void registerErr(unsigned errcode, CompProducer* alloc);
    //binds error codes to OPERATION
    void OperationFactory::bindErrors(unsigned opcode, unsigned errNum, ...);
    void setLogger(Logger * logInst);

protected:
    typedef std::vector<unsigned> OPERATION_ERRORS;
    typedef std::map<unsigned, OPERATION_ERRORS*> OPER_ERRORS_MAP;

    CompFactory     argPlant; 
    CompFactory     resPlant; 
    CompFactory     errPlant; 
    OPER_ERRORS_MAP errMap;

    unsigned        _ac_idx;
    Logger*         logger;
};

typedef OperationFactory * (*FactoryInitFunc)(OperationFactory * fact);

template < unsigned ac_idx >
class OperationFactoryInstanceT : public OperationFactory {
public:
    // this function is for dynamical factory initialization
    static void Init(FactoryInitFunc fif)
    {
        if (fif) {
            fif(getInstance());
            ApplicationContextFactory::getInstance()->markFIF(ac_idx, fif);
        }
    }

    OperationFactoryInstanceT() : OperationFactory(ac_idx) { }

    static OperationFactoryInstanceT * getInstance()
    {
        static OperationFactoryInstanceT instance;
        return &instance;
    }

protected:
// forbid destruction and instantiation
    void* operator new(size_t);
    ~OperationFactoryInstanceT() {}
};


class ApplicationContextFactory {
public:
    ApplicationContextFactory(); 
    static ApplicationContextFactory* getInstance();

    //creates, initializes and registers the Operation factory
    static OperationFactory* Init(unsigned ac_idx, FactoryInitFunc fif);
    //returns the operation factory for given AC, the factory should be
    //initialized by preceeding call to ApplicationContextFactory::Init()
    static OperationFactory* getFactory(unsigned ac_idx);

    //searches for the FactoryInitializationFunction for given AC
    FactoryInitFunc findFIF(unsigned ac_idx);

protected:
    friend class OperationFactory;
    //registers FactoryInitializationFunction in ApplicationContextFactory
    void markFIF(unsigned ac_idx, FactoryInitFunc fif);
    //registers OperationFactory in ApplicationContextFactory
    void markFactory(unsigned ac_idx, OperationFactory* fact);
    void unmarkFactory(unsigned ac_idx);

private:
    typedef std::map<unsigned, OperationFactory*> AppCtxFactMap;
    typedef std::map<unsigned, FactoryInitFunc>   FIFMap;
    //searches the instantiated operation factories for given AC
    OperationFactory* findFactory(unsigned ac_idx);

    AppCtxFactMap    plants;
    FIFMap	     fifs;
    Logger*	     acLogger;

// forbid destruction and instantiation
    void* operator new(size_t);    
    ~ApplicationContextFactory() {}
};


} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_COMP_OPERFACTORY__ */



