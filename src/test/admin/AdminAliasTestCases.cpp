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

#define __cmd__(id, cmd, humanResp) \
	runTestCase(id, cmd, humanResp)

void AdminAliasTestCases::correctCommands()
{
	vector<const char*> masks = Masks::correct();
	for (int i = 0; i < masks.size(); i++)
	{
		char cmd[128];
		char resp[256];
		sprintf(cmd, "add alias '%s' '%s' nohide %s",
			masks[i], masks[i], shit);
		sprintf(resp, "/^\\QOk. Alias '%s' for address '%s' added\\E$/m", masks[i], masks[i]);
		__cmd__("adminConsole.alias.add.correct", cmd, resp);
	}
	for (int i = 0; i < masks.size(); i++)
	{
		char cmd[128];
		char resp[256];
		sprintf(cmd, "view alias '%s' %s", masks[i], shit);
		sprintf(resp, "/^\\QOk. Alias '%s' Address '%s' (nohide)\\E$/m",
			masks[i], masks[i]);
		__cmd__("adminConsole.alias.view.existent", cmd, resp);
	}
	__cmd__("adminConsole.alias.add.correct",
		"add alias '123' '123' hide",
		"/^Ok. Alias '123' for address '123' added$/m");
	__cmd__("adminConsole.alias.delete.existent",
		"delete alias '123'", "/^Ok. Alias '123' deleted$/m");
	char cmd[128];
	//delete
	sprintf(cmd, "delete alias '00' %s", shit);
	__cmd__("adminConsole.alias.delete.existent",
		cmd, "/^Ok. Alias '00' deleted$/m");
	sprintf(cmd, "delete alias '00?' %s", shit);
	__cmd__("adminConsole.alias.delete.existent",
		cmd, "/^\\QOk. Alias '00?' deleted\\E$/m");
	//alter
	sprintf(cmd, "alter alias '01' '+01' nohide %s", shit);
	__cmd__("adminConsole.alias.alter.correct",
		cmd, "/^Ok. Alias '01' altered$/m");
	sprintf(cmd, "alter alias '01?' '+01?'" /*, shit*/);
	__cmd__("adminConsole.alias.alter.correct",
		cmd, "/^\\QOk. Alias '01?' altered\\E$/m");
	//view
	sprintf(cmd, "view alias '02' %s", shit);
	__cmd__("adminConsole.alias.view.existent",
		cmd, "/^\\QOk. Alias '02' Address '+01' (hide)\\E$/m");
	sprintf(cmd, "view alias '02?' %s", shit);
	__cmd__("adminConsole.alias.view.existent",
		cmd, "/^\\QOk. Alias '02?' Address '+01?' (nohide)\\E$/m");
	//list (список не сортированный)
	static const string list[] = {
		"01",
		"+12345678901234567890",
		"+123456789012345678??",
		".0.0.12345678901234567890",
		"12345678901234567890",
		"02",
		"0",
		"123456789012345678??",
		".0.0.123456789012345678??",
		"02?",
		"+0",
		".0.0.0",
		"01?"
	};
	static const int listSize = sizeof(list) / sizeof(*list);
	string humanResp;
	humanResp.reserve(1024);
	humanResp += "/^Alias list:$";
	for (int i = 0; i < listSize; i++)
	{
		humanResp += "^ \\Q";
		humanResp += list[i];
		humanResp += "\\E$";
	}
	humanResp += "/ms";
	sprintf(cmd, "list alias %s", shit);
	__cmd__("adminConsole.alias.list",
		cmd, humanResp.c_str());
}

