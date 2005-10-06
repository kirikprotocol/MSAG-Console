#include <stdlib.h>
#include <assert.h>
#include <typeinfo.h>

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"

#include "inman/comp/comps.hpp"
#include "inman/comp/compsutl.hpp"
#include "inman/comp/comfactory.hpp"

#include "inman/comp/usscomp.hpp"

using smsc::inman::comp::InapOpCode;
using smsc::inman::usscomp::MAPUSS_OpCode;
using smsc::inman::comp::ApplicationContextFactory;

using smsc::ac::CAP4SMSFactory;
using smsc::ac::MAPUSS2Factory;

                                                               
int main(int argc, char * argv[])
{
    int   method = 0;
    Component * comp = NULL, *res = NULL;
    unsigned    ac_id = id_ac_cap4_sms_AC;
    unsigned    opcode = InapOpCode::RequestReportSMSEvent;

    Logger::Init();
    Logger	*logger = Logger::getInstance("smsc.ac.factory");

    if (argc > 1)
	method = atoi(argv[1]);


    switch (method) {
    case 2 : {
	CAP4SMSFactory::getInstance();
        comp = ApplicationContextFactory::getFactory(ac_id)->createArg(opcode);
    } break;
    case 1 : { //dynamical initialization
	CAP4SMSFactory::Init(smsc::inman::comp::initCAP4SMSComponents);
	comp = CAP4SMSFactory::getInstance()->createArg(opcode);
    } break;
    //static initialization: FactoryInitFunction is called in ApplicationContextFactory()
    default: method = 0;    
    case 0 : {
	comp = CAP4SMSFactory::getInstance()->createArg(opcode);
    } break;
    }
    assert(comp);
    smsc_log_debug(logger, "method %d: arg %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*comp)).name(), ac_id, opcode);
    delete comp;


    ac_id = id_ac_map_networkUnstructuredSs_v2;
    opcode = MAPUSS_OpCode::processUSS_Request;

    switch (method) {
    case 2 : {
	MAPUSS2Factory::getInstance();
	comp = ApplicationContextFactory::getFactory(ac_id)->createArg(opcode);
	res = ApplicationContextFactory::getFactory(ac_id)->createRes(opcode);
    } break;
    case 1 : { //dynamical initialization
	MAPUSS2Factory::Init(smsc::inman::usscomp::initMAPUSS2Components);
	comp = MAPUSS2Factory::getInstance()->createArg(opcode);
	res = MAPUSS2Factory::getInstance()->createRes(opcode);
    } break;
    //static initialization: FactoryInitFunction is called in ApplicationContextFactory()
    default: method = 0;    
    case 0 : {
	comp = MAPUSS2Factory::getInstance()->createArg(opcode);
	res = MAPUSS2Factory::getInstance()->createRes(opcode);
    } break;
    }
    assert(comp);
    smsc_log_debug(logger, "method %d: arg %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*comp)).name(), ac_id, opcode);
    assert(res);
    smsc_log_debug(logger, "method %d: res %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*res)).name(), ac_id, opcode);

    delete comp; delete res;


    return 0;
}
