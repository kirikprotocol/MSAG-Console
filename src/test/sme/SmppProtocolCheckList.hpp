#ifndef TEST_SME_SMPP_PROTOCOL_CHECK_LIST
#define TEST_SME_SMPP_PROTOCOL_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmppProtocolCheckList : public CheckList
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
	__reg_tc__("bindCorrectSme.checkCommandStatus",
		"В bind респонсе значение поля command_status равно ESME_ROK");
	__reg_tc__("bindCorrectSme.checkInterfaceVersion",
		"В bind респонсе значение поля sc_interface_version равно 0x34");
}

void bindIncorrectSmeTc()
{
	__reg_tc__("bindIncorrectSme",
		"Установление IP соединения с SC и регистрация с неправильными параметрами");
	__reg_tc__("bindIncorrectSme.systemIdNotRegistered",
		"Значение system_id не прописано в конфигурации SC");
	__reg_tc__("bindIncorrectSme.invalidPassword",
		"Неправильный пароль");
	__reg_tc__("bindIncorrectSme.unknownHost",
		"Установка соединения с недоступным SC (неизвестный хост) завершается корректно");
	__reg_tc__("bindIncorrectSme.invalidPort",
		"Установка соединения с недоступным SC (неправильный порт) завершается корректно");
	__reg_tc__("bindIncorrectSme.invalidSystemIdLength",
		"Длина поля system_id в bind pdu больше максимально допустимой");
	__reg_tc__("bindIncorrectSme.invalidPasswordLength",
		"Длина поля password в bind pdu больше максимально допустимой");
	__reg_tc__("bindIncorrectSme.invalidSystemTypeLength",
		"Длина поля system_type в bind pdu больше максимально допустимой");
	__reg_tc__("bindIncorrectSme.invalidAddressRangeLength",
		"Длина поля address_range в bind pdu больше максимально допустимой");
}

