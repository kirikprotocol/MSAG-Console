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
		: CheckList("Результаты функционального тестирования Profiler", "profiler.chk")
	{
		//createProfileMatch
		__reg_tc__("createProfileMatch",
			"Создание нового профиля для фиксированного адреса");
		__reg_tc__("createProfileMatch.noSubstSymbols",
			"Маска в точности соответствует адресу (без знаков подстановки)");
		__reg_tc__("createProfileMatch.withQuestionMarks",
			"Маска соответствует адресу с одним или несколькими '?' в конце");
		__reg_tc__("createProfileMatch.entirelyQuestionMarks",
			"Маска целиком из '?', длина соответствует исходному адресу");
		//createProfileNotMatch
		__reg_tc__("createProfileNotMatch",
			"Создание нового профиля с отличиями для фиксированного адреса");
		__reg_tc__("createProfileNotMatch.diffType",
			"В маске отличается typeOfNumber");
		__reg_tc__("createProfileNotMatch.diffPlan",
			"В маске отличается numberingPlan");
		__reg_tc__("createProfileNotMatch.diffValue",
			"В строке адреса маски отличается один символ, но длина строки соответствует");
		__reg_tc__("createProfileNotMatch.diffValueLength",
			"В строке адреса маски все символы соответствуют исходным, но количество '?' делает строку отличающейся длины");
		//other
		__reg_tc__("updateProfile", "Обновление уже существующего профиля");
		__reg_tc__("lookup", "Поиск профиля для заданного адреса");
	}
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_CHECK_LIST */

