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
		__reg_tc__("addCorrectRoute.sourceMatchNoSubstSymbols",
			"����� ����������� � �������� ������������� ��������� (��� ������ �����������)");
		__reg_tc__("addCorrectRoute.sourceMatchWithQuestionMarks",
			"����� ����������� � ����� ��� ����������� '?' � �����");
		__reg_tc__("addCorrectRoute.sourceMatchEntirelyQuestionMarks",
			"����� ����������� ������� �� '?'");
		__reg_tc__("addCorrectRoute.sourceNotMatchType",
			"� ������ ����������� ���������� typeOfNumber");
		__reg_tc__("addCorrectRoute.sourceNotMatchPlan",
			"� ������ ����������� ���������� numberingPlan");
		__reg_tc__("addCorrectRoute.sourceNotMatchValue",
			"� ������ ����������� ���������� ���� ������, �� ����� ������ �������������");
		__reg_tc__("addCorrectRoute.sourceNotMatchValueLength",
			"� ������ ����������� ��� ������� � ������ ������ ������������� ��������, �� ���������� '?' ������ ������ ������������ �����");
		__reg_tc__("addCorrectRoute.destMatchNoSubstSymbols",
			"����� ���������� � �������� ������������� ��������� (��� ������ �����������)");
		__reg_tc__("addCorrectRoute.destMatchWithQuestionMarks",
			"����� ���������� � ����� ��� ����������� '?' � �����");
		__reg_tc__("addCorrectRoute.destMatchEntirelyQuestionMarks",
			"����� ���������� ������� �� '?'");
		__reg_tc__("addCorrectRoute.destNotMatchType",
			"� ������ ���������� ���������� typeOfNumber");
		__reg_tc__("addCorrectRoute.destNotMatchPlan",
			"� ������ ���������� ���������� numberingPlan");
		__reg_tc__("addCorrectRoute.destNotMatchValue",
			"� ������ ���������� ���������� ���� ������, �� ����� ������ �������������");
		__reg_tc__("addCorrectRoute.destNotMatchValueLength",
			"� ������ ���������� ��� ������� � ������ ������ ������������� ��������, �� ���������� '?' ������ ������ ������������ �����");
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

