#include "AdminAliasTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::util::Logger;

Category& AdminAliasTestCases::getLog()
{
	static Category& log = Logger::getCategory("AdminAliasTestCases");
	return log;
}

#define __cmd__(id, cmd, resp) \
	if (hasRights) { \
		runTestCase(id, cmd, resp); \
	} else { \
		runTestCase(id, cmd, noRights); \
	}

#define __cmd2__(id, cmd, code, resp) \
	__cmd__(id, cmd, simpleResp(code, resp))

void AdminAliasTestCases::correctCommands()
{
	char cmd[128];
	char resp[256];
	//add correct
	vector<const char*> masks = Masks::correct();
	for (int i = 0; i < masks.size(); i++)
	{
		sprintf(cmd, "add alias %s %s nohide %s",
			masks[i], masks[i], shit);
		sprintf(resp, "/^\\Q%s Alias '%s' for address '%s' added\\E$/",
			respOk(100), masks[i], masks[i]);
		__cmd__("adminConsole.alias.add.correct", cmd, resp);
	}
	//view added
	for (int i = 0; i < masks.size(); i++)
	{
		sprintf(cmd, "view alias %s %s", masks[i], shit);
		sprintf(resp, "/^\\Q%s Alias '%s' Address '%s' (nohide)\\E$/",
			respOk(100), masks[i], masks[i]);
		__cmd__("adminConsole.alias.view.existent", cmd, resp);
	}
	//add & delete
	sprintf(resp, "/^\\Q%s Alias '123' for address '123' added\\E$/", respOk(100));
	__cmd__("adminConsole.alias.add.correct",
		"add alias 123 123 hide", resp);
	sprintf(resp, "/^\\Q%s Alias '123' deleted\\E$/", respOk(100));
	__cmd__("adminConsole.alias.delete.existent",
		"delete alias 123", resp);
	//delete
	//const char* deleteList[] = {".0.1.00", ".1.1.00", ".0.1.00?", ".1.1.00?"};
	const char* deleteList[] = {"00", "+00", "00?", "+00?"};
	for (int i = 0; i < sizeof(deleteList) / sizeof(*deleteList); i++)
	{
		sprintf(cmd, "delete alias %s %s", deleteList[i], shit);
		sprintf(resp, "/^\\Q%s Alias '%s' deleted\\E$/",
			respOk(100), deleteList[i]);
		__cmd__("adminConsole.alias.delete.existent", cmd, resp);
	}
	//alter
	//const char* alterList[] = {".0.1.10", ".1.1.10", ".0.1.10?", ".1.1.10?"};
	const char* alterList[] = {"10", "+10", "10?", "+10?"};
	const char* alterOptions[] = {"hide", "nohide", "", ""};
	for (int i = 0; i < sizeof(alterList) / sizeof(*alterList); i++)
	{
		sprintf(cmd, "alter alias %s %s %s %s", alterList[i], alterList[i],
			alterOptions[i], strlen(alterOptions[i]) ? shit : "");
		sprintf(resp, "/^\\Q%s Alias '%s' altered\\E$/",
			respOk(100), alterList[i]);
		__cmd__("adminConsole.alias.alter.correct", cmd, resp);
	}
	//view
	sprintf(cmd, "view alias 20 %s", shit);
	sprintf(resp, "/^\\Q%s Alias '20' Address '02' (hide)\\E$/", respOk(100));
	__cmd__("adminConsole.alias.view.existent", cmd, resp);
	sprintf(cmd, "view alias 20? %s", shit);
	sprintf(resp, "/^\\Q%s Alias '20?' Address '02?' (nohide)\\E$/", respOk(100));
	__cmd__("adminConsole.alias.view.existent", cmd, resp);
	//list (список не сортированный)
	static const string list[] = {
		"20?",
		"20",
		"+20?",
		"0",
		"123456789012345678??",
		"+20",
		"+0",
		"+12345678901234567890",
		"+10?",
		"10?",
		"10",
		"+123456789012345678??",
		".0.0.12345678901234567890",
		"12345678901234567890",
		".0.0.123456789012345678??",
		".0.0.0",
		"+10"
	};
	static const int listSize = sizeof(list) / sizeof(*list);
	string listResp;
	listResp.reserve(1024);
	listResp += fixture->humanConsole ? "/^Alias list:$" : "";
	for (int i = 0; i < listSize; i++)
	{
		listResp += "^ \\Q";
		listResp += list[i];
		listResp += "\\E$";
	}
	listResp += fixture->humanConsole ? "" : "+ 200 Alias list";
	listResp += "/ms";
	sprintf(cmd, "list alias %s", shit);
	__cmd__("adminConsole.alias.list", cmd, listResp.c_str());
}

