#ifndef TEST_DBSME_DBSME_CHECK_LIST
#define TEST_DBSME_DBSME_CHECK_LIST

#include "test/sme/SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::test::sme::SmppProtocolCheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
#define __hide_tc__(id) \
	hideTc(id)

class DbSmeCheckList : public SmppProtocolCheckList
{

void correctJobTc()
{
	__reg_tc__("submitDbSmeCmd.correct.job",
		"Отправка команд выполняющих различные типы задач");
	__reg_tc__("submitDbSmeCmd.correct.job.insert",
		"Выполнение insert записей в таблицу");
	__reg_tc__("submitDbSmeCmd.correct.job.update",
		"Выполнение update таблицы");
	__reg_tc__("submitDbSmeCmd.correct.job.delete",
		"Выполнение delete записей из таблицы");
	__reg_tc__("submitDbSmeCmd.correct.job.select",
		"Выполнение select записей из таблицы");
}

void correctInputTc()
{
	//input
	__reg_tc__("submitDbSmeCmd.correct.input",
		"Отправка команд с различными типами параметров");
	__reg_tc__("submitDbSmeCmd.correct.input.noParams",
		"Команда без параметров");
	__reg_tc__("submitDbSmeCmd.correct.input.int",
		"Один из параметров является числом типа int8, int16, int32 или int64");
	__reg_tc__("submitDbSmeCmd.correct.input.uint",
		"Один из параметров является числом типа uint8, uint16, uint32 или uint64");
	__reg_tc__("submitDbSmeCmd.correct.input.float",
		"Один из параметров является числом типа float, double или long double");
	__reg_tc__("submitDbSmeCmd.correct.input.string",
		"Один из параметров является строкой (GSM 03.38) из одного слова (без пробелов)");
	__reg_tc__("submitDbSmeCmd.correct.input.quotedString",
		"Один из параметров является строкой (GSM 03.38) в двойных ковычках (включая пробелы)");
	__reg_tc__("submitDbSmeCmd.correct.input.date",
		"Один из параметров является датой или временем");
	__reg_tc__("submitDbSmeCmd.correct.input.jobName",
		"Один из параметров является именем задачи job-name");
	__reg_tc__("submitDbSmeCmd.correct.input.toAddress",
		"Один из параметров является адресом получателя to-address");
	__reg_tc__("submitDbSmeCmd.correct.input.fromAddress",
		"Один из параметров является адресом отправителя from-address");
	//defaultInput
	__reg_tc__("submitDbSmeCmd.correct.defaultInput",
		"Отправка команд с отсутствующими значениями для параметров допускающих значения по умолчанию");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.int",
		"Значения по умолчанию для целочисленных параметров типа int8, int16, int32 или int64");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.uint",
		"Значения по умолчанию для целочисленных параметров типа uint8, uint16, uint32 или uint64");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.float",
		"Значения по умолчанию для параметров типа float, double или long double");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.string",
		"Значения по умолчанию для строковых (GSM 03.38) параметров");
	__reg_tc__("submitDbSmeCmd.correct.defaultInput.date",
		"Значения по умолчанию для параметров типа даты или времени");
}

void incorrectInputTc()
{
	__reg_tc__("submitDbSmeCmd.incorrect", "Отправка неправильной команды db sme");
	__reg_tc__("submitDbSmeCmd.incorrect.missingParams",
		"Отсутствуют обязательные параметры");
	__reg_tc__("submitDbSmeCmd.incorrect.extraParams",
		"Присутствуют лишние параметры");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidJob",
		"Неправильное имя задачи");
	//dateFormat
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat",
		"Формат даты не соответствует ожидаемому");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidDay",
		"Неправильное задано число (равно 0, больше 31 и т.п.)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth",
		"Неправильно задан месяц (равен 0, больше 13, ожидается Jan, получено 01 и т.п.)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidYear",
		"Неправильно задан год (по формату ожидается 2002, получено 02 и наоборот)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour",
		"Неправильно задан час (0 am, 0 pm, 13am, 13 pm, 24 и больше)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMinute",
		"Неправильно задана минута (60 и больше)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidSecond",
		"Неправильно задана секунда (60 и больше)");
	__reg_tc__("submitDbSmeCmd.incorrect.dateFormat.invalidIndicator",
		"Неправильно задан индикатор am/pm");
	//invalidNumber
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber",
		"Недопустимые значения числовых параметров");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber.int",
		"Недопустимые значения параметров int8, int16, int32, int64 (больше максимально допустимого, меньше минимально допустимого, нецелочисленное, строка и т.п.)");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber.uint",
		"Недопустимые значения параметров uint8, uint16, uint32, uint64 (больше максимально допустимого, меньше минимально допустимого, нецелочисленное, строка и т.п.)");
	__reg_tc__("submitDbSmeCmd.incorrect.invalidNumber.float",
		"Недопустимые значения параметров float, double, long-double (больше максимально допустимого, строка и т.п.)");
	//
	__reg_tc__("submitDbSmeCmd.incorrect.input.stringTooLong",
		"Длина строки больше длины поля");
}

