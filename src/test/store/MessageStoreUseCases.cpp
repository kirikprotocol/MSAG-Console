#include "MessageStoreUseCases.h"

namespace smsc  {
namespace test  {
namespace store {

bool MessageStoreUseCases::storeSM()
{
	return tc.storeCorrectSM() &&
		tc.storeIncorrectSM();
}

bool MessageStoreUseCases::setSMStatus()
{
	return tc.setCorrectSMStatus() &&
		tc.setIncorrectSMStatus() &&
		tc.setNonExistentSMStatus();
}

bool MessageStoreUseCases::updateSM()
{
	return tc.updateCorrectExistentSM() &&
		tc.updateIncorrectExistentSM() &&
		tc.updateNonExistentSM();
}

bool MessageStoreUseCases::deleteSM()
{
	return tc.deleteExistingSM() &&
		tc.deleteNonExistingSM();
}
	
bool MessageStoreUseCases::loadSM()
{
	return tc.loadExistingSM() &&
		tc.loadNonExistingSM();
}

bool MessageStoreUseCases::createBillingRecord()
{
	return tc.createBillingRecord();
}

}
}
}

