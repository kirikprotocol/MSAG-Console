#include "AdminAliasTestCases.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::util::Logger;
using namespace smsc::test::util;

Category& AdminAliasTestCases::getLog()
{
	static Category& log = Logger::getCategory("AdminAliasTestCases");
	return log;
}

void AdminAliasTestCases::correctCommands()
{
	vector<const char*> masks = Masks::correct();
	for (int i = 0; i < masks.size(); i++)
	{
		char cmd[128];
		char resp[256];
		sprintf(cmd, "add alias '%s' '%s' %s %s",
			masks[i], masks[i], i % 2 ? "hide" : "nohide", symbolChars);
		sprintf(resp, "Ok. Alias '%s' for address '%s' added", masks[i], masks[i]);
		addTestCase("adminConsole.alias.add.correct", cmd, resp);
	}
	for (int i = 0; i < masks.size(); i++)
	{
		char cmd[128];
		char resp[256];
		sprintf(cmd, "view alias '%s' %s", masks[i], symbolChars);
		sprintf(resp, "Ok. Alias '%s' Address '%s' (%s)",
			masks[i], masks[i], i % 2 ? "hide" : "nohide");
		addTestCase("adminConsole.alias.view.existent", cmd, resp);
	}
	char cmd[128];
	sprintf(cmd, "delete alias '00' %s", symbolChars);
	addTestCase("adminConsole.alias.delete.existent",
		cmd, "Ok. Alias '00' deleted");
	sprintf(cmd, "alter alias '01' '+11' nohide %s", symbolChars);
	addTestCase("adminConsole.alias.alter.existent",
		cmd, "Ok. Alias '01' altered");
	sprintf(cmd, "view alias '02' %s", symbolChars);
	addTestCase("adminConsole.alias.view.existent",
		cmd, "Ok. Alias '02' Address '+22' (nohide)");
}

