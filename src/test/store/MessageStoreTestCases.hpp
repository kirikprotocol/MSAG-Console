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

const std::string TC_STORE_CORRECT_SM = "storeCorrectSM";
const std::string TC_STORE_INCORRECT_SM = "storeIncorrectSM";
const std::string TC_STORE_ASSERT_SM = "storeAssertSM";
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
const std::string TC_LOAD_NON_EXISTENT_SM = "loadNonExistentSM";
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
	MessageStoreTestCases();

	/**
	 * Деинициализирует подсистему Message Store.
	 */
	~MessageStoreTestCases();

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
	 * Сохранение неправильного SM с проверкой на assert.
	 */
	TCResult* storeAssertSM(int num);

	/**
	 * Корректное изменение статуса SM.
	 * Сохраняет в базу правильное SMS сообщение, затем корректно изменяет статус SM.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* setCorrectSMStatus();

	/**
	 * Некорректное изменение статуса SM.
	 * Сохраняет в базу правильное SMS сообщение, затем пытается выставить некорректный статус SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* setIncorrectSMStatus();

	/**
	 * Изменение статуса несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* setNonExistentSMStatus();

	/**
	 * Корректное обновление существующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* updateCorrectExistentSM();

	/**
	 * Обновление существующего SM некорректными данными.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* updateIncorrectExistentSM();
	
	/**
	 * Обновление несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* updateNonExistentSM();

	/**
	 * Удаление существующего SM.
	 * Сохраняет в базу правильное SMS сообщение, затем удаляет его.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* deleteExistentSM();
	
	/**
	 * Удаление несуществующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* deleteNonExistentSM();

	/**
	 * Удаление существующих SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* deleteExistentWaitingSMByNumber();
	
	/**
	 * Удаление несуществующих SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* deleteNonExistentWaitingSMByNumber();
	
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
	TCResult* loadExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка пустого списка SM ожидающих доставки на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка непустого архива SM доставленных на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка пустого архива SM доставленных на определенный номер.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка непустого архива SM доставленных с определенного номера.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * Загрузка пустого архива SM доставленных с определенного номера.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * Просмотр непустой статистики причин недоставки сообщений.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* getExistentSMDeliveryFailureStatistics();

	/**
	 * Просмотр пустой статистики причин недоставки сообщений.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* getNonExistentSMDeliveryFailureStatistics();

	/**
	 * Создание записи для начисления оплаты.
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

