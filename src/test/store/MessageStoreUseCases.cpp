#include "MessageStoreUseCases.hpp"

namespace smsc  {
namespace test  {
namespace store {

/*
bool MessageStoreUseCases::storeSM()
{
	return tc.storeCorrectSM() &&
		tc.storeIncorrectSM();
}
*/

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
	
bool MessageStoreUseCases::deleteWaitingSMByNumber()
{
	return tc.deleteExistentWaitingSMByNumber() &&
		tc.deleteNonExistentWaitingSMByNumber();

}

bool MessageStoreUseCases::loadSM()
{
	return tc.loadExistingSM() &&
		tc.loadNonExistingSM();
}

bool MessageStoreUseCases::loadWaitingSMByDestinationNumber()
{
	return tc.loadExistentWaitingSMByDestinationNumber() &&
		tc.loadNonExistentWaitingSMByDestinationNumber();
}

bool MessageStoreUseCases::loadSMArchieveByDestinationNumber()
{
	return tc.loadExistentSMArchieveByDestinationNumber() &&
		tc.loadNonExistentSMArchieveByDestinationNumber();
}

bool MessageStoreUseCases::loadSMArchieveByOriginatingNumber()
{
	return tc.loadExistentSMArchieveByOriginatingNumber() &&
		tc.loadNonExistentSMArchieveByOriginatingNumber();
}

bool MessageStoreUseCases::getSMDeliveryFailureStatistics()
{
	return tc.getExistentSMDeliveryFailureStatistics() &&
		tc.getNonExistentSMDeliveryFailureStatistics();
}

bool MessageStoreUseCases::createBillingRecord()
{
	return tc.createBillingRecord();
}

}
}
}

