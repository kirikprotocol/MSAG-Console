#ifndef TEST_SME_SMPP_PROFILER_CHECK_LIST
#define TEST_SME_SMPP_PROFILER_CHECK_LIST

#include "SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace sme {

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmppProfilerCheckList : public SmppProtocolCheckList
{
void allProfilerTc()
{
	//updateProfile
	__reg_tc__("updateProfile",
		"Отправка smpp сообщений менеджеру профилей");
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
	//processUpdateProfile
	__reg_tc__("processUpdateProfile",
		"Сообщения от менеджера профилей");
	__reg_tc__("processUpdateProfile.checkFields",
		"Поля pdu service_type, source_addr, data_coding, protocol_id, user_message_reference и т.д. правильные (соответствуют настройкам менеджера профилей и полученному сообщению)");
	__reg_tc__("processUpdateProfile.checkText",
		"Тект сообщений правильный");
	__reg_tc__("processUpdateProfile.multipleMessages",
		"Если в конфигурации менеджера профилей прописаны длинные тексты уведомлений, тогда отправляется несколько sms");
	__reg_tc__("processUpdateProfile.reportOptions",
		"Подтверждения об изменении настроек получения уведомлений о доставке");
	__reg_tc__("processUpdateProfile.reportOptions.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в 7-bit кодировке, при наличии русских симовлов приходит либо в 7-bit (транслитерация), либо в UCS2 в зависимости от текущих настроек профиля");
	__reg_tc__("processUpdateProfile.codePage",
		"Подтверждения об изменении настроек режима приема сообщений на русском языке");
	__reg_tc__("processUpdateProfile.codePage.dataCoding",
		"Текст сообщения в новой кодировке");
	__reg_tc__("processUpdateProfile.incorrectCmdText",
		"Уведомление о неправильном тексте команды");
	__reg_tc__("processUpdateProfile.incorrectCmdText.dataCoding",
		"При отсутствии в тексте сообщения русских символов приходит в 7-bit кодировке, при наличии русских симовлов приходит либо в 7-bit (транслитерация), либо в UCS2 в зависимости от текущих настроек профиля");
}

public:
	SmppProfilerCheckList()
		: SmppProtocolCheckList("Результаты функционального тестирования Profiler через протокол SMPP", "smpp_profiler.chk")
	{
		allProfilerTc();
	}

protected:
	SmppProfilerCheckList(const char* name, const char* fileName)
		: SmppProtocolCheckList(name, fileName)
	{
		allProfilerTc();
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_CHECK_LIST */

