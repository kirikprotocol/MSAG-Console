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
		: SmppProtocolCheckList("–езультаты функционального тестировани€ Profiler через протокол SMPP", "smpp_profiler.chk")
	{
		//updateProfile
		__reg_tc__("updateProfile.cmdTextDefault",
			"“екст команды в дефолтной кодировке SC");
		__reg_tc__("updateProfile.cmdTextUcs2",
			"“екст команды в кодировке UCS2");
		__reg_tc__("updateProfile.reportOptions",
			"»зменение опций уведомлени€ о доставке");
		__reg_tc__("updateProfile.reportOptions.reportNoneMixedCase",
			"ќтказатьс€ от уведомлени€ о доставке (текст команды в смешанном регистре)");
		__reg_tc__("updateProfile.reportOptions.reportNoneSpaces",
			"ќтказатьс€ от уведомлени€ о доставке (текст команды с лишними пробелами)");
		__reg_tc__("updateProfile.reportOptions.reportFullMixedCase",
			"”становить прием уведомлений о доставке (текст команды в смешанном регистре)");
		__reg_tc__("updateProfile.reportOptions.reportFullSpaces",
			"”становить прием уведомлений о доставке (текст команды с лишними пробелами)");
		__reg_tc__("updateProfile.dataCoding",
			"»зменение режима приема сообщений");
		__reg_tc__("updateProfile.dataCoding.ucs2CodepageMixedCase",
			"”становить режим приема сообщений на русском €зыке (кодировка ucs2, текст команды в смешанном регистре)");
		__reg_tc__("updateProfile.dataCoding.ucs2CodepageSpaces",
			"”становить режим приема сообщений на русском €зыке (кодировка ucs2, текст команды с лишними пробелами)");
		__reg_tc__("updateProfile.dataCoding.defaultCodepageMixedCase",
			"ќтказатьс€ от режима приема сообщений на русском €зыке (текст команды в смешанном регистре)");
		__reg_tc__("updateProfile.dataCoding.defaultCodepageSpaces",
			"ќтказатьс€ от режима приема сообщений на русском €зыке (текст команды с лишними пробелами)");
		__reg_tc__("updateProfile.incorrectCmdText",
			"Ќеправильный текст команды");
		//processUpdateProfile
		__reg_tc__("processUpdateProfile",
			"—ообщени€ от менеджера профилей");
		__reg_tc__("processUpdateProfile.reportOptions",
			"ѕодтверждени€ об изменении настроек получени€ уведомлений о доставке");
		__reg_tc__("processUpdateProfile.codePage",
			"ѕодтверждени€ об изменении настроек режима приема сообщений на русском €зыке");
		__reg_tc__("processUpdateProfile.incorrectCmdText",
			"”ведомление о неправильном тексте команды");
	}
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_CHECK_LIST */

