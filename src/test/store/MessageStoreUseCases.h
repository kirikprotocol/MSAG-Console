#ifndef TEST_STORE_MESSAGE_STORE_USE_CASES
#define TEST_STORE_MESSAGE_STORE_USE_CASES

#include "MessageStoreTestCases.h"

namespace smsc  {
namespace test  {
namespace store {

/**
 * ���� ����� �������� ��� use cases ����������� ��� ������������ ����������
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreUseCases
{
public:
	/**
	 * ���������� SM.
	 */
	bool storeSM();

	/**
	 * ��������� ������� SM.
	 */
	bool setSMStatus();

	/**
	 * ���������� SM.
	 */
	bool updateSM();

	/**
	 * �������� SM.
	 */
	bool deleteSM();
	
	/**
	 * ������ SM.
	 */
	bool loadSM();

	/**
	 * �������� ������ ��� ���������� ������.
	 */
	bool createBillingRecord();

private:
	MessageStoreTestCases tc;
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_USE_CASES */
