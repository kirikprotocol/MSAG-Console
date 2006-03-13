/* $Id$ */

#include "CmdDispatcher.h"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;

int CommandDispatcher::Execute(SerialBuffer* sb)
{
/*		Property prop;
		prop.setInt("test_val", 234567, FIXED, -1, 20);
		smsc_log_debug(logger,  "setProperty: %s", prop.toString().c_str());
		StringProfileKey spk("+79232446251");
		AbonentStore.setProperty(spk, &prop);
		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		prop.setString("test_val_string", L"test_string", W_ACCESS, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		prop.setDate("test_val_string1", 111111, INFINIT, -1, 25);
		AbonentStore.setProperty(spk, &prop);
		auto_ptr<Property> pp( AbonentStore.getProperty(spk, "test_val"));
		if(pp.get() != NULL)
			smsc_log_debug(logger,  "pers %s", pp.get()->toString().c_str());
		smsc_log_debug(logger,  "end");

		prop.setInt("test_val", 234567, FIXED, -1, 20);
		smsc_log_debug(logger,  "setProperty: %s", prop.toString().c_str());
		IntProfileKey ipk(12);
		ServiceStore.setProperty(ipk, &prop);
		prop.setBool("test_val_bool", false, R_ACCESS, -1, 25);
		ServiceStore.setProperty(ipk, &prop);
		prop.setString("test_val_string", L"test_string", W_ACCESS, -1, 25);
		ServiceStore.setProperty(ipk, &prop);
		prop.setDate("test_val_string1", 111111, INFINIT, -1, 25);
		ServiceStore.setProperty(ipk, &prop);
		auto_ptr<Property> pp1( ServiceStore.getProperty(ipk, "test_val"));
		if(pp1.get() != NULL)
			smsc_log_debug(logger,  ">>pers int %s", pp1.get()->toString().c_str());
		auto_ptr<Property> pp2( ServiceStore.getProperty(ipk, "test_val_string"));
		if(pp2.get() != NULL && pp2.get()->getStringValue() == L"test_string")
			smsc_log_debug(logger,  "####>>pers int %s", pp1.get()->toString().c_str());
		smsc_log_debug(logger,  "end");*/
	return 1;
}

}}
