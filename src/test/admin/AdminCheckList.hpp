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
	__reg_tc__("adminConsole.logout",
		"���������� ������ ������ (������� quit)");
	__reg_tc__("adminConsole.invalidCommands",
		"������������ �������");
	__reg_tc__("adminConsole.apply",
		"���������� �������� (������� apply)");
}

void aliasTc()
{
	__reg_tc__("adminConsole.alias",
		"������ � �������� ����� human admin console � script admin console");
	//adminConsole.alias.add
	__reg_tc__("adminConsole.alias.add",
		"���������� ������ (������� add alias)");
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
		"�������� ������ (������� delete alias)");
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
		"�������� ������ (������� view alias)");
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
		"��������� ������ (������� alter alias)");
	__reg_tc__("adminConsole.alias.alter.correct",
		"��������� ��������� ������ (������������ ����� � ����� ������ ������ �� 1 �� 20 ��������)");
	//adminConsole.alias.alter.incorrect
	__reg_tc__("adminConsole.alias.alter.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �����)");
	__reg_tc__("adminConsole.alias.alter.incorrect.nonExistent",
		"��������� ��������������� ������");
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
	//adminConsole.alias.list
	__reg_tc__("adminConsole.alias.list",
		"�������� ������ ���� ������� (������� list alias)");
}

void profileTc()
{
	__reg_tc__("adminConsole.profile",
		"������ � ��������� ����� human admin console � script admin console");
	//adminConsole.profile.add
	__reg_tc__("adminConsole.profile.add",
		"���������� ������� (������� add profile)");
	__reg_tc__("adminConsole.profile.add.correct",
		"��������� ��������� ������ (����� ������ � �������� �������)");
	__reg_tc__("adminConsole.profile.add.existentProfile",
		"��� ���������� ��� ������������� ������� ���������� ���������");
	//adminConsole.profile.add.incorrect
	__reg_tc__("adminConsole.profile.add.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.profile.add.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �������� �������)");
	__reg_tc__("adminConsole.profile.add.incorrect.invalidAddrFormat",
		"������������ ������ �����");
	__reg_tc__("adminConsole.profile.add.incorrect.identicalToDefault",
		"����������� ������� ��������� � ���������");
	//adminConsole.profile.delete
	__reg_tc__("adminConsole.profile.delete",
		"�������� ������� (������� delete profile)");
	__reg_tc__("adminConsole.profile.delete.existent",
		"�������� ������������� �������");
	__reg_tc__("adminConsole.profile.delete.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.profile.delete.incorrect.missingRequiredParam",
		"�� ������ �����");
	__reg_tc__("adminConsole.profile.delete.incorrect.nonExistent",
		"�������� ��������������� �������");
	__reg_tc__("adminConsole.profile.delete.incorrect.invalidAliasFormat",
		"������������ ������ �����");
	//adminConsole.profile.view
	__reg_tc__("adminConsole.profile.view",
		"�������� ������� (������� view profile)");
	__reg_tc__("adminConsole.profile.view.existent",
		"�������� ������������� �������");
	__reg_tc__("adminConsole.profile.view.nonExistent",
		"��� ��������� ��������������� ������� ��������� �������� ������� �� ���������");
	__reg_tc__("adminConsole.profile.view.missingRequiredParam",
		"�� ������ �����");
	__reg_tc__("adminConsole.profile.view.invalidAddrFormat",
		"������������ ������ �����");
	//adminConsole.profile.alter
	__reg_tc__("adminConsole.profile.alter",
		"��������� ������� (������� alter profile)");
	__reg_tc__("adminConsole.profile.alter.correct",
		"��������� ��������� ������");
	//adminConsole.profile.alter.incorrect
	__reg_tc__("adminConsole.profile.alter.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.profile.alter.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �������� �������)");
	__reg_tc__("adminConsole.profile.alter.incorrect.nonExistent",
		"��������� ��������������� �������");
	__reg_tc__("adminConsole.profile.alter.incorrect.invalidAddrFormat",
		"������������ ������ �����");
	//adminConsole.profile.list
	__reg_tc__("adminConsole.profile.list",
		"�������� ������ ���� �������� (������� list profile)");
}

