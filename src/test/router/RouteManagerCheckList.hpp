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
		__reg_tc__("addCorrectRoute.sourceMatchNoSubstSymbols",
			"Адрес отправителя в точности соответствует исходному (без знаков подстановки)");
		__reg_tc__("addCorrectRoute.sourceMatchWithQuestionMarks",
			"Адрес отправителя с одним или несколькими '?' в конце");
		__reg_tc__("addCorrectRoute.sourceMatchEntirelyQuestionMarks",
			"Адрес отправителя целиком из '?'");
		__reg_tc__("addCorrectRoute.sourceNotMatchType",
			"В адресе отправителя отличается typeOfNumber");
		__reg_tc__("addCorrectRoute.sourceNotMatchPlan",
			"В адресе отправителя отличается numberingPlan");
		__reg_tc__("addCorrectRoute.sourceNotMatchValue",
			"В адресе отправителя отличается один символ, но длина строки соответствует");
		__reg_tc__("addCorrectRoute.sourceNotMatchValueLength",
			"В адресе отправителя все символы в строке адреса соответствуют исходным, но количество '?' делает строку отличающейся длины");
		__reg_tc__("addCorrectRoute.destMatchNoSubstSymbols",
			"Адрес получателя в точности соответствует исходному (без знаков подстановки)");
		__reg_tc__("addCorrectRoute.destMatchWithQuestionMarks",
			"Адрес получателя с одним или несколькими '?' в конце");
		__reg_tc__("addCorrectRoute.destMatchEntirelyQuestionMarks",
			"Адрес получателя целиком из '?'");
		__reg_tc__("addCorrectRoute.destNotMatchType",
			"В адресе получателя отличается typeOfNumber");
		__reg_tc__("addCorrectRoute.destNotMatchPlan",
			"В адресе получателя отличается numberingPlan");
		__reg_tc__("addCorrectRoute.destNotMatchValue",
			"В адресе получателя отличается один символ, но длина строки соответствует");
		__reg_tc__("addCorrectRoute.destNotMatchValueLength",
			"В адресе получателя все символы в строке адреса соответствуют исходным, но количество '?' делает строку отличающейся длины");
		//addIncorrectRoute
		/*
		__reg_tc__("addIncorrectRoute",
			"Регистрация маршрута с недопустимыми значениями адресов");
		__reg_tc__("addIncorrectRoute.invalidType",
			"Значение typeOfNumber вне диапазона разрешенного в SMPP v3.4");
		__reg_tc__("addIncorrectRoute.invalidPlan",
			"Значение numberingPlan вне диапазона разрешенного в SMPP v3.4");
		*/
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

