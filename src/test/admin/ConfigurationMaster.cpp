#include <algorithm>
#include <iostream>
#include <fstream>
#include "util/debug.h"

using namespace std;

struct ConfigurationMaster
{
	const int count;
	
	ConfigurationMaster(int _count) : count(_count) {}

	void genUserConfig();
	void genSmeConfig();
	void genRouteConfig();
	void genAliasConfig();
	const string str1(const char* name, int i);
	const string str2(const char* name, int i);
};

const string ConfigurationMaster::str1(const char* name, int i)
{
	__require__(name);
	char buf[strlen(name) + 10];
	sprintf(buf, "%s%02d", name, i);
	return buf;
}

const string ConfigurationMaster::str2(const char* name, int i)
{
	__require__(name);
	char buf[strlen(name) + 10];
	sprintf(buf, " %02d", i);
	int hash = 0;
	for (int i = strlen(buf) - 1; i > 0; i--)
	{
		hash = hash * 31 + buf[i];
	}
	static const string values = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890*#";
	buf[0] = values[hash & 0x3f];
	return buf;
}

void ConfigurationMaster::genUserConfig()
{
	ofstream os("users.txt");
	for (int i = 0; i < count; i++)
	{
		os << "\t<user login=\"" << str1("user", i) <<
			"\" password=\"" << str1("pass", i) <<
			"\" firstName=\"" << str2("first", i) <<
			"\" lastName=\"" << str2("last", i) <<
			"\" dept=\"" << str2("dept", i) <<
			"\" workPhone=\"" << str1("work", i) <<
			"\" homePhone=\"" << str1("home", i) <<
			"\" cellPhone=\"" << str1("cell", i) <<
			"\" email=\"" << str1("user", i) << "@mail.ru\">" <<
			"\n\t</user>" << endl;
	}
}

void ConfigurationMaster::genSmeConfig()
{
	ofstream os("sme.txt");
	for (int i = 0; i < count; i++)
	{
		os << "<smerecord type=\"smpp\" uid=\"" << str1("sme", i) << "\">" << endl;
		os << "\t<param name=\"typeOfNumber\" value=\"0\"/>" << endl;
		os << "\t<param name=\"numberingPlan\" value=\"0\"/>" << endl;
		os << "\t<param name=\"interfaceVersion\" value=\"52\"/>" << endl;
		os << "\t<param name=\"systemType\" value=\"\"/>" << endl;
		os << "\t<param name=\"password\" value=\"\"/>" << endl;
		os << "\t<param name=\"addrRange\" value=\"\"/>" << endl;
		os << "\t<param name=\"smeN\" value=\"0\"/>" << endl;
		os << "\t<param name=\"wantAlias\" value=\"yes\"/>" << endl;
		os << "\t<param name=\"forceDC\" value=\"false\"/>" << endl;
		os << "\t<param name=\"timeout\" value=\"10\"/>" << endl;
		os << "\t<param name=\"priority\" value=\"32\"/>" << endl;
		os << "</smerecord>" << endl;
	}
}

void ConfigurationMaster::genRouteConfig()
{
	ofstream os("routes.txt");
	//id - latin, masks with "?"
	for (int i = 0; i < count; i++)
	{
		os << "<subject_def id=\"" << str1("subject", i) << "\" " <<
			"defSme=\"" << str1("sme", i) << "\">" << endl;
		os << "\t<mask value=\"" << str1("", i) << "?\"/>" << endl;
		os << "\t<mask value=\"" << str1("+", i) << "?\"/>" << endl;
		os << "</subject_def>" << endl;
	}
	//id - cyrillic, masks without "?"
	for (int i = 0; i < count; i++)
	{
		os << "<subject_def id=\"" << str1("субъект", i) << "\" " <<
			"defSme=\"" << str1("sme", i) << "\">" << endl;
		os << "\t<mask value=\"" << str1("", i) << "\"/>" << endl;
		os << "\t<mask value=\"" << str1("+", i) << "\"/>" << endl;
		os << "</subject_def>" << endl;
	}
	//id - latin, route to itself, with "?"
	for (int i = 0; i < count; i++)
	{
		os << "<route id=\"" << str1("route", i) <<
			"\" billing=\"false\" archiving=\"false\" enabling=\"false\"" <<
			" priority=\"" << i << "\" serviceId=\"" << i <<
			"\" suppressDeliveryReports=\"false\">" << endl;
		os << "\t<source>" << endl;
		os << "\t\t<subject id=\"" << str1("subject", i) << "\"/>" << endl;
		os << "\t</source>" << endl;
		os << "\t<source>" << endl;
		os << "\t\t<mask value=\"" << str1(".0.0.", i) << "?\"/>" << endl;
		os << "\t</source>" << endl;
		os << "\t<destination sme=\"" << str1("sme", i) << "\">" << endl;
		os << "\t\t<subject id=\"" << str1("subject", i) << "\"/>" << endl;
		os << "\t</destination>" << endl;
		os << "\t<destination sme=\"" << str1("sme", i) << "\">" << endl;
		os << "\t\t<mask value=\"" << str1(".0.0.", i) << "?\"/>" << endl;
		os << "\t</destination>" << endl;
		os << "</route>" << endl;
	}
	//id - cyrillic, route to next, without "?"
	for (int i = 0; i < count; i++)
	{
		int i2 = (i + 1) % count;
		os << "<route id=\"" << str1("маршрут", i) <<
			"\" billing=\"true\" archiving=\"true\" enabling=\"true\"" <<
			" priority=\"" << i << "\" serviceId=\"" << i <<
			"\" suppressDeliveryReports=\"true\">" << endl;
		os << "\t<source>" << endl;
		os << "\t\t<subject id=\"" << str1("субъект", i) << "\"/>" << endl;
		os << "\t</source>" << endl;
		os << "\t<source>" << endl;
		os << "\t\t<mask value=\"" << str1(".0.0.", i) << "\"/>" << endl;
		os << "\t</source>" << endl;
		os << "\t<destination sme=\"" << str1("sme", i2) << "\">" << endl;
		os << "\t\t<subject id=\"" << str1("субъект", i2) << "\"/>" << endl;
		os << "\t</destination>" << endl;
		os << "\t<destination sme=\"" << str1("sme", i2) << "\">" << endl;
		os << "\t\t<mask value=\"" << str1(".0.0.", i2) << "\"/>" << endl;
		os << "\t</destination>" << endl;
		os << "</route>" << endl;
	}
}

void ConfigurationMaster::genAliasConfig()
{
	ofstream os("alias.txt");
	for (int i = 0; i < count; i++)
	{
		string addr = str1("", i);
		string alias = addr;
		reverse(alias.begin(), alias.end());
		//without "?"
		os << "\t<record addr=\"" << addr <<
			"\" alias=\"" << alias <<
			"\" hide=\"true\"/>" << endl;
		os << "\t<record addr=\"+" << addr <<
			"\" alias=\"+" << alias <<
			"\" hide=\"true\"/>" << endl;
		//with "?"
		os << "\t<record addr=\"" << addr <<
			"?\" alias=\"" << alias <<
			"?\" hide=\"false\"/>" << endl;
		os << "\t<record addr=\"+" << addr <<
			"?\" alias=\"+" << alias <<
			"?\" hide=\"false\"/>" << endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "usage: ConfigurationMaster <count>" << endl;
		exit(-1);
	}
	int count = atoi(argv[1]);
	ConfigurationMaster gen(count);
	gen.genUserConfig();
	gen.genSmeConfig();
	gen.genRouteConfig();
	gen.genAliasConfig();
	return 0;
}