void AdminAliasTestCases::incorrectCommands()
{
	//missing params
	__cmd2__("adminConsole.alias.add.incorrect.missingRequiredParam",
		"add alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.alias.add.incorrect.missingRequiredParam",
		"add alias 123",
		-600, "Alias address expected");
	__cmd2__("adminConsole.alias.delete.missingRequiredParam",
		"delete alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.alias.view.missingRequiredParam",
		"view alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"alter alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"alter alias 123",
		-600, "Alias address expected");
	//неправильные маски
	char cmd[64];
	char resp[128];
	vector<const char*> invalidMasks = Masks::invalid();
	for (int i = 0; i < invalidMasks.size(); i++)
	{
		//add
		sprintf(cmd, "add alias %s 123", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't add Alias '%s' for address '123'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.alias.add.incorrect.invalidAliasFormat", cmd, resp);
		sprintf(cmd, "add alias 123 %s", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't add Alias '123' for address '%s'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.alias.add.incorrect.invalidAddrFormat", cmd, resp);
		//delete
		sprintf(cmd, "delete alias %s", invalidMasks[i]);
        sprintf(resp, "/^\\Q%s Alias '%s' not found\\E$/",
			respFail(700), invalidMasks[i]);
		__cmd__("adminConsole.alias.delete.invalidAliasFormat", cmd, resp);
		//view
		sprintf(cmd, "view alias %s", invalidMasks[i]);
		__cmd__("adminConsole.alias.view.invalidAliasFormat", cmd, resp);
		//alter
		sprintf(cmd, "alter alias %s 123", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't alter Alias '%s'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.alias.alter.incorrect.invalidAliasFormat", cmd, resp);
		sprintf(cmd, "alter alias 123 %s", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't alter Alias '123'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i]);
		__cmd__("adminConsole.alias.alter.incorrect.invalidAliasFormat", cmd, resp);
	}
	//add
	__cmd2__("adminConsole.alias.add.incorrect.duplicateAlias",
		"add alias .0.1.00 123",
		-700, "Alias '.0.1.00' already exists");
	__cmd2__("adminConsole.alias.add.incorrect.duplicateAddr",
		"add alias 123 .1.1.00 hide",
		-700, "Alias for address '.1.1.01' already exists");
	__cmd2__("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias 1? 123",
		-700, "Couldn't add Alias '1?' for address '123'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias 123 1?",
		-700, "Couldn't add Alias '123?' for address '1?'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.alias.add.incorrect.hideWithQuestionMarks",
		"add alias 123? 123? hide",
		-700, "Couldn't add Alias '123?' for address '123?'. Cause: alias incorrect - question marks aren't allowed for aliases with hide option");
	//delete
	__cmd2__("adminConsole.alias.delete.nonExistent",
		"delete alias 111",
		-700, "Alias '111' not found");
	//view
	__cmd2__("adminConsole.alias.view.nonExistent",
		"view alias 111",
		-700, "Alias '111' not found");
	//alter
	__cmd2__("adminConsole.alias.alter.incorrect.nonExistent",
		"alter alias 123 123 hide",
		-700, "Alias '123' not found");
	__cmd2__("adminConsole.alias.alter.incorrect.duplicateAddr",
		"alter alias 00? 123 hide",
		-700, "Couldn't alter Alias '00?'. Cause: alias incorrect - question marks aren't allowed for aliases with hide option");
	__cmd2__("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias 00 123?",
		-700, "Couldn't alter Alias '00' for address '123?'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias 00? 123",
		-700, "Couldn't alter Alias '00?' for address '123'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.alias.alter.incorrect.hideWithQuestionMarks",
		"alter alias 00? 00? hide",
		-700, "Couldn't alter Alias '00?' for address '00?'. Cause: alias incorrect - question marks aren't allowed for aliases with hide option");
}

}
}
}

