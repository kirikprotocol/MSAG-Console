#include "MessageStoreLoadTestTaskManager.hpp"
#include <iostream>

using namespace std;
using namespace smsc::test::store;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: MessageStoreLoadTest <numThreads>" << endl;
		exit(-1);
	}
	
	const int numThreads = atoi(argv[1]);
	MessageStoreLoadTestTaskManager tm;
	tm.startTasks(numThreads);
	while (true)
	{
		char ch;
		cin >> ch;
		switch (ch)
		{
			case 'q':
				tm.stopTasks();
				cout << "Total operations = " << tm.getOps() << endl;
				exit(0);
				break;
			default:
				cout << "Rate = " << tm.getRate()
					<< " messages/second" << endl;
		}
	}
	return 0;
}

