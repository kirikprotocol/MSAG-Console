static char const ident[] = "$Id$";

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"


using smsc::inman::comp::OperationFactory;

//add here declarations for FactoryInitFunctions
namespace smsc { namespace inman {

namespace comp {
OperationFactory * initCAP3SMSComponents(OperationFactory * fact);
} //comp

namespace usscomp {
OperationFactory * initMAPUSS2Components(OperationFactory * fact);
} //usscomp

} //inman 
} //smsc


namespace smsc {
namespace inman {
namespace comp {




/* ************************************************************************** *
 * class ApplicationContextFactory implementation:
 * -----------------------------------------------
 * add markFIF() calls to constructor for static initialization of component
 * factory for specified application context.
 * ************************************************************************** */
ApplicationContextFactory::ApplicationContextFactory()
{
//calling rule:
//   markFIF( my_id_ac_idx, FactoryInitFunc);
    markFIF(id_ac_cap3_sms_AC, smsc::inman::comp::initCAP3SMSComponents);
    markFIF(id_ac_map_networkUnstructuredSs_v2, smsc::inman::usscomp::initMAPUSS2Components);
}

ApplicationContextFactory* ApplicationContextFactory::getInstance()
{
    static ApplicationContextFactory	ACFact;
    return &ACFact;
}

void ApplicationContextFactory::markFIF(unsigned ac_idx, FactoryInitFunc fif)
{
    fifs.insert(FIFMap::value_type(ac_idx, fif));
}

FactoryInitFunc ApplicationContextFactory::findFIF(unsigned ac_idx)
{
    FIFMap::iterator it = fifs.find(ac_idx);
    if (it == fifs.end())
	return NULL;
    return (*it).second;
}


void ApplicationContextFactory::markFactory(unsigned acIdx, OperationFactory* fact)
{
    plants.insert(AppCtxFactMap::value_type(acIdx, fact));
}

void ApplicationContextFactory::unmarkFactory(unsigned acIdx)
{
    plants.erase(acIdx);
}

OperationFactory* ApplicationContextFactory::findFactory(unsigned ac_idx)
{
    AppCtxFactMap::iterator it = plants.find(ac_idx);
    if (it == plants.end())
	return NULL;
    return (*it).second;
}

OperationFactory* ApplicationContextFactory::getFactory(unsigned ac_idx)
{
    ApplicationContextFactory * acf = ApplicationContextFactory::getInstance();
    OperationFactory          * fact = acf->findFactory(ac_idx);

    if (!fact) { //factory is not initialized, check for registered FIF
        FactoryInitFunc fif = acf->findFIF(ac_idx);
        if (fif)
            fact = fif(NULL); //create factory
    }
    return fact;
}


/* ************************************************************************** *
 * class OperationFactory implementation:
 * ************************************************************************** */
OperationFactory::OperationFactory(unsigned ac_idx) : _ac_idx(ac_idx), logger(NULL)
{
    ApplicationContextFactory * acf = ApplicationContextFactory::getInstance();
    acf->markFactory(_ac_idx, this);

    FactoryInitFunc fif = acf->findFIF(_ac_idx);
    if (fif)
	fif(this);
}

OperationFactory::~OperationFactory()
{ 
    ApplicationContextFactory::getInstance()->unmarkFactory(_ac_idx);
}

void OperationFactory::setLogger(Logger * logInst)
{
    logger = logInst;
}

void OperationFactory::registerArg(unsigned opcode, CompProducer* alloc) 
{
    argPlant.registerProduct(opcode, alloc);
}

void OperationFactory::registerRes(unsigned opcode, CompProducer* alloc) 
{
    resPlant.registerProduct(opcode, alloc);
}

void OperationFactory::registerErr(unsigned opcode, CompProducer* alloc) 
{
    errPlant.registerProduct(opcode, alloc);
}


Component * OperationFactory::createArg(unsigned opcode)
{
    return argPlant.create(opcode); 
}

Component * OperationFactory::createRes(unsigned opcode)
{
    return resPlant.create(opcode);
}

Component * OperationFactory::createErr(unsigned opcode)
{
    return errPlant.create(opcode);
}

} //smsc
} //inman
} //comp

/* __SMSC_INMAN_COMP_OPERFACTORY__ */
