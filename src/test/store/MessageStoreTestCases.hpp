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

const int MAX_ADDRESS_LENGTH = 21;
const int MAX_MSG_BODY_LENGTH = 200;

const char* const TC_STORE_CORRECT_SM = "storeCorrectSM";
const char* const TC_STORE_INCORRECT_SM = "storeIncorrectSM";
const char* const TC_STORE_ASSERT_SM = "storeAssertSM";
const char* const TC_SET_CORRECT_SM_STATUS = "setCorrectSMStatus";
const char* const TC_SET_INCORRECT_SM_STATUS = "setIncorrectSMStatus";
const char* const TC_SET_NON_EXISTENT_SM_STATUS = "setNonExistentSMStatus";
const char* const TC_REPLACE_CORRECT_SM = "replaceCorrectSM";
const char* const TC_REPLACE_INCORRECT_SM = "replaceIncorrectSM";
const char* const TC_REPLACE_NON_EXISTENT_SM = "replaceNonExistentSM";
const char* const TC_DELETE_EXISTENT_SM = "deleteExistentSM";
const char* const TC_DELETE_NON_EXISTENT_SM = "deleteNonExistentSM";
const char* const TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteExistentWaitingSMByNumber";
const char* const TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteNonExistentWaitingSMByNumber";
const char* const TC_LOAD_EXISTENT_SM = "loadExistentSM";
const char* const TC_LOAD_NON_EXISTENT_SM = "loadNonExistentSM";
const char* const TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER = 
	"loadExistentWaitingSMByDestinationNumber";
const char* const TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER = 
	"loadNonExistentWaitingSMByDestinationNumber";
const char* const TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER = 
	"loadExistentSMArchieveByDestinationNumber";
const char* const TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER = 
	"loadNonExistentSMArchieveByDestinationNumber";
const char* const TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER = 
	"loadExistentSMArchieveByOriginatingNumber";
const char* const TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER = 
	"loadNonExistentSMArchieveByOriginatingNumber";
const char* const TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS = 
	"getExistentSMDeliveryFailureStatistics";
const char* const TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS = 
	"getNonExistentSMDeliveryFailureStatistics";
const char* const TC_CREATE_BILLING_RECORD = "createBillingRecord";

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
	MessageStoreTestCases();

	/**
	 * ���������������� ���������� Message Store.
	 */
	~MessageStoreTestCases();

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
	 * ���������� ������������� SM � ��������� �� assert.
	 */
	TCResult* storeAssertSM(int num);

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
	TCResult* replaceCorrectSM(smsc::sms::SMSId id, smsc::sms::SMS& sms, int num);

	/**
	 * ���������� ������������� SM ������������� �������.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* replaceIncorrectSM(smsc::sms::SMSId id, smsc::sms::SMS& sms, int num);
	
	/**
	 * ���������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* replaceNonExistentSM(smsc::sms::SMSId id, int num);

	/**
	 * �������� ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteExistentSM(smsc::sms::SMSId id);
	
	/**
	 * �������� ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteNonExistentSM(smsc::sms::SMSId id, int num);

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
	TCResult* loadNonExistentSM(smsc::sms::SMSId id, int num);

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

	void clearSM(smsc::sms::SMS& sms);
	void setupRandomCorrectSM(smsc::sms::SMS& sms);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

