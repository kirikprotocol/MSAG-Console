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
		"Авторизация на human admin console и script admin console");
	__reg_tc__("adminConsole.login.correct",
		"Правильные логин и пароль");
	__reg_tc__("adminConsole.login.incorrect",
		"Неправильные логин или пароль");
	__reg_tc__("adminConsole.invalidCommands",
		"Неправильные команды");
}

void aliasTc()
{
	__reg_tc__("adminConsole.alias",
		"Работа с алиасами через human admin console и script admin console");
	//adminConsole.alias.add
	__reg_tc__("adminConsole.alias.add",
		"Команда add alias");
	__reg_tc__("adminConsole.alias.add.correct",
		"Правильно введенные данные (маски алиаса и адреса длиной от 1 до 20 символов)");
	//adminConsole.alias.add.incorrect
	__reg_tc__("adminConsole.alias.add.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.alias.add.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (алиас, адрес)");
	__reg_tc__("adminConsole.alias.add.incorrect.invalidAliasFormat",
		"Неправильный формат алиаса");
	__reg_tc__("adminConsole.alias.add.incorrect.invalidAddrFormat",
		"Неправильный формат адреса");
	__reg_tc__("adminConsole.alias.add.incorrect.duplicateAlias",
		"Нарушается однозначное преобразование alias->address (уже существует такой alias)");
	__reg_tc__("adminConsole.alias.add.incorrect.duplicateAddr",
		"Нарушается однозначное преобразование address->alias (hide = true и уже существует такой address тоже с hide = true)");
	__reg_tc__("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"Поля alias и address содержат различное количество знаков \"?\"");
	__reg_tc__("adminConsole.alias.add.incorrect.hideWithQuestionMarks",
		"Выставлен признак сокрытия hide и поля address и alias содержат знаки \"?\"");
	//adminConsole.alias.delete
	__reg_tc__("adminConsole.alias.delete",
		"Команда delete alias");
	__reg_tc__("adminConsole.alias.delete.existent",
		"Удаление существующего алиаса");
	__reg_tc__("adminConsole.alias.delete.missingRequiredParam",
		"Не задан алиас");
	__reg_tc__("adminConsole.alias.delete.nonExistent",
		"Удаление несуществующего алиаса");
	__reg_tc__("adminConsole.alias.delete.invalidAliasFormat",
		"Неправильный формат алиаса");
	//adminConsole.alias.view
	__reg_tc__("adminConsole.alias.view",
		"Команда view alias");
	__reg_tc__("adminConsole.alias.view.existent",
		"Просмотр существующего алиаса");
	__reg_tc__("adminConsole.alias.view.missingRequiredParam",
		"Не задан алиас");
	__reg_tc__("adminConsole.alias.view.nonExistent",
		"Просмотр несуществующего алиаса");
	__reg_tc__("adminConsole.alias.view.invalidAliasFormat",
		"Неправильный формат алиаса");
	//adminConsole.alias.alter
	__reg_tc__("adminConsole.alias.alter",
		"Команда alter alias");
	__reg_tc__("adminConsole.alias.alter.correct",
		"Правильно введенные данные (существующий алиас и маска адреса длиной от 1 до 20 символов)");
	//adminConsole.alias.alter.incorrect
	__reg_tc__("adminConsole.alias.add.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (алиас, адрес)");
	__reg_tc__("adminConsole.alias.alter.incorrect.invalidAliasFormat",
		"Неправильный формат алиаса");
	__reg_tc__("adminConsole.alias.alter.incorrect.invalidAddrFormat",
		"Неправильный формат адреса");
	__reg_tc__("adminConsole.alias.alter.incorrect.duplicateAddr",
		"Нарушается однозначное преобразование address->alias (hide = true и уже существует такой address тоже с hide = true)");
	__reg_tc__("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"Поля alias и address содержат различное количество знаков \"?\"");
	__reg_tc__("adminConsole.alias.alter.incorrect.hideWithQuestionMarks",
		"Выставлен признак сокрытия hide и поля address и alias содержат знаки \"?\"");
}

AdminCheckList()
	: CheckList("Результаты функционального тестирования Admin Console", "admin.chk")
{
	baseTc();
	aliasTc();
}

};

}
}
}

#endif /* TEST_ADMIN_ADMIN_CHECK_LIST */

