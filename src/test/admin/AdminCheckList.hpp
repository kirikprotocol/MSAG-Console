#ifndef TEST_ADMIN_ADMIN_CHECK_LIST
#define TEST_ADMIN_ADMIN_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class AdminCheckList : public CheckList
{
public:

void baseTc()
{
	__reg_tc__("adminConsole.login",
		"����������� �� human admin console � script admin console");
	__reg_tc__("adminConsole.login.correct",
		"���������� ����� � ������");
	__reg_tc__("adminConsole.login.incorrect",
		"������������ ����� ��� ������");
	__reg_tc__("adminConsole.invalidCommands",
		"������������ �������");
}

void aliasTc()
{
	__reg_tc__("adminConsole.alias",
		"������ � �������� ����� human admin console � script admin console");
	//adminConsole.alias.add
	__reg_tc__("adminConsole.alias.add",
		"������� add alias");
	__reg_tc__("adminConsole.alias.add.correct",
		"��������� ��������� ������ (����� ������ � ������ ������ �� 1 �� 20 ��������)");
	//adminConsole.alias.add.incorrect
	__reg_tc__("adminConsole.alias.add.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.alias.add.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �����)");
	__reg_tc__("adminConsole.alias.add.incorrect.invalidAliasFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.alias.add.incorrect.invalidAddrFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.alias.add.incorrect.duplicateAlias",
		"���������� ����������� �������������� alias->address (��� ���������� ����� alias)");
	__reg_tc__("adminConsole.alias.add.incorrect.duplicateAddr",
		"���������� ����������� �������������� address->alias (hide = true � ��� ���������� ����� address ���� � hide = true)");
	__reg_tc__("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"���� alias � address �������� ��������� ���������� ������ \"?\"");
	__reg_tc__("adminConsole.alias.add.incorrect.hideWithQuestionMarks",
		"��������� ������� �������� hide � ���� address � alias �������� ����� \"?\"");
	//adminConsole.alias.delete
	__reg_tc__("adminConsole.alias.delete",
		"������� delete alias");
	__reg_tc__("adminConsole.alias.delete.existent",
		"�������� ������������� ������");
	__reg_tc__("adminConsole.alias.delete.missingRequiredParam",
		"�� ����� �����");
	__reg_tc__("adminConsole.alias.delete.nonExistent",
		"�������� ��������������� ������");
	__reg_tc__("adminConsole.alias.delete.invalidAliasFormat",
		"������������ ������ ������");
	//adminConsole.alias.view
	__reg_tc__("adminConsole.alias.view",
		"������� view alias");
	__reg_tc__("adminConsole.alias.view.existent",
		"�������� ������������� ������");
	__reg_tc__("adminConsole.alias.view.missingRequiredParam",
		"�� ����� �����");
	__reg_tc__("adminConsole.alias.view.nonExistent",
		"�������� ��������������� ������");
	__reg_tc__("adminConsole.alias.view.invalidAliasFormat",
		"������������ ������ ������");
	//adminConsole.alias.alter
	__reg_tc__("adminConsole.alias.alter",
		"������� alter alias");
	__reg_tc__("adminConsole.alias.alter.correct",
		"��������� ��������� ������ (������������ ����� � ����� ������ ������ �� 1 �� 20 ��������)");
	//adminConsole.alias.alter.incorrect
	__reg_tc__("adminConsole.alias.add.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �����)");
	__reg_tc__("adminConsole.alias.alter.incorrect.invalidAliasFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.alias.alter.incorrect.invalidAddrFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.alias.alter.incorrect.duplicateAddr",
		"���������� ����������� �������������� address->alias (hide = true � ��� ���������� ����� address ���� � hide = true)");
	__reg_tc__("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"���� alias � address �������� ��������� ���������� ������ \"?\"");
	__reg_tc__("adminConsole.alias.alter.incorrect.hideWithQuestionMarks",
		"��������� ������� �������� hide � ���� address � alias �������� ����� \"?\"");
}

AdminCheckList()
	: CheckList("���������� ��������������� ������������ Admin Console", "admin.chk")
{
	baseTc();
	aliasTc();
}

};

}
}
}

#endif /* TEST_ADMIN_ADMIN_CHECK_LIST */

