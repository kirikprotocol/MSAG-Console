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
public:
	SmppProfilerCheckList()
		: SmppProtocolCheckList("Результаты функционального тестирования Profiler через протокол SMPP", "smpp_profiler.chk")
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
			"Поля pdu source_addr, data_coding, service_type, protocol_id соответствуют настройкам менеджера профилей");
		__reg_tc__("processUpdateProfile.reportOptions",
			"Подтверждения об изменении настроек получения уведомлений о доставке");
		__reg_tc__("processUpdateProfile.codePage",
			"Подтверждения об изменении настроек режима приема сообщений на русском языке");
		__reg_tc__("processUpdateProfile.incorrectCmdText",
			"Уведомление о неправильном тексте команды");
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_CHECK_LIST */

