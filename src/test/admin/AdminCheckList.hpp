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
	__reg_tc__("adminConsole.logout",
		"Завершение сеанса работы (команда quit)");
	__reg_tc__("adminConsole.invalidCommands",
		"Неправильные команды");
	__reg_tc__("adminConsole.apply",
		"Сохранение конфигов (команда apply)");
}

void aliasTc()
{
	__reg_tc__("adminConsole.alias",
		"Работа с алиасами через human admin console и script admin console");
	//adminConsole.alias.add
	__reg_tc__("adminConsole.alias.add",
		"Добавление алиаса (команда add alias)");
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
		"Удаление алиаса (команда delete alias)");
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
		"Просмотр алиаса (команда view alias)");
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
		"Изменение алиаса (команда alter alias)");
	__reg_tc__("adminConsole.alias.alter.correct",
		"Правильно введенные данные (существующий алиас и маска адреса длиной от 1 до 20 символов)");
	//adminConsole.alias.alter.incorrect
	__reg_tc__("adminConsole.alias.alter.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (алиас, адрес)");
	__reg_tc__("adminConsole.alias.alter.incorrect.nonExistent",
		"Изменение несуществующего алиаса");
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
	//adminConsole.alias.list
	__reg_tc__("adminConsole.alias.list",
		"Просмотр списка всех алиасов (команда list alias)");
}

void profileTc()
{
	__reg_tc__("adminConsole.profile",
		"Работа с профилями через human admin console и script admin console");
	//adminConsole.profile.add
	__reg_tc__("adminConsole.profile.add",
		"Добавление профиля (команда add profile)");
	__reg_tc__("adminConsole.profile.add.correct",
		"Правильно введенные данные (маска адреса и атрибуты профиля)");
	__reg_tc__("adminConsole.profile.add.existentProfile",
		"При добавлении уже существующего профиля происходит замещение");
	//adminConsole.profile.add.incorrect
	__reg_tc__("adminConsole.profile.add.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.profile.add.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (маска, атрибуты профиля)");
	__reg_tc__("adminConsole.profile.add.incorrect.invalidAddrFormat",
		"Неправильный формат маски");
	__reg_tc__("adminConsole.profile.add.incorrect.identicalToDefault",
		"Добавляемый профиль совпадает с дефолтным");
	//adminConsole.profile.delete
	__reg_tc__("adminConsole.profile.delete",
		"Удаление профиля (команда delete profile)");
	__reg_tc__("adminConsole.profile.delete.existent",
		"Удаление существующего профиля");
	__reg_tc__("adminConsole.profile.delete.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.profile.delete.incorrect.missingRequiredParam",
		"Не задана маска");
	__reg_tc__("adminConsole.profile.delete.incorrect.nonExistent",
		"Удаление несуществующего профиля");
	__reg_tc__("adminConsole.profile.delete.incorrect.invalidAliasFormat",
		"Неправильный формат маски");
	//adminConsole.profile.view
	__reg_tc__("adminConsole.profile.view",
		"Просмотр профиля (команда view profile)");
	__reg_tc__("adminConsole.profile.view.existent",
		"Просмотр существующего профиля");
	__reg_tc__("adminConsole.profile.view.nonExistent",
		"При просмотре несуществующего профиля выводятся значения профиля по умолчанию");
	__reg_tc__("adminConsole.profile.view.missingRequiredParam",
		"Не задана маска");
	__reg_tc__("adminConsole.profile.view.invalidAddrFormat",
		"Неправильный формат маски");
	//adminConsole.profile.alter
	__reg_tc__("adminConsole.profile.alter",
		"Изменение профиля (команда alter profile)");
	__reg_tc__("adminConsole.profile.alter.correct",
		"Правильно введенные данные");
	//adminConsole.profile.alter.incorrect
	__reg_tc__("adminConsole.profile.alter.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.profile.alter.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (маска, атрибуты профиля)");
	__reg_tc__("adminConsole.profile.alter.incorrect.nonExistent",
		"Изменение несуществующего профиля");
	__reg_tc__("adminConsole.profile.alter.incorrect.invalidAddrFormat",
		"Неправильный формат маски");
	//adminConsole.profile.list
	__reg_tc__("adminConsole.profile.list",
		"Просмотр списка всех профилей (команда list profile)");
}

