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
		: CheckList("���������� ��������������� ������������ Route Manager", "route_man.chk")
	{
		//addCorrectRoute
		__reg_tc__("addCorrectRoute",
			"����������� �������� ��� �������� (�������� �����������) ���� �������");
		__reg_tc__("addCorrectRoute.matchNoSubstSymbols",
			"���� �� ������� � �������� ������������� ��������� (��� ������ �����������)");
		__reg_tc__("addCorrectRoute.matchWithQuestionMarks",
			"���� �� ������� � ����� ��� ����������� '?' � �����");
		__reg_tc__("addCorrectRoute.matchEntirelyQuestionMarks",
			"���� �� ������� ������� �� '?'");
		__reg_tc__("addCorrectRoute.notMatchType",
			"� ����� �� ������� ���������� typeOfNumber");
		__reg_tc__("addCorrectRoute.notMatchPlan",
			"� ����� �� ������� ���������� numberingPlan");
		__reg_tc__("addCorrectRoute.notMatchValue",
			"� ����� �� ������� ���������� ���� ������, �� ����� ������ �������������");
		__reg_tc__("addCorrectRoute.notMatchValueLength",
			"� ����� �� ������� ��� ������� � ������ ������ ������������� ��������, �� ���������� '?' ������ ������ ������������ �����");
		//addIncorrectRoute
		__reg_tc__("addIncorrectRoute",
			"����������� �������� � ������������� ���������� �������");
		__reg_tc__("addIncorrectRoute.invalidType",
			"�������� typeOfNumber ��� ��������� ������������ � SMPP v3.4");
		__reg_tc__("addIncorrectRoute.invalidPlan",
			"�������� numberingPlan ��� ��������� ������������ � SMPP v3.4");
		__reg_tc__("addIncorrectRoute.invalidSmeId",
			"�������������������� smeSystemId");
		//other
		__reg_tc__("lookupRoute", "����� �������� ����������� ���������");
		//__reg_tc__("iterateRoutes", "������������ �� ������ ��������� ����������� ���������");
	}
};

}
}
}

#endif /* TEST_ROUTER_ROUTER_MANAGER_CHECK_LIST */