void unbindTc()
{
	__reg_tc__("unbind", "Завершение соединения с SC");
	__reg_tc__("unbind.checkCommandStatus",
		"В unbind респонсе значение поля command_status равно ESME_ROK");
	__reg_tc__("unbind.checkSocketClose",
		"После отправки unbind респонса SC закрывает соединение");
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
	__reg_tc__("submitSm.incorrect.dataCoding",
		"Недопустимые значения dataCoding");
	__reg_tc__("submitSm.incorrect.transactionRollback",
		"Специальные sms эмулирующие transaction rollback на стороне сервера БД");
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

void sendInvalidPduTc()
{
	__reg_tc__("sendInvalidPdu",
		"Отправка некорректной pdu");
	__reg_tc__("sendInvalidPdu.sync",
		"Синхронная отправка pdu");
	__reg_tc__("sendInvalidPdu.async",
		"Асинхронная отправка pdu");
	__reg_tc__("sendInvalidPdu.request",
		"Реквест неправильной длины");
	__reg_tc__("sendInvalidPdu.response",
		"Респонс неправильной длины");
	__reg_tc__("sendInvalidPdu.invalidCommandId",
		"Неправильный command_id");
}

void processRespTc()
{
	//processSubmitSmResp
	__reg_tc__("processSubmitSmResp",
		"Получение submit_sm_resp pdu");
	__reg_tc__("processSubmitSmResp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
	__reg_tc__("processSubmitSmResp.sync",
		"Получение submit_sm_resp pdu при синхронных submit_sm запросах");
	__reg_tc__("processSubmitSmResp.async",
		"Получение submit_sm_resp pdu при асинхронных submit_sm запросах");
	//processReplaceSmResp
	__reg_tc__("processReplaceSmResp",
		"Получение replace_sm_resp pdu");
	__reg_tc__("processReplaceSmResp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
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
	__reg_tc__("processResp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status, выполняются все условия для нормальной доставки сообщения (поля реквеста заданы корректно, существует маршрут и т.п.)");
	__reg_tc__("processResp.checkCmdStatusInvalidDestAddr",
		"Если код ошибки ESME_RINVDSTADR в поле command_status, то маршрута действительно не существует");
	__reg_tc__("processResp.checkCmdStatusInvalidWaitTime",
		"Если код ошибки ESME_RINVSCHED в поле command_status, то время schedule_delivery_time действительно задано неправильно");
	__reg_tc__("processResp.checkCmdStatusInvalidValidTime",
		"Если код ошибки ESME_RINVEXPIRY в поле command_status, то время validity_period действительно задано неправильно");
	__reg_tc__("processResp.checkCmdStatusInvalidDataCoding",
		"Если код ошибки ESME_RINVDCS в поле command_status, то кодировка сообщения действительно задана неправильно");
	__reg_tc__("processResp.checkCmdStatusInvalidServiceType",
		"Если код ошибки ESME_RINVSERTYP в поле command_status, то длина поля service_type действительно задана неправильно");
	__reg_tc__("processResp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то адрес отправителя действительно не соответствует address range для данной sme в конфигурации sme.xml SC");
	__reg_tc__("processResp.checkCmdStatusSystemError",
		"Если код ошибки ESME_RSYSERR в поле command_status, то на стороне SC действительно возникла неустранимая ошибка (transaction rollback при сохранении сообщения)");
	__reg_tc__("processResp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки INVALID_BIND_STATUS в поле command_status, то действительно sme зарегистрированная как receiver отправляет реквесты (submit_sm, replace_sm, cancel_sm и т.п.)");
	__reg_tc__("processResp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void processGenericNackTc()
{
	__reg_tc__("processGenericNack", "Получение generic_nack pdu");
	__reg_tc__("processGenericNack.sync",
		"Exception при некорректных синхронных запросах");
	__reg_tc__("processGenericNack.async",
		"Получение generic_nack pdu при при некорректных асинхронных запросах");
	__reg_tc__("processGenericNack.checkHeader",
		"Правильные значения полей хедера (command_length, command_id, sequence_number)");
	__reg_tc__("processGenericNack.checkTime",
		"Правильное время получения pdu");
	__reg_tc__("processGenericNack.checkStatusInvalidCommandLength",
		"Если код ошибки ESME_RINVCMDLEN в поле command_status, то тип pdu задан корректно, а длина неправильная");
	__reg_tc__("processGenericNack.checkStatusInvalidCommandId",
		"Если код ошибки ESME_RINVCMDID в поле command_status, то тип pdu действительно задан некорректно");
	__reg_tc__("processGenericNack.checkStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void sendDeliverySmRespTc()
{
	__reg_tc__("sendDeliverySmResp", "Отправка deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.sync",
		"Отправка синхронного deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.async",
		"Отправка асинхронного deliver_sm_resp pdu");
	__reg_tc__("sendDeliverySmResp.delay",
		"Отправка deliver_sm_resp pdu с задержкой меньшей sme timeout");
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
	__reg_tc__("sendDeliverySmResp.sendRetry.sendAfterSmeTimeout",
		"Отправить респонс после sme timeout");
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
}

void processDeliveryReportTc()
{
	__reg_tc__("processDeliverySm.deliveryReport",
		"Отчеты о доставке (подтверждения доставки и промежуточные нотификации)");
	__reg_tc__("processDeliverySm.deliveryReport.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
	__reg_tc__("processDeliverySm.deliveryReport.checkFields",
		"Общая проверка правильности полей");
	__reg_tc__("processDeliverySm.deliveryReport.multipleMessages",
		"Длинный текст сообщения о доставки разбивается на несколько sms");
	//deliveryReceipt
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt",
		"Подтверждения доставки");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkAllowed",
		"Проверка правомерности получения подтверждений доставки (в зависимости от настроек профиля и поля pdu registered_delivery, единственный раз по окончании доставки оригинальной pdu)");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.failureDeliveryReceipt",
		"Подтверждение доставки на ошибку не доставляется в случае успешной доставки оригинального сообщения");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.expiredDeliveryReceipt",
		"Подтверждение доставки на ошибку при истечении срока валидности доставляется в момент времени validity period");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.recvTimeChecks",
		"Подтверждения доставки приходят в момент окончании доставки оригинальной pdu");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkStatus",
		"Информации о статусе доставленной pdu (состояние, код ошибки) является корректной");
	__reg_tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkText",
		"Текст сообщения соответствует настройкам SC");
	//intermediateNotification
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification",
		"Промежуточные нотификации");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.checkAllowed",
		"Проверка правомерности получения промежуточных нотификаций (в зависимости от настроек профиля и поля pdu registered_delivery, единственный раз после первой зарешедуленой попытки доставки)");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.noRescheduling",
		"Промежуточная нотификация не доставляется в случае, если sms не была зарешедулена");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.recvTimeChecks",
		"Время доставки промежуточной нотификации соответсвует времени первой зарешедуленой попытки доставки оригинальной pdu");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.checkStatus",
		"Информации о статусе pdu (состояние, код ошибки) является корректной");
	__reg_tc__("processDeliverySm.deliveryReport.intermediateNotification.checkText",
		"Текст сообщения соответствует настройкам SC");
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
	__reg_tc__("processDeliverySm.smeAck.recvTimeChecks",
		"Правильное время получения сообщений");
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

