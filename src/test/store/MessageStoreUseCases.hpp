#ifndef TEST_STORE_MESSAGE_STORE_USE_CASES
#define TEST_STORE_MESSAGE_STORE_USE_CASES

#include "MessageStoreTestCases.hpp"

namespace smsc  {
namespace test  {
namespace store {

/**
 * Ётот класс содержит все use cases необходимые дл€ тестировани€ подсистемы
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreUseCases
{
public:
	/**
	 * —охранение SM.
	 */
	bool storeSM();

	/**
	 * »зменение статуса SM.
	 */
	bool setSMStatus();

	/**
	 * ќбновление SM.
	 */
	bool updateSM();

	/**
	 * ”даление SM.
	 */
	bool deleteSM();
	
	/**
	 * ”даление SM ожидающих доставки на определенный номер.
	 */
	bool deleteWaitingSMByNumber();

	/**
	 * „тение SM.
	 */
	bool loadSM();

	/**
	 * «агрузка SM ожидающих доставки на определенный номер.
	 */
	bool loadWaitingSMByDestinationNumber();

	/**
	 * «агрузка архива SM доставленных на определенный номер.
	 */
	bool loadSMArchieveByDestinationNumber();

	/**
	 * «агрузка архива SM доставленных с определенного номера.
	 */
	bool loadSMArchieveByOriginatingNumber();

	/**
	 * ѕросмотр статистики причин недоставки сообщений с указанием количества 
	 * абонентов, которым не были доставлены сообщени€ по этим причинам.
	 */
	bool getSMDeliveryFailureStatistics();

	/**
	 * —оздание записи дл€ начислени€ оплаты.
	 */
	bool createBillingRecord();

private:
	MessageStoreTestCases tc;
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_USE_CASES */
