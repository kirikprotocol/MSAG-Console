#include "AdminAliasTestCases.hpp"
#include "AdminCheckList.hpp"

using namespace std;
using namespace smsc::test::admin;

class AdminFunctionalTest
{
	AdminBaseTestCases baseTc;
	AdminAliasTestCases aliasTc;

public:
	AdminFunctionalTest(AdminFixture* fixture)
		: baseTc(fixture), aliasTc(fixture) {}
	~AdminFunctionalTest() {}
	void executeTest();
};

void AdminFunctionalTest::executeTest()
{
	//base
	baseTc.loginCommands();
	baseTc.invalidCommands();
	baseTc.executeTestCases();
	//alias
	aliasTc.loginCommands();
	aliasTc.incorrectCommands(); //не меняют конфигурации
	aliasTc.correctCommands();
	aliasTc.executeTestCases();
}

/**
 * Выполняет тестирование admin human и script консолей.
 */
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Usage: AdminFunctionalTest <host> <port> [isHuman=yes]" << endl;
		exit(-1);
	}
	AdminCheckList chkList;
	AdminFixture fixture;
	fixture.host = argv[1];
	fixture.port = atoi(argv[2]);
	fixture.humanConsole = true;
	fixture.chkList = &chkList;
	if (argc == 4)
	{
		if (!strcmp(argv[3], "yes")) { fixture.humanConsole = true; }
		else if (!strcmp(argv[3], "no")) { fixture.humanConsole = false; }
		else
		{
			cout << "Invalid isHuman value" << endl;
			return -1;
		}
	}
	try
	{
		AdminFunctionalTest test(&fixture);
		test.executeTest();
		chkList.saveHtml();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}
	return 0;
}

