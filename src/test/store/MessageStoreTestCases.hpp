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
	TCResult* replaceCorrectSM(smsc::sms::SMSId id, smsc::sms::SMS& sms, int num);

	/**
	 * Обновление существующего SM некорректными данными.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* replaceIncorrectSM(smsc::sms::SMSId id, smsc::sms::SMS& sms, int num);
	
	/**
	 * Обновление несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	TCResult* replaceNonExistentSM(smsc::sms::SMSId id, int num);

	/**
	 * Удаление существующего SM.
	 * Сохраняет в базу правильное SMS сообщение, затем удаляет его.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* deleteExistentSM(smsc::sms::SMSId id);
	
	/**
	 * Удаление несуществующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	TCResult* deleteNonExistentSM(smsc::sms::SMSId id, int num);

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
	TCResult* loadNonExistentSM(smsc::sms::SMSId id, int num);

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

	void clearSM(smsc::sms::SMS& sms);
	void setupRandomCorrectSM(smsc::sms::SMS& sms);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

