#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

#include "sms/sms.h"
#include "store/MessageStore.h"
#include "test/util/Util.hpp"
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
const int MAX_SERVICE_TYPE_LENGTH = 6;

//implemented
const char* const TC_STORE_CORRECT_SMS = "storeCorrectSms";
const char* const TC_STORE_REJECT_DUPLICATE_SMS = "storeRejectDuplicateSms";
const char* const TC_STORE_REPLACE_CORRECT_SMS = "storeReplaceCorrectSms";
const char* const TC_STORE_REPLACE_SMS_IN_FINAL_STATE = "storeReplaceSmsInFinalState";
const char* const TC_STORE_INCORRECT_SMS = "storeIncorrectSms";
const char* const TC_STORE_ASSERT_SMS = "storeAssertSms";
const char* const TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_ENROUTE = 
	"changeExistentSmsStateEnrouteToEnroute";
const char* const TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_FINAL = 
	"changeExistentSmsStateEnrouteToFinal";
const char* const TC_CHANGE_FINAL_SMS_STATE_TO_ANY = "changeFinalSmsStateToAny";
const char* const TC_REPLACE_CORRECT_SMS = "replaceCorrectSms";
const char* const TC_REPLACE_INCORRECT_SMS = "replaceIncorrectSms";
const char* const TC_LOAD_EXISTENT_SMS = "loadExistentSms";
const char* const TC_LOAD_NON_EXISTENT_SMS = "loadNonExistentSms";
const char* const TC_DELETE_EXISTENT_SMS = "deleteExistentSms";
const char* const TC_DELETE_NON_EXISTENT_SMS = "deleteNonExistentSms";
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
	 * @exception std::exception ���� ������������� �� ������.
	 */
	MessageStoreTestCases();

	/**
	 * ���������������� ���������� Message Store.
	 */
	~MessageStoreTestCases() {}

	/**
	 * ���������� ����������� sms.
	 */
	TCResult* storeCorrectSms(SMSId* id, SMS* sms, int num);

	/**
	 * ���������� ����������� sms, �� � ����������� ����� �������� �� ��� 
	 * ������������ sms.
	 */
	TCResult* storeCorrectSms(SMSId* id, SMS* sms,
		const SMSId existentId, const SMS& existentSms, int num);

	/**
	 * ���������� �������������� sms � �������.
	 */
	TCResult* storeRejectDuplicateSms(const SMS& existentSms);

	/**
	 * ���������� ����������� sms � ���������� ��� �������������.
	 */
	TCResult* storeReplaceCorrectSms(const SMSId existentId,
		SMS* existentSms);

	/**
	 * ���������� sms � ���������� ������������� sms ��������� ���������.
	 */
	TCResult* storeReplaceSmsInFinalState(SMSId* id, SMS* sms,
		const SMSId existentId, const SMS& existentSms);

	/**
	 * ���������� ������������� sms.
	 */
	TCResult* storeIncorrectSms(int num);

	/**
	 * ���������� ������������� sms � ��������� �� assert.
	 */
	TCResult* storeAssertSms(int num);

	/**
	 * ���������� ������� sms � ��������� ENROUTE.
	 */
	TCResult* changeExistentSmsStateEnrouteToEnroute(const SMSId id, SMS* sms, int num);

	/**
	 * ������� sms �� ENROUTE � ��������� ���������.
	 */
	TCResult* changeExistentSmsStateEnrouteToFinal(const SMSId id, SMS* sms, int num);

	/**
	 * ������� ��������������� sms ��� sms � ��������� ��������� � ����� ������ ���������.
	 */
	TCResult* changeFinalSmsStateToAny(const SMSId id, int num);

	/**
	 * ���������� ���������� ������������� sms.
	 */
	TCResult* replaceCorrectSms(const SMSId id, SMS* sms, int num);

	/**
	 * ������������ ���������� ������������� ��� ���������� ��������������� sms.
	 */
	TCResult* replaceIncorrectSms(const SMSId id, const SMS& sms, int num);

	/**
	 * ������ ������������� sms.
	 */
	TCResult* loadExistentSms(const SMSId id, const SMS& sms);

	/**
	 * ������ ��������������� sms.
	 */
	TCResult* loadNonExistentSms(const SMSId id);

	/**
	 * �������� ������������� SM.
	 * ���� ���� ���� �� ������������� ����������� � �������, ��������� � 
	 * �������� ������ ��������� ��������� (��������� � �����) ���������� 
	 * ���������.
	 */
	TCResult* deleteExistentSms(const SMSId id);
	
	/**
	 * �������� ��������������� SM.
	 * ���� ���� ���� �� ������������� ����������� � �������, ��������� � 
	 * �������� ������ ��������� ��������� (��������� � �����) ���������� 
	 * ���������.
	 */
	TCResult* deleteNonExistentSms(const SMSId id);

	/**
	 * �������� ������������ SM ��������� �������� �� ������������ �����.
	 */
	TCResult* deleteExistentWaitingSMByNumber();
	
	/**
	 * �������� �������������� SM ��������� �������� �� ������������ �����.
	 */
	TCResult* deleteNonExistentWaitingSMByNumber();
	
	/**
	 * �������� ��������� ������ SM ��������� �������� �� ������������ �����.
	 */
	TCResult* loadExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ������� ������ SM ��������� �������� �� ������������ �����.
	 */
	TCResult* loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ��������� ������ SM ������������ �� ������������ �����.
	 */
	TCResult* loadExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ������� ������ SM ������������ �� ������������ �����.
	 */
	TCResult* loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ��������� ������ SM ������������ � ������������� ������.
	 */
	TCResult* loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * �������� ������� ������ SM ������������ � ������������� ������.
	 */
	TCResult* loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * �������� �������� ���������� ������ ���������� ���������.
	 */
	TCResult* getExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ���������� ������ ���������� ���������.
	 */
	TCResult* getNonExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ��� ���������� ������.
	 */
	TCResult* createBillingRecord();

private:
	MessageStore* msgStore;
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

