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
const std::string TC_LOAD_EXISTENT_SM = "loadExistentSM";
const std::string TC_LOAD_NONEXISTENT_SM = "loadNonExistentSM";

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
	bool setCorrectSMStatus();

	/**
	 * ������������ ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� �������� ��������� ������������ ������ SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool setIncorrectSMStatus();

	/**
	 * ��������� ������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool setNonExistentSMStatus();

	/**
	 * ���������� ���������� ������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool updateCorrectExistentSM();

	/**
	 * ���������� ������������� SM ������������� �������.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool updateIncorrectExistentSM();
	
	/**
	 * ���������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool updateNonExistentSM();

	/**
	 * �������� ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool deleteExistentSM();
	
	/**
	 * �������� ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool deleteNonExistentSM();

	/**
	 * �������� ������������ SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool deleteExistentWaitingSMByNumber();
	
	/**
	 * �������� �������������� SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool deleteNonExistentWaitingSMByNumber();
	
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
	bool loadExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ������� ������ SM ��������� �������� �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * �������� ��������� ������ SM ������������ �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ������� ������ SM ������������ �� ������������ �����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * �������� ��������� ������ SM ������������ � ������������� ������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * �������� ������� ������ SM ������������ � ������������� ������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * �������� �������� ���������� ������ ���������� ���������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool getExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ���������� ������ ���������� ���������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool getNonExistentSMDeliveryFailureStatistics();

	/**
	 * �������� ������ ��� ���������� ������.
	 */
	bool createBillingRecord();

private:
	smsc::store::MessageStore* msgStore;
	smsc::test::store::SMUtil smUtil;
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */
