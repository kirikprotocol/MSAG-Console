#include <fstream>

using namespace std;

struct ConfigurationMaster
{
	const int count;
	const int shift;
	
	ConfigurationMaster(int _count, int _shift)
	: count(_count), shift(_shift) {}

	void genUserConfig();
	void genSmeConfig();
	void genRouteConfig();
	void genAliasConfig();
	const string str(int i);
};

const string ConfigurationMaster::str(int i)
{
	char buf[4];
	sprintf(buf, "%02d", i);
	return buf;
}

void ConfigurationMaster::genUserConfig()
{
	ofstream os("users.txt");
	for (int i = 0; i < count; i++)
	{
		os << "\t<user login=\"user" << str(i) <<
			"\" password=\"pass" << str(i) <<
			"\" firstName=\"first" << str((i + shift) % count) <<
			"\" lastName=\"last" << str((i + 2 * shift) % count) <<
			"\" dept=\"dept" << str((i + 3 * shift) % count) <<
			"\" workPhone=\"123\" homePhone=\"123\" cellPhone=\"123\"" <<
			" email=\"user" << str(i) <<
			"@mail.ru\">\n\t</user>" << endl;
	}
}

void ConfigurationMaster::genSmeConfig()
{
	ofstream os("sme.txt");
	for (int i = 0; i < count; i++)
	{
		os << "<smerecord type=\"smpp\" uid=\"sme" << str(i) << "\">" << endl;
		os << "\t<param name=\"typeOfNumber\" value=\"0\"/>" << endl;
		os << "\t<param name=\"numberingPlan\" value=\"0\"/>" << endl;
		os << "\t<param name=\"interfaceVersion\" value=\"52\"/>" << endl;
		os << "\t<param name=\"systemType\" value=\"\"/>" << endl;
		os << "\t<param name=\"password\" value=\"\"/>" << endl;
		os << "\t<param name=\"addrRange\" value=\"\"/>" << endl;
		os << "\t<param name=\"smeN\" value=\"0\"/>" << endl;
		os << "\t<param name=\"wantAlias\" value=\"yes\"/>" << endl;
		os << "\t<param name=\"timeout\" value=\"10\"/>" << endl;
		os << "\t<param name=\"priority\" value=\"32\"/>" << endl;
		os << "</smerecord>" << endl;
	}
}

void ConfigurationMaster::genRouteConfig()
{
	ofstream os("routes.txt");
	for (int i = 0; i < count; i++)
	{
		os << "<subject_def id=\"subj" << str(i) <<
			"\" defSme=\"sme" << str((i + shift) % count) << "\">" << endl;
		os << "\t<mask value=\"" << str((i + 2 * shift) % count) << "???\"/>" << endl;
		os << "\t<mask value=\"+" << str((i + 3 * shift) % count) << "???\"/>" << endl;
		os << "\t<mask value=\".2.1.789\"/>" << endl;
		os << "</subject_def>" << endl;
	}
	for (int i = 0; i < count; i++)
	{
		const string id = str(i);
		const string val = i % 2 ? "true" : "false";
		os << "<route id=\"route" << id << "\" billing=\"" << val <<
			"\" archiving=\"" << val << "\"" << " enabling=\""<< val <<
			"\" priority=\"" << i << "\" serviceId=\"" << i <<
			"\">" << endl;
		os << "\t<source>" << endl;
		os << "\t\t<subject id=\"subj" << id << "\"/>" << endl;
		os << "\t</source>" << endl;
		os << "\t<source>" << endl;
		os << "\t\t<mask value=\"123????\"/>" << endl;
		os << "\t</source>" << endl;
		os << "\t<destination sme=\"sme" << id << "\">" << endl;
		os << "\t\t<mask value=\"+456????\"/>" << endl;
		os << "\t</destination>" << endl;
		os << "\t<destination sme=\"sme" << id << "\">" << endl;
		os << "\t\t<subject id=\"subj" << id << "\"/>" << endl;
		os << "\t</destination>" << endl;
		os << "</route>" << endl;
	}
}

void ConfigurationMaster::genAliasConfig()
{
	ofstream os("alias.txt");
	for (int i = 0; i < count; i++)
	{
		os << "\t<record addr=\"+" << str(i) <<
			"\" alias=\"" << str((i + shift) % count) <<
			"\" hide=\"true\"/>" << endl;
		os << "\t<record addr=\"+" << str(i) <<
			"?\" alias=\"" << str((i + shift) % count) <<
			"?\" hide=\"false\"/>" << endl;
	}
}

int main(int argc, char* argv[])
{
	int count = 23;
	int shift = 1;
	if (argc == 2)
	{
		count = atoi(argv[1]);
	}
	else if (argc == 3)
	{
		count = atoi(argv[1]);
		shift = atoi(argv[2]);
	}
	ConfigurationMaster gen(count, shift);
	gen.genUserConfig();
	gen.genSmeConfig();
	gen.genRouteConfig();
	gen.genAliasConfig();
	return 0;
}
