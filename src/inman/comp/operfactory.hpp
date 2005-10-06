#ident "$Id$"
/*
USAGE EXAMPLE: see inman/tstfactr.cpp

1) add to acdefs.hpp application context index
2) implement the function type of FactoryInitFunc, that registers components in factory,
   for ex.:

void initCAP4SMSComponents(OperationFactory * fact)
{
    fact->setLogger(Logger::getInstance("smsc.inman.comp.CAP4SMSFactory"));
    fact->registerArg(InapOpCode::RequestReportSMSEvent,
	new CompFactory::ProducerT<smsc::inman::comp::RequestReportSMSEventArg>() );
    // register other args ...
}

3)
  a) [dynamical instantiation]
   create factory instance by executing:

OperationFactoryInstanceT <id_ac_cap4_sms_AC>::getInstance()->Init(initCAP4SMSComponents);

  b) [static instantiation]
   add following line to ApplicationContextFactory constructor in operfactory.cpp

   markFIF(id_ac_cap4_sms_AC, initCAP4SMSComponents);
   
   create factory instance by executing:

OperationFactoryInstanceT <id_ac_cap4_sms_AC>::getInstance();

4) now you may get components by calling
 either:
    OperationFactoryInstanceT <id_ac_cap4_sms_AC>::getInstance()->createArg(opcode);
 or:
    ApplicationContextFactory::getFactory(id_ac_cap4_sms_AC)->createArg(opcode);
*/


#ifndef __SMSC_INMAN_COMP_OPERFACTORY__
#define __SMSC_INMAN_COMP_OPERFACTORY__

#include <map>

#include "inman/comp/compdefs.hpp"
#include "inman/common/factory.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;
using smsc::inman::comp::Component;

namespace smsc {
namespace inman {
namespace comp {

typedef smsc::inman::common::FactoryT<unsigned, Component>  CompFactory; 
typedef smsc::inman::common::FactoryT<unsigned, Component>::Producer  CompProducer; 

class OperationFactory
{
public:
    //constructor registers factory in AppCtxFactory
    OperationFactory(unsigned ac_idx);
    virtual ~OperationFactory();

    Component * createArg(unsigned opcode);
    Component * createRes(unsigned opcode);
    Component * createErr(unsigned opcode);

    unsigned getACidx(void) { return _ac_idx; }
    //const char * getACoid() { return _AC_oid[_ac_idx]; }

    //calling rule: registerArg(opcode1, new CompFactory::ProducerT<ArgType1>());
    void registerArg(unsigned opcode, CompProducer* alloc);
    void registerRes(unsigned opcode, CompProducer* alloc);
    void registerErr(unsigned opcode, CompProducer* alloc);

    void setLogger(Logger * logInst);

protected:
    CompFactory argPlant; 
    CompFactory resPlant; 
    CompFactory errPlant; 

    unsigned		_ac_idx;
    Logger* 		logger;
};

typedef void (*FactoryInitFunc)(OperationFactory * fact);

template < unsigned ac_idx >
class OperationFactoryInstanceT : public OperationFactory
{
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


class ApplicationContextFactory
{
public:
    ApplicationContextFactory(); 
    static ApplicationContextFactory* getInstance();

    typedef std::map<unsigned, OperationFactory*> AppCtxFactMap;
    typedef std::map<unsigned, FactoryInitFunc>   FIFMap;

    void markFactory(unsigned ac_idx, OperationFactory* fact);
    void unmarkFactory(unsigned ac_idx);
    OperationFactory* findFactory(unsigned ac_idx);

    static OperationFactory* getFactory(unsigned ac_idx);

    void markFIF(unsigned ac_idx, FactoryInitFunc fif);
    FactoryInitFunc findFIF(unsigned ac_idx);

protected:
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



