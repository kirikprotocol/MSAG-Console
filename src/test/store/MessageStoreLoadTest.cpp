#include "MessageStoreTestCases.h"

namespace smsc  {
namespace test  {
namespace store {

MessageStoreTestCases tc;

int businessCycle()
{
	int ops = 0;
	for (; ops < 200; ops++)
	{
		//store
		if (ops % 100 == 0)
		{
			tc.storeIncorrectSM();
		}
		else if (ops % 50 == 0)
		{
			tc.storeCorrectSM();
			tc.setIncorrectSMStatus();
		}
		else
		{
			tc.storeCorrectSM();
			tc.setCorrectSMStatus();
		}
		//tc.createBillingRecord();
	}
	tc.updateCorrectExistentSM(); ops++;
	tc.deleteExistingSM(); ops++;
	tc.loadExistingSM(); ops++;

	return ops;
}

}
}
}

int main(int argc, char* argv[])
{
	return 0;
}
