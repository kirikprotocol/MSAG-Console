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
	__reg_tc__("bind.correct.afterUnbind",
		"Отправка bind реквеста после unbind для того же самого tcp/ip соединения");
	//bind.incorrect
	__reg_tc__("bind.incorrect",
		"Установление IP соединения с SC и регистрация с неправильными параметрами");
	__reg_tc__("bind.incorrect.secondBind",
		"Отправка повторного bind реквеста");
	__reg_tc__("bind.incorrect.systemIdNotRegistered",
		"Значение system_id не прописано в конфигурации SC");
	__reg_tc__("bind.incorrect.invalidPassword",
		"Неправильный пароль");
	__reg_tc__("bind.incorrect.unknownHost",
		"Установка соединения с недоступным SC (неизвестный хост) завершается корректно");
	__reg_tc__("bind.incorrect.invalidPort",
		"Установка соединения с недоступным SC (неправильный порт) завершается корректно");
	__reg_tc__("bind.incorrect.invalidSystemIdLength",
		"Длина поля system_id в bind pdu больше максимально допустимой (SmppSession::connect() бросает exception)");
	__reg_tc__("bind.incorrect.invalidPasswordLength",
		"Длина поля password в bind pdu больше максимально допустимой (SmppSession::connect() бросает exception)");
	__reg_tc__("bind.incorrect.invalidSystemTypeLength",
		"Длина поля system_type в bind pdu больше максимально допустимой (SmppSession::connect() бросает exception)");
	__reg_tc__("bind.incorrect.invalidAddressRangeLength",
		"Длина поля address_range в bind pdu больше максимально допустимой (SmppSession::connect() бросает exception)");
	//bind.resp
	__reg_tc__("bind.resp",
		"На bind реквест SC отправляет соответствующий bind респонс");
	__reg_tc__("bind.resp.receiver",
		"На bind_receiver реквест SC отправляет bind_receiver_resp респонс");
	__reg_tc__("bind.resp.transmitter",
		"На bind_transmitter реквест SC отправляет bind_transmitter_resp респонс");
	__reg_tc__("bind.resp.transceiver",
		"На bind_transceiver реквест SC отправляет bind_transceiver_resp респонс");
	__reg_tc__("bind.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("bind.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("bind.resp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, sequence_number)");
	__reg_tc__("bind.resp.checkFields",
		"Проверка sc_interface_version = 0x34 (system_id не проверяется)");
	__reg_tc__("bind.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status, выполняются все условия для нормальной регистрации sme (правильный пароль, зарегистрированный system_id и т.п.)");
	__reg_tc__("bind.resp.checkCmdStatusAlreadyBound",
		"Если код ошибки ESME_RALYBND в поле command_status, то действительно sme пытается зарегистрироваться повторно");
	__reg_tc__("bind.resp.checkCmdStatusInvalidPassword",
		"Если код ошибки ESME_RINVPASWD в поле command_status, то действительно длина поля password превышает 8 символов");
	__reg_tc__("bind.resp.checkCmdStatusInvalidSystemId",
		"Если код ошибки ESME_RINVSYSID в поле command_status, то действительно длина поля system_id превышает 15 символов");
	__reg_tc__("bind.resp.checkCmdStatusInvalidSystemType",
		"Если код ошибки ESME_RINVSYSTYP в поле command_status, то действительно длина поля system_type превышает 12 символов");
	__reg_tc__("bind.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void unbindTc()
{
	__reg_tc__("unbind", "Завершение соединения с SC и unbind тест кейсы");
	__reg_tc__("unbind.secondUnbind",
		"Отправка повторного unbind реквеста");
	__reg_tc__("unbind.resp",
		"На unbind реквест SC отправляет unbind_resp респонс");
	__reg_tc__("unbind.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("unbind.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("unbind.resp.checkPdu",
		"Правильные значения полей респонса (command_length, sequence_number, command_status = ESME_ROK");
}

void enquireLinkTc()
{
	__reg_tc__("enquireLink",
		"Отправка enquire_link реквеста");
	__reg_tc__("enquireLink.receiver",
		"Отправка enquire_link реквеста с sme зарегистрированной как receiver");
	__reg_tc__("enquireLink.transmitter",
		"Отправка enquire_link реквеста с sme зарегистрированной как transmitter");
	__reg_tc__("enquireLink.transceiver",
		"Отправка enquire_link реквеста с sme зарегистрированной как transceiver");
	__reg_tc__("enquireLink.beforeBind",
		"Отправка enquire_link до отправки bind реквеста");
	__reg_tc__("enquireLink.afterUnbind",
		"Отправка enquire_link после отправки unbind реквеста");
	//enquireLink.resp
	__reg_tc__("enquireLink.resp",
		"На enquire_link реквест SC отправляет enquire_link_resp респонс");
	__reg_tc__("enquireLink.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("enquireLink.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("enquireLink.resp.checkPdu",
		"Правильные значения полей респонса (command_length, sequence_number, command_status = ESME_ROK независимо от bind статуса");
}

void submitSmTc()
{
	__reg_tc__("submitSm", "Тест кейсы для submit_sm");
	__reg_tc__("submitSm.sync",
		"Отправка синхронного submit_sm реквеста");
	__reg_tc__("submitSm.async",
		"Отправка асинхронного submit_sm реквеста");
	__reg_tc__("submitSm.receiver",
		"Отправка submit_sm реквеста с sme зарегистрированной как receiver (SmppSession бросает exception)");
	__reg_tc__("submitSm.transmitter",
		"Отправка submit_sm реквеста с sme зарегистрированной как transmitter");
	__reg_tc__("submitSm.transceiver",
		"Отправка submit_sm реквеста с sme зарегистрированной как transceiver");
	__reg_tc__("submitSm.forceDc",
		"Отправка submit_sm реквеста с sme с установленным признаком forceDC");
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
		"Предельные значения длины поля short_message");
	__reg_tc__("submitSm.correct.messagePayloadLengthMarginal",
		"Предельные значения длины поля message_payload");
	__reg_tc__("submitSm.correct.ussdRequest",
		"Отправка корректного ussd запроса");
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
	__reg_tc__("submitSm.incorrect.dataCodingNormalSme",
		"Недопустимые значения dataCoding для нормальных sme");
	__reg_tc__("submitSm.incorrect.dataCodingForceDcSme",
		"Недопустимые значения dataCoding для больных sme");
	__reg_tc__("submitSm.incorrect.bothMessageFields",
		"Заданы оба поля short_message и message_payload");
	__reg_tc__("submitSm.incorrect.udhiLength",
		"Длина udh при выставленном флаге udhi задана некорректно");
	__reg_tc__("submitSm.incorrect.serviceTypeLength",
		"Длина поля service_type больше допустимой");
	__reg_tc__("submitSm.incorrect.sourceAddrLength",
		"Длина поля source_addr больше допустимой");
	__reg_tc__("submitSm.incorrect.destAddrLength",
		"Длина поля dest_addr больше допустимой");
	__reg_tc__("submitSm.incorrect.validTimeLength",
		"Длина поля validity_period больше допустимой");
	__reg_tc__("submitSm.incorrect.waitTimeLength",
		"Длина поля schedule_delivery_time больше допустимой");
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
		"Если код ошибки ESME_RINVSCHED в поле command_status, то значение поля schedule_delivery_time или директивы def действительно задано неправильно");
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidValidTime",
		"Если код ошибки ESME_RINVEXPIRY в поле command_status, то значение поля validity_period действительно задано неправильно");
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
	__reg_tc__("submitSm.resp.checkCmdStatusInvalidMsgLen",
		"Если код ошибки ESME_RINVMSGLEN в поле command_status, то действительно длина сообщения неправильная (например, наличие udhi в длинном MT сообщении)");
	__reg_tc__("submitSm.resp.checkCmdStatusSubmitFailed",
		"Если код ошибки ESME_RSUBMITFAIL в поле command_status, то pdu действительно некорректная (присутствуют оба поля short_message и message_payload и т.п.)");
	__reg_tc__("submitSm.resp.checkCmdStatusNoRoute",
		"Если код ошибки NOROUTE в поле command_status, то маршрута действительно не существует");
	__reg_tc__("submitSm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void dataSmTc()
{
	__reg_tc__("dataSm", "Тест кейсы для data_sm");
	__reg_tc__("dataSm.sync",
		"Отправка синхронного data_sm реквеста");
	__reg_tc__("dataSm.async",
		"Отправка асинхронного data_sm реквеста");
	__reg_tc__("dataSm.receiver",
		"Отправка data_sm реквеста с sme зарегистрированной как receiver (SmppSession бросает exception)");
	__reg_tc__("dataSm.transmitter",
		"Отправка data_sm реквеста с sme зарегистрированной как transmitter");
	__reg_tc__("dataSm.transceiver",
		"Отправка data_sm реквеста с sme зарегистрированной как transceiver");
	__reg_tc__("dataSm.forceDc",
		"Отправка data_sm реквеста с sme с установленным признаком forceDC");
	//dataSm.correct
	__reg_tc__("dataSm.correct",
		"Отправка data_sm pdu с правильными значениями полей");
	__reg_tc__("dataSm.correct.serviceTypeMarginal",
		"Предельные значения длины поля service_type");
	__reg_tc__("dataSm.correct.validTimeExceeded",
		"Время qos_time_to_live больше максимально срока валидности установленного для SC");
	__reg_tc__("dataSm.correct.messagePayloadLengthMarginal",
		"Предельные значения длины поля message_payload");
	__reg_tc__("dataSm.correct.ussdRequest",
		"Отправка корректного ussd запроса");
	__reg_tc__("dataSm.correct.notReplace",
		"Дублированное сообщение (совпадают source_addr, dest_addr и service_type со значениями сообщения находящегося в очереди доставки)");
	//dataSm.incorrect
	__reg_tc__("dataSm.incorrect",
		"Отправка data_sm pdu с неправильными значениями полей");
	__reg_tc__("dataSm.incorrect.sourceAddr",
		"Неправильный адрес отправителя");
	__reg_tc__("dataSm.incorrect.destAddr",
		"Неправильный адрес получателя");
	__reg_tc__("dataSm.incorrect.dataCodingNormalSme",
		"Недопустимые значения dataCoding для нормальных sme");
	__reg_tc__("dataSm.incorrect.dataCodingForceDcSme",
		"Недопустимые значения dataCoding для больных sme");
	__reg_tc__("dataSm.incorrect.udhiLength",
		"Длина udh при выставленном флаге udhi задана некорректно");
	__reg_tc__("dataSm.incorrect.serviceTypeLength",
		"Длина поля service_type больше допустимой");
	__reg_tc__("dataSm.incorrect.sourceAddrLength",
		"Длина поля source_addr больше допустимой");
	__reg_tc__("dataSm.incorrect.destAddrLength",
		"Длина поля dest_addr больше допустимой");
	__reg_tc__("dataSm.incorrect.validTimeLength",
		"Длина поля validity_period больше допустимой");
	__reg_tc__("dataSm.incorrect.waitTimeLength",
		"Длина поля schedule_delivery_time больше допустимой");
	__reg_tc__("dataSm.incorrect.transactionRollback",
		"Специальные sms эмулирующие transaction rollback на стороне сервера БД");
	//dataSm.assert
	__reg_tc__("dataSm.assert",
		"Заполнение и отправка data_sm pdu с недопустимыми значениями полей");
	__reg_tc__("dataSm.assert.serviceTypeInvalid",
		"Недопустимое значение длины поля service_type");
	__reg_tc__("dataSm.assert.destAddrLengthInvalid",
		"Недопустимое значение длины поля адреса получателя");
	//dataSm.resp
	__reg_tc__("dataSm.resp",
		"Получение data_sm_resp pdu");
	__reg_tc__("dataSm.resp.sync",
		"Получение data_sm_resp pdu при синхронных data_sm запросах");
	__reg_tc__("dataSm.resp.async",
		"Получение data_sm_resp pdu при асинхронных data_sm запросах");
	__reg_tc__("dataSm.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("dataSm.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("dataSm.resp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
	__reg_tc__("dataSm.resp.checkOptional",
		"Правильные значения опциональных полей респонса (delivery_failure_reason, network_error_code, additional_status_info_text и dpf_result)");
	__reg_tc__("dataSm.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status, выполняются все условия для нормальной доставки сообщения (поля реквеста заданы корректно, существует маршрут и т.п.)");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidDestAddr",
		"Если код ошибки ESME_RINVDSTADR в поле command_status, то маршрута действительно не существует");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidWaitTime",
		"Если код ошибки ESME_RINVSCHED в поле command_status, то значение директивы def действительно задано неправильно");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidDataCoding",
		"Если код ошибки ESME_RINVDCS в поле command_status, то кодировка сообщения действительно задана неправильно");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidServiceType",
		"Если код ошибки ESME_RINVSERTYP в поле command_status, то длина поля service_type действительно задана неправильно");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то адрес отправителя действительно не соответствует address range для данной sme в конфигурации sme.xml SC");
	__reg_tc__("dataSm.resp.checkCmdStatusSystemError",
		"Если код ошибки ESME_RSYSERR в поле command_status, то на стороне SC действительно возникла неустранимая ошибка (transaction rollback при сохранении сообщения)");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки ESME_RINVBNDSTS в поле command_status, то действительно sme зарегистрированна как receiver");
	__reg_tc__("dataSm.resp.checkCmdStatusInvalidMsgLen",
		"Если код ошибки ESME_RINVMSGLEN в поле command_status, то действительно длина сообщения неправильная (например, наличие udhi в длинном MT сообщении)");
	__reg_tc__("dataSm.resp.checkCmdStatusSubmitFailed",
		"Если код ошибки ESME_RSUBMITFAIL в поле command_status, то pdu действительно некорректная (присутствуют оба поля short_message и message_payload и т.п.)");
	__reg_tc__("dataSm.resp.checkCmdStatusNoRoute",
		"Если код ошибки NOROUTE в поле command_status, то маршрута действительно не существует");
	__reg_tc__("dataSm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void directiveTc()
{
	__reg_tc__("directive.submitSm",
		"Отправка submit_sm pdu с директивами (в short_message или message_payload)");
	__reg_tc__("directive.dataSm",
		"Отправка data_sm pdu с директивами (в message_payload)");
	__reg_tc__("directive.mixedCase",
		"Текст директивы в смешанном регистре");
	__reg_tc__("directive.correct",
		"Отправка sms с корректными директивами");
	__reg_tc__("directive.correct.ack",
		"Корректные ack директивы");
	__reg_tc__("directive.correct.noack",
		"Корректные noack директивы");
	__reg_tc__("directive.correct.def",
		"Корректные def директивы");
	__reg_tc__("directive.correct.template",
		"Корректные template директивы с различным количеством параметров (0..2), русские и латинские символы в параметрах и т.п.");
	__reg_tc__("directive.correct.hide",
		"Корректные hide директивы");
	__reg_tc__("directive.correct.unhide",
		"Корректные unhide директивы");
	__reg_tc__("directive.incorrect",
		"Отправка sms с некорректными директивами");
	__reg_tc__("directive.incorrect.invalidDir",
		"Несуществующие директивы");
	__reg_tc__("directive.incorrect.invalidDefDir",
		"Неправильно заданные параметры для def директивы");
	__reg_tc__("directive.incorrect.invalidTemplateDir",
		"Неправильно заданные параметры для template директивы");
}

void replaceSmTc()
{
	__reg_tc__("replaceSm", "Тест кейсы для replace_sm");
	__reg_tc__("replaceSm.sync",
		"Отправка синхронного replace_sm pdu");
	__reg_tc__("replaceSm.async",
		"Отправка асинхронного replace_sm pdu");
	__reg_tc__("replaceSm.receiver",
		"Отправка replace_sm реквеста с sme зарегистрированной как receiver (SmppSession бросает exception)");
	__reg_tc__("replaceSm.transmitter",
		"Отправка replace_sm реквеста с sme зарегистрированной как transmitter");
	__reg_tc__("replaceSm.transceiver",
		"Отправка replace_sm реквеста с sme зарегистрированной как transceiver");
	__reg_tc__("replaceSm.map",
		"Отправка replace_sm реквеста для sms, которая будет доставляться на map proxy");
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
	__reg_tc__("replaceSm.incorrect.udhiLength",
		"Длина udh при выставленном флаге udhi (в замещаемом сообщении) задана некорректно");
	__reg_tc__("replaceSm.incorrect.messageIdLength",
		"Длина поля message_id больше допустимой");
	__reg_tc__("replaceSm.incorrect.sourceAddrLength",
		"Длина поля source_addr больше допустимой");
	__reg_tc__("replaceSm.incorrect.validTimeLength",
		"Длина поля validity_period больше допустимой");
	__reg_tc__("replaceSm.incorrect.waitTimeLength",
		"Длина поля schedule_delivery_time больше допустимой");
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
	__reg_tc__("replaceSm.resp.checkCmdStatusInvalidMsgLen",
		"Если код ошибки ESME_RINVMSGLEN в поле command_status, то действительно длина сообщения неправильная (например, наличие udhi в длинном MT сообщении)");
	__reg_tc__("replaceSm.resp.checkCmdStatusReplaceFailed",
		"Если код ошибки ESME_RREPLACEFAIL в поле command_status, то действительно замещаемое сообщение находится в финальном состоянии");
	__reg_tc__("replaceSm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void querySmTc()
{
	__reg_tc__("querySm", "Тест кейсы для query_sm");
	__reg_tc__("querySm.sync",
		"Отправка синхронного query_sm pdu");
	__reg_tc__("querySm.async",
		"Отправка асинхронного query_sm pdu");
	__reg_tc__("querySm.receiver",
		"Отправка query_sm реквеста с sme зарегистрированной как receiver (SmppSession бросает exception)");
	__reg_tc__("querySm.transmitter",
		"Отправка query_sm реквеста с sme зарегистрированной как transmitter");
	__reg_tc__("querySm.transceiver",
		"Отправка query_sm реквеста с sme зарегистрированной как transceiver");
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
	__reg_tc__("querySm.incorrect.messageIdLength",
		"Длина поля message_id больше допустимой");
	__reg_tc__("querySm.incorrect.sourceAddrLength",
		"Длина поля source_addr больше допустимой");
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
	__reg_tc__("querySm.resp.checkFields",
		"Правильные значения полей message_id, final_date, message_state, error_code");
	__reg_tc__("querySm.resp.checkFields.enroute",
		"Сообщение находящееся в состоянии ENROUTE");
	__reg_tc__("querySm.resp.checkFields.delivered",
		"Сообщение находящееся в состоянии DELIVERED");
	__reg_tc__("querySm.resp.checkFields.expired",
		"Сообщение находящееся в состоянии EXPIRED");
	__reg_tc__("querySm.resp.checkFields.undeliverable",
		"Сообщение находящееся в состоянии UNDELIVERABLE");
	__reg_tc__("querySm.resp.checkFields.deleted",
		"Сообщение находящееся в состоянии DELETED");
	__reg_tc__("querySm.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status реквест query_sm действительно не содержит ошибок (сообщение существует, адрес отправителя совпадает и т.п.)");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то source_addr не совпадает с адресом отправителя сообщения в БД");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки ESME_RINVBNDSTS в поле command_status, то действительно sme зарегистрированна как receiver");
	__reg_tc__("querySm.resp.checkCmdStatusInvalidMsgId",
		"Если код ошибки ESME_RINVMSGID в поле command_status, то действительно message_id задан неправильно");
	__reg_tc__("querySm.resp.checkCmdStatusQueryFail",
		"Если код ошибки ESME_RQUERYFAIL в поле command_status, то действительно обраюотка query_sm не может быть выполнена корректно");
	__reg_tc__("querySm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void cancelSmTc()
{
	__reg_tc__("cancelSm", "Тест кейсы для cancel_sm");
	__reg_tc__("cancelSm.sync",
		"Отправка синхронного cancel_sm pdu");
	__reg_tc__("cancelSm.async",
		"Отправка асинхронного cancel_sm pdu");
	__reg_tc__("cancelSm.receiver",
		"Отправка cancel_sm реквеста с sme зарегистрированной как receiver (SmppSession бросает exception)");
	__reg_tc__("cancelSm.transmitter",
		"Отправка cancel_sm реквеста с sme зарегистрированной как transmitter");
	__reg_tc__("cancelSm.transceiver",
		"Отправка cancel_sm реквеста с sme зарегистрированной как transceiver");
	//cancelSm.correct
	__reg_tc__("cancelSm.correct",
		"Отправка cancel_sm реквеста с правильными значениями полей");
	__reg_tc__("cancelSm.correct.messageIdWithoutDestAddr",
		"Задан message_id существующего сообщения в ENROUTE состоянии, source_addr совпадает, dest_addr и service_type нулевые");
	__reg_tc__("cancelSm.correct.messageIdWithDestAddr",
		"Задан message_id существующего сообщения в ENROUTE состоянии, source_addr и dest_addr совпадают, service_type нулевой");
	__reg_tc__("cancelSm.correct.destAddrWithoutServiceType",
		"Заданы правильные source_addr и dest_addr и нулевые message_id и service_type");
	__reg_tc__("cancelSm.correct.destAddrWithServiceType",
		"Заданы правильные source_addr, dest_addr и service_type и нулевой message_id");
	//cancelSm.incorrect
	__reg_tc__("cancelSm.incorrect",
		"Отправка cancel_sm реквеста с неправильными значениями полей");
	__reg_tc__("cancelSm.incorrect.messageId",
		"Несуществующий message_id и нулевые dest_addr и service_type");
	__reg_tc__("cancelSm.incorrect.sourceAddr",
		"Задан message_id существующего сообщения в ENROUTE состоянии, dest_addr и service_type нулевые, но source_addr не совпадает");
	__reg_tc__("cancelSm.incorrect.destAddr",
		"Задан message_id существующего сообщения в ENROUTE состоянии, совпадающий source_addr, нулевой service_type и несовпадающий dest_addr");
	__reg_tc__("cancelSm.incorrect.sourceAddrOnly",
		"Задан только source_addr, а message_id, dest_addr и service_type нулевые");
	__reg_tc__("cancelSm.incorrect.destAddrOnly",
		"Задан только dest_addr, а message_id, source_addr и service_type нулевые");
	__reg_tc__("cancelSm.incorrect.messageIdOnly",
		"Задан только message_id, а source_addr, dest_addr и service_type нулевые");
	__reg_tc__("cancelSm.incorrect.emptyFields",
		"Задан правильный source_addr и  пустые message_id, dest_addr и service_type");
	__reg_tc__("cancelSm.incorrect.serviceTypeWithoutDestAddr",
		"Заданы правильные source_addr и service_type, но dest_addr нулевой");
	__reg_tc__("cancelSm.incorrect.messageIdWithServiceType",
		"Заданы одновременно message_id и service_type для существующего сообщения в ENROUTE состоянии");
	__reg_tc__("cancelSm.incorrect.allFields",
		"Заданы все поля message_id, source_addr, dest_addr и service_type для существующего сообщения в ENROUTE состоянии");
	__reg_tc__("cancelSm.incorrect.cancelFinal",
		"Задан message_id существующего сообщения в финальном состоянии, source_addr совпадает, dest_addr и service_type нулевые");
	__reg_tc__("cancelSm.incorrect.noSms",
		"Заданы правильные source_addr, dest_addr и service_type и нулевой message_id, но нет сообщений в БД удовлетворяющих условиям запроса");
	__reg_tc__("cancelSm.incorrect.messageIdLength",
		"Длина поля message_id больше допустимой");
	__reg_tc__("cancelSm.incorrect.serviceTypeLength",
		"Длина поля service_type больше допустимой");
	__reg_tc__("cancelSm.incorrect.sourceAddrLength",
		"Длина поля source_addr больше допустимой");
	__reg_tc__("cancelSm.incorrect.destAddrLength",
		"Длина поля dest_addr больше допустимой");
	//cancelSm.resp
	__reg_tc__("cancelSm.resp",
		"Получение cancel_sm_resp pdu");
	__reg_tc__("cancelSm.resp.sync",
		"Получение cancel_sm_resp pdu при синхронных cancel_sm запросах");
	__reg_tc__("cancelSm.resp.async",
		"Получение cancel_sm_resp pdu при асинхронных cancel_sm запросах");
	__reg_tc__("cancelSm.resp.checkDuplicates",
		"На каждый реквест приходит единственный респонс");
	__reg_tc__("cancelSm.resp.checkTime",
		"Правильное время получения респонса");
	__reg_tc__("cancelSm.resp.checkHeader",
		"Правильные значения полей хедера респонса (command_length, command_id, sequence_number)");
	__reg_tc__("cancelSm.resp.checkCmdStatusOk",
		"При отсутствии кода ошибки в поле command_status реквест cancel_sm действительно не содержит ошибок (сообщение существует и находится в ENROUTE состоянии, адреса source_addr и dest_addr заданы правильно и т.п.)");
	/*
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidSourceAddr",
		"Если код ошибки ESME_RINVSRCADR в поле command_status, то source_addr действительно не соответствует source_addr оригинального сообщения в БД");
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidDestAddr",
		"Если код ошибки ESME_RINVDSTADR в поле command_status, то dest_addr действительно не соответствует dest_addr оригинального сообщения в БД");
	*/
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidBindStatus",
		"Если код ошибки ESME_RINVBNDSTS в поле command_status, то действительно sme зарегистрированна как receiver");
	__reg_tc__("cancelSm.resp.checkCmdStatusInvalidMsgId",
		"Если код ошибки ESME_RINVMSGID в поле command_status, то действительно message_id задан неправильно");
	__reg_tc__("cancelSm.resp.checkCmdStatusCancelFailed",
		"Если код ошибки ESME_RCANCELFAIL в поле command_status, то действительно либо реквест неправильный (source_addr или dest_addr не совпадает с оригинальным сообщением и т.п.), либо нет сообщений удовлетворяющих условиям (нет сообщений, сообщение в финальном состоянии и т.п.)");
	__reg_tc__("cancelSm.resp.checkCmdStatusOther",
		"Прочие коды ошибок соответствуют спецификации");
}

void smsTc()
{
	__reg_tc__("sms", "Получение sms от SC");
	__reg_tc__("sms.deliverySm", "Получение deliver_sm");
	__reg_tc__("sms.deliverySm.checkFields",
		"Общие проверки полей хедера и некоторых других полей deliver_sm");
	__reg_tc__("sms.dataSm", "Получение data_sm");
	__reg_tc__("sms.dataSm.checkFields",
		"Общие проверки полей хедера и некоторых других полей data_sm");
}

void normalSmsTc()
{
	//sms.normalSms
	__reg_tc__("sms.normalSms",
		"Получение обычной sms (deliver_sm, data_sm) не являющаяся нотификацией");
	__reg_tc__("sms.normalSms.checkAllowed",
		"Проверка правомерности получения sms");
	__reg_tc__("sms.normalSms.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и алиасу получателя)");
	__reg_tc__("sms.normalSms.checkMandatoryFields",
		"Сравнение обязательных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm, data_sm) pdu");
	__reg_tc__("sms.normalSms.checkSourceAddr",
		"Проверка правильности адреса отправителя");
	__reg_tc__("sms.normalSms.checkSourceAddr.hideDirective",
		"Если в настройках sme получателя wantAlias = true и в sms задана директива hide, то происходит сокрытие адреса отправителя (независимо от настроек профиля отправителя)");
	__reg_tc__("sms.normalSms.checkSourceAddr.unhideDirective",
		"Если в настройках sme получателя wantAlias = true и в sms задана директива unhide, то сокрытие адреса отправителя не происходит (независимо от настроек профиля отправителя)");
	__reg_tc__("sms.normalSms.checkSourceAddr.nohideDirective",
		"Если в настройках sme получателя wantAlias = true и в sms не задана hide/unhide директива, то сокрытие адреса отправителя происходит в зависимости настроек профиля отправителя");
	__reg_tc__("sms.normalSms.checkSourceAddr.notWantAlias",
		"Если в настройках sme получателя wantAlias = false, то сокрытие адреса отправителя не происходит (независимо от настроек профиля отправителя и наличия hide директивы в sms)");
	__reg_tc__("sms.normalSms.checkOptionalFields",
		"Сравнение опциональных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm, data_sm) pdu");
	__reg_tc__("sms.normalSms.scheduleChecks",
		"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
	__reg_tc__("sms.normalSms.ussdServiceOp",
		"Если установлено опциональное поле ussd_service_op, то SC выполняет единственную попытку доставки сообщения");
	//sms.normalSms.notMap
	__reg_tc__("sms.normalSms.notMap",
		"Sms получателем которых являются sme, а не мобильные телефоны");
	__reg_tc__("sms.normalSms.notMap.checkDeliverSm",
		"Если sms доставляется как deliver_sm, то оригинальная sms либо была отправлена как submit_sm, либо была замещена replace_sm или submit_sm с установленным replace_if_present");
	__reg_tc__("sms.normalSms.notMap.checkDataSm",
		"Если sms доставляется как data_sm, то оригинальная sms была отправлена как data_sm и не была замещена replace_sm и submit_sm");
	__reg_tc__("sms.normalSms.notMap.checkDataCoding",
		"Кодировка текста полученного sms соответствует настройкам профиля получателя на момент начала доставки");
	__reg_tc__("sms.normalSms.notMap.checkEqualDataCoding",
		"Если сообщение отправлено в кодировке, которую понимает получатель, поля short_message (для deliver_sm) и/или message_payload проносятся без изменений");
	__reg_tc__("sms.normalSms.notMap.checkDiffDataCoding",
		"Если сообщение отправлено в ucs2, а в профиле получателя прописано default, то значение полей short_message (для deliver_sm) и/или message_payload правильно транслитерируется с учетом udh");
	//sms.normalSms.map
	__reg_tc__("sms.normalSms.map",
		"Sms получателем которых являются мобильные телефоны");
	__reg_tc__("sms.normalSms.map.checkDataCoding",
		"Кодировка текста полученного sms соответствует настройкам профиля получателя на момент отправки сообщения");
	//sms.normalSms.map.shortSms
	__reg_tc__("sms.normalSms.map.shortSms",
		"Короткие сообщения (<=140 байт) доставляются одним sms");
	__reg_tc__("sms.normalSms.map.shortSms.checkDeliverSm",
		"Если sms доставляется как deliver_sm, то оригинальная sms либо была отправлена как submit_sm, либо была замещена replace_sm или submit_sm с установленным replace_if_present");
	__reg_tc__("sms.normalSms.map.shortSms.checkDataSm",
		"Если sms доставляется как data_sm, то оригинальная sms была отправлена как data_sm и не была замещена replace_sm и submit_sm");
	__reg_tc__("sms.normalSms.map.shortSms.checkEqualDataCoding",
		"Если сообщение отправлено в кодировке, которую понимает получатель, поля short_message (для deliver_sm) и/или message_payload проносятся без изменений");
	__reg_tc__("sms.normalSms.map.shortSms.checkDiffDataCoding",
		"Если сообщение отправлено в ucs2, а в профиле получателя прописано default, то значение полей short_message (для deliver_sm) и/или message_payload правильно транслитерируется с учетом udh");
	//sms.normalSms.map.longSms
	__reg_tc__("sms.normalSms.map.longSms",
		"Длинные сообщения (>140 байт) без udhi нарезаются на сегменты");
	__reg_tc__("sms.normalSms.map.longSms.checkDeliverSm",
		"Сегменты доставляются как deliver_sm независимо от того была оригинальная sms отправлена как submit_sm или как data_sm");
	__reg_tc__("sms.normalSms.map.longSms.checkEqualDataCoding",
		"Если сообщение отправлено в кодировке, которую понимает получатель, сегменты оригинального текста пересылаются в поле short_message");
	__reg_tc__("sms.normalSms.map.longSms.checkDiffDataCoding",
		"Если сообщение отправлено в ucs2, а в профиле получателя прописано default, то сегменты транслитерированного текста пересылаются в поле short_message");
}

void reportsTc()
{
	//sms.reports
	__reg_tc__("sms.reports",
		"Отчеты о доставке (подтверждения доставки и промежуточные нотификации)");
	__reg_tc__("sms.reports.checkDeliverSm",
		"Отчеты о доставке всегда доставляются как deliver_sm независимо от того, была отправлена оригинальная sms как submit_sm или как data_sm");
	__reg_tc__("sms.reports.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
	__reg_tc__("sms.reports.checkFields",
		"Общая проверка правильности полей");
	//sms.reports.priorityCheck
	__reg_tc__("sms.reports.priorityCheck",
		"Приоритеты проверки необходимости отправки отчеты о доставке");
	__reg_tc__("sms.reports.priorityCheck.ussdServiceOp",
		"Если установлено опциональное поле ussd_service_op, то SC не отправляет отчеты о доставке (независимо от наличия директивы #ack#)");
	__reg_tc__("sms.reports.priorityCheck.directiveAck",
		"Если задана директива #ack#, то SC отправляет отчеты о доставке (независимо от наличия на маршруте флага suppressDeliveryReports)");
	__reg_tc__("sms.reports.priorityCheck.directiveNoAck",
		"Если задана директива #noack#, то SC не отправляет отчеты о доставке (независимо от значения поля registered_delivery в pdu)");
	__reg_tc__("sms.reports.priorityCheck.suppressDeliveryReports",
		"Если на маршруте установлен флаг suppressDeliveryReports, то SC не отправляет отчеты о доставке (независимо от значения поля registered_delivery в pdu)");
	__reg_tc__("sms.reports.priorityCheck.profileReportFull",
		"Если в профиле отправителя на момент отправки sms установлен режим отчетов REPORT_FULL, то SC отправляет отчеты о доставке (независимо от значения поля registered_delivery в pdu)");
	//sms.reports.deliveryReceipt
	__reg_tc__("sms.reports.deliveryReceipt",
		"Подтверждения доставки");
	__reg_tc__("sms.reports.deliveryReceipt.transmitter",
		"SC отправляет подтверждение доставки со статусом EXPIRED в момент времени validity_period, если sme-получатель зарегистрирован как transmitter");
	__reg_tc__("sms.reports.deliveryReceipt.notBound",
		"SC отправляет подтверждение доставки со статусом EXPIRED в момент времени validity_period, если нет соединения с sme-получателем");
	__reg_tc__("sms.reports.deliveryReceipt.failureDeliveryReceipt",
		"Подтверждение доставки на ошибку не доставляется в случае успешной доставки оригинального сообщения");
	__reg_tc__("sms.reports.deliveryReceipt.expiredDeliveryReceipt",
		"Подтверждение доставки при истечении срока валидности доставляется в момент времени validity_period, даже если последняя попытка доставки прошла раньше validity_period");
	__reg_tc__("sms.reports.deliveryReceipt.checkAllowed",
		"Проверка правомерности получения подтверждений доставки (в зависимости от настроек профиля и поля pdu registered_delivery, единственный раз по окончании доставки оригинальной pdu)");
	__reg_tc__("sms.reports.deliveryReceipt.recvTimeChecks",
		"Подтверждения доставки приходят в момент окончании доставки оригинальной pdu");
	__reg_tc__("sms.reports.deliveryReceipt.checkStatus",
		"Информации о статусе доставленной pdu (состояние, код ошибки) является корректной");
	__reg_tc__("sms.reports.deliveryReceipt.checkText",
		"Текст сообщения соответствует настройкам SC");
	//sms.reports.intermediateNotification
	__reg_tc__("sms.reports.intermediateNotification",
		"Промежуточные нотификации");
	__reg_tc__("sms.reports.intermediateNotification.transmitter",
		"SC отправляет промежуточную нотификацию в момент времени schedule_delivery_time, если sme-получатель зарегистрирован как transmitter");
	__reg_tc__("sms.reports.intermediateNotification.notBound",
		"SC отправляет промежуточную нотификацию в момент времени schedule_delivery_time, если нет соединения с sme-получателем");
	__reg_tc__("sms.reports.intermediateNotification.checkAllowed",
		"Проверка правомерности получения промежуточных нотификаций (в зависимости от настроек профиля и поля pdu registered_delivery, единственный раз после первой зарешедуленой попытки доставки)");
	__reg_tc__("sms.reports.intermediateNotification.noRescheduling",
		"Промежуточная нотификация не доставляется в случае, если sms не была зарешедулена");
	__reg_tc__("sms.reports.intermediateNotification.recvTimeChecks",
		"Время доставки промежуточной нотификации соответсвует времени первой зарешедуленой попытки доставки оригинальной pdu");
	__reg_tc__("sms.reports.intermediateNotification.checkStatus",
		"Информации о статусе pdu (состояние, код ошибки) является корректной");
	__reg_tc__("sms.reports.intermediateNotification.checkText",
		"Текст сообщения соответствует настройкам SC");
	//sms.reports.smsCancelledNotification
	__reg_tc__("sms.reports.smsCancelledNotification",
		"Нотификации на удаляемые sms");
}

void smeAckTc()
{
	//sms.smeAck
	__reg_tc__("sms.smeAck",
		"Ответные сообщения от внутренних и внешних sme (profiler, abonent info, db sme и т.п.)");
	__reg_tc__("sms.smeAck.checkDeliverSm",
		"Ответные сообщение всегда доставляются как deliver_sm независимо от того, была отправлена оригинальная sms как submit_sm или как data_sm");
	__reg_tc__("sms.smeAck.checkAllowed",
		"Проверка правомерности получения сообщения");
	__reg_tc__("sms.smeAck.checkRoute",
		"Проверка правильности маршрута (определение sme по адресу отправителя и адресу внутренней sme SC)");
	__reg_tc__("sms.smeAck.checkFields",
		"Общая проверка правильности полей");
	__reg_tc__("sms.smeAck.recvTimeChecks",
		"Правильное время получения сообщений");
}

void smsRespTc()
{
	__reg_tc__("smsResp", "Отправка респонсов на SC");
	//smsResp.deliverySm
	__reg_tc__("smsResp.deliverySm", "Отправка deliver_sm_resp pdu");
	__reg_tc__("smsResp.deliverySm.sync",
		"Отправка синхронного deliver_sm_resp pdu");
	__reg_tc__("smsResp.deliverySm.async",
		"Отправка асинхронного deliver_sm_resp pdu");
	__reg_tc__("smsResp.deliverySm.delay",
		"Отправка deliver_sm_resp pdu с задержкой меньшей sme timeout");
	__reg_tc__("smsResp.deliverySm.sendOk",
		"Отправить респонс подтверждающий получение правильного deliver_sm");
	//smsResp.deliverySm.sendRetry
	__reg_tc__("smsResp.deliverySm.sendRetry",
		"Отправить респонс для форсирования повторной доставки deliver_sm");
	__reg_tc__("smsResp.deliverySm.sendRetry.notSend",
		"Не отпралять респонс, имитация неполучения deliver_sm");
	__reg_tc__("smsResp.deliverySm.sendRetry.tempAppError",
		"Отправить статус ESME_RX_T_APPN (временная ошибка на стороне sme, запрос на повторную доставку)");
	__reg_tc__("smsResp.deliverySm.sendRetry.msgQueueFull",
		"Отправить статус ESME_RMSGQFUL (переполнение очереди стороне sme)");
	__reg_tc__("smsResp.deliverySm.sendRetry.invalidSequenceNumber",
		"Отправить респонс с неправильным sequence_number");
	__reg_tc__("smsResp.deliverySm.sendRetry.sendAfterSmeTimeout",
		"Отправить респонс после sme timeout");
	//smsResp.deliverySm.sendError
	__reg_tc__("smsResp.deliverySm.sendError",
		"Отправить респонс для прекращения повторной доставки deliver_sm");
	__reg_tc__("smsResp.deliverySm.sendError.standardError",
		"Респонс со стандартным кодом ошибки (диапазон 0x1-0x10f)");
	__reg_tc__("smsResp.deliverySm.sendError.reservedError",
		"Респонс с зарезирвированным кодом ошибки (диапазоны 0x110-0x3ff и 0x400-0x4ff)");
	__reg_tc__("smsResp.deliverySm.sendError.outRangeError",
		"Респонс с кодом ошибки вне диапазона определенного спецификацией SMPP (>0x500)");
	__reg_tc__("smsResp.deliverySm.sendError.permanentAppError",
		"Респонс с кодом ошибки ESME_RX_P_APPN (неустранимая ошибка на стороне sme, отказ от всех последующих сообщений)");
	//smsResp.dataSm
	__reg_tc__("smsResp.dataSm", "Отправка data_sm_resp pdu");
	__reg_tc__("smsResp.dataSm.sync",
		"Отправка синхронного data_sm_resp pdu");
	__reg_tc__("smsResp.dataSm.async",
		"Отправка асинхронного data_sm_resp pdu");
	__reg_tc__("smsResp.dataSm.delay",
		"Отправка data_sm_resp pdu с задержкой меньшей sme timeout");
	__reg_tc__("smsResp.dataSm.sendOk",
		"Отправить респонс подтверждающий получение правильного data_sm");
	//smsResp.dataSm.sendRetry
	__reg_tc__("smsResp.dataSm.sendRetry",
		"Отправить респонс для форсирования повторной доставки data_sm");
	__reg_tc__("smsResp.dataSm.sendRetry.notSend",
		"Не отпралять респонс, имитация неполучения data_sm");
	__reg_tc__("smsResp.dataSm.sendRetry.tempAppError",
		"Отправить статус ESME_RX_T_APPN (временная ошибка на стороне sme, запрос на повторную доставку)");
	__reg_tc__("smsResp.dataSm.sendRetry.msgQueueFull",
		"Отправить статус ESME_RMSGQFUL (переполнение очереди стороне sme)");
	__reg_tc__("smsResp.dataSm.sendRetry.invalidSequenceNumber",
		"Отправить респонс с неправильным sequence_number");
	__reg_tc__("smsResp.dataSm.sendRetry.sendAfterSmeTimeout",
		"Отправить респонс после sme timeout");
	//smsResp.dataSm.sendError
	__reg_tc__("smsResp.dataSm.sendError",
		"Отправить респонс для прекращения повторной доставки data_sm");
	__reg_tc__("smsResp.dataSm.sendError.standardError",
		"Респонс со стандартным кодом ошибки (диапазон 0x1-0x10f)");
	__reg_tc__("smsResp.dataSm.sendError.reservedError",
		"Респонс с зарезирвированным кодом ошибки (диапазоны 0x110-0x3ff и 0x400-0x4ff)");
	__reg_tc__("smsResp.dataSm.sendError.outRangeError",
		"Респонс с кодом ошибки вне диапазона определенного спецификацией SMPP (>0x500)");
	__reg_tc__("smsResp.dataSm.sendError.permanentAppError",
		"Респонс с кодом ошибки ESME_RX_P_APPN (неустранимая ошибка на стороне sme, отказ от всех последующих сообщений)");
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
	//protocolError.invalidBind
	__reg_tc__("protocolError.invalidBind",
		"Отправка неправильной bind pdu (сразу после установления соединения с SC)");
	//protocolError.invalidBind.pduSize
	__reg_tc__("protocolError.invalidBind.pduSize",
		"Отправка bind pdu неправильный размера");
	__reg_tc__("protocolError.invalidBind.pduSize.smallerSize1",
		"Размер pdu меньше 16 байт (размер хедера)");
	__reg_tc__("protocolError.invalidBind.pduSize.smallerSize2",
		"Размер pdu меньше правильного");
	__reg_tc__("protocolError.invalidBind.pduSize.greaterSize1",
		"Размер pdu больше правильного, но меньше 64kb");
	__reg_tc__("protocolError.invalidBind.pduSize.greaterSize2",
		"Размер pdu больше 100kb");
	__reg_tc__("protocolError.invalidBind.pduSize.connectionClose",
		"При получении bind pdu неправильного размера SC закрывает соединение");
	//protocolError.invalidBind.cmdId
	__reg_tc__("protocolError.invalidBind.cmdId",
		"Отправка pdu с command_id отличным от bind_transmitter, bind_receiver и bind_transceiver");
	__reg_tc__("protocolError.invalidBind.cmdId.allowedCmdId",
		"Правильная pdu с command_id разрешенным для отправки со стороны sme");
	__reg_tc__("protocolError.invalidBind.cmdId.unbindBeforeBind",
		"Отправка unbind реквеста");
	__reg_tc__("protocolError.invalidBind.cmdId.notAllowedCmdId",
		"Правильная pdu с command_id разрешенным для отправки исключительно со стороны SC");
	__reg_tc__("protocolError.invalidBind.cmdId.nonExistentCmdId",
		"Неправильная pdu с command_id несоответствующим спецификации");
	__reg_tc__("protocolError.invalidBind.cmdId.generickNack",
		"При получении pdu с command_id отличным от bind SC отправляет generic_nack с command_status = ESME_RINVBNDSTS");
	//protocolError.invalidPdu
	__reg_tc__("protocolError.invalidPdu",
		"Отправка неправильных pdu после установления соединения и успешного bind");
	//protocolError.invalidPdu.pduSize
	__reg_tc__("protocolError.invalidPdu.pduSize",
		"Отправка pdu неправильного размера");
	__reg_tc__("protocolError.invalidPdu.pduSize.smallerSize1",
		"Размер pdu меньше 16 байт (размер хедера)");
	__reg_tc__("protocolError.invalidPdu.pduSize.smallerSize2",
		"Размер pdu меньше правильного");
	__reg_tc__("protocolError.invalidPdu.pduSize.greaterSize1",
		"Размер pdu больше правильного, но меньше 64kb");
	__reg_tc__("protocolError.invalidPdu.pduSize.greaterSize2",
		"Размер pdu больше 100kb");
	__reg_tc__("protocolError.invalidPdu.pduSize.connectionClose",
		"При получении pdu неправильного размера SC закрывает соединение");
	//protocolError.invalidPdu.cmdId
	__reg_tc__("protocolError.invalidPdu.cmdId",
		"Отправка pdu с неправильным command_id");
	__reg_tc__("protocolError.invalidPdu.cmdId.notAllowedCmdId",
		"Правильная pdu с command_id разрешенным для отправки исключительно со стороны SC");
	__reg_tc__("protocolError.invalidPdu.cmdId.nonExistentCmdId",
		"Неправильная pdu с command_id несоответствующим спецификации");
	__reg_tc__("protocolError.invalidPdu.cmdId.generickNack",
		"При получении pdu с неправильным command_id SC отправляет generic_nack с command_status = ESME_RINVCMDID");
	//protocolError.submitAfterUnbind
	__reg_tc__("protocolError.submitAfterUnbind",
		"Отправка pdu после unbind (сетевое соединение остается открытым)");
	//protocolError.submitAfterUnbind.pduSize
	__reg_tc__("protocolError.submitAfterUnbind.pduSize",
		"Отправка pdu неправильного размера");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.smallerSize1",
		"Размер pdu меньше 16 байт (размер хедера)");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.smallerSize2",
		"Размер pdu меньше правильного");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.greaterSize1",
		"Размер pdu больше правильного, но меньше 64kb");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.greaterSize2",
		"Размер pdu больше 100kb");
	__reg_tc__("protocolError.submitAfterUnbind.pduSize.connectionClose",
		"При получении pdu неправильного размера SC закрывает соединение");
	//protocolError.submitAfterUnbind.cmdId
	__reg_tc__("protocolError.submitAfterUnbind.cmdId",
		"Отправка pdu с command_id отличным от bind_transmitter, bind_receiver и bind_transceiver");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.allowedCmdId",
		"Правильная pdu с command_id разрешенным для отправки со стороны sme");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.duplicateUnbind",
		"Повторный unbind реквест");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.notAllowedCmdId",
		"Правильная pdu с command_id разрешенным для отправки исключительно со стороны SC");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.nonExistentCmdId",
		"Неправильная pdu с command_id несоответствующим спецификации");
	__reg_tc__("protocolError.submitAfterUnbind.cmdId.generickNack",
		"При получении pdu с command_id отличным от bind SC отправляет generic_nack с command_status = ESME_RINVBNDSTS");
	//protocolError.smeInactivity
	__reg_tc__("protocolError.smeInactivity",
		"Если sme длительное время не проявляет активности, SC начинает отправлять на sme enquire_link реквесты, а потом закрывает соединение");
	__reg_tc__("protocolError.smeInactivity.checkEnquireLinkTime",
		"Первый enquire_link SC отправляет через период времени inactivityTime задаваемый в конфигурации, все последующие через 1 секунду");
	__reg_tc__("protocolError.smeInactivity.checkEnquireLinkFileds",
		"Реквест enquire_link отправляемый SC является корректным");
	__reg_tc__("protocolError.smeInactivity.checkConnectionClose",
		"Если с момента первого enquire_link в течение времени inactivityTimeout задаваемого в конфигурации sme ничего не отправляет, то SC закрывает соединение");
	//other
	__reg_tc__("protocolError.equalSeqNum",
		"Отправка нескольких submit_sm с одинаковым sequence_number не влияет на логику обработки pdu");
	__reg_tc__("protocolError.nullPdu",
		"Отправка pdu с пустыми (NULL) полями");
}