void processDbSmeRes()
{
	__reg_tc__("processDbSmeRes", "Ответные сообщения от db sme");
	__reg_tc__("processDbSmeRes.dataCoding",
		"Ответные сообщения приходят в дефолтной кодировке SC");
	__reg_tc__("processDbSmeRes.serviceType",
		"Ответные сообщения приходят с service_type в соответствии с настройками db sme");
	__reg_tc__("processDbSmeRes.protocolId",
		"Ответные сообщения приходят с protocol_id в соответствии с настройками db sme");
	__reg_tc__("processDbSmeRes.longOutput",
		"В случае большого размера ответного сообщения db sme правильно распределяет ответ по нескольким sms");
	__reg_tc__("processDbSmeRes.longOutputLimit",
		"В случае большого размера ответного сообщения db sme правильно ограничивает количество sms");
	//processDbSmeRes.input
	__reg_tc__("processDbSmeRes.input",
		"Значения параметров правильно зачитываются из input");
	__reg_tc__("processDbSmeRes.input.int",
		"Параметры типа int8, int16, int32, int64, uint8, uint16, uint32 и uint64");
	__reg_tc__("processDbSmeRes.input.float",
		"Параметры типа float, double и long double с учетом формата");
	__reg_tc__("processDbSmeRes.input.string",
		"Строковые параметры");
	__reg_tc__("processDbSmeRes.input.date",
		"Дата и время с учетом формата");
	__reg_tc__("processDbSmeRes.input.jobName",
		"Значение параметра имени задачи job-name вычисляется правильно");
	__reg_tc__("processDbSmeRes.input.toAddress",
		"Значение параметра адреса получателя to-address вычисляется правильно");
	__reg_tc__("processDbSmeRes.input.fromAddress",
		"Значение параметра адреса отправителя from-address вычисляется правильно");
	//processDbSmeRes.defaultInput
	__reg_tc__("processDbSmeRes.defaultInput",
		"Для отсутствующих input параметров берутся значения по умолчанию");
	__reg_tc__("processDbSmeRes.defaultInput.int",
		"Константа по умолчанию для целочисленных параметров типа int8, int16, int32, int64, uint8, uint16, uint32 и uint64");
	__reg_tc__("processDbSmeRes.defaultInput.float",
		"Константа по умолчанию для параметров типа float, double и long double");
	__reg_tc__("processDbSmeRes.defaultInput.string",
		"Константа по умолчанию для строковых (GSM 03.38) параметров");
	__reg_tc__("processDbSmeRes.defaultInput.now",
		"Значение по умолчанию now для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.defaultInput.today",
		"Значение по умолчанию today для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.defaultInput.yesterday",
		"Значение по умолчанию yesterday для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.defaultInput.tomorrow",
		"Значение по умолчанию tomorrow для даты вычисляется правильно");
	//processDbSmeRes.output
	__reg_tc__("processDbSmeRes.output",
		"Текст сообщений соответствует шаблону (значения из БД, выравнивание полей, формат даты, escape символы)");
	__reg_tc__("processDbSmeRes.output.dateFormat",
		"Дата выводится в соответствии с output форматом");
	__reg_tc__("processDbSmeRes.output.jobName",
		"Значение параметра имени задачи job-name вычисляется правильно");
	__reg_tc__("processDbSmeRes.output.toAddress",
		"Значение параметра адреса получателя to-address вычисляется правильно");
	__reg_tc__("processDbSmeRes.output.fromAddress",
		"Значение параметра адреса отправителя from-address вычисляется правильно");
	__reg_tc__("processDbSmeRes.output.string",
		"Форматирование строковых полей выполняется правильно");
	__reg_tc__("processDbSmeRes.output.string.left",
		"Выравнивание по левому краю");
	__reg_tc__("processDbSmeRes.output.string.right",
		"Выравнивание по правому краю");
	__reg_tc__("processDbSmeRes.output.string.center",
		"Выравнивание по центру");
	//processDbSmeRes.select
	__reg_tc__("processDbSmeRes.select",
		"Запросы на выборку данных");
	__reg_tc__("processDbSmeRes.select.defaultOutput",
		"При отсутствии записей удовлетворяющих условиям запроса или нулевых (NULL) значениях полей берутся output значения по умолчанию");
	__reg_tc__("processDbSmeRes.select.defaultOutput.int",
		"Константа по умолчанию для целочисленных параметров типа int8, int16, int32, int64, uint8, uint16, uint32 и uint64");
	__reg_tc__("processDbSmeRes.select.defaultOutput.float",
		"Константа по умолчанию для параметров типа float, double и long double");
	__reg_tc__("processDbSmeRes.select.defaultOutput.string",
		"Константа по умолчанию для строковых (GSM 03.38) параметров");
	__reg_tc__("processDbSmeRes.select.defaultOutput.now",
		"Значение по умолчанию now для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.select.defaultOutput.today",
		"Значение по умолчанию today для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.select.defaultOutput.yesterday",
		"Значение по умолчанию yesterday для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.select.defaultOutput.tomorrow",
		"Значение по умолчанию tomorrow для даты вычисляется правильно");
	__reg_tc__("processDbSmeRes.select.noRecordNoDefaults",
		"При отсутствии записей удовлетворяющих условиям запроса и отсутствии значений по умолчанию, возвращается сообщение об ошибке");
	__reg_tc__("processDbSmeRes.select.nullsNoDefaults",
		"Если в какой-либо записи из результата запроса есть поле со значением NULL, для которого нет значения по умолчанию, тогда для данной записи возвращается сообщение об ошибке");
	__reg_tc__("processDbSmeRes.select.singleRecord",
		"Если условиям запроса удовлетворяет единственная запись, значения полей в output берутся из результатов запроса");
	__reg_tc__("processDbSmeRes.select.multipleRecords",
		"Если несколько записей удовлетворяющих условиям запроса, значения полей в output берутся для каждой записи и отделяются пустой строкой");
	//processDbSmeRes.insert
	__reg_tc__("processDbSmeRes.insert",
		"Запросы на добавление (insert) записей в таблицу");
	__reg_tc__("processDbSmeRes.insert.ok",
		"При успешном выполнении операции db sme отправляет подтверждение согласно output шаблона");
	__reg_tc__("processDbSmeRes.insert.duplicateKey",
		"При нарушении уникальности ключей таблицы db sme отправляет сообщение об ошибке");
	//processDbSmeRes.update
	__reg_tc__("processDbSmeRes.update",
		"Запросы на обновление (update) записей в таблице");
	__reg_tc__("processDbSmeRes.update.ok",
		"При успешном выполнении операции db sme отправляет подтверждение согласно output шаблона");
	__reg_tc__("processDbSmeRes.update.duplicateKey",
		"При нарушении уникальности ключей таблицы db sme отправляет сообщение об ошибке");
	__reg_tc__("processDbSmeRes.update.recordsAffected",
		"Количество обновленных записей подсчитывается правильно");
	//processDbSmeRes.delete
	__reg_tc__("processDbSmeRes.delete",
		"Запросы на удаление (delete) записей из таблицы");
	__reg_tc__("processDbSmeRes.delete.ok",
		"При успешном выполнении операции db sme отправляет подтверждение согласно output шаблона");
	__reg_tc__("processDbSmeRes.delete.failure",
		"При невозможности удаления записей db sme отправляет сообщение об ошибке");
	__reg_tc__("processDbSmeRes.delete.recordsAffected",
		"Количество удаленных записей подсчитывается правильно");
	//processDbSmeRes.errors
	__reg_tc__("processDbSmeRes.errors",
		"Для неправильных команд db sme отправляет соответствующие сообщения об ошибках");
	__reg_tc__("processDbSmeRes.errors.invalidConfig",
		"Неправильная конфигурация задачи (sql, input, output)");
}

