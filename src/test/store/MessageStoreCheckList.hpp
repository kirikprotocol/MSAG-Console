#ifndef TEST_STORE_MESSAGE_STORE_CHECK_LIST
#define TEST_STORE_MESSAGE_STORE_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace store {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class MessageStoreCheckList : public CheckList
{
public:
	MessageStoreCheckList()
		: CheckList("Результаты функционального тестирования Message Store", "msg_store.chk")
	{
		//storeCorrectSms
		__reg_tc__("storeCorrectSms",
			"Сохранение sms с правильными значениями полей");
		__reg_tc__("storeCorrectSms.origAddrMarginalLength",
			"Предельные значения длины адреса отправителя");
		__reg_tc__("storeCorrectSms.destAddrMarginalLength",
			"Предельные значения длины адреса получателя");
		__reg_tc__("storeCorrectSms.dealiasedDestAddrMarginalLength",
			"Предельные значения длины деалиасенного адреса получателя");
		__reg_tc__("storeCorrectSms.origDescMarginal",
			"Предельные значения imsi и msc адресов в дескрипторе отправителя");
		__reg_tc__("storeCorrectSms.bodyEmpty",
			"Пустое тело сообщения");
		__reg_tc__("storeCorrectSms.bodyMaxLength",
			"Поля short_message и message_payload максимальной длины");
		__reg_tc__("storeCorrectSms.serviceTypeMarginal",
			"Предельные значения длины serviceType");
		__reg_tc__("storeCorrectSms.rcptMsgIdMaxLength",
			"Поле receipted_message_id максимальной длины");
		__reg_tc__("storeCorrectSms.routeIdMarginal",
			"Предельные значения длины routeId");
		__reg_tc__("storeCorrectSms.sourceSmeIdMarginal",
			"Предельные значения длины sourceSmeId");
		__reg_tc__("storeCorrectSms.destinationSmeIdMarginal",
			"Предельные значения длины destinationSmeId");
		//storeSimilarSms
		__reg_tc__("storeSimilarSms",
			"Сохранение sms с ключевыми параметрами похожими на уже существующий sms");
		__reg_tc__("storeSimilarSms.diffMsgRef",
			"Адреса отправителя и получателя подходят, отличается msgRef");
		__reg_tc__("storeSimilarSms.diffOrigAddr",
			"MsgRef и адрес получателя подходят, отличается адрес отправителя");
		__reg_tc__("storeSimilarSms.diffOrigAddr.diffType",
			"Отличается тип адреса отправителя");
		__reg_tc__("storeSimilarSms.diffOrigAddr.diffPlan",
			"Отличается план адреса отправителя");
		__reg_tc__("storeSimilarSms.diffOrigAddr.diffValue",
			"Отличается строка адреса отправителя");
		__reg_tc__("storeSimilarSms.diffDestAddr",
			"MsgRef и адрес отправителя подходят, отличается адреса получателя");
		__reg_tc__("storeSimilarSms.diffDestAddr.diffType",
			"Отличается тип адреса получателя");
		__reg_tc__("storeSimilarSms.diffDestAddr.diffPlan",
			"Отличается план адреса получателя");
		__reg_tc__("storeSimilarSms.diffDestAddr.diffValue",
			"Отличается строка адреса получателя");
		//other
		__reg_tc__("storeDuplicateSms",
			"Сохранение дублированного sms");
		__reg_tc__("storeRejectDuplicateSms",
			"Сохранение дублированного sms с отказом");
		__reg_tc__("storeReplaceCorrectSms",
			"Сохранение корректного sms с замещением уже существующего");
		__reg_tc__("storeReplaceSmsInFinalState",
			"Сохранение sms с попыткой заместить существующее sms в финальном состоянии");
		__reg_tc__("storeIncorrectSms",
			"Сохранение неправильного sms (message store не выполняет проверки правильности полей sms)");
		//storeAssertSms
		__reg_tc__("storeAssertSms",
			"Сохранение неправильного sms с проверкой на assert");
		__reg_tc__("storeAssertSms.destAddrMarginal",
			"Недопустимые значения длины адреса получателя");
		__reg_tc__("storeAssertSms.dealiasedDestAddrMarginal",
			"Недопустимые значения длины деалиасенного адреса получателя");
		__reg_tc__("storeAssertSms.origAddrMarginal",
			"Недопустимые значения длины адреса отправителя");
		__reg_tc__("storeAssertSms.origDescMscAddrGreaterMaxLength",
			"Длина строки адреса msc в дескрипторе отправителя больше максимальной");
		__reg_tc__("storeAssertSms.origDescImsiAddrGreaterMaxLength",
			"Длина строки адреса imsi в дескрипторе отправителя больше максимальной");
		__reg_tc__("storeAssertSms.descMscAddrGreaterMaxLength",
			"Длина строки адреса msc в дескрипторе больше максимальной");
		__reg_tc__("storeAssertSms.descImsiAddrGreaterMaxLength",
			"Длина строки адреса imsi в дескрипторе больше максимальной");
		__reg_tc__("storeAssertSms.serviceTypeGreaterMaxLength",
			"Длина поля service_type больше максимальной");
		__reg_tc__("storeAssertSms.routeIdGreaterMaxLength",
			"Длина поля routeId больше максимальной");
		__reg_tc__("storeAssertSms.sourceSmeIdGreaterMaxLength",
			"Длина поля sourceSmeId больше максимальной");
		__reg_tc__("storeAssertSms.destinationSmeIdGreaterMaxLength",
			"Длина поля destinationSmeId больше максимальной");
		/*
		__reg_tc__("storeAssertSms.smGreaterMaxLength",
			"Длина поля short_message больше максимальной");
		__reg_tc__("storeAssertSms.payloadGreaterMaxLength",
			"Длина поля message_payload больше максимальной");
		__reg_tc__("storeAssertSms.rcptMsgIdGreaterMaxLength",
			"Длина поля receipted_message_id больше максимальной");
		*/
		//changeExistentSmsStateEnrouteToEnroute
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute",
			"Обновление статуса sms в состоянии ENROUTE (при повторной доставке)");
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute.destDescMarginal",
			"Предельные значения длины imsi и msc адресов в дескрипторе получателя");
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute.nextTimePast",
			"Поле nextTryTime в прошлом времени");
		__reg_tc__("changeExistentSmsStateEnrouteToEnroute.lastResultZero",
			"Не задано значение lastResult");
		//changeExistentSmsStateEnrouteToFinal
		__reg_tc__("changeExistentSmsStateEnrouteToFinal",
			"Перевод sms из ENROUTE в финальное состояние");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateDeliveredDestDescMarginal",
			"Перевод в DELIVERED состояние, предельные значения imsi и msc адресов в дескрипторе получателя");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableDestDescMarginal",
			"Перевод в UNDELIVERABLE состояние, предельные значения imsi и msc адресов в дескрипторе получателя");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateUndeliverableLastResultEmpty",
			"Перевод в UNDELIVERABLE состояние, не задано значение lastResult");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateExpired",
			"Перевод в EXPIRED состояние");
		__reg_tc__("changeExistentSmsStateEnrouteToFinal.stateDeleted",
			"Перевод в DELETED состояние");
		//changeFinalSmsStateToAny
		__reg_tc__("changeFinalSmsStateToAny",
			"Перевод несуществующего sms или sms в финальном состоянии в любое другое состояние");
		__reg_tc__("changeFinalSmsStateToAny.stateEnrote",
			"Перевод в ENROUTE состояние");
		__reg_tc__("changeFinalSmsStateToAny.stateDelivered",
			"Перевод в DELIVERED состояние");
		__reg_tc__("changeFinalSmsStateToAny.stateUndeliverable",
			"Перевод в UNDELIVERABLE состояние");
		__reg_tc__("changeFinalSmsStateToAny.stateExpired",
			"Перевод в EXPIRED состояние");
		__reg_tc__("changeFinalSmsStateToAny.stateDeleted",
			"Перевод в DELETED состояние");
		//replaceCorrectSms
		__reg_tc__("replaceCorrectSms",
			"Корректное обновление существующего sms");
		__reg_tc__("replaceCorrectSms.replaceAll",
			"Заменить все возможные поля");
		__reg_tc__("replaceCorrectSms.nextTimeUnchanged",
			"Оставить schedule_delivery_time без изменений (установить равным 0)");
		__reg_tc__("replaceCorrectSms.validTimeUnchanged",
			"Оставить validity_period без изменений (установить равным 0)");
		__reg_tc__("replaceCorrectSms.smMarginal",
			"Предельные значения длины тела сообщения");
		//replaceIncorrectSms
		__reg_tc__("replaceIncorrectSms",
			"Некорректное обновление существующего или обновление несуществующего sms");
		__reg_tc__("replaceIncorrectSms.diffType",
			"Отличие в типе адреса отправителя");
		__reg_tc__("replaceIncorrectSms.diffPlan",
			"Отличие в плане адреса отправителя");
		__reg_tc__("replaceIncorrectSms.diffValue",
			"Отличие в строке адреса отправителя");
		__reg_tc__("replaceIncorrectSms.replaceFinalSms",
			"Обновление несуществующего sms или sms в финальном состоянии");
		//loadSms
		__reg_tc__("loadSms", "Чтение sms из базы");
		__reg_tc__("loadSms.existentSms", "Чтение существующего sms");
		__reg_tc__("loadSms.nonExistentSms", "Чтение несуществующего sms");
		//deleteSms
		__reg_tc__("deleteSms",
			"Удаление sms из базы (в реальной работе сообщения удаляются архиватором)");
		__reg_tc__("deleteSms.existentSms",
			"Удаление существующего sms");
		__reg_tc__("deleteSms.nonExistentSms",
			"Удаление несуществующего sms");
		//checkReadyForRetrySms
		__reg_tc__("checkReadyForRetrySms",
			"Получение sms для повторной доставки");
		__reg_tc__("checkReadyForRetrySms.nextRetryTime",
			"Ближайшее время следующей доставки sms вычисляется правильно");
		__reg_tc__("checkReadyForRetrySms.readyForRetrySmsList",
			"Список sms подлежащих повторной доставке полный и правильно упорядочен");
		//checkReadyForDeliverySms
		__reg_tc__("checkReadyForDeliverySms",
			"Список sms ожидающих повторной доставки на заданный адрес получателя");
		__reg_tc__("checkReadyForDeliverySms.noSms",
			"Нет sms для заданного адреса");
		__reg_tc__("checkReadyForDeliverySms.noAttempts",
			"Sms для заданного адреса существуют, но attempt count = 0");
		__reg_tc__("checkReadyForDeliverySms.finalState",
			"Sms для заданного адреса существуют, но в финальном состоянии");
		__reg_tc__("checkReadyForDeliverySms.allOk",
			"Sms для заданного адреса существуют, находятся в состоянии ENROUTE и attempt count > 0");
		//checkReadyForCancelSms
		__reg_tc__("checkReadyForCancelSms",
			"Списка sms для выполнения операции cancel");
		__reg_tc__("checkReadyForCancelSms.noSmsWithoutServiceType",
			"Нет sms удовлетворяющих условиям, serviceType не задан");
		__reg_tc__("checkReadyForCancelSms.finalStateWithoutServiceType",
			"Есть sms удовлетворяющие условиям, но в финальном состоянии, serviceType не задан");
		__reg_tc__("checkReadyForCancelSms.allOkWithoutServiceType",
			"Есть sms удовлетворяющие условиям, в состоянии ENROUTE, serviceType не задан");
		__reg_tc__("checkReadyForCancelSms.noSmsWithServiceType",
			"Нет sms удовлетворяющих условиям, serviceType задан");
		__reg_tc__("checkReadyForCancelSms.finalStateWithServiceType",
			"Есть sms удовлетворяющие условиям, но в финальном состоянии, serviceType задан");
		__reg_tc__("checkReadyForCancelSms.allOkWithServiceType",
			"Есть sms удовлетворяющие условиям, в состоянии ENROUTE, serviceType задан");
		//changeSmsConcatSequenceNumber
		__reg_tc__("changeSmsConcatSequenceNumber",
			"Изменение ConcatSequenceNumber атрибута sms");
		__reg_tc__("changeSmsConcatSequenceNumber.correct",
			"Сообщение находится в ENROUTE состоянии и установлен опциональный атрибут SMSC_CONCATINFO");
		__reg_tc__("changeSmsConcatSequenceNumber.noConcatInfo",
			"Сообщение находится в ENROUTE состоянии, но опциональный атрибут SMSC_CONCATINFO не установлен");
		__reg_tc__("changeSmsConcatSequenceNumber.finalState",
			"Сообщение находится в финальном состоянии");
		//getConcatInitInfo
		__reg_tc__("getConcatInitInfo",
			"Получение данных для инициализации механизмов обработки конкатенированных сообщений");
		//прочее
		//not implemented
		/*
		__reg_tc__("deleteExistentWaitingSMByNumber",
			"Удаление существующих sms ожидающих доставки на определенный номер");
		__reg_tc__("deleteNonExistentWaitingSMByNumber",
			"Удаление несуществующих sms ожидающих доставки на определенный номер");
		__reg_tc__("loadExistentWaitingSMByDestinationNumber",
			"Загрузка непустого списка sms ожидающих доставки на определенный номер");
		__reg_tc__("loadNonExistentWaitingSMByDestinationNumber",
			"Загрузка пустого списка sms ожидающих доставки на определенный номер");
		__reg_tc__("loadExistentSMArchieveByDestinationNumber",
			"Загрузка непустого архива sms доставленных на определенный номер");
		__reg_tc__("loadNonExistentSMArchieveByDestinationNumber",
			"Загрузка пустого архива sms доставленных на определенный номер");
		__reg_tc__("loadExistentSMArchieveByOriginatingNumber",
			"Загрузка непустого архива sms доставленных с определенного номера");
		__reg_tc__("loadNonExistentSMArchieveByOriginatingNumber",
			"Загрузка пустого архива sms доставленных с определенного номера");
		__reg_tc__("getExistentSMDeliveryFailureStatistics",
			"Просмотр непустой статистики причин недоставки сообщений");
		__reg_tc__("getNonExistentSMDeliveryFailureStatistics",
			"Просмотр пустой статистики причин недоставки сообщений");
		*/
	}
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_CHECK_LIST */

