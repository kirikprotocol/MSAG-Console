#include "MessageStoreTestCases.h"
#include "../util/Util.h"

using namespace smsc::test::util;

namespace smsc  {
namespace test  {
namespace store {

bool MessageStoreTestCases::storeCorrectSM(int num = -1)
{
	switch(selectTestProc(num, 2))
	{
		case 0:
			return false;
		case 1:
			return true;
		default:
			return storeCorrectSM();
	}
}

bool MessageStoreTestCases::storeIncorrectSM()
{
	return false;
}

bool MessageStoreTestCases::setCorrectSMStatus()
{
	return false;
}

bool MessageStoreTestCases::setIncorrectSMStatus()
{
	return false;
}

bool MessageStoreTestCases::setNonExistentSMStatus()
{
	return false;
}

bool MessageStoreTestCases::updateCorrectExistentSM()
{
	return false;
}

bool MessageStoreTestCases::updateIncorrectExistentSM()
{
	return false;
}
	
bool MessageStoreTestCases::updateNonExistentSM()
{
	return false;
}

bool MessageStoreTestCases::deleteExistingSM()
{
	return false;
}
	
bool MessageStoreTestCases::deleteNonExistingSM()
{
	return false;
}
	
bool MessageStoreTestCases::loadExistingSM()
{
	return false;
}

bool MessageStoreTestCases::loadNonExistingSM()
{
	return false;
}

bool MessageStoreTestCases::createBillingRecord()
{
	return false;
}

}
}
}