public:
	DbSmeCheckList()
		: SmppProtocolCheckList("Результаты функционального тестирования db sme", "dbsme.chk")
	{
		__hide_tc__("bindIncorrectSme");
		__hide_tc__("submitSm.correct");
		__hide_tc__("submitSm.incorrect");
		__hide_tc__("submitSm.assert");
		__hide_tc__("replaceSm");
		__hide_tc__("sendDeliverySmResp.sendRetry");
		__hide_tc__("sendDeliverySmResp.sendError");
		__hide_tc__("processReplaceSmResp");
		__hide_tc__("processDeliverySm.normalSms");
		//__hide_tc__("processDeliverySm.deliveryReceipt");
		__hide_tc__("processDeliverySm.intermediateNotification");
		__hide_tc__("notImplemented");

		//submitDbSmeCmd.correct
		__reg_tc__("submitDbSmeCmd", "Отправка sms на db sme");
		__reg_tc__("submitDbSmeCmd.cmdTextDefault",
			"Текст команды в дефолтной кодировке SC");
		__reg_tc__("submitDbSmeCmd.cmdTextUcs2",
			"Текст команды в кодировке UCS2");
		__reg_tc__("submitDbSmeCmd.correct", "Отправка правильной команды db sme");
		correctJobTc();
		correctInputTc();
		incorrectInputTc();
		//result
		processDbSmeRes();
	}
};

}
}
}

#endif /* TEST_DBSME_DBSME_CHECK_LIST */

