#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

#include "sms/sms.h"
#include "store/MessageStore.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include "test/util/CheckList.hpp"
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
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

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
	MessageStoreTestCases(MessageStore* msgStore, CheckList* chkList = NULL);

	/**
	 * ���������������� ���������� Message Store.
	 */
	~MessageStoreTestCases() {}

	/**
	 * ���������� ����������� sms.
	 */
	void storeCorrectSms(SMSId* id, SMS* sms, int num);

	/**
	 * ���������� ����������� sms, �� � ����������� ����� �������� �� ��� 
	 * ������������ sms.
	 */
	void storeSimilarSms(SMSId* id, SMS* sms,
		const SMSId existentId, const SMS& existentSms, int num);

	/**
	 * ���������� �������������� sms.
	 */
	void storeDuplicateSms(SMSId* idp, SMS* smsp, const SMSId existentId,
		const SMS& existentSms);

	/**
	 * ���������� �������������� sms � �������.
	 */
	void storeRejectDuplicateSms(const SMS& existentSms);

	/**
	 * ���������� ����������� sms � ���������� ��� �������������.
	 */
	void storeReplaceCorrectSms(SMSId* id, SMS* existentSms);

	/**
	 * ���������� sms � ���������� ������������� sms ��������� ���������.
	 */
	void storeReplaceSmsInFinalState(SMSId* id, SMS* sms,
		const SMS& existentSms);

	/**
	 * ���������� ������������� sms.
	 */
	void storeIncorrectSms(int num);

	/**
	 * ���������� ������������� sms � ��������� �� assert.
	 */
	void storeAssertSms(int num);

	/**
	 * ���������� ������� sms � ��������� ENROUTE.
	 */
	void changeExistentSmsStateEnrouteToEnroute(const SMSId id, SMS* sms, int num);

	/**
	 * ������� sms �� ENROUTE � ��������� ���������.
	 */
	void changeExistentSmsStateEnrouteToFinal(const SMSId id, SMS* sms, int num);

	/**
	 * ������� ��������������� sms ��� sms � ��������� ��������� � ����� ������ ���������.
	 */
	void changeFinalSmsStateToAny(const SMSId id, int num);

	/**
	 * ���������� ���������� ������������� sms.
	 */
	void replaceCorrectSms(const SMSId id, SMS* sms, int num);

	/**
	 * ������������ ���������� ������������� ��� ���������� ��������������� sms.
	 */
	void replaceIncorrectSms(const SMSId id, const SMS& sms, int num);

	/**
	 * ���������� ��������������� sms ��� sms � ��������� ���������.
	 */
	void replaceFinalSms(const SMSId id, const SMS& sms);

	/**
	 * ������ ������������� sms.
	 */
	void loadExistentSms(const SMSId id, const SMS& sms);

	/**
	 * ������ ��������������� sms.
	 */
	void loadNonExistentSms(const SMSId id);

	/**
	 * �������� ������������� sms.
	 * ���� ���� ���� �� ������������� ����������� � �������, ��������� � 
	 * �������� ������ ��������� ��������� (��������� � �����) ���������� 
	 * ���������.
	 */
	void deleteExistentSms(const SMSId id);
	
	/**
	 * �������� ��������������� sms.
	 * ���� ���� ���� �� ������������� ����������� � �������, ��������� � 
	 * �������� ������ ��������� ��������� (��������� � �����) ���������� 
	 * ���������.
	 */
	void deleteNonExistentSms(const SMSId id);

	/**
	 * ��������� ������ sms ��� ��������� ��������.
	 */
	void checkReadyForRetrySms(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, int num);

	/**
	 * �������� ������������ sms ��������� �������� �� ������������ �����.
	 */
	void deleteExistentWaitingSMByNumber();
	
	/**
	 * �������� �������������� sms ��������� �������� �� ������������ �����.
	 */
	void deleteNonExistentWaitingSMByNumber();
	
	/**
	 * �������� ��������� ������ sms ��������� �������� �� ������������ �����.
	 */
	void loadExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ������� ������ sms ��������� �������� �� ������������ �����.
	 */
	void loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ��������� ������ sms ������������ �� ������������ �����.
	 */
	void loadExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ������� ������ sms ������������ �� ������������ �����.
	 */
	void loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ��������� ������ sms ������������ � ������������� ������.
	 */
	void loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * �������� ������� ������ sms ������������ � ������������� ������.
	 */
	void loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * �������� �������� ���������� ������ ���������� ���������.
	 */
	void getExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ���������� ������ ���������� ���������.
	 */
	void getNonExistentSMDeliveryFailureStatistics();

private:
	MessageStore* msgStore;
	CheckList* chkList;

	virtual Category& getLog();
	void checkNextRetryTime(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, time_t& minNextTime,
		time_t& middleNextTime, time_t& maxNextTime);
	void compareReadyForRetrySmsList(const vector<SMSId*>& ids, 
		const vector<SMS*>& sms, time_t time, int shift);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