void checkMissingPduTc()
{
	__reg_tc__("checkMissingPdu",
		"Все отправленные pdu доставляются и не теряются");
	//response
	__reg_tc__("checkMissingPdu.response",
		"На все реквесты приходят респонсы (не теряются) с фиксированной задержкой с момента отправки реквеста");
	__reg_tc__("checkMissingPdu.delivery",
		"Сообщения доставляются (не теряются) в интервале с schedule_delivery_time по validity_period с фиксированной задержкой");
	__reg_tc__("checkMissingPdu.deliveryReceipt",
		"Подтерждения доставки доставляются (не теряются) в интервале с момента доставки оригинального сообщения в течение максимального времени валидности сообщений для SC");
	__reg_tc__("checkMissingPdu.intermediateNotification",
		"Промежуточные нотификации доставляются в интервале с submit_time по validity_period");
	__reg_tc__("checkMissingPdu.smeAcknoledgement",
		"Ответные сообщения от тестируемых sme доставляются в интервале с момента доставки оригинального сообщения в течение максимального времени валидности сообщений для SC");
	__reg_tc__("checkMissingPdu.genericNack",
		"GenericNack pdu доставляются (не теряются) с фиксированной задержкой с момента отправки некорректных pdu");
}

void protocolError()
{
	__reg_tc__("protocolError", "Тестирование ошибочных ситуаций");
	//invalidBind
	__reg_tc__("protocolError.invalidBind",
		"Отправка неправильной bind pdu (сразу после установления соединения с SC)");
	__reg_tc__("protocolError.invalidBind.smallerSize1",
		"Если размер pdu меньше 16 байт (размер хедера), SC закрывает соединение");
	__reg_tc__("protocolError.invalidBind.smallerSize2",
		"Если размер pdu меньше правильного, SC закрывает соединение");
	__reg_tc__("protocolError.invalidBind.greaterSize1",
		"Если размер pdu больше правильного, но меньше 64kb, SC закрывает соединение после таймаута");
	__reg_tc__("protocolError.invalidBind.greaterSize2",
		"Если размер pdu больше 100kb, SC закрывает соединение");
	__reg_tc__("protocolError.invalidBind.allowedCommandId",
		"Если задан command_id разрешенный для sme, но не bind, SC отправляет generic_nack с command_status = ESME_RINVBNDSTS");
	__reg_tc__("protocolError.invalidBind.notAllowedCommandId",
		"Если задан command_id неразрешенный для sme (pdu отправляемые исключительно SC), SC отправляет generic_nack с command_status = ESME_RINVCMDID");
	__reg_tc__("protocolError.invalidBind.nonExistentCommandId",
		"Если задан несуществующий в спецификации command_id, SC отправляет generic_nack с command_status = ESME_RINVBNDSTS");
	//corruptedPdu
	__reg_tc__("protocolError.invalidPdu",
		"Отправка неправильных pdu после установления соединения и успешного bind");
	__reg_tc__("protocolError.invalidPdu.smallerSize1",
		"Если размер pdu меньше 16 байт (размер хедера), SC закрывает соединение");
	__reg_tc__("protocolError.invalidPdu.smallerSize2",
		"Если размер pdu меньше правильного, SC закрывает соединение");
	__reg_tc__("protocolError.invalidPdu.greaterSize1",
		"Если размер pdu больше правильного, но меньше 64kb, SC закрывает соединение после таймаута");
	__reg_tc__("protocolError.invalidPdu.greaterSize2",
		"Если размер pdu больше 100kb, SC закрывает соединение");
	__reg_tc__("protocolError.invalidPdu.notAllowedCommandId",
		"Если задан command_id неразрешенный для sme (pdu отправляемые исключительно SC), SC отправляет generic_nack с command_status = ESME_RINVCMDID");
	__reg_tc__("protocolError.invalidPdu.nonExistentCommandId",
		"Если задан несуществующий в спецификации command_id, SC отправляет generic_nack с command_status = ESME_RINVCMDID");
	//submitAfterUnbind
	__reg_tc__("protocolError.submitAfterUnbind",
		"Pdu отправленные после unbind игнорируются SC");
	__reg_tc__("protocolError.submitAfterUnbind.allowedCommandId",
		"Если задан command_id разрешенный для sme, но не bind, SC отправляет generic_nack с command_status = ESME_RINVBNDSTS");
	__reg_tc__("protocolError.submitAfterUnbind.notAllowedCommandId",
		"Если задан command_id неразрешенный для sme (pdu отправляемые исключительно SC), SC отправляет generic_nack с command_status = ESME_RINVCMDID");
	__reg_tc__("protocolError.submitAfterUnbind.nonExistentCommandId",
		"Если задан несуществующий в спецификации command_id, SC отправляет generic_nack с command_status = ESME_RINVBNDSTS");
	//other
	__reg_tc__("protocolError.equalSeqNum",
		"Отправка нескольких submit_sm с одинаковым sequence_number не влияет на логику обработки pdu");
	__reg_tc__("protocolError.nullPdu",
		"Отправка pdu с пустыми (NULL) полями");
}