void subjectTc()
{
	__reg_tc__("adminConsole.subject",
		"Работа с субъектами через human admin console и script admin console");
	//adminConsole.subject.add
	__reg_tc__("adminConsole.subject.add",
		"Добавление субъекта (команда add subject)");
	__reg_tc__("adminConsole.subject.add.correct",
		"Правильно введенные данные (имя субъекта с пробелами, русскими символами, одна/несколько масок)");

	//adminConsole.subject.add.incorrect
	__reg_tc__("adminConsole.subject.add.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (алиас, адрес)");
	__reg_tc__("adminConsole.subject.add.incorrect.invalidAliasFormat",
		"Неправильный формат алиаса");
	__reg_tc__("adminConsole.subject.add.incorrect.invalidAddrFormat",
		"Неправильный формат адреса");
	__reg_tc__("adminConsole.subject.add.incorrect.duplicateAlias",
		"Нарушается однозначное преобразование subject->address (уже существует такой subject)");
	__reg_tc__("adminConsole.subject.add.incorrect.duplicateAddr",
		"Нарушается однозначное преобразование address->subject (hide = true и уже существует такой address тоже с hide = true)");
	__reg_tc__("adminConsole.subject.add.incorrect.questionMarksNotMatch",
		"Поля subject и address содержат различное количество знаков \"?\"");
	__reg_tc__("adminConsole.subject.add.incorrect.hideWithQuestionMarks",
		"Выставлен признак сокрытия hide и поля address и subject содержат знаки \"?\"");
	
	//adminConsole.subject.delete
	__reg_tc__("adminConsole.subject.delete",
		"Удаление субъекта (команда delete subject)");
	__reg_tc__("adminConsole.subject.delete.existent",
		"Удаление существующего субъекта");


	__reg_tc__("adminConsole.subject.delete.missingRequiredParam",
		"Не задан алиас");
	__reg_tc__("adminConsole.subject.delete.nonExistent",
		"Удаление несуществующего алиаса");
	__reg_tc__("adminConsole.subject.delete.invalidAliasFormat",
		"Неправильный формат алиаса");
	
	//adminConsole.subject.view
	__reg_tc__("adminConsole.subject.view",
		"Просмотр субъекта (команда view subject)");
	__reg_tc__("adminConsole.subject.view.existent",
		"Просмотр существующего субъекта");


	__reg_tc__("adminConsole.subject.view.missingRequiredParam",
		"Не задан алиас");
	__reg_tc__("adminConsole.subject.view.nonExistent",
		"Просмотр несуществующего алиаса");
	__reg_tc__("adminConsole.subject.view.invalidAliasFormat",
		"Неправильный формат алиаса");
	
	
	//adminConsole.subject.alter
	__reg_tc__("adminConsole.subject.alter",
		"Изменение субъекта (команда alter subject)");
	__reg_tc__("adminConsole.subject.alter.correct",
		"Правильно введенные данные (существующий субъект, имя с пробелами/русскими символами, одна/несколько масок))");
	__reg_tc__("adminConsole.subject.alter.correct.addMask",
		"Добавление маски");
	__reg_tc__("adminConsole.subject.alter.correct.deleteMask",
		"Удаление маски");
	__reg_tc__("adminConsole.subject.alter.correct.changeDefSme",
		"Добавление маски");


	//adminConsole.subject.alter.incorrect
	__reg_tc__("adminConsole.subject.alter.incorrect",
		"Для неправильно введенных данных выдаются соответствующие сообщения об ошибках");
	__reg_tc__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"Не заданы обязательные параметры (алиас, адрес)");
	__reg_tc__("adminConsole.subject.alter.incorrect.nonExistent",
		"Изменение несуществующего алиаса");
	__reg_tc__("adminConsole.subject.alter.incorrect.invalidAliasFormat",
		"Неправильный формат алиаса");
	__reg_tc__("adminConsole.subject.alter.incorrect.invalidAddrFormat",
		"Неправильный формат адреса");
	__reg_tc__("adminConsole.subject.alter.incorrect.duplicateAddr",
		"Нарушается однозначное преобразование address->subject (hide = true и уже существует такой address тоже с hide = true)");
	__reg_tc__("adminConsole.subject.alter.incorrect.questionMarksNotMatch",
		"Поля subject и address содержат различное количество знаков \"?\"");
	__reg_tc__("adminConsole.subject.alter.incorrect.hideWithQuestionMarks",
		"Выставлен признак сокрытия hide и поля address и subject содержат знаки \"?\"");
	//adminConsole.subject.list
	__reg_tc__("adminConsole.subject.list",
		"Просмотр списка всех алиасов (команда list subject)");
}

AdminCheckList()
	: CheckList("Результаты функционального тестирования Admin Console", "admin.chk")
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

