#include "AdminAliasTestCases.hpp"
#include "AdminCheckList.hpp"

using namespace std;
using namespace smsc::test::admin;

class AdminFunctionalTest
{
	AdminFixture* fixture;

public:
	AdminFunctionalTest(AdminFixture* _fixture) : fixture(_fixture) {}
	~AdminFunctionalTest() {}
	void executeTest();
};

void AdminFunctionalTest::executeTest()
{
	//base
	AdminBaseTestCases baseTc(fixture);
	//baseTc.loginCommands();
	//baseTc.invalidCommands();
	//baseTc.executeTestCases();
	//alias
	AdminAliasTestCases aliasTc(fixture, true);
	aliasTc.loginCommands();
	//aliasTc.incorrectCommands(); //не меняют конфигурации
	aliasTc.correctCommands();
	//aliasTc.executeTestCases();
	aliasTc.apply();
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