void subjectTc()
{
	__reg_tc__("adminConsole.subject",
		"������ � ���������� ����� human admin console � script admin console");
	//adminConsole.subject.add
	__reg_tc__("adminConsole.subject.add",
		"���������� �������� (������� add subject)");
	__reg_tc__("adminConsole.subject.add.correct",
		"��������� ��������� ������ (��� �������� � ���������, �������� ���������, ����/��������� �����)");

	//adminConsole.subject.add.incorrect
	__reg_tc__("adminConsole.subject.add.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �����)");
	__reg_tc__("adminConsole.subject.add.incorrect.invalidAliasFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.subject.add.incorrect.invalidAddrFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.subject.add.incorrect.duplicateAlias",
		"���������� ����������� �������������� subject->address (��� ���������� ����� subject)");
	__reg_tc__("adminConsole.subject.add.incorrect.duplicateAddr",
		"���������� ����������� �������������� address->subject (hide = true � ��� ���������� ����� address ���� � hide = true)");
	__reg_tc__("adminConsole.subject.add.incorrect.questionMarksNotMatch",
		"���� subject � address �������� ��������� ���������� ������ \"?\"");
	__reg_tc__("adminConsole.subject.add.incorrect.hideWithQuestionMarks",
		"��������� ������� �������� hide � ���� address � subject �������� ����� \"?\"");
	
	//adminConsole.subject.delete
	__reg_tc__("adminConsole.subject.delete",
		"�������� �������� (������� delete subject)");
	__reg_tc__("adminConsole.subject.delete.existent",
		"�������� ������������� ��������");


	__reg_tc__("adminConsole.subject.delete.missingRequiredParam",
		"�� ����� �����");
	__reg_tc__("adminConsole.subject.delete.nonExistent",
		"�������� ��������������� ������");
	__reg_tc__("adminConsole.subject.delete.invalidAliasFormat",
		"������������ ������ ������");
	
	//adminConsole.subject.view
	__reg_tc__("adminConsole.subject.view",
		"�������� �������� (������� view subject)");
	__reg_tc__("adminConsole.subject.view.existent",
		"�������� ������������� ��������");


	__reg_tc__("adminConsole.subject.view.missingRequiredParam",
		"�� ����� �����");
	__reg_tc__("adminConsole.subject.view.nonExistent",
		"�������� ��������������� ������");
	__reg_tc__("adminConsole.subject.view.invalidAliasFormat",
		"������������ ������ ������");
	
	
	//adminConsole.subject.alter
	__reg_tc__("adminConsole.subject.alter",
		"��������� �������� (������� alter subject)");
	__reg_tc__("adminConsole.subject.alter.correct",
		"��������� ��������� ������ (������������ �������, ��� � ���������/�������� ���������, ����/��������� �����))");
	__reg_tc__("adminConsole.subject.alter.correct.addMask",
		"���������� �����");
	__reg_tc__("adminConsole.subject.alter.correct.deleteMask",
		"�������� �����");
	__reg_tc__("adminConsole.subject.alter.correct.changeDefSme",
		"���������� �����");


	//adminConsole.subject.alter.incorrect
	__reg_tc__("adminConsole.subject.alter.incorrect",
		"��� ����������� ��������� ������ �������� ��������������� ��������� �� �������");
	__reg_tc__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"�� ������ ������������ ��������� (�����, �����)");
	__reg_tc__("adminConsole.subject.alter.incorrect.nonExistent",
		"��������� ��������������� ������");
	__reg_tc__("adminConsole.subject.alter.incorrect.invalidAliasFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.subject.alter.incorrect.invalidAddrFormat",
		"������������ ������ ������");
	__reg_tc__("adminConsole.subject.alter.incorrect.duplicateAddr",
		"���������� ����������� �������������� address->subject (hide = true � ��� ���������� ����� address ���� � hide = true)");
	__reg_tc__("adminConsole.subject.alter.incorrect.questionMarksNotMatch",
		"���� subject � address �������� ��������� ���������� ������ \"?\"");
	__reg_tc__("adminConsole.subject.alter.incorrect.hideWithQuestionMarks",
		"��������� ������� �������� hide � ���� address � subject �������� ����� \"?\"");
	//adminConsole.subject.list
	__reg_tc__("adminConsole.subject.list",
		"�������� ������ ���� ������� (������� list subject)");
}

AdminCheckList()
	: CheckList("���������� ��������������� ������������ Admin Console", "admin.chk")
{
	baseTc();
	aliasTc();
	profileTc();
}

};

}
}
}

#endif /* TEST_ADMIN_ADMIN_CHECK_LIST */

