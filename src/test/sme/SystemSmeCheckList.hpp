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
	__reg_tc__("updateProfile.cmdTextMixedCase",
		"Текст команды в смешанном регистре");
	__reg_tc__("updateProfile.cmdTextExtraWhiteSpaces",
		"Текст команды с лишними пробелами (в начале, середине, конце)");
	__reg_tc__("updateProfile.reportOptions",
		"Изменение опций уведомления о доставке");
	__reg_tc__("updateProfile.reportOptions.reportNone",
		"Отказаться от уведомления о доставке (report none)");
	__reg_tc__("updateProfile.reportOptions.reportFull",
		"Установить прием уведомлений о доставке (report full)");
	__reg_tc__("updateProfile.reportOptions.reportFinal",
		"Установить прием уведомлений о результате доставке (report final)");
	__reg_tc__("updateProfile.reportOptions.reportNoneUssd",
		"Отказаться от уведомления о доставке командой ussd (*50*0#)");
	__reg_tc__("updateProfile.reportOptions.reportFullUssd",
		"Установить прием уведомлений о доставке командой ussd (*50*2#)");
	__reg_tc__("updateProfile.reportOptions.reportFinalUssd",
		"Установить прием уведомлений о результате доставке командой ussd (*50*1#)");
	__reg_tc__("updateProfile.dataCoding",
		"Изменение режима приема сообщений");
	__reg_tc__("updateProfile.dataCoding.ucs2",
		"Установить режим приема сообщений на русском языке (ucs2)");
	__reg_tc__("updateProfile.dataCoding.default",
		"Отказаться от режима приема сообщений на русском языке (default)");
	__reg_tc__("updateProfile.dataCoding.ucs2Ussd",
		"Установить режим приема сообщений на русском языке командой ussd (*50*4#)");
	__reg_tc__("updateProfile.dataCoding.defaultUssd",
		"Отказаться от режима приема сообщений на русском языке командой ussd (*50*3#)");
	__reg_tc__("updateProfile.locale",
		"Изменение настроек локали");
	__reg_tc__("updateProfile.locale.existentLocale",
		"Правильно заданная локаль (locale en_us, locale ru_ru)");
	__reg_tc__("updateProfile.locale.nonExistentLocale",
		"Заданная локаль не прописана в списке локалей поддерживаемых SC");
	__reg_tc__("updateProfile.locale.enLocaleUssd",
		"Установить локаль en_us командой ussd (*50*21#)");
	__reg_tc__("updateProfile.locale.ruLocaleUssd",
		"Установить локаль ru_ru командой ussd (*50*20#)");
	__reg_tc__("updateProfile.locale.nonExistentLocaleUssd",
		"Заданная локаль не прописана в списке ussd запросов");
	__reg_tc__("updateProfile.hide",
		"Изменение hide опций");
	__reg_tc__("updateProfile.hide.hide",
		"Установить опцию hide (команда hide)");
	__reg_tc__("updateProfile.hide.unhide",
		"Установить опцию unhide (команда unhide)");
	__reg_tc__("updateProfile.hide.hideUssd",
		"Установить опцию hide командой ussd (*50*5#)");
	__reg_tc__("updateProfile.hide.unhideUssd",
		"Установить опцию unhide командой ussd (*50*6#)");
	__reg_tc__("updateProfile.hide.hideDenied",
		"Попытка изменить опцию hide, если для профиля запрещено изменение hide опции");
	__reg_tc__("updateProfile.incorrectCmdText",
		"Неправильный текст команды");
	__reg_tc__("updateProfile.incorrectUssdCmd",
		"Неправильный ussd запрос");
	__reg_tc__("updateProfile.incorrectUssdServiceOp",
		"Неправильное значение поля ussd_service_op (profiler проигнорирует такой ussd запрос)");
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
		"При отсутствии в тексте сообщения русских символов приходит в default кодировке, при наличии русских симовлов приходит либо в default (транслитерация), либо в ucs2 в зависимости от текущих настроек профиля");
	__reg_tc__("updateProfile.ack.codePage",
		"Подтверждения об изменении настроек режима приема сообщений на русском языке");
	__reg_tc__("updateProfile.ack.codePage.dataCoding",
		"Текст сообщения в новой кодировке (ucs2 или default)");
	__reg_tc__("updateProfile.ack.locale",
		"Подтверждения об изменении настроек локали");
	__reg_tc__("updateProfile.ack.locale.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в default кодировке, при наличии русских симовлов приходит либо в default (транслитерация), либо в ucs2 в зависимости от текущих настроек профиля");
	__reg_tc__("updateProfile.ack.hide",
		"Подтверждения об изменении hide опции");
	__reg_tc__("updateProfile.ack.hide.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в default кодировке, при наличии русских симовлов приходит либо в default (транслитерация), либо в ucs2 в зависимости от текущих настроек профиля");
	__reg_tc__("updateProfile.ack.hideDenied",
		"Сообщение об ошибке при попытке изменения hide опции, если для профиля запрещено изменение hide");
	__reg_tc__("updateProfile.ack.hideDenied.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в default кодировке, при наличии русских симовлов приходит либо в default (транслитерация), либо в ucs2 в зависимости от текущих настроек профиля");
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
		"Отправка submit_sm на smsc sme");
	__reg_tc__("dataPduToSmscSme",
		"Отправка data_sm на smsc sme");
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

