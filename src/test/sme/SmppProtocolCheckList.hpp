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

void bindTc()
{
	__reg_tc__("bind",
		"Установление IP соединения с SC и bind тест кейсы");
	//bind.correct
	__reg_tc__("bind.correct",
		"Установление IP соединения с SC и регистрация с правильными параметрами");
	__reg_tc__("bind.correct.receiver",
		"Регистрация ресивера с помощью bind_receiver pdu");
	__reg_tc__("bind.correct.transmitter",
		"Регистрация трансмиттера с помощью bind_transmitter pdu");
	__reg_tc__("bind.correct.transceiver",
		"Регистрация трансивера с помощью bind_transceiver pdu");
	//bind.incorrect
	__reg_tc__("bind.incorrect",
		"Установление IP соединения с SC и регистрация с неправильными параметрами");
	__reg_tc__("bind.incorrect.systemIdNotRegistered",
		"Значение system_id не прописано в конфигурации SC");
	__reg_tc__("bind.incorrect.invalidPassword",
		"Неправильный пароль");
	__reg_tc__("bind.incorrect.unknownHost",
		"Установка соединения с недоступным SC (неизвестный хост) завершается корректно");
	__reg_tc__("bind.incorrect.invalidPort",
		"Установка соединения с недоступным SC (неправильный порт) завершается корректно");
	__reg_tc__("bind.incorrect.invalidSystemIdLength",
		"Длина поля system_id в bind pdu больше максимально допустимой");
	__reg_tc__("bind.incorrect.invalidPasswordLength",
		"Длина поля password в bind pdu больше максимально допустимой");
	__reg_tc__("bind.incorrect.invalidSystemTypeLength",
		"Длина поля system_type в bind pdu больше максимально допустимой");
	__reg_tc__("bind.incorrect.invalidAddressRangeLength",
		"Длина поля address_range в bind pdu больше максимально допустимой");
	//bind.resp
	__reg_tc__("bind.resp.checkCommandStatus",
		"В bind респонсе значение поля command_status равно ESME_ROK");
	__reg_tc__("bind.resp.checkInterfaceVersion",
		"В bind респонсе значение поля sc_interface_version равно 0x34");
}

