#ifndef TEST_ALIAS_ALIAS_MANAGER_CHECK_LIST
#define TEST_ALIAS_ALIAS_MANAGER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace alias {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class AliasManagerCheckList : public CheckList
{
public:
	AliasManagerCheckList()
		: CheckList("���������� ��������������� ������������ Alias Manager", "alias_man.chk")
	{
		//addCorrectAlias
		__reg_tc__("addCorrectAlias",
			"����������� ������ ��� �������� (�������� �����������) ���� (�����, �����)");
		__reg_tc__("addCorrectAlias.allMatchNoSubstSymbols",
			"����� � ����� � �������� ������������� �������� (��� ������ �����������)");
		__reg_tc__("addCorrectAlias.allMatchWithQuestionMarks",
			"����� � ����� ������������� �������� � ����� ��� ����������� '?' � �����");
		__reg_tc__("addCorrectAlias.allMatchEntirelyQuestionMarks",
			"����� � ����� ������������� ��������, ����� �������� ��� ��� ������� �� '?'");
		__reg_tc__("addCorrectAlias.allMatchWithAsterisks",
			"����� � ����� ������������� �������� �� '*' � �����");
		//addCorrectAliasNotMatchAddress
		__reg_tc__("addCorrectAlias.addrNotMatch",
			"����� ������������� ���������, ����� ����������");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffType",
			"������������ typeOfNumber");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffPlan",
			"������������ numberingPlan");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffValue",
			"� ������ ������ ���������� ���� ������, �� ����� ������ �������������");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffValueLength",
			"� ������ ������ ��� ������� ������������� ��������, �� ���������� '?' ������ ������ ������������ �����");
		//addCorrectAliasNotMatchAlias
		__reg_tc__("addCorrectAlias.aliasNotMatch",
			"����� ������������� ���������, ����� ����������");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffType",
			"������������ typeOfNumber");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffPlan",
			"������������ numberingPlan");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffValue",
			"� ������ ������ ���������� ���� ������, �� ����� ������ �������������");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffValueLength",
			"� ������ ������ ��� ������� ������������� ��������, �� ���������� '?' ������ ������ ������������ �����");
		//addCorrectAliasException
		__reg_tc__("addCorrectAliasException",
			"����������� ������ �� '*' � ����� � ������������� ������ ��� alias->addr ��� ������ ��� addr->alias");
		/*
		__reg_tc__("addIncorrectAlias",
			"����������� ������ � ������������� �����������");
		*/
		__reg_tc__("deleteAliases",
			"��������� ������� �������");
		__reg_tc__("findAliasByAddress",
			"���������� ��������� ������ ������ �� ��������� ������ (�� ������ �������� ����������� �������)");
		__reg_tc__("findAddressByAlias",
			"���������� ��������� ������ ������ �� ��������� ������ (�� ������ �������� ����������� �������)");
		__reg_tc__("checkInverseTransformation",
			"�������������� addr->alias->addr ���������� �������� ����� (��� alias->addr->alias ��� �������������)");
		__reg_tc__("iterateAliases",
			"������������ �� ������ ������������������ �������");
	}
};

}
}
}

#endif /* TEST_ALIAS_ALIAS_MANAGER_CHECK_LIST */

