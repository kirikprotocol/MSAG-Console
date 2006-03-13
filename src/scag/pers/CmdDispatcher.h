#ifndef SCAG_PERS_COMMAND_DISPATCHER
#define SCAG_PERS_COMMAND_DISPATCHER

#include "logger/Logger.h"
#include "SerialBuffer.h"
#include "ProfileStore.h"

namespace scag { namespace pers {

using smsc::logger::Logger;

class CommandDispatcher {
	StringProfileStore* AbonentStore;
	IntProfileStore	*ProviderStore;
	IntProfileStore* OperatorStore; 
	IntProfileStore* ServiceStore;
public:
    CommandDispatcher(StringProfileStore *abonent, IntProfileStore *service, IntProfileStore *oper, IntProfileStore *provider) :
		log(Logger::getInstance("cmd")), AbonentStore(abonent), ServiceStore(service), ProviderStore(provider), OperatorStore(oper) {};
    ~CommandDispatcher() {};
    int Execute(SerialBuffer* sb);
protected:
    Logger * log;
};

}
}

#endif

