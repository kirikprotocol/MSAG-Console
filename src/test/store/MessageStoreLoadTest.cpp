#include "MessageStoreLoadTest.h"
#include <iostream>
#include <string>

using namespace std;

namespace smsc  {
namespace test  {
namespace store {

void MessageStoreLoadTest::runInteractiveMode()
{
	const string RESULT_PROMPT = "Result> ";
	const string CMD_PROMPT = "Command> ";

	bool started = false;
	while (true)
	{
		cout << CMD_PROMPT;
		string cmd;
		cin >> cmd;
		if (cmd == "start")
		{
			int numThreads;
			cin >> numThreads;
			if (started)
			{
				cout << RESULT_PROMPT << "Already started" << endl;
			}
			else
			{
				startTest(numThreads);
				started = true;
				cout << RESULT_PROMPT << "Starting test with " << numThreads 
					<< " threads" << endl;
			}
		}
		else if (cmd == "stop")
		{
			if (started)
			{
				int res = stopTest();
				started = false;
				cout << RESULT_PROMPT << res << " messages/second" << endl;
			}
			else
			{
				cout << RESULT_PROMPT << "Not started" << endl;
			}
		}
		else if (cmd == "stat")
		{
			int res = getStat();
			cout << RESULT_PROMPT << res << " messages/second" << endl;
		}
		else if (cmd == "exit")
		{
			break;
		}
		else
		{
			cout << RESULT_PROMPT << "Valid commands: start <numThreads>, " 
				"stop, stat, exit" << endl;
		}
	}
}

}
}
}