void lockTc()
{
	__reg_tc__("lockedSm", "Обработка ситуаций с залоченными sms");
	__reg_tc__("lockedSm.deliveringState",
		"Если sms находится в DELIVERING состоянии, то выполнение cancel_sm, replace_sm, query_sm и submit_sm с установленным replace_if_present задерживается до выхода sms из DELIVERING состояния или команда протухает по таймауту");
	__reg_tc__("lockedSm.segmentedMap",
		"Если sms отправленная на map proxy доставляется с разбивкой по сегментам и хотя бы один сегмент уже доставлен, то выполнение cancel_sm, replace_sm и submit_sm с установленным replace_if_present становится невозможным");
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

void notImplementedTc()
{
	__reg_tc__("notImplemented", "Не имплементированные тест кейсы");
	__reg_tc__("notImplemented.submitMulti", "Отправка submit_multi pdu");
	__reg_tc__("notImplemented.submitMulti.resp", "Получение submit_multi_resp pdu");
	__reg_tc__("notImplemented.alertNotification", "Получение alert_notification pdu");
}

void allProtocolTc()
{
	bindTc();
	unbindTc();
	enquireLinkTc();
	submitSmTc();
	dataSmTc();
	directiveTc();
	replaceSmTc();
	querySmTc();
	cancelSmTc();

	smsTc();
	normalSmsTc();
	reportsTc();
	smeAckTc();
	smsRespTc();

	checkMissingPduTc();
	protocolError();

	lockTc();
	sendInvalidPduTc();
	processGenericNackTc();
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

