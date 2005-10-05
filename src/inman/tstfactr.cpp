#include <stdlib.h>
#include <assert.h>
#include <typeinfo.h>

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"


#include "inman/comp/comps.hpp"
#include "inman/comp/compsutl.hpp"

using smsc::inman::comp::InapOpCode;
using smsc::inman::comp::CompFactory;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

typedef OperationFactoryInstanceT <id_ac_cap4_sms_AC> CAP4SMSFactory;

void initCAP4SMSComponents(OperationFactory * fact)
{
    fact->setLogger(Logger::getInstance("smsc.inman.comp.CAP4SMSFactory"));
    fact->registerArg(InapOpCode::RequestReportSMSEvent,
	new CompFactory::ProducerT<smsc::inman::comp::RequestReportSMSEventArg>() );
}


int main(int argc, char * argv[])
{
    int   method = 0;
    Component * comp = NULL;
    unsigned    opcode = InapOpCode::RequestReportSMSEvent;


    Logger::Init();

    Logger	*logger = Logger::getInstance("smsc.ac.factory");

    CAP4SMSFactory::getInstance()->Init(initCAP4SMSComponents);

    if (argc > 1)
	method = atoi(argv[1]);

    if (method) {
	method = 1;
	comp = CAP4SMSFactory::getInstance()->createArg(opcode);
    } else {
	comp = ApplicationContextFactory::operationFactory(id_ac_cap4_sms_AC)->createArg(opcode);
    }
    assert(comp);
    smsc_log_debug(logger, "method %d: %s is returned for AC: %d, OPcode: %d", method,
		   (typeid(*comp)).name(), id_ac_cap4_sms_AC, opcode);

    return 0;
}