void unbindTc()
{
	__reg_tc__("unbind", "Завершение соединения с SC и unbind тест кейсы");
	__reg_tc__("unbind.resp.checkCommandStatus",
		"В unbind респонсе значение поля command_status равно ESME_ROK");
	__reg_tc__("unbind.resp.checkSocketClose",
		"После отправки unbind респонса SC закрывает соединение");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "Тест кейсы для submit_sm");
	__reg_tc__("submitSm.sync",
		"Отправка синхронного submit_sm pdu");
	__reg_tc__("submitSm.async",
		"Отправка асинхронного submit_sm pdu");
	__reg_tc__("submitSm.receiver",
		"При отправке submit_sm с sme зарегистрированой как receiver SmppSession бросает exception");
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
		"Если совпадают source_addr и dest_addr, но отличается service_type для сообщения находящегося в очереди доставки, то создается новое сообщение");
	__reg_tc__("submitSm.correct.sourceAddrNotMatch",
		"Если совпадают dest_addr и service_type, но отличается source_addr для сообщения находящегося в очереди доставки, то создается новое сообщение");
	__reg_tc__("submitSm.correct.destAddrNotMatch",
		"Если совпадают source_addr и service_type, но отличается dest_addr для сообщения находящегося в очереди доставки, то создается новое сообщение");
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
	//submitSm.resp
	__reg_tc__("submitSm.resp",
		"Получение submit_sm_resp pdu");
	__reg_tc__("submitSm.resp.sync",
		"Получение submit_sm_resp pdu при синхронных submit_sm запросах");
	__reg_tc__("submitSm.resp.async",
		"Получение submit_sm_resp pdu при асинхронных submit_sm запросах");
	__reg_tc__("submitSm.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("submitSm.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("submitSm.resp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
	__reg_tc__("submitSm.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status, выполняются все условия для нормальной доставки сообщения (поля реквеста заданы корректно, существует маршрут и т.п.)");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidDestAddr",
		"Если код ошибки ESME_RINVDSTADR в поле command_status, то маршрута действительно не существует");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidWaitTime",
		"Если код ошибки ESME_RINVSCHED в поле command_status, то время schedule_delivery_time действительно задано неправильно");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidValidTime",
		"Если код ошибки ESME_RINVEXPIRY в поле command_status, то время validity_period действительно задано неправильно");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidDataCoding",
		"Если код ошибки ESME_RINVDCS в поле command_status, то кодировка сообщения действительно задана неправильно");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidServiceType",
		"Если код ошибки ESME_RINVSERTYP в поле command_status, то длина поля service_type действительно задана неправильно");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то адрес отправителя действительно не соответствует address range для данной sme в конфигурации sme.xml SC");
	__reg_tc__("submitSm.resp.checkCmdStatusSystemError",
		"Если код ошибки ESME_RSYSERR в поле command_status, то на стороне SC действительно возникла неустранимая ошибка (transaction rollback при сохранении сообщения)");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки ESME_RINVBNDSTS в поле command_status, то действительно sme зарегистрированна как receiver");
	__reg_tc__("submitSm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void replaceSmTc()
{
	__reg_tc__("replaceSm", "Тест кейсы для replace_sm");
	__reg_tc__("replaceSm.sync",
		"Отправка синхронного replace_sm pdu");
	__reg_tc__("replaceSm.async",
		"Отправка асинхронного replace_sm pdu");
	__reg_tc__("replaceSm.receiver",
		"При отправке replace_sm с sme зарегистрированой как receiver SmppSession бросает exception");
	//replaceSm.correct
	__reg_tc__("replaceSm.correct",
		"Отправка replace_sm pdu с правильными значениями полей");
	__reg_tc__("replaceSm.correct.waitTimePast",
		"Время schedule_delivery_time в прошлом (доставка уже должна была начаться)");
	__reg_tc__("replaceSm.correct.validTimeExceeded",
		"Время validity_period больше максимально срока валидности установленного для SC");
	__reg_tc__("replaceSm.correct.waitTimeNull",
		"Время начала доставки равно NULL (остается без изменений)");
	__reg_tc__("replaceSm.correct.validTimeNull",
		"Время validity_period равно NULL (сохраняется validity_period из замещаемой sms)");
	__reg_tc__("replaceSm.correct.smLengthMarginal",
		"Предельные значения длины тела сообщения short_message");
	__reg_tc__("replaceSm.correct.replaceReplacedEnrote",
		"Замещение уже ранее замещенного сообщения и находящегося в очереди доставки");
	__reg_tc__("replaceSm.correct.replaceRepeatedDeliveryEnrote",
	   "Замещение сообщения находящегося в очереди повторной доставки");
	//replaceSm.incorrect
	__reg_tc__("replaceSm.incorrect",
		"Отправка replace_sm pdu с неправильными значениями полей");
	__reg_tc__("replaceSm.incorrect.messageId",
		"Поле message_id задано неправильно (несуществующая sms)");
	__reg_tc__("replaceSm.incorrect.sourceAddr",
		"Если message_id задан правильно, но отличается source_addr, то существующее sms не замещается");
	__reg_tc__("replaceSm.incorrect.validTimeFormat",
		"Неправильный формат validity_period");
	__reg_tc__("replaceSm.incorrect.waitTimeFormat",
		"Неправильный формат schedule_delivery_time");
	__reg_tc__("replaceSm.incorrect.validTimePast",
		"Время validity_period в прошлом (сообщение никогда не будет доставлено)");
	__reg_tc__("replaceSm.incorrect.waitTimeInvalid1",
		"Значение schedule_delivery_time больше validity_period");
	__reg_tc__("replaceSm.incorrect.waitTimeInvalid2",
		"Значение validity_period = NULL (сохранить оригинальное значение), а schedule_delivery_time больше validity_period замещаемого сообщения");
	__reg_tc__("replaceSm.incorrect.waitTimeInvalid3",
		"Значение schedule_delivery_time = NULL (сохранить оригинальное значение), а validity_period меньше schedule_delivery_time замещаемого сообщения");
	__reg_tc__("replaceSm.incorrect.replaceFinal",
		"Замещение сообщения находящегося в финальном состоянии (код ошибки ESME_RREPLACEFAIL)");
	__reg_tc__("replaceSm.incorrect.transactionRollback",
		"Специальные sms эмулирующие transaction rollback на стороне сервера БД");
	//replaceSm.resp
	__reg_tc__("replaceSm.resp",
		"Получение replace_sm_resp pdu");
	__reg_tc__("replaceSm.resp.sync",
		"Получение replace_sm_resp pdu при синхронных replace_sm запросах");
	__reg_tc__("replaceSm.resp.async",
		"Получение replace_sm_resp pdu при асинхронных replace_sm запросах");
	__reg_tc__("replaceSm.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("replaceSm.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("replaceSm.resp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
	__reg_tc__("replaceSm.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status реквест replace_sm действительно не содержит ошибок (сообщение существует и находится в ENROUTE состоянии, адрес отправителя совпадает и т.п.)");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidWaitTime",
		"Если код ошибки ESME_RINVSCHED в поле command_status, то время schedule_delivery_time действительно задано неправильно");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidValidTime",
		"Если код ошибки ESME_RINVEXPIRY в поле command_status, то время validity_period действительно задано неправильно");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то source_addr действительно не соответствует адресу отправителя сообщения в БД");
	__reg_tc__("replaceSm.resp.checkCmdStatusSystemError",
		"Если код ошибки ESME_RSYSERR в поле command_status, то на стороне SC действительно возникла неустранимая ошибка (transaction rollback при сохранении сообщения)");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки ESME_RINVBNDSTS в поле command_status, то действительно sme зарегистрированна как receiver");
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidMsgId",
		"Если код ошибки ESME_RINVMSGID в поле command_status, то действительно message_id задан неправильно");
	__reg_tc__("replaceSm.resp.checkCmdStatusReplaceFiled",
		"Если код ошибки ESME_RREPLACEFAIL в поле command_status, то действительно замещаемое сообщение находится в финальном состоянии");
	__reg_tc__("replaceSm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void deliverySmTc()
{
	__reg_tc__("deliverySm", "Тест кейсы для deliver_sm");
	__reg_tc__("deliverySm.checkFields",
		"Общие проверки полей хедера и некоторых других полей");
	//deliverySm.normalSms
	__reg_tc__("deliverySm.normalSms",
		"Обычная sms не являющаяся нотификацией");
	__reg_tc__("deliverySm.normalSms.checkAllowed",
		"Проверка правомерности получения sms");
	__reg_tc__("deliverySm.normalSms.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и алиасу получателя)");
	__reg_tc__("deliverySm.normalSms.checkMandatoryFields",
		"Сравнение обязательных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm) pdu");
	__reg_tc__("deliverySm.normalSms.checkDataCoding",
		"Кодировка текста sms соответствует настройкам профиля получателя");
	__reg_tc__("deliverySm.normalSms.checkTextEqualDataCoding",
		"Для одинаковых кодировок в профилях отправителя и получателя текст sms совпадает");
	__reg_tc__("deliverySm.normalSms.checkTextDiffDataCoding",
		"Для различных кодировок в профилях отправителя и получателя текст sms правильно преобразуется из одной кодировки в другую");
	__reg_tc__("deliverySm.normalSms.checkOptionalFields",
		"Сравнение опциональных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm) pdu");
	__reg_tc__("deliverySm.normalSms.scheduleChecks",
		"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
	//deliverySm.reports
	__reg_tc__("deliverySm.reports",
		"Отчеты о доставке (подтверждения доставки и промежуточные нотификации)");
	__reg_tc__("deliverySm.reports.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
	__reg_tc__("deliverySm.reports.checkFields",
		"Общая проверка правильности полей");
	__reg_tc__("deliverySm.reports.multipleMessages",
		"Длинный текст сообщения о доставки разбивается на несколько sms");
	//deliverySm.reports.deliveryReceipt
	__reg_tc__("deliverySm.reports.deliveryReceipt",
		"Подтверждения доставки");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkAllowed",
		"Проверка правомерности получения подтверждений доставки (в зависимости от настроек профиля и поля pdu registered_delivery, единственный раз по окончании доставки оригинальной pdu)");
	__reg_tc__("deliverySm.reports.deliveryReceipt.failureDeliveryReceipt",
		"Подтверждение доставки на ошибку не доставляется в случае успешной доставки оригинального сообщения");
	__reg_tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt",
		"Подтверждение доставки на ошибку при истечении срока валидности доставляется в момент времени validity period");
	__reg_tc__("deliverySm.reports.deliveryReceipt.recvTimeChecks",
		"Подтверждения доставки приходят в момент окончании доставки оригинальной pdu");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkStatus",
		"Информации о статусе доставленной pdu (состояние, код ошибки) является корректной");
	__reg_tc__("deliverySm.reports.deliveryReceipt.checkText",
		"Текст сообщения соответствует настройкам SC");
	//deliverySm.reports.intermediateNotification
	__reg_tc__("deliverySm.reports.intermediateNotification",
		"Промежуточные нотификации");
	__reg_tc__("deliverySm.reports.intermediateNotification.checkAllowed",
		"Проверка правомерности получения промежуточных нотификаций (в зависимости от настроек профиля и поля pdu registered_delivery, единственный раз после первой зарешедуленой попытки доставки)");
	__reg_tc__("deliverySm.reports.intermediateNotification.noRescheduling",
		"Промежуточная нотификация не доставляется в случае, если sms не была зарешедулена");
	__reg_tc__("deliverySm.reports.intermediateNotification.recvTimeChecks",
		"Время доставки промежуточной нотификации соответсвует времени первой зарешедуленой попытки доставки оригинальной pdu");
	__reg_tc__("deliverySm.reports.intermediateNotification.checkStatus",
		"Информации о статусе pdu (состояние, код ошибки) является корректной");
	__reg_tc__("deliverySm.reports.intermediateNotification.checkText",
		"Текст сообщения соответствует настройкам SC");
	//deliverySm.smeAck
	__reg_tc__("deliverySm.smeAck",
		"Ответное сообщение от внутренних sme SC (менеджер профилей, db sme и т.п.)");
	__reg_tc__("deliverySm.smeAck.checkAllowed",
		"Проверка правомерности получения сообщения");
	__reg_tc__("deliverySm.smeAck.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу внутренней sme SC)");
	__reg_tc__("deliverySm.smeAck.checkFields",
		"Общая проверка правильности полей");
	__reg_tc__("deliverySm.smeAck.recvTimeChecks",
		"Правильное время получения сообщений");
	//deliverySm.resp
	__reg_tc__("deliverySm.resp", "Отправка deliver_sm_resp pdu");
	__reg_tc__("deliverySm.resp.sync",
		"Отправка синхронного deliver_sm_resp pdu");
	__reg_tc__("deliverySm.resp.async",
		"Отправка асинхронного deliver_sm_resp pdu");
	__reg_tc__("deliverySm.resp.delay",
		"Отправка deliver_sm_resp pdu с задержкой меньшей sme timeout");
	__reg_tc__("deliverySm.resp.sendOk",
		"Отправить респонс подтверждающий получение правильного deliver_sm");
	//deliverySm.resp.sendRetry
	__reg_tc__("deliverySm.resp.sendRetry",
		"Отправить респонс для форсирования повторной доставки deliver_sm");
	__reg_tc__("deliverySm.resp.sendRetry.notSend",
		"Не отпралять респонс, имитация неполучения deliver_sm");
	__reg_tc__("deliverySm.resp.sendRetry.tempAppError",
		"Отправить статус ESME_RX_T_APPN (временная ошибка на стороне sme, запрос на повторную доставку)");
	__reg_tc__("deliverySm.resp.sendRetry.msgQueueFull",
		"Отправить статус ESME_RMSGQFUL (переполнение очереди стороне sme)");
	__reg_tc__("deliverySm.resp.sendRetry.invalidSequenceNumber",
		"Отправить респонс с неправильным sequence_number");
	__reg_tc__("deliverySm.resp.sendRetry.sendAfterSmeTimeout",
		"Отправить респонс после sme timeout");
	//deliverySm.resp.sendError
	__reg_tc__("deliverySm.resp.sendError",
		"Отправить респонс для прекращения повторной доставки deliver_sm");
	__reg_tc__("deliverySm.resp.sendError.standardError",
		"Респонс со стандартным кодом ошибки (диапазон 0x1-0x10f)");
	__reg_tc__("deliverySm.resp.sendError.reservedError",
		"Респонс с зарезирвированным кодом ошибки (диапазоны 0x110-0x3ff и 0x400-0x4ff)");
	__reg_tc__("deliverySm.resp.sendError.outRangeError",
		"Респонс с кодом ошибки вне диапазона определенного спецификацией SMPP (>0x500)");
	__reg_tc__("deliverySm.resp.sendError.permanentAppError",
		"Респонс с кодом ошибки ESME_RX_P_APPN (неустранимая ошибка на стороне sme, отказ от всех последующих сообщений)");
}

void querySmTc()
{
	__reg_tc__("querySm", "Тест кейсы для query_sm");
	__reg_tc__("querySm.sync",
		"Отправка синхронного query_sm pdu");
	__reg_tc__("querySm.async",
		"Отправка асинхронного query_sm pdu");
	__reg_tc__("querySm.receiver",
		"При отправке query_sm с sme зарегистрированой как receiver SmppSession бросает exception");
	//querySm.correct
	__reg_tc__("querySm.correct",
		"Запрос статуса существующего sms");
	__reg_tc__("querySm.correct.enroute",
		"Сообщение находящееся в состоянии ENROUTE");
	__reg_tc__("querySm.correct.delivered",
		"Сообщение находящееся в состоянии DELIVERED");
	__reg_tc__("querySm.correct.expired",
		"Сообщение находящееся в состоянии EXPIRED");
	__reg_tc__("querySm.correct.undeliverable",
		"Сообщение находящееся в состоянии UNDELIVERABLE");
	__reg_tc__("querySm.correct.deleted",
		"Сообщение находящееся в состоянии DELETED");
	//querySm.incorrect
	__reg_tc__("querySm.incorrect",
		"Запрос статуса несуществующего sms");
	__reg_tc__("querySm.incorrect.messageId",
		"Неправильный message_id");
	__reg_tc__("querySm.incorrect.sourceAddr",
		"Значение message_id правильное, но source_addr не совпадает");
	//querySm.resp
	__reg_tc__("querySm.resp", "Получение query_sm_resp pdu");
	__reg_tc__("querySm.resp.sync",
		"Получение query_sm_resp pdu при синхронных query_sm запросах");
	__reg_tc__("querySm.resp.async",
		"Получение query_sm_resp pdu при асинхронных query_sm запросах");
	__reg_tc__("querySm.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("querySm.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("querySm.resp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
	__reg_tc__("querySm.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status реквест query_sm действительно не содержит ошибок (сообщение существует, адрес отправителя совпадает и т.п.)");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то source_addr не совпадает с адресом отправителя сообщения в БД");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки ESME_RINVBNDSTS в поле command_status, то действительно sme зарегистрированна как receiver");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidMsgId",
		"Если код ошибки ESME_RINVMSGID в поле command_status, то действительно message_id задан неправильно");
	__reg_tc__("querySm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
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
	bindTc();
	unbindTc();
	submitSmTc();
	replaceSmTc();
	deliverySmTc();


	sendInvalidPduTc();
	//receiver
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

