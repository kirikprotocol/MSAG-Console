#include <util/config/Manager.h>
#include <xercesc/parsers/DOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <logger/Logger.h>

#include <iostream>

std::ostream & operator <<(std::ostream &out, smsc::util::config::Manager &man)
{
	out << "***********************************************";
	out << "DB" << std::endl;

	out << "  instance : \"" << man.getString("db.instance")      << '"' << std::endl;
  out << "  user     : \"" << man.getString("db.user")          << '"' << std::endl;
  out << "  password : \"" << man.getString("db.password")      << '"' << std::endl;
  out << "  con max  : \"" << man.getInt("db.connections.max")  << '"' << std::endl;
	out << "  con init : \"" << man.getInt("db.connections.init") << '"' << std::endl;
	out << "Map"                                                         << std::endl;
  out << "  hlr: \""       << man.getString("map.hlr")          << '"' << std::endl;
  out << "  vlr: \""       << man.getString("map.vlr")          << '"' << std::endl;
	out << "Log" << std::endl;
  //out << "  location: \"" << man.getString("log.location") << '"' << std::endl;
}

int main(int argc, char ** argv)
{
	try {
		//XMLPlatformUtils::Initialize();
		smsc::util::config::Manager::init("config.xml");
		smsc::util::config::Manager &manager = smsc::util::config::Manager::getInstance();
		std::cout << manager;
		//db.setDBUserName("newUser");

        //DOM_Attr attr = manager.getDocument().createAttribute("user");
		//attr.setValue("veryNewUser");
		//db.setAttr(attr);

		std::cout << manager;
		manager.save();
	} catch (smsc::util::config::ConfigException & e) {
		std::cerr << "Exception in config...\n" << e.what();
	} catch (...) {
		std::cerr << "Damn! some exceptions...\n";
	}
}
