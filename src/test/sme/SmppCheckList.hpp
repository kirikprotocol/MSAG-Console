#ifndef TEST_SME_SMPP_CHECK_LIST
#define TEST_SME_SMPP_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmppCheckList : public CheckList
{

void bindCorrectSmeTc()
{
	__reg_tc__("bindCorrectSme",
		"Установление IP соединения с SC и регистрация");
	__reg_tc__("bindCorrectSme.bindReceiver",
		"Регистрация ресивера с помощью bind_receiver pdu");
	__reg_tc__("bindCorrectSme.bindTransmitter",
		"Регистрация трансмиттера с помощью bind_transmitter pdu");
	__reg_tc__("bindCorrectSme.bindTransceiver",
		"Регистрация трансивера с помощью bind_transceiver pdu");
}

void bindIncorrectSmeTc()
{
	__reg_tc__("bindIncorrectSme",
		"Установление IP соединения с SC и регистрация с неправильными параметрами");
	__reg_tc__("bindIncorrectSme.smeNotRegistered",
		"Значение system_id не прописано в конфигурации SC");
	__reg_tc__("bindIncorrectSme.unknownHost",
		"Установка соединения с недоступным SC (неизвестный хост) завершается корректно");
	__reg_tc__("bindIncorrectSme.invalidPort",
		"Установка соединения с недоступным SC (неправильный порт) завершается корректно");
}

void unbindTc()
{
	__reg_tc__("unbind", "Завершение соединения с SC");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "Отправка submit_sm pdu");
	__reg_tc__("submitSm.sync", "Отправка синхронного submit_sm pdu");
	__reg_tc__("submitSm.async", "Отправка асинхронного submit_sm pdu");
	//submitSm.correct
	__reg_tc__("submitSm.correct",
		"Отправка submit_sm pdu с правильными значениями полей");
	__reg_tc__("submitSm.correct.serviceTypeMarginal",
		"Предельные значения длины поля service_type");
	__reg_tc__("submitSm.correct.waitTimePast",
		"Время schedule_delivery_time в прошлом (доставка уже должна была начаться)");
	__reg_tc__("submitSm.correct.validTimeExceeded",
		"Время validity_period больше максимально срока валидности установленного для SC");
	__reg_tc__("submitSm.correct.waitTimeImmediate",
		"Установить немедленную доставку сообщения (schedule_delivery_time = NULL)");
	__reg_tc__("submitSm.correct.validTimeDefault",
		"Установить срок валидности по умолчанию (validity_period = NULL)");
	__reg_tc__("submitSm.correct.smLengthMarginal",
		"Предельные значения длины тела сообщения");
	__reg_tc__("submitSm.correct.checkMap",
		"Дублированное сообщение согласно MAP (совпадают source_addr, dest_addr и user_message_reference), но согласно SMPP существующее сообщение находящееся в очереди доставки замещено быть не может");
	__reg_tc__("submitSm.correct.notReplace",
		"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) без замещения существующего, находящегося в очереди доставки");
	__reg_tc__("submitSm.correct.serviceTypeNotMatch",
		"Совпадают source_addr и dest_addr, отличается service_type для сообщения находящегося в очереди доставки");
	__reg_tc__("submitSm.correct.replaceEnrote",
		"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением существующего, находящегося в очереди доставки");
	__reg_tc__("submitSm.correct.replaceReplacedEnrote",
		"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением уже ранее замещенного и находящегося в очереди доставки");
	__reg_tc__("submitSm.correct.replaceFinal",
		"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением уже доставленного");
	__reg_tc__("submitSm.correct.replaceRepeatedDeliveryEnrote",
		"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением существующего, находящегося в очереди повторной доставки");
	//submitSm.incorrect
	__reg_tc__("submitSm.incorrect",
		"Отправка submit_sm pdu с неправильными значениями полей");
	__reg_tc__("submitSm.incorrect.sourceAddr",
		"Неправильный адрес отправителя");
	__reg_tc__("submitSm.incorrect.destAddr",
		"Неправильный адрес получателя");
	__reg_tc__("submitSm.incorrect.validTimeFormat",
		"Неправильный формат validity_period");
	__reg_tc__("submitSm.incorrect.waitTimeFormat",
		"Неправильный формат schedule_delivery_time");
	__reg_tc__("submitSm.incorrect.validTimePast",
		"Время validity_period в прошлом (сообщение никогда не будет доставлено)");
	__reg_tc__("submitSm.incorrect.waitTimeInvalid1",
		"Значение schedule_delivery_time больше validity_period");
	__reg_tc__("submitSm.incorrect.waitTimeInvalid2",
		"Значение schedule_delivery_time больше максимального validity_period для SC, срок валидности по умолчанию (validity_period = NULL)");
	__reg_tc__("submitSm.incorrect.waitTimeInvalid3",
		"Значения schedule_delivery_time меньше validity_period, но оба больше максимального validity_period для SC");
	//submitSm.assert
	__reg_tc__("submitSm.assert",
		"Заполнение и отправка submit_sm pdu с недопустимыми значениями полей");
	__reg_tc__("submitSm.assert.serviceTypeInvalid",
		"Недопустимое значение длины поля service_type");
	__reg_tc__("submitSm.assert.destAddrLengthInvalid",
		"Недопустимое значение длины поля адреса получателя");
	__reg_tc__("submitSm.assert.msgLengthInvalid",
		"Недопустимое значение длины текста сообщения short_message");
}