void AdminAliasTestCases::incorrectCommands()
{
	//missing params
	addTestCase("adminConsole.alias.add.incorrect.missingRequiredParam",
		"add alias", "Failed: expecting ADDRESS, found 'null'");
	addTestCase("adminConsole.alias.add.incorrect.missingRequiredParam",
		"add alias '123'", "Failed: expecting ADDRESS, found 'null'");
	addTestCase("adminConsole.alias.delete.missingRequiredParam",
		"delete alias", "Failed: expecting ADDRESS, found 'null'");
	addTestCase("adminConsole.alias.view.missingRequiredParam",
		"view alias", "Failed: expecting ADDRESS, found 'null'");
	addTestCase("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"alter alias", "Failed: expecting ADDRESS, found 'null'");
	addTestCase("adminConsole.alias.alter.incorrect.missingRequiredParam",
		"alter alias '123'", "Failed: expecting ADDRESS, found 'null'");
	//неправильные маски
	vector<const char*> invalidMasks1 = Masks::invalid1();
	for (int i = 0; i < invalidMasks1.size(); i++)
	{
		char cmd[64];
		const char* resp = "Failed: expecting '.', found '.'";
		//add
		sprintf(cmd, "add alias '%s' '123'", invalidMasks1[i]);
		addTestCase("adminConsole.alias.add.incorrect.invalidAliasFormat",
			cmd, resp);
		sprintf(cmd, "add alias '123' '%s'", invalidMasks1[i]);
		addTestCase("adminConsole.alias.add.incorrect.invalidAddrFormat",
			cmd, resp);
		//delete
		sprintf(cmd, "delete alias '%s'", invalidMasks1[i]);
		addTestCase("adminConsole.alias.delete.invalidAliasFormat",
			cmd, resp);
		//view
		sprintf(cmd, "view alias '%s'", invalidMasks1[i]);
		addTestCase("adminConsole.alias.view.invalidAliasFormat",
			cmd, resp);
		//alter
		sprintf(cmd, "alter alias '%s' '123'", invalidMasks1[i]);
		addTestCase("adminConsole.alias.alter.invalidAliasFormat",
			cmd, resp);
	}
	vector<const char*> invalidMasks2 = Masks::invalid2();
	for (int i = 0; i < invalidMasks2.size(); i++)
	{
		char cmd[64];
		char resp[128];
		//add
		sprintf(cmd, "add alias '%s' '123'", invalidMasks2[i]);
		sprintf(resp, "Failed: Failed to add alias '%s' for address '123'. Cause: Mask \"%s\" is not valid",
			invalidMasks2[i], invalidMasks2[i]);
		addTestCase("adminConsole.alias.add.incorrect.invalidAliasFormat",
			cmd, resp);
		sprintf(cmd, "add alias '123' '%s'", invalidMasks2[i]);
		addTestCase("adminConsole.alias.add.incorrect.invalidAddrFormat",
			cmd, resp);
		//delete
		sprintf(resp, "Failed: Alias '%s' not found", invalidMasks2[i]);
		sprintf(cmd, "delete alias '%s'", invalidMasks2[i]);
		addTestCase("adminConsole.alias.delete.invalidAliasFormat",
			cmd, resp);
		//view
		sprintf(cmd, "view alias '%s'", invalidMasks2[i]);
		addTestCase("adminConsole.alias.view.invalidAliasFormat",
			cmd, resp);
		//alter
		sprintf(cmd, "alter alias '%s' '123'", invalidMasks2[i]);
		addTestCase("adminConsole.alias.alter.incorrect.invalidAliasFormat",
			cmd, resp);
		sprintf(cmd, "alter alias '01' '%s'", invalidMasks2[i]);
		addTestCase("adminConsole.alias.alter.incorrect.invalidAddrFormat",
			cmd, resp);
	}
	//add
	addTestCase("adminConsole.alias.add.incorrect.duplicateAlias",
		"add alias '00' '123'",
		"Failed: Alias '00' for address '123' already exists");
	addTestCase("adminConsole.alias.add.incorrect.duplicateAddr",
		"add alias '123' '+01' hide",
		"Failed: Alias '123' for address '+01' already exists");
	addTestCase("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias '1?' '123'",
		"Failed: ...");
	addTestCase("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias '123' '1?'",
		"Failed: ...");
	addTestCase("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias '1??' '123?'",
		"Failed: ...");
	addTestCase("adminConsole.alias.add.incorrect.questionMarksNotMatch",
		"add alias '123?' '1??'",
		"Failed: ...");
	addTestCase("adminConsole.alias.add.incorrect.hideWithQuestionMarks",
		"add alias '123?' '123?' hide",
		"Failed: ...");
	//delete
	addTestCase("adminConsole.alias.delete.nonExistent",
		"delete alias '111???'",
		"Failed: Alias '111???' not found");
	//view
	addTestCase("adminConsole.alias.view.nonExistent",
		"view alias '111???'",
		"Failed: Alias '111???' not found");
	//alter
	addTestCase("adminConsole.alias.add.correct",
		"add alias '123???' '123???'", ".*");
	addTestCase("adminConsole.alias.alter.incorrect.duplicateAddr",
		"alter alias '00' '+01' hide",
		"Failed: ...");
	addTestCase("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias '00' '123?'",
		"Failed: ...");
	addTestCase("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias '123???' '123'",
		"Failed: ...");
	addTestCase("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias '123???' '123??'",
		"Failed: ...");
	addTestCase("adminConsole.alias.alter.incorrect.questionMarksNotMatch",
		"alter alias '123???' '123????'",
		"Failed: ...");
	addTestCase("adminConsole.alias.alter.incorrect.hideWithQuestionMarks",
		"alter alias '123???' '123????' hide",
		"Failed: ...");
	addTestCase("adminConsole.alias.delete.correct",
		"delete alias '123???'", ".*");
}

}
}
}

