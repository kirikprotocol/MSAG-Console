#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

#include "sms/sms.h"
#include "store/MessageStore.h"
#include "test/util/Util.hpp"
#include "SMUtil.hpp"
#include <exception>

namespace smsc  {
namespace test  {
namespace store {

using namespace smsc::test::util;

const std::string TC_STORE_CORRECT_SM = "storeCorrectSM";
const std::string TC_STORE_INCORRECT_SM = "storeIncorrectSM";
const std::string TC_SET_CORRECT_SM_STATUS = "setCorrectSMStatus";
const std::string TC_SET_INCORRECT_SM_STATUS = "setIncorrectSMStatus";
const std::string TC_SET_NON_EXISTENT_SM_STATUS = "setNonExistentSMStatus";
const std::string TC_UPDATE_CORRECT_EXISTENT_SM = "updateCorrectExistentSM";
const std::string TC_UPDATE_INCORRECT_EXISTENT_SM = "updateIncorrectExistentSM";
const std::string TC_UPDATE_NON_EXISTENT_SM = "updateNonExistentSM";
const std::string TC_DELETE_EXISTENT_SM = "deleteExistentSM";
const std::string TC_DELETE_NON_EXISTENT_SM = "deleteNonExistentSM";
const std::string TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteExistentWaitingSMByNumber";
const std::string TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteNonExistentWaitingSMByNumber";
const std::string TC_LOAD_EXISTENT_SM = "loadExistentSM";
const std::string TC_LOAD_NONEXISTENT_SM = "loadNonExistentSM";
const std::string TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER = 
	"loadExistentWaitingSMByDestinationNumber";
const std::string TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER = 
	"loadNonExistentWaitingSMByDestinationNumber";
const std::string TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER = 
	"loadExistentSMArchieveByDestinationNumber";
const std::string TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER = 
	"loadNonExistentSMArchieveByDestinationNumber";
const std::string TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER = 
	"loadExistentSMArchieveByOriginatingNumber";
const std::string TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER = 
	"loadNonExistentSMArchieveByOriginatingNumber";
const std::string TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS = 
	"getExistentSMDeliveryFailureStatistics";
const std::string TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS = 
	"getNonExistentSMDeliveryFailureStatistics";
const std::string TC_CREATE_BILLING_RECORD = "createBillingRecord";

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ ����������
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases
{
public:
	/**
	 * �������������� ���������� Message Store.
	 * 
	 * @exception std::exception ���� ������������� �� ������.
	 */
	MessageStoreTestCases() throw (smsc::store::StoreException);

	/**
	 * ���������� ����������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 * 
	 * @param smsId ���������� id ���������� � �� ���������
	 * @param num ����� ����-���������
	 */
	TCResult* storeCorrectSM(smsc::sms::SMSId* id, smsc::sms::SMS* sms, int num);

	/**
	 * ���������� ������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* storeIncorrectSM(smsc::sms::SMS& existentSMS, int num);

	/**
	 * ���������� ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� �������� ������ SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* setCorrectSMStatus();

	/**
	 * ������������ ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� �������� ��������� ������������ ������ SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* setIncorrectSMStatus();

	/**
	 * ��������� ������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* setNonExistentSMStatus();

	/**
	 * ���������� ���������� ������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* updateCorrectExistentSM();

	/**
	 * ���������� ������������� SM ������������� �������.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* updateIncorrectExistentSM();
	
	/**
	 * ���������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* updateNonExistentSM();

	/**
	 * �������� ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteExistentSM();
	
	/**
	 * �������� ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteNonExistentSM();

	/**
	 * �������� ������������ SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteExistentWaitingSMByNumber();
	
	/**
	 * �������� �������������� SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteNonExistentWaitingSMByNumber();
	
	/**
	 * ������ ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadExistentSM(smsc::sms::SMSId id, smsc::sms::SMS& sms);

	/**
	 * ������ ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadNonExistentSM();

	/**
	 * �������� ��������� ������ SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ������� ������ SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ��������� ������ SM ������������ �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ������� ������ SM ������������ �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ��������� ������ SM ������������ � ������������� ������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * �������� ������� ������ SM ������������ � ������������� ������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * �������� �������� ���������� ������ ���������� ���������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* getExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ���������� ������ ���������� ���������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* getNonExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ��� ���������� ������.
	 */
	TCResult* createBillingRecord();

private:
	smsc::store::MessageStore* msgStore;
	smsc::test::store::SMUtil smUtil;

	void setupRandomCorrectSM(smsc::sms::SMS& sms);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

