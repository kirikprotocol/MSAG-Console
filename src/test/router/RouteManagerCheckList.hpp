#ifndef TEST_ROUTER_ROUTER_MANAGER_CHECK_LIST
#define TEST_ROUTER_ROUTER_MANAGER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace router {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class RouteManagerCheckList : public CheckList
{
public:
	RouteManagerCheckList()
		: CheckList("Результаты функционального тестирования Route Manager", "route_man.chk")
	{
		//addCorrectRoute
		__reg_tc__("addCorrectRoute",
			"Регистрация маршрута для заданной (случайно сгенеренной) пары адресов");
		__reg_tc__("addCorrectRoute.matchNoSubstSymbols",
			"Один из адресов в точности соответствует исходному (без знаков подстановки)");
		__reg_tc__("addCorrectRoute.matchWithQuestionMarks",
			"Один из адресов с одним или несколькими '?' в конце");
		__reg_tc__("addCorrectRoute.matchEntirelyQuestionMarks",
			"Один из адресов целиком из '?'");
		__reg_tc__("addCorrectRoute.notMatchType",
			"В одном из адресов отличается typeOfNumber");
		__reg_tc__("addCorrectRoute.notMatchPlan",
			"В одном из адресов отличается numberingPlan");
		__reg_tc__("addCorrectRoute.notMatchValue",
			"В одном из адресов отличается один символ, но длина строки соответствует");
		__reg_tc__("addCorrectRoute.notMatchValueLength",
			"В одном из адресов все символы в строке адреса соответствуют исходным, но количество '?' делает строку отличающейся длины");
		//addIncorrectRoute
		__reg_tc__("addIncorrectRoute",
			"Регистрация маршрута с недопустимыми значениями адресов");
		__reg_tc__("addIncorrectRoute.invalidType",
			"Значение typeOfNumber вне диапазона разрешенного в SMPP v3.4");
		__reg_tc__("addIncorrectRoute.invalidPlan",
			"Значение numberingPlan вне диапазона разрешенного в SMPP v3.4");
		__reg_tc__("addIncorrectRoute.invalidSmeId",
			"Незарегистрированный smeSystemId");
		//other
		__reg_tc__("lookupRoute", "Поиск маршрута выполняется правильно");
		//__reg_tc__("iterateRoutes", "Итерирование по списку маршрутов выполняется правильно");
	}
};

}
}
}

#endif /* TEST_ROUTER_ROUTER_MANAGER_CHECK_LIST */

