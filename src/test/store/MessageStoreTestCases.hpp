#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

#include "sms/sms.h"
#include "store/MessageStore.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include <exception>
#include <vector>

namespace smsc  {
namespace test  {
namespace store {

using std::vector;
using log4cpp::Category;
using smsc::sms::SMSId;
using smsc::sms::SMS;
using smsc::store::MessageStore;
using smsc::test::util::TCResult;
using smsc::test::util::BaseTestCases;

//implemented
const char* const TC_STORE_CORRECT_SMS = "storeCorrectSms";
const char* const TC_STORE_DUPLICATE_SMS = "storeDuplicateSms";
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
const char* const TC_REPLACE_FINAL_SMS = "replaceFinalSms";
const char* const TC_LOAD_EXISTENT_SMS = "loadExistentSms";
const char* const TC_LOAD_NON_EXISTENT_SMS = "loadNonExistentSms";
const char* const TC_DELETE_EXISTENT_SMS = "deleteExistentSms";
const char* const TC_DELETE_NON_EXISTENT_SMS = "deleteNonExistentSms";
const char* const TC_CHECK_READY_FOR_RETRY_SMS = "checkReadyForRetrySms";
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

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ ����������
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases : BaseTestCases
{
public:
	/**
	 * �������������� ���������� Message Store.
	 * @exception std::exception ���� ������������� �� ������.
	 */
	MessageStoreTestCases(MessageStore* msgStore);

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
	 * ���������� �������������� sms.
	 */
	TCResult* storeDuplicateSms(SMSId* idp, SMS* smsp, const SMSId existentId,
		const SMS& existentSms);

	/**
	 * ���������� �������������� sms � �������.
	 */
	TCResult* storeRejectDuplicateSms(const SMS& existentSms);

	/**
	 * ���������� ����������� sms � ���������� ��� �������������.
	 */
	TCResult* storeReplaceCorrectSms(SMSId* id, SMS* existentSms);

	/**
	 * ���������� sms � ���������� ������������� sms ��������� ���������.
	 */
	TCResult* storeReplaceSmsInFinalState(SMSId* id, SMS* sms,
		const SMS& existentSms);

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
	 * ���������� ��������������� sms ��� sms � ��������� ���������.
	 */
	TCResult* replaceFinalSms(const SMSId id, const SMS& sms);

	/**
	 * ������ ������������� sms.
	 */
	TCResult* loadExistentSms(const SMSId id, const SMS& sms);

	/**
	 * ������ ��������������� sms.
	 */
	TCResult* loadNonExistentSms(const SMSId id);

	/**
	 * �������� ������������� sms.
	 * ���� ���� ���� �� ������������� ����������� � �������, ��������� � 
	 * �������� ������ ��������� ��������� (��������� � �����) ���������� 
	 * ���������.
	 */
	TCResult* deleteExistentSms(const SMSId id);
	
	/**
	 * �������� ��������������� sms.
	 * ���� ���� ���� �� ������������� ����������� � �������, ��������� � 
	 * �������� ������ ��������� ��������� (��������� � �����) ���������� 
	 * ���������.
	 */
	TCResult* deleteNonExistentSms(const SMSId id);

	/**
	 * ��������� ������ sms ��� ��������� ��������.
	 */
	TCResult* checkReadyForRetrySms(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, int num);

	/**
	 * �������� ������������ sms ��������� �������� �� ������������ �����.
	 */
	TCResult* deleteExistentWaitingSMByNumber();
	
	/**
	 * �������� �������������� sms ��������� �������� �� ������������ �����.
	 */
	TCResult* deleteNonExistentWaitingSMByNumber();
	
	/**
	 * �������� ��������� ������ sms ��������� �������� �� ������������ �����.
	 */
	TCResult* loadExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ������� ������ sms ��������� �������� �� ������������ �����.
	 */
	TCResult* loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ��������� ������ sms ������������ �� ������������ �����.
	 */
	TCResult* loadExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ������� ������ sms ������������ �� ������������ �����.
	 */
	TCResult* loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ��������� ������ sms ������������ � ������������� ������.
	 */
	TCResult* loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * �������� ������� ������ sms ������������ � ������������� ������.
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

private:
	MessageStore* msgStore;

	virtual Category& getLog();
	void compareReadyForRetrySmsList(const vector<SMSId*>& ids, 
		const vector<SMS*>& sms, time_t time, TCResult* res, int shift);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

