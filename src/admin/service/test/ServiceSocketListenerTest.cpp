#include <admin/service/ServiceSocketListener.h>
#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <admin/service/test/DumbServiceCommandHandler.h>

int main (int argc, char *argv[])
{
	using smsc::admin::service::ServiceSocketListener;
	try {
		XMLPlatformUtils::Initialize();
		std::cout << "Initializing service\n";
		smsc::admin::service::test::DumbServiceCommandHandler handler;
		ServiceSocketListener listener(6677, &handler);
		std::cout << "Starting service\n";
		listener.run();
		std::cout << "Service finished\n";
	} catch (smsc::admin::AdminException &e) {
		std::cerr << "Exception occured: '" << e.what() << "'\n";
	} catch (...) {
		std::cerr << "Unknown Exception occured\n";
	}
	return 0;
}
