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
public:
	SmppCheckList()
		: CheckList("Результаты функционального тестирования BaseSme и протокола SMPP", "smpp.chk")
	{
		//bindCorrectSme
		__reg_tc__("bindCorrectSme",
			"Установление IP соединения с SC и регистрация");
		__reg_tc__("bindCorrectSme.bindReceiver",
			"Регистрация ресивера с помощью bind_receiver pdu");
		__reg_tc__("bindCorrectSme.bindTransmitter",
			"Регистрация трансмиттера с помощью bind_transmitter pdu");
		__reg_tc__("bindCorrectSme.bindTransceiver",
			"Регистрация трансивера с помощью bind_transceiver pdu");
		//bindIncorrectSme
		__reg_tc__("bindIncorrectSme",
			"Установление IP соединения с SC и регистрация с неправильными параметрами");
		__reg_tc__("bindIncorrectSme.smeNotRegistered",
			"Значение system_id не прописано в конфигурации SC");
		__reg_tc__("bindIncorrectSme.unknownHost",
			"Установка соединения с недоступным SC (неизвестный хост) завершается корректно");
		__reg_tc__("bindIncorrectSme.invalidPort",
			"Установка соединения с недоступным SC (неправильный порт) завершается корректно");
		//unbind
		__reg_tc__("unbind", "Завершение соединения с SC");
		//submitSm
		__reg_tc__("submitSm",
			"Отправка submit_sm pdu с правильными значениями полей");
		__reg_tc__("submitSm.serviceTypeMarginal",
			"Предельные значения длины поля service_type");
		__reg_tc__("submitSm.waitTimePast",
			"Время schedule_delivery_time в прошлом (доставка уже должна была начаться)");
		__reg_tc__("submitSm.validTimePast",
			"Время validity_period в прошлом (сообщение никогда не будет доставлено)");
		__reg_tc__("submitSm.validTimeExceeded",
			"Время validity_period больше максимально срока валидности установленного для SC");
		__reg_tc__("submitSm.waitTimeInvalid",
			"Значение schedule_delivery_time превышает validity_period");
		__reg_tc__("submitSm.smLengthMarginal",
			"Предельные значения длины тела сообщения");
		__reg_tc__("submitSm.checkMap",
			"Дублированное сообщение согласно MAP (совпадают source_addr, dest_addr и user_message_reference), но согласно SMPP существующее сообщение находящееся в очереди доставки замещено быть не может");
		__reg_tc__("submitSm.notReplace",
			"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) без замещения существующего, находящегося в очереди доставки");
		__reg_tc__("submitSm.serviceTypeNotMatch",
			"Совпадают source_addr и dest_addr, отличается service_type для сообщения находящегося в очереди доставки");
		__reg_tc__("submitSm.replaceEnrote",
			"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением существующего, находящегося в очереди доставки");
		__reg_tc__("submitSm.replaceReplacedEnrote",
			"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением уже ранее замещенного и находящегося в очереди доставки");
		__reg_tc__("submitSm.replaceFinal",
			"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением уже доставленного");
		__reg_tc__("submitSm.replaceRepeatedDeliveryEnrote",
			"Дублированное сообщение (совпадают source_addr, dest_addr и service_type) с замещением существующего, находящегося в очереди повторной доставки");
		__reg_tc__("submitSm.sync"
			"Отправка синхронного submit_sm pdu");
		__reg_tc__("submitSm.async"
			"Отправка асинхронного submit_sm pdu");
		//submitSmAssert
		__reg_tc__("submitSmAssert",
			"Заполнение и отправка submit_sm pdu с недопустимыми значениями полей");
		__reg_tc__("submitSmAssert.serviceTypeInvalid",
			"Недопустимое значение длины поля service_type");
		__reg_tc__("submitSmAssert.destAddrLengthInvalid",
			"Недопустимое значение длины поля адреса получателя");
		__reg_tc__("submitSmAssert.msgLengthInvalid",
			"Недопустимое значение длины текста сообщения short_message");
		//replaceSm
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
		__reg_tc__("replaceSm.sync"
			"Отправка синхронного replace_sm pdu");
		__reg_tc__("replaceSm.async"
			"Отправка асинхронного replace_sm pdu");
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
		//sendDeliverySmResp
		__reg_tc__("sendDeliverySmResp",
			"Отправка deliver_sm_resp pdu");
		__reg_tc__("sendDeliverySmResp.notSend",
			"Не отпралять респонс для форсирования повторной доставки deliver_sm со стороны SC");
		__reg_tc__("sendDeliverySmResp.sendWithErrCode",
			"Отправить респонс с кодом ошибки для форсирования повторной доставки deliver_sm со стороны SC");
		__reg_tc__("sendDeliverySmResp.sendInvalidSequenceNumber",
			"Отправить респонс с неправильным sequence_number");
		__reg_tc__("sendDeliverySmResp.sendOk",
			"Отправить респонс подтверждающий получение правильного deliver_sm");
		__reg_tc__("sendDeliverySmResp.sync",
			"Отправка синхронного deliver_sm_resp pdu");
		__reg_tc__("sendDeliverySmResp.async",
			"Отправка асинхронного deliver_sm_resp pdu");
		//processDeliverySm
		__reg_tc__("processDeliverySm", "Получение deliver_sm pdu");
		__reg_tc__("processDeliverySm.checkFields",
			"Общие проверки полей хедера и некоторых других полей");
		//normalSms
		__reg_tc__("processDeliverySm.normalSms",
			"Обычная sms не являющаяся нотификацией");
		__reg_tc__("processDeliverySm.normalSms.checkAllowed",
			"Проверка правомерности получения sms");
		__reg_tc__("processDeliverySm.normalSms.checkRoute",
			"Проверка правильности маршрута (определение sme по адресу отправителя и алиасу получателя)");
		__reg_tc__("processDeliverySm.normalSms.checkMandatoryFields",
			"Сравнение обязательных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm) pdu");
		__reg_tc__("processDeliverySm.normalSms.checkOptionalFields",
			"Сравнение опциональных полей отправленной (submit_sm, data_sm, replace_sm) и полученной (deliver_sm) pdu");
		__reg_tc__("processDeliverySm.normalSms.scheduleChecks",
			"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
		__reg_tc__("processDeliverySm.normalSms.checkDeliveryReceipt",
			"Подтверждение доставки доставляется не раньше самой sms");
		//deliveryReceipt
		__reg_tc__("processDeliverySm.deliveryReceipt",
			"Подтверждение доставки");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkAllowed",
			"Проверка правомерности получения подтверждения доставки");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkRoute",
			"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkFields",
			"Общая проверка правильности полей");
		__reg_tc__("processDeliverySm.deliveryReceipt.checkStatus",
			"Информации о доставленной pdu является корректной");
		__reg_tc__("processDeliverySm.deliveryReceipt.scheduleChecks",
			"Корректная работа механизма повторной доставки (правильное время, нет пропусков между повторными доставками, отсутствие дублей)");
		//intermediateNotification
		__reg_tc__("processDeliverySm.intermediateNotification",
			"Промежуточные нотификации");
		__reg_tc__("processDeliverySm.intermediateNotification.checkAllowed",
			"Проверка правомерности получения подтверждения промежуточной нотификации");
		__reg_tc__("processDeliverySm.intermediateNotification.checkRoute",
			"Проверка правильности маршрута (определение sme по адресу отправителя и адресу SC)");
		__reg_tc__("processDeliverySm.intermediateNotification.checkFields",
			"Общая проверка правильности полей");
		__reg_tc__("processDeliverySm.intermediateNotification.checkStatus",
			"Информации о состоянии отправленной pdu является корректной");
		__reg_tc__("processDeliverySm.intermediateNotification.checkTime",
			"Правильное время доставки");
		//checkMissingPdu
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
		//not implemented
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
};

}
}
}

#endif /* TEST_SME_SMPP_CHECK_LIST */

