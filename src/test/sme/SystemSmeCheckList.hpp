#ifndef TEST_SME_SYSTEM_SME_CHECK_LIST
#define TEST_SME_SYSTEM_SME_CHECK_LIST

#include "SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SystemSmeCheckList : public SmppProtocolCheckList
{
void profilerTc()
{
	//updateProfile
	__reg_tc__("updateProfile",
		"Отправка smpp сообщений менеджеру профилей");
	__reg_tc__("updateProfile.submitSm",
		"Отправка submit_sm pdu");
	__reg_tc__("updateProfile.dataSm",
		"Отправка data_sm pdu");
	__reg_tc__("updateProfile.cmdTextDefault",
		"Текст команды в дефолтной кодировке SC");
	__reg_tc__("updateProfile.cmdText7bit",
		"Текст команды в 7bit кодировке SC");
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
	//updateProfile.ack
	__reg_tc__("updateProfile.ack",
		"Ответные сообщения от менеджера профилей");
	__reg_tc__("updateProfile.ack.checkFields",
		"Поля pdu соответствуют настройкам менеджера профилей");
	__reg_tc__("updateProfile.ack.checkText",
		"Тект сообщений правильный");
	__reg_tc__("updateProfile.ack.reportOptions",
		"Подтверждения об изменении настроек получения уведомлений о доставке");
	__reg_tc__("updateProfile.ack.reportOptions.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в 7-bit кодировке, при наличии русских симовлов приходит либо в 7-bit (транслитерация), либо в UCS2 в зависимости от текущих настроек профиля");
	__reg_tc__("updateProfile.ack.codePage",
		"Подтверждения об изменении настроек режима приема сообщений на русском языке");
	__reg_tc__("updateProfile.ack.codePage.dataCoding",
		"Текст сообщения в новой кодировке");
	__reg_tc__("updateProfile.ack.incorrectCmdText",
		"Уведомление о неправильном тексте команды");
	__reg_tc__("updateProfile.ack.incorrectCmdText.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в 7-bit кодировке, при наличии русских симовлов приходит либо в 7-bit (транслитерация), либо в UCS2 в зависимости от текущих настроек профиля");
}

void abonentInfoTc()
{
	//queryAbonentInfo
	__reg_tc__("queryAbonentInfo",
		"Отправка smpp сообщений abonent info sme");
	__reg_tc__("queryAbonentInfo.submitSm",
		"Отправка submit_sm pdu");
	__reg_tc__("queryAbonentInfo.dataSm",
		"Отправка data_sm pdu");
	__reg_tc__("queryAbonentInfo.cmdTextDefault",
		"Текст команды в дефолтной кодировке SC");
	__reg_tc__("queryAbonentInfo.cmdText7bit",
		"Текст команды в 7bit кодировке SC");
	__reg_tc__("queryAbonentInfo.cmdTextUcs2",
		"Текст команды в кодировке UCS2");
	__reg_tc__("queryAbonentInfo.smppAddr",
		"Отправка команды на smpp адрес abonent info sme");
	__reg_tc__("queryAbonentInfo.mapAddr",
		"Отправка команды на map (mobile) адрес abonent info sme");
	//queryAbonentInfo.correct
	__reg_tc__("queryAbonentInfo.correct",
		"Отправка корректных запросов на abonent info sme");
	__reg_tc__("queryAbonentInfo.correct.existentAddr",
		"Запрос статуса существующего абонента");
	__reg_tc__("queryAbonentInfo.correct.nonExistentAddr",
		"Запрос статуса несуществующего абонента");
	__reg_tc__("queryAbonentInfo.correct.unifiedAddrFormat",
		"Команда с адресом в унифицированном (.ton.npi.addr) формате");
	__reg_tc__("queryAbonentInfo.correct.humanFormat",
		"Команда с адресом в сокращенной (+7902... или 83832...) записи");
	//queryAbonentInfo.incorrect
	__reg_tc__("queryAbonentInfo.incorrect",
		"Отправка некорректных запросов на abonent info sme");
	__reg_tc__("queryAbonentInfo.incorrect.addrFormat",
		"Неправильный формат адреса");
	__reg_tc__("queryAbonentInfo.incorrect.addrLength",
		"Длина адреса больше максимальной (20 символов)");
	__reg_tc__("queryAbonentInfo.incorrect.invalidSymbols",
		"Недопустимые символы в строке адреса");
	__reg_tc__("queryAbonentInfo.incorrect.extraWords",
		"В тексте команды лишние слова (должен быть только адрес)");
	__reg_tc__("queryAbonentInfo.incorrect.tonNpi",
		"Неправильные ton и/или npi в адресе");
	//queryAbonentInfo.ack
	__reg_tc__("queryAbonentInfo.ack",
		"Ответные сообщения от abonent info sme");
	__reg_tc__("queryAbonentInfo.ack.checkFields",
		"Поля pdu соответствуют настройкам abonent info sme");
	__reg_tc__("queryAbonentInfo.ack.checkText",
		"Тект сообщений правильный");
}

void smscSmeTc()
{
	//updateProfile
	__reg_tc__("submitPduToSmscSme",
		"Отправка smpp сообщений smsc sme");
}

public:
	SystemSmeCheckList()
	: SmppProtocolCheckList("Результаты функционального тестирования Profiler, AbonentInfo и SmscSme через протокол SMPP", "system_sme.chk")
	{
		profilerTc();
		abonentInfoTc();
		smscSmeTc();
	}

protected:
	SystemSmeCheckList(const char* name, const char* fileName)
	: SmppProtocolCheckList(name, fileName)
	{
		profilerTc();
		abonentInfoTc();
		smscSmeTc();
	}
};

}
}
}

#endif /* TEST_SME_SYSTEM_SME_CHECK_LIST */

