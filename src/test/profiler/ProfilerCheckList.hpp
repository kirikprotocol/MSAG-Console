#ifndef TEST_PROFILER_PROFILER_CHECK_LIST
#define TEST_PROFILER_PROFILER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace profiler {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class ProfilerCheckList : public CheckList
{
public:
	ProfilerCheckList()
		: CheckList("–езультаты функционального тестировани€ Profiler", "profiler.chk")
	{
		//createProfileMatch
		__reg_tc__("createProfileMatch",
			"—оздание нового профил€ дл€ фиксированного адреса");
		__reg_tc__("createProfileMatch.noSubstSymbols",
			"ћаска в точности соответствует адресу (без знаков подстановки)");
		__reg_tc__("createProfileMatch.withQuestionMarks",
			"ћаска соответствует адресу с одним или несколькими '?' в конце");
		__reg_tc__("createProfileMatch.entirelyQuestionMarks",
			"ћаска целиком из '?', длина соответствует исходному адресу");
		//createProfileNotMatch
		__reg_tc__("createProfileNotMatch",
			"—оздание нового профил€ с отличи€ми дл€ фиксированного адреса");
		__reg_tc__("createProfileNotMatch.diffType",
			"¬ маске отличаетс€ typeOfNumber");
		__reg_tc__("createProfileNotMatch.diffPlan",
			"¬ маске отличаетс€ numberingPlan");
		__reg_tc__("createProfileNotMatch.diffValue",
			"¬ строке адреса маски отличаетс€ один символ, но длина строки соответствует");
		__reg_tc__("createProfileNotMatch.diffValueLength",
			"¬ строке адреса маски все символы соответствуют исходным, но количество '?' делает строку отличающейс€ длины");
		//other
		__reg_tc__("updateProfile", "ќбновление уже существующего профил€");
		__reg_tc__("lookup", "ѕоиск профил€ дл€ заданного адреса");
		//putCommand
		__reg_tc__("putCommand", "–абота с профилем через submit_sm pdu");
		__reg_tc__("putCommand.reportNoneMixedCase",
			"ќтказатьс€ от уведомлени€ о доставке (текст команды в смешанном регистре)");
		__reg_tc__("putCommand.reportNoneSpaces",
			"ќтказатьс€ от уведомлени€ о доставке (текст команды с лишними пробелами)");
		__reg_tc__("putCommand.reportFullMixedCase",
			"”становить прием уведомлений о доставке (текст команды в смешанном регистре)");
		__reg_tc__("putCommand.reportFullSpaces",
			"”становить прием уведомлений о доставке (текст команды с лишними пробелами)");
		__reg_tc__("putCommand.ucs2CodepageMixedCase",
			"”становить режим приема сообщений на русском €зыке (кодировка ucs2, текст команды в смешанном регистре)");
		__reg_tc__("putCommand.ucs2CodepageSpaces",
			"”становить режим приема сообщений на русском €зыке (кодировка ucs2, текст команды с лишними пробелами)");
		__reg_tc__("putCommand.defaultCodepageMixedCase",
			"ќтказатьс€ от режима приема сообщений на русском €зыке (текст команды в смешанном регистре)");
		__reg_tc__("putCommand.defaultCodepageSpaces",
			"ќтказатьс€ от режима приема сообщений на русском €зыке (текст команды с лишними пробелами)");
		__reg_tc__("putCommand.incorrectText",
			"Ќеправильный текст команды");
	}
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_CHECK_LIST */

