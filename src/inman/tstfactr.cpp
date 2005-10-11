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

using smsc::ac::CAP3SMSFactory;
using smsc::ac::MAPUSS2Factory;


#define MAX_METHOD 3

int main(int argc, char * argv[])
{
    int   method = 0, singlePass = 0;
    Component * comp = NULL, *res = NULL;
    unsigned    ac_id = id_ac_cap3_sms_AC;
    unsigned    opcode = InapOpCode::RequestReportSMSEvent;

    Logger::Init();
    Logger	*logger = Logger::getInstance("smsc.ac.factory");

    if (argc > 1) {
	if ((method = atoi(argv[1])) > MAX_METHOD)
            method = 0;
        singlePass = 1;
    }


    do {
        switch (method) {
        case 3 : {
            comp = CAP3SMSFactory::getInstance()->createArg(opcode);
        } break;
        case 2 : {
	    CAP3SMSFactory::getInstance();
            comp = ApplicationContextFactory::getFactory(ac_id)->createArg(opcode);
        } break;
        case 1 : { //dynamical initialization
	    CAP3SMSFactory::Init(smsc::inman::comp::initCAP3SMSComponents);
	    comp = CAP3SMSFactory::getInstance()->createArg(opcode);
        } break;
        //static initialization: FactoryInitFunction is called in ApplicationContextFactory()
        default: method = 0;    
        case 0 : {
            comp = ApplicationContextFactory::getFactory(ac_id)->createArg(opcode);
        } break;
        } //eosw
        assert(comp);
        smsc_log_debug(logger, "method %d: arg %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*comp)).name(), ac_id, opcode);
        delete comp;
    } while ( !singlePass && (++method <= MAX_METHOD) );


    ac_id = id_ac_map_networkUnstructuredSs_v2;
    opcode = MAPUSS_OpCode::processUSS_Request;

    do {
        switch (method) {
        case 3 : {
            comp = MAPUSS2Factory::getInstance()->createArg(opcode);
            res = MAPUSS2Factory::getInstance()->createRes(opcode);
        } break;
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
            comp = ApplicationContextFactory::getFactory(ac_id)->createArg(opcode);
            res = ApplicationContextFactory::getFactory(ac_id)->createRes(opcode);
        } break;
        }
        assert(comp);
        smsc_log_debug(logger, "method %d: arg %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*comp)).name(), ac_id, opcode);
        assert(res);
        smsc_log_debug(logger, "method %d: res %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*res)).name(), ac_id, opcode);

        delete comp; delete res;
    } while ( !singlePass && (++method <= MAX_METHOD) );


    return 0;
}