void notImplementedTc()
{
	__reg_tc__("notImplemented", "Не имплементированные тест кейсы");
	__reg_tc__("notImplemented.submitMulti", "Отправка submit_multi pdu");
	__reg_tc__("notImplemented.processMultiResp", "Получение submit_multi_resp pdu");
	__reg_tc__("notImplemented.dataSm", "Отправка data_sm pdu");
	__reg_tc__("notImplemented.processDataSm", "Получение data_sm pdu");
	__reg_tc__("notImplemented.processDataSmResp", "Получение data_sm_resp pdu");
	__reg_tc__("notImplemented.querySm", "Отправка query_sm pdu");
	__reg_tc__("notImplemented.processQuerySmResp", "Получение query_sm_resp pdu");
	__reg_tc__("notImplemented.cancelSm", "Отправка cancel_sm pdu");
	__reg_tc__("notImplemented.processCancelSmResp", "Получение cancel_sm_resp pdu");
	__reg_tc__("notImplemented.processAlertNotification", "Получение alert_notification pdu");
}

void allProtocolTc()
{
	//transmitter
	bindCorrectSmeTc();
	bindIncorrectSmeTc();
	unbindTc();
	submitSmTc();
	replaceSmTc();
	sendDeliverySmRespTc();
	sendInvalidPduTc();
	//receiver
	processRespTc();
	processDeliverySmTc();
	processNormalSmsTc();
	processDeliveryReportTc();
	processSmeAckTc();
	processGenericNackTc();
	//processIntermediateNotificationTc();
	//other
	checkMissingPduTc();
	protocolError();
	notImplementedTc();
}

public:
	SmppProtocolCheckList()
		: CheckList("Результаты функционального тестирования протокола SMPP", "smpp.chk")
	{
		allProtocolTc();
	}

protected:
	SmppProtocolCheckList(const char* name, const char* fileName)
		: CheckList(name, fileName)
	{
		allProtocolTc();
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_CHECK_LIST */