void replaceSmTc()
{
	__reg_tc__("replaceSm",
		"Отправка replace_sm pdu с правильными значениями полей");
	__reg_tc__("replaceSm.waitTimePast",
		"Время schedule_delivery_time в прошлом (доставка уже должна была начаться)");
	__reg_tc__("replaceSm.validTimePast",
		"Время validity_period в прошлом (сообщение никогда не будет доставлено)");
	__reg_tc__("replaceSm.validTimeExceeded",
		"Время validity_period больше максимально срока валидности установленного для SC");
	__reg_tc__("replaceSm.waitTimeInvalid",
		"Значение schedule_delivery_time превышает validity_period");
	__reg_tc__("replaceSm.smLengthMarginal",
		"Предельные значения длины тела сообщения short_message");
	__reg_tc__("replaceSm.msgIdNotExist",
		"Поле message_id задает несуществующее сообщение");
	__reg_tc__("replaceSm.replaceReplacedEnrote",
		"Замещение уже ранее замещенного сообщения и находящегося в очереди доставки");
	__reg_tc__("replaceSm.replaceFinal",
		"Замещение уже доставленного сообщения");
	__reg_tc__("replaceSm.replaceRepeatedDeliveryEnrote",
	   "Замещение сообщения находящегося в очереди повторной доставки");
	__reg_tc__("replaceSm.sync",
		"Отправка синхронного replace_sm pdu");
	__reg_tc__("replaceSm.async",
		"Отправка асинхронного replace_sm pdu");
}

void updateProfileTc()
{
	__reg_tc__("updateProfile",
		"Отправка сообщений менеджеру профилей");
	__reg_tc__("updateProfile.cmdTextDefault",
		"Текст команды в дефолтной кодировке SC");
	__reg_tc__("updateProfile.cmdTextUcs2",
		"Текст команды в кодировке UCS2");
	__reg_tc__("updateProfile.reportOptions",
		"Изменение опций уведомления о доставке");
	__reg_tc__("updateProfile.reportOptions.reportNoneMixedCase",
		"Отказаться от уведомления о доставке (текст команды в смешанном регистре)");
	__reg_tc__("updateProfile.reportOptions.reportNoneSpaces",
		"Отказаться от уведомления о доставке (текст команды с лишними пробелами)");
	__reg_tc__("updateProfile.reportOptions.reportFullMixedCase",
		"Установить прием уведомлений о доставке (текст команды в смешанном регистре)");
	__reg_tc__("updateProfile.reportOptions.reportFullSpaces",
		"Установить прием уведомлений о доставке (текст команды с лишними пробелами)");
	__reg_tc__("updateProfile.dataCoding",
		"Изменение режима приема сообщений");
	__reg_tc__("updateProfile.dataCoding.ucs2CodepageMixedCase",
		"Установить режим приема сообщений на русском языке (кодировка ucs2, текст команды в смешанном регистре)");
	__reg_tc__("updateProfile.dataCoding.ucs2CodepageSpaces",
		"Установить режим приема сообщений на русском языке (кодировка ucs2, текст команды с лишними пробелами)");
	__reg_tc__("updateProfile.dataCoding.defaultCodepageMixedCase",
		"Отказаться от режима приема сообщений на русском языке (текст команды в смешанном регистре)");
	__reg_tc__("updateProfile.dataCoding.defaultCodepageSpaces",
		"Отказаться от режима приема сообщений на русском языке (текст команды с лишними пробелами)");
	__reg_tc__("updateProfile.incorrectCmdText",
		"Неправильный текст команды");
}

