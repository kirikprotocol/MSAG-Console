#include "ProxySmeConfig.h"
#include <util/Exception.hpp>
#include <util/xml/DOMTreeReader.h>
#include <core/buffers/Hash.hpp>
#include <sys/stat.h>

namespace smsc {
namespace proxysme {

using namespace smsc::util::xml;
using smsc::util::config::Config;
using smsc::util::Exception;

char * const findConfigFile(const char * const config_filename)
{
	if (config_filename == NULL)
		return NULL;

	struct stat s;
	std::auto_ptr<char> tmp_name(new char[strlen(config_filename)+10]);

	strcpy(tmp_name.get(), config_filename);
	if (stat(tmp_name.get(), &s) == 0)
		return tmp_name.release();

	strcpy(tmp_name.get(), "conf/");
	strcat(tmp_name.get(), config_filename);
	if (stat(tmp_name.get(), &s) == 0)
		return tmp_name.release();

	strcpy(tmp_name.get(), "../conf/");
	strcat(tmp_name.get(), config_filename);
	if (stat(tmp_name.get(), &s) == 0)
		return tmp_name.release();

	return NULL;
}

void setStrParam(std::string & param, const char * const prefix, const char * const paramName, const Config config)
{
	try {
		char fullParamName[128];
		if (prefix == NULL)
			param = config.getString(paramName);
		else
		{
			snprintf(fullParamName, sizeof(fullParamName), "%s.%s", prefix, paramName);
			param = config.getString(fullParamName);
		}
	} catch (smsc::core::buffers::HashInvalidKeyException & e) {
		throw Exception("Parameter \"%s\" for %s sme not found", paramName, prefix == NULL ? "" : prefix);
	}
}

void setIntParam(int & param, const char * const prefix, const char * const paramName, const Config config)
{
	try {
		char fullParamName[128];
		if (prefix == NULL)
			param = config.getInt(paramName);
		else
		{
			snprintf(fullParamName, sizeof(fullParamName), "%s.%s", prefix, paramName);
			param = config.getInt(fullParamName);
		}
	} catch (smsc::core::buffers::HashInvalidKeyException & e) {
		throw Exception("Parameter \"%s\" for %s sme not found", paramName, prefix == NULL ? "" : prefix);
	}
}

void ProxySmeConfig::fillSme(ProxySmeConfig::sme& smeItem, const char * const prefix, const Config config)
{
	setStrParam(smeItem.host,        prefix, "host", config);
	setIntParam(smeItem.port,        prefix, "port", config);
	setStrParam(smeItem.sid,         prefix, "sid",  config);
	setStrParam(smeItem.password,    prefix, "password", config);
	setStrParam(smeItem.origAddr,    prefix, "origAddr", config);
	setStrParam(smeItem.systemType,  prefix, "systemType", config);
	setIntParam(smeItem.timeOut,     prefix, "timeOut", config);
	setIntParam(smeItem.defaultDCS,  prefix, "defaultDCS", config);}

ProxySmeConfig::ProxySmeConfig(const char * const config_filename) //throw(smsc::util::Exception)
{
	std::auto_ptr<char> filename(findConfigFile(config_filename));
	if (filename.get() != NULL)
	{
		try
		{
			DOMTreeReader reader;
			Config config(reader.read(filename.get()).getDocumentElement());
			fillSme(left,  "left",  config);
			fillSme(right, "right", config);
			setIntParam(queueLength, NULL, "queueLength", config);
			setIntParam(adminPort, NULL, "adminPort", config);
			setStrParam(adminHost, NULL, "adminHost", config);
		} catch (DOMTreeReader::ParseException &e) {
			throw Exception("couldn't read or parse config file, nested: %s", e.what());
		} catch (Exception &e) {
			throw e;
		} catch (std::exception & e) {
			throw Exception("%s", e.what());
		} catch (...) {
			throw Exception("unknown exception");
		}
	}
}

}
}
