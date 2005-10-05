static char const ident[] = "$Id$";

#include "inman/comp/operfactory.hpp"

namespace smsc {
namespace inman {
namespace comp {


OperationFactory::OperationFactory(unsigned ac_idx) : _ac_idx(ac_idx), logger(NULL)
{
    ApplicationContextFactory::getInstance()->markFactory(_ac_idx, this);
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

/* ************************************************************************** *
 *
 * ************************************************************************** */

ApplicationContextFactory* ApplicationContextFactory::getInstance()
{
    static ApplicationContextFactory	ACFact;
    return &ACFact;
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

OperationFactory* ApplicationContextFactory::operationFactory(unsigned ac_idx)
{
    ApplicationContextFactory * acf = ApplicationContextFactory::getInstance();
    return acf->findFactory(ac_idx);
}

} //smsc
} //inman
} //comp

/* __SMSC_INMAN_COMP_OPERFACTORY__ */
