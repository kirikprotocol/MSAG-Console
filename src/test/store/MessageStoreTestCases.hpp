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

using smsc::sms::SMSId;
using smsc::sms::SMS;
using smsc::store::MessageStore;
using smsc::test::util::TCResult;

const int MAX_ADDRESS_LENGTH = 21;
const int MAX_MSG_BODY_LENGTH = 200;

//implemented
const char* const TC_STORE_CORRECT_SM = "storeCorrectSM";
const char* const TC_STORE_REJECT_DUPLICATE_SM = "storeRejectDuplicateSM";
const char* const TC_STORE_INCORRECT_SM = "storeIncorrectSM";
const char* const TC_STORE_ASSERT_SM = "storeAssertSM";
const char* const TC_REPLACE_CORRECT_SM = "replaceCorrectSM";
const char* const TC_REPLACE_INCORRECT_SM = "replaceIncorrectSM";
const char* const TC_REPLACE_NON_EXISTENT_SM = "replaceNonExistentSM";
const char* const TC_DELETE_EXISTENT_SM = "deleteExistentSM";
const char* const TC_DELETE_NON_EXISTENT_SM = "deleteNonExistentSM";
const char* const TC_LOAD_EXISTENT_SM = "loadExistentSM";
const char* const TC_LOAD_NON_EXISTENT_SM = "loadNonExistentSM";
const char* const TC_SET_CORRECT_SM_STATUS = "setCorrectSMStatus";
const char* const TC_SET_INCORRECT_SM_STATUS = "setIncorrectSMStatus";
const char* const TC_SET_NON_EXISTENT_SM_STATUS = "setNonExistentSMStatus";
//not implemented yet
const char* const TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteExistentWaitingSMByNumber";
const char* const TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteNonExistentWaitingSMByNumber";
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
	 */
	TCResult* storeCorrectSM(SMSId* id, SMS* sms, int num);

	/**
	 * ���������� ����������� SM, �� � ����������� ����� �������� �� ��� 
	 * ������������ SM.
	 */
	TCResult* storeCorrectSM(SMSId* idp, SMS* smsp,
		const SMSId existentId, const SMS& existentSMS, int num);

	/**
	 * ���������� �������������� SM � �������.
	 */
	TCResult* storeRejectDuplicateSM(const SMS& existentSMS);

	/**
	 * ���������� ������������� SM.
	 */
	TCResult* storeIncorrectSM(int num);

	/**
	 * ���������� ������������� SM � ��������� �� assert.
	 */
	TCResult* storeAssertSM(int num);

	/**
	 * ���������� ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� �������� ������ SM.
	 */
	TCResult* setCorrectSMStatus(SMSId id, SMS* sms, int num);

	/**
	 * ������������ ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� �������� ��������� ������������ ������ SM.
	 */
	TCResult* setIncorrectSMStatus(SMSId id);

	/**
	 * ��������� ������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* setNonExistentSMStatus(SMSId id, int num);

	/**
	 * ���������� ���������� ������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* replaceCorrectSM(SMSId id, SMS* sms, int num);

	/**
	 * ���������� ������������� SM ������������� �������.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* replaceIncorrectSM(SMSId id, const SMS& sms, int num);

	/**
	 * ���������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	TCResult* replaceNonExistentSM(SMSId id, int num);

	/**
	 * �������� ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteExistentSM(SMSId id);
	
	/**
	 * �������� ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* deleteNonExistentSM(SMSId id, int num);

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
	TCResult* loadExistentSM(SMSId id, const SMS& sms);

	/**
	 * ������ ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	TCResult* loadNonExistentSM(SMSId id, int num);

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
	MessageStore* msgStore;
	SMUtil smUtil;

	void setupRandomCorrectSM(SMS* sms);
	void clearSM(SMS* sms);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