void processRespTc()
{
	//processSubmitSmResp
	__reg_tc__("processSubmitSmResp",
		"Получение submit_sm_resp pdu");
	__reg_tc__("processSubmitSmResp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id)");
	__reg_tc__("processSubmitSmResp.sync",
		"Получение submit_sm_resp pdu при синхронных submit_sm запросах");
	__reg_tc__("processSubmitSmResp.async",
		"Получение submit_sm_resp pdu при асинхронных submit_sm запросах");
	//processReplaceSmResp
	__reg_tc__("processReplaceSmResp",
		"Получение replace_sm_resp pdu");
	__reg_tc__("processReplaceSmResp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id)");
	__reg_tc__("processReplaceSmResp.sync",
		"Получение replace_sm_resp pdu при синхронных replace_sm запросах");
	__reg_tc__("processReplaceSmResp.async",
		"Получение replace_sm_resp pdu при асинхронных replace_sm запросах");
	//processResp
	__reg_tc__("processResp",
		"Общие проверки для submit_sm_resp, replace_sm_resp и data_sm_resp");
	__reg_tc__("processResp.checkHeader",
		"Правильные значения полей хедера респонса (sequence_number)");
	__reg_tc__("processResp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("processResp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("processResp.checkDelivery",
		"При наличии кода ошибки в поле command_status, доставки deliver_sm не происходит");
	__reg_tc__("processResp.checkDeliveryReceipt",
		"При наличии кода ошибки в поле command_status, подтверждение доставки не происходит");
	__reg_tc__("processResp.checkIntermediateNotification",
		"При наличии кода ошибки в поле command_status, промежуточные нотификации не происходят");
	__reg_tc__("processResp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status, выполняются все условия для нормальной доставки сообщения (поля реквеста заданы корректно, существует маршрут и т.п.)");
	__reg_tc__("processResp.checkCmdStatusInvalidDestAddr",
		"Если код ошибки ESME_RINVDSTADR в поле command_status, то маршрута действительно не существует");
	__reg_tc__("processResp.checkCmdStatusInvalidWaitTime",
		"Если код ошибки ESME_RINVSCHED в поле command_status, то время schedule_delivery_time действительно задано неправильно");
	__reg_tc__("processResp.checkCmdStatusInvalidValidTime",
		"Если код ошибки ESME_RINVEXPIRY в поле command_status, то время validity_period действительно задано неправильно");
	__reg_tc__("processResp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void sendDeliverySmRespTc()
{
	__reg_tc__("sendDeliverySmResp", "Отправка deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.sync",
		"Отправка синхронного deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.async",
		"Отправка асинхронного deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.sendOk",
		"Отправить респонс подтверждающий получение правильного deliver_sm");
	//sendDeliverySmResp.sendRetry
	__reg_tc__("sendDeliverySmResp.sendRetry",
		"Отправить респонс для форсирования повторной доставки deliver_sm");
	__reg_tc__("sendDeliverySmResp.sendRetry.notSend",
		"Не отпралять респонс, имитация неполучения deliver_sm");
	__reg_tc__("sendDeliverySmResp.sendRetry.tempAppError",
		"Отправить статус ESME_RX_T_APPN (временная ошибка на стороне sme, запрос на повторную доставку)");
	__reg_tc__("sendDeliverySmResp.sendRetry.msgQueueFull",
		"Отправить статус ESME_RMSGQFUL (переполнение очереди стороне sme)");
	__reg_tc__("sendDeliverySmResp.sendRetry.invalidSequenceNumber",
		"Отправить респонс с неправильным sequence_number");
	//sendDeliverySmResp.sendError
	__reg_tc__("sendDeliverySmResp.sendError",
		"Отправить респонс для прекращения повторной доставки deliver_sm");
	__reg_tc__("sendDeliverySmResp.sendError.standardError",
		"Респонс со стандартным кодом ошибки (диапазон 0x1-0x10f)");
	__reg_tc__("sendDeliverySmResp.sendError.reservedError",
		"Респонс с зарезирвированным кодом ошибки (диапазоны 0x110-0x3ff и 0x400-0x4ff)");
	__reg_tc__("sendDeliverySmResp.sendError.outRangeError",
		"Респонс с кодом ошибки вне диапазона определенного спецификацией SMPP (>0x500)");
	__reg_tc__("sendDeliverySmResp.sendError.permanentAppError",
		"Респонс с кодом ошибки ESME_RX_P_APPN (неустранимая ошибка на стороне sme, отказ от всех последующих сообщений)");
}

void processDeliverySmTc()
{
	__reg_tc__("processDeliverySm", "Получение deliver_sm pdu");
	__reg_tc__("processDeliverySm.checkFields",
		"Общие проверки полей хедера и некоторых других полей");
}

void processNormalSmsTc()
{
	__reg_tc__("processDeliverySm.normalSms",
		"Обычная sms не являющаяся нотификацией");
	__reg_tc__("processDeliverySm.normalSms.checkAllowed",
		"Проверка правомерности получения sms");
	__reg_tc__("processDeliverySm.normalSms.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и алиасу получателя)");
	__reg_tc__("processDeliverySm.normalSms.checkMandatoryFields",
		"Сравнение обязательных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm) pdu");
	__reg_tc__("processDeliverySm.normalSms.checkDataCoding",
		"Кодировка текста sms соответствует настройкам профиля получателя");
	__reg_tc__("processDeliverySm.normalSms.checkTextEqualDataCoding",
		"Для одинаковых кодировок в профилях отправителя и получателя текст sms совпадает");
	__reg_tc__("processDeliverySm.normalSms.checkTextDiffDataCoding",
		"Для различных кодировок в профилях отправителя и получателя текст sms правильно преобразуется из одной кодировки в другую");
	__reg_tc__("processDeliverySm.normalSms.checkOptionalFields",
		"Сравнение опциональных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm) pdu");
	__reg_tc__("processDeliverySm.normalSms.scheduleChecks",
		"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
	__reg_tc__("processDeliverySm.normalSms.checkDeliveryReceipt",
		"Подтверждение доставки доставляется не раньше самой sms");
}

void processDeliveryReceiptTc()
{
	__reg_tc__("processDeliverySm.deliveryReceipt",
		"Подтверждение доставки");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkAllowed",
		"Проверка правомерности получения подтверждения доставки");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkFields",
		"Общая проверка правильности полей");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkFieldsExt",
		"Проверка правильности полей в с учетом запрошенных опций (none, final, failure) подтверждения и настроек профиля на момент отправки оригинальной pdu");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkState",
		"Информации о статусе доставленной pdu является корректной");
	__reg_tc__("processDeliverySm.deliveryReceipt.checkErrorCode",
		"Информации о коде ошибке в случае недоставки pdu является корректной");
	__reg_tc__("processDeliverySm.deliveryReceipt.scheduleChecks",
		"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
}

void processSmeAckTc()
{
	__reg_tc__("processDeliverySm.smeAck",
		"Ответное сообщение от внутренних sme SC (менеджер профилей, db sme и т.п.)");
	__reg_tc__("processDeliverySm.smeAck.checkAllowed",
		"Проверка правомерности получения сообщения");
	__reg_tc__("processDeliverySm.smeAck.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу внутренней sme SC)");
	__reg_tc__("processDeliverySm.smeAck.checkFields",
		"Общая проверка правильности полей");
	__reg_tc__("processDeliverySm.smeAck.scheduleChecks",
		"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
}

void processIntermediateNotificationTc()
{
	__reg_tc__("processDeliverySm.intermediateNotification",
		"Промежуточные нотификации");
	__reg_tc__("processDeliverySm.intermediateNotification.checkAllowed",
		"Проверка правомерности получения подтверждения промежуточной нотификации");
	__reg_tc__("processDeliverySm.intermediateNotification.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
	__reg_tc__("processDeliverySm.intermediateNotification.checkFields",
		"Общая проверка правильности полей");
	/*
	__reg_tc__("processDeliverySm.intermediateNotification.checkProfile",
		"Проверка соответствия настройкам профиля");
	__reg_tc__("processDeliverySm.intermediateNotification.checkStatus",
		"Информации о состоянии отправленной pdu является корректной");
	*/
	__reg_tc__("processDeliverySm.intermediateNotification.checkTime",
		"Правильное время доставки");
}

void processUpdateProfileTc()
{
	__reg_tc__("processUpdateProfile",
		"Получение сообщений от менеджера профилей");
	__reg_tc__("processUpdateProfile.reportOptions",
		"Подтверждения об изменении настроек получения уведомлений о доставке");
	__reg_tc__("processUpdateProfile.codePage",
		"Подтверждения об изменении настроек режима приема сообщений на русском языке");
	__reg_tc__("processUpdateProfile.incorrectCmdText",
		"Уведомление о неправильном тексте команды");
}

void checkMissingPduTc()
{
	__reg_tc__("checkMissingPdu",
		"Все отправленные pdu доставляются и не теряются");
	//response
	__reg_tc__("checkMissingPdu.response",
		"На все реквесты приходят респонсы (не теряются) с фиксированной задержкой с момента отправки реквеста");
	//delivery
	__reg_tc__("checkMissingPdu.delivery",
		"Сообщения доставляются (не теряются) в интервале с schedule_delivery_time по validity_period с фиксированной задержкой");
	__reg_tc__("checkMissingPdu.delivery.waitTime",
		"Сообщения с отложенной доставкой и доступным получателем доставляются (не теряются) в момент времени schedule_delivery_time");
	__reg_tc__("checkMissingPdu.delivery.validTime",
		"При недоступном получателе повторная доставка сообщения продолжается вплоть до момент времени validity_period");
	//deliveryReceipt
	__reg_tc__("checkMissingPdu.deliveryReceipt",
		"Подтерждения доставки доставляются (не теряются) в интервале с schedule_delivery_time по validity_period с фиксированной задержкой");
	__reg_tc__("checkMissingPdu.deliveryReceipt.waitTime",
		"Подтерждения доставки при доступном получателе сообщения доставляются (не теряются) в момент времени schedule_delivery_time");
	__reg_tc__("checkMissingPdu.deliveryReceipt.validTime",
		"При недоступном получателе подтерждение доставки (недоставки) сообщения доставляется отправителю в момент времени validity_period");
	//intermediateNotification
	__reg_tc__("checkMissingPdu.intermediateNotification",
		"Промежуточные нотификации доставляются в интервале с submit_time по validity_period с фиксированной задержкой");
	__reg_tc__("checkMissingPdu.intermediateNotification.waitTime",
		"При доступном получателе сообщения на момент времени schedule_delivery_time должна доставляться промежуточная нотификация");
	__reg_tc__("checkMissingPdu.intermediateNotification.validTime",
		"При недоступном получателе промежуточные нотификации доставляются вплоть до момента времени validity_period");
}

void notImplementedTc()
{
	__reg_tc__("processGenericNack", "Получение generic_nack pdu");
	__reg_tc__("submitMulti", "Отправка submit_multi pdu");
	__reg_tc__("processMultiResp", "Получение submit_multi_resp pdu");
	__reg_tc__("dataSm", "Отправка data_sm pdu");
	__reg_tc__("processDataSm", "Получение data_sm pdu");
	__reg_tc__("processDataSmResp", "Получение data_sm_resp pdu");
	__reg_tc__("querySm", "Отправка query_sm pdu");
	__reg_tc__("processQuerySmResp", "Получение query_sm_resp pdu");
	__reg_tc__("cancelSm", "Отправка cancel_sm pdu");
	__reg_tc__("processCancelSmResp", "Получение cancel_sm_resp pdu");
	__reg_tc__("processAlertNotification", "Получение alert_notification pdu");
}

public:
	SmppCheckList()
		: CheckList("Результаты функционального тестирования BaseSme и протокола SMPP", "smpp.chk")
	{
		//transmitter
		bindCorrectSmeTc();
		bindIncorrectSmeTc();
		unbindTc();
		submitSmTc();
		replaceSmTc();
		sendDeliverySmRespTc();
		//receiver
		processRespTc();
		processDeliverySmTc();
		processNormalSmsTc();
		processDeliveryReceiptTc();
		processSmeAckTc();
		processIntermediateNotificationTc();
		//profiler
		updateProfileTc();
		processUpdateProfileTc();
		//other
		checkMissingPduTc();
		notImplementedTc();
	}
};

}
}
}

#endif /* TEST_SME_SMPP_CHECK_LIST */