void AdminAliasTestCases::incorrectCommands()
{
	//missing params
	__cmd__("adminConsole.alias.add.incorrect.missingRequiredParam",
		"add alias", "/^Failed: expecting ADDRESS, found 'null'$/m");
	__cmd__("adminConsole.alias.add.incorrect.missingRequiredParam",
		"add alias '123'", "/^Failed: expecting ADDRESS, found 'null'$/m");
	__cmd__("adminConsole.alias.delete.missingRequiredParam",
		"delete alias", "/^Failed: expecting ADDRESS, found 'null'$/m");
	__cmd__("adminConsole.alias.view.missingRequiredParam",
		"view alias", "/^Failed: expecting ADDRESS, found 'null'$/m");
	__cmd__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"alter alias", "/^Failed: expecting ADDRESS, found 'null'$/m");
	__cmd__("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"alter alias '123'", "/^Failed: expecting ADDRESS, found 'null'$/m");
	//неправильные маски
	vector<const char*> invalidMasks1 = Masks::invalid1();
	for (int i = 0; i < invalidMasks1.size(); i++)
	{
		char cmd[64];
		const char* resp = "/^Failed: expecting '.', found '.'$/m";
		//add
		sprintf(cmd, "add alias '%s' '123'", invalidMasks1[i]);
		__cmd__("adminConsole.alias.add.incorrect.invalidAliasFormat",
			cmd, resp);
		sprintf(cmd, "add alias '123' '%s'", invalidMasks1[i]);
		__cmd__("adminConsole.alias.add.incorrect.invalidAddrFormat",
			cmd, resp);
		//delete
		sprintf(cmd, "delete alias '%s'", invalidMasks1[i]);
		__cmd__("adminConsole.alias.delete.invalidAliasFormat",
			cmd, resp);
		//view
		sprintf(cmd, "view alias '%s'", invalidMasks1[i]);
		__cmd__("adminConsole.alias.view.invalidAliasFormat",
			cmd, resp);
		//alter
		sprintf(cmd, "alter alias '%s' '123'", invalidMasks1[i]);
		__cmd__("adminConsole.alias.alter.incorrect.invalidAliasFormat",
			cmd, resp);
	}
	vector<const char*> invalidMasks2 = Masks::invalid2();
	for (int i = 0; i < invalidMasks2.size(); i++)
	{
		char cmd[64];
		char resp[128];
		//add
		sprintf(cmd, "add alias '%s' '123'", invalidMasks2[i]);
		sprintf(resp, "/^\\QFailed: Failed to add alias '%s' for address '123'. Cause: Mask \"%s\" is not valid\\E$/m",
			invalidMasks2[i], invalidMasks2[i]);
		__cmd__("adminConsole.alias.add.incorrect.invalidAliasFormat",
			cmd, resp);
		sprintf(cmd, "add alias '123' '%s'", invalidMasks2[i]);
		__cmd__("adminConsole.alias.add.incorrect.invalidAddrFormat",
			cmd, resp);
		//delete
		sprintf(resp, "/^\\QFailed: Alias '%s' not found\\E$/m", invalidMasks2[i]);
		sprintf(cmd, "delete alias '%s'", invalidMasks2[i]);
		__cmd__("adminConsole.alias.delete.invalidAliasFormat",
			cmd, resp);
		//view
		sprintf(cmd, "view alias '%s'", invalidMasks2[i]);
		__cmd__("adminConsole.alias.view.invalidAliasFormat",
			cmd, resp);
		//alter
		sprintf(cmd, "alter alias '%s' '123'", invalidMasks2[i]);
		__cmd__("adminConsole.alias.alter.incorrect.invalidAliasFormat",
			cmd, resp);
		sprintf(cmd, "alter alias '01' '%s'", invalidMasks2[i]);
		__cmd__("adminConsole.alias.alter.incorrect.invalidAddrFormat",
			cmd, resp);
	}
	//add
	__cmd__("adminConsole.alias.add.incorrect.duplicateAlias",
		"add alias '.0.1.00' '123'",
		"/^Failed: Alias '.0.1.00' for address '123' already exists$/m");
	__cmd__("adminConsole.alias.add.incorrect.duplicateAddr",
		"add alias '123' '.1.1.00' hide",
		"/^Failed: Alias '123' for address '+01' already exists$/m");
	__cmd__("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias '1?' '123'",
		"/^Failed: ...$/m");
	__cmd__("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias '123' '1?'",
		"/^Failed: ...$/m");
	__cmd__("adminConsole.alias.add.incorrect.hideWithQuestionMarks",
		"add alias '123?' '123?' hide",
		"/^Failed: ...$/m");
	//delete
	__cmd__("adminConsole.alias.delete.nonExistent",
		"delete alias '111'",
		"/^Failed: Alias '111' not found$/m");
	//view
	__cmd__("adminConsole.alias.view.nonExistent",
		"view alias '111'",
		"/^Failed: Alias '111' not found$/m");
	//alter
	__cmd__("adminConsole.alias.alter.incorrect.nonExistent",
		"alter alias '123' '123' hide",
		"/^Failed: ...$/m");
	__cmd__("adminConsole.alias.alter.incorrect.duplicateAddr",
		"alter alias '00' '.1.1.01' hide",
		"/^Failed: ...$/m");
	__cmd__("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias '00' '123?'",
		"/^Failed: ...$/m");
	__cmd__("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias '00?' '123'",
		"/^Failed: ...$/m");
	__cmd__("adminConsole.alias.alter.incorrect.hideWithQuestionMarks",
		"alter alias '00?' '00?' hide",
		"/^Failed: ...$/m");
}

}
}
}

