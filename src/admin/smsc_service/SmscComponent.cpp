#include "SmscComponent.h"

namespace smsc {
namespace admin {
namespace smsc_service {

Variant SmscComponent::call(const Method & method,
																		const Arguments & args)
	throw (AdminException)
{
	switch (method.getId())
	{
	case applyMethod:
		apply();
		return Variant("");
	default:
		throw AdminException("Unknown method \"%s\"", method.getName());
	}
	return Variant("");
}

void SmscComponent::runSmsc()
	throw (AdminException)
{
	if (smsc_app_runner.get() == 0)
	{
		smsc_app_runner.reset(new SmscAppRunner(configs));
		smsc_app_runner->Start();
	}
	else
		throw AdminException("SMSC Application started already");
}

void SmscComponent::stopSmsc()
	throw (AdminException)
{
	if (smsc_app_runner.get() != 0)
	{
		smsc_app_runner->stop();
		smsc_app_runner->WaitFor();
		smsc_app_runner.reset(0);
	}
	else
		throw AdminException("SMSC Application started already");
}

void SmscComponent::apply()
	throw (AdminException)
{
	stopSmsc();
	reReadConfigs();
	runSmsc();
}

void SmscComponent::reReadConfigs()
	throw (AdminException)
{
	configs.smemanconfig->reload();
	configs.cfgman->reinit();
}

}
}
}
