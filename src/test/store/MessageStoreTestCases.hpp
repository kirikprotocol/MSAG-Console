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
 * Этот класс содержит все test cases необходимые для тестирования подсистемы
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases
{
public:
	/**
	 * Инициализирует подсистему Message Store.
	 * 
	 * @exception std::exception если инициализация не прошла.
	 */
	MessageStoreTestCases() throw (smsc::store::StoreException);

	/**
	 * Сохранение правильного SM.
	 * Только debug информация должна выводиться в лог.
	 * 
	 * @param smsId возвращает id созданного в БД сообщения
	 * @param num номер тест-процедуры
	 */
	TCResult* storeCorrectSM(smsc::sms::SMSId* id, smsc::sms::SMS* sms, int num);

	/**
	 * Сохранение неправильного SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* storeIncorrectSM(smsc::sms::SMS& existentSMS, int num);

	/**
	 * Корректное изменение статуса SM.
	 * Сохраняет в базу правильное SMS сообщение, затем корректно изменяет статус SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool setCorrectSMStatus();

	/**
	 * Некорректное изменение статуса SM.
	 * Сохраняет в базу правильное SMS сообщение, затем пытается выставить некорректный статус SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool setIncorrectSMStatus();

	/**
	 * Изменение статуса несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool setNonExistentSMStatus();

	/**
	 * Корректное обновление существующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool updateCorrectExistentSM();

	/**
	 * Обновление существующего SM некорректными данными.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool updateIncorrectExistentSM();
	
	/**
	 * Обновление несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool updateNonExistentSM();

	/**
	 * Удаление существующего SM.
	 * Сохраняет в базу правильное SMS сообщение, затем удаляет его.
	 * Только debug информация должна выводиться в лог.
	 */
	bool deleteExistentSM();
	
	/**
	 * Удаление несуществующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool deleteNonExistentSM();

	/**
	 * Удаление существующих SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	bool deleteExistentWaitingSMByNumber();
	
	/**
	 * Удаление несуществующих SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	bool deleteNonExistentWaitingSMByNumber();
	
	/**
	 * Чтение существующего SM.
	 * Сохраняет в базу правильное SMS сообщение, затем загружает его.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadExistentSM(smsc::sms::SMSId id, smsc::sms::SMS& sms);

	/**
	 * Чтение несуществующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadNonExistentSM();

	/**
	 * Загрузка непустого списка SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка пустого списка SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка непустого архива SM доставленных на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка пустого архива SM доставленных на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка непустого архива SM доставленных с определенного номера.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * Загрузка пустого архива SM доставленных с определенного номера.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * Просмотр непустой статистики причин недоставки сообщений.
	 * Только debug информация должна выводиться в лог.
	 */
	bool getExistentSMDeliveryFailureStatistics();

	/**
	 * Просмотр пустой статистики причин недоставки сообщений.
	 * Только debug информация должна выводиться в лог.
	 */
	bool getNonExistentSMDeliveryFailureStatistics();

	/**
	 * Создание записи для начисления оплаты.
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
