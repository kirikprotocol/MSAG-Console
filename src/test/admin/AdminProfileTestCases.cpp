#include "AdminProfileTestCases.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::util::Logger;
using namespace smsc::test::util;

Category& AdminProfileTestCases::getLog()
{
	static Category& log = Logger::getCategory("AdminProfileTestCases");
	return log;
}

#define __cmd__(id, cmd, resp) \
	if (!hasRights) { \
		runTestCase(id, cmd, noRights); \
	} else if (!scRunning) { \
		runTestCase(id, cmd, noSC); \
	} else { \
		runTestCase(id, cmd, resp); \
	}

#define __cmd2__(id, cmd, code, resp) \
	__cmd__(id, cmd, simpleResp(code, resp))

const char* AdminProfileTestCases::getReport(const char* mask)
{
	return (hash(mask) & 0x1 ? "none" : "full");
}

const char* AdminProfileTestCases::getEncoding(const char* mask)
{
	return (hash(mask) & 0x2 ? "default" : "ucs2");
}

void AdminProfileTestCases::correctCommands()
{
	char cmd[128];
	char resp[256];
	//add correct
	vector<const char*> masks = Masks::correct();
	for (int i = 0; i < masks.size(); i++)
	{
		sprintf(cmd, "add profile %s report %s encoding %s",
			masks[i], getReport(masks[i]), getEncoding(masks[i]));
		sprintf(resp, "/^\\Q%s Profile for mask '%s' added\\E$/",
			respOk(100), masks[i]);
		__cmd__("adminConsole.profile.add.correct", cmd, resp);
	}
	//view added
	for (int i = 0; i < masks.size(); i++)
	{
		sprintf(cmd, "view profile %s", masks[i]);
		sprintf(resp, "/^\\Q%s Profile '%s' Report: %s Encoding: %s\\E$/",
			respOk(100), masks[i], getReport(masks[i]), getEncoding(masks[i]));
		__cmd__("adminConsole.profile.view.existent", cmd, resp);
	}
	//delete
	/*
	//const char* deleteList[] = {".0.1.00", ".1.1.00", ".0.1.00?", ".1.1.00?"};
	const char* deleteList[] = {"00", "+00", "00?", "+00?"};
	for (int i = 0; i < sizeof(deleteList) / sizeof(*deleteList); i++)
	{
		sprintf(cmd, "delete profile %s", deleteList[i]);
		sprintf(resp, "/^\\Q%s Alias '%s' deleted\\E$/",
			__human__("Ok.", "+ 100"), deleteList[i]);
		__cmd__("adminConsole.profile.delete.existent", cmd, resp);
	}
	*/
	//add existent
	__cmd__("adminConsole.profile.add.existentProfile",
		"add profile +02? report full encoding default",
		"Profile for mask '+02?' added");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile +02?",
		100, "Profile '+02?' Report: full Encoding: default");
	//veiw nonexistent
	__cmd__("adminConsole.profile.view.nonExistent",
		"view profile 111",
		"Profile '111' Report: none Encoding: default");
	//alter & view 00 (report: none -> full)
	__cmd2__("adminConsole.profile.alter.correct",
		"alter profile 00 report full",
		100, "Profile for address '00' altered");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile .0.1.00",
		100, "Profile '.0.1.00' Report: full Encoding: default");
	//alter & view .0.1.00 (encoding: default -> ucs2)
	__cmd2__("adminConsole.profile.alter.correct",
		"alter profile .0.1.00 encoding ucs2",
		100, "Profile for address '.0.1.00' altered");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile 00",
		100, "Profile '00' Report: full Encoding: ucs2");
	//alter & view 00 (report: full -> none)
	__cmd2__("adminConsole.profile.alter.correct",
		"alter profile 00 report none",
		100, "Profile for address '00' altered");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile .0.1.00",
		100, "Profile '.0.1.00' Report: none Encoding: ucs2");
	//alter & view .0.1.00 (encoding: ucs2 -> default)
	__cmd2__("adminConsole.profile.alter.correct",
		"alter profile .0.1.00 encoding default",
		100, "Profile for address '.0.1.00' altered");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile 00",
		100, "Profile '00' Report: none Encoding: default");
	//alter & view +00? (report: none -> full, encoding: default -> ucs2)
	__cmd2__("adminConsole.profile.alter.correct",
		"alter profile +00? report full encoding ucs2",
		100, "Profile for address '+00?' altered");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile .1.1.00?",
		100, "Profile '.1.1.00?' Report: full Encoding: ucs2");
	//alter & view .1.1.00? (report: full -> none, encoding: ucs2 -> default)
	__cmd2__("adminConsole.profile.alter.correct",
		"alter profile .1.1.00? report none encoding default",
		100, "Profile for address '.1.1.00?' altered");
	__cmd2__("adminConsole.profile.view.existent",
		"view profile +00?",
		100, "Profile '+00?' Report: none Encoding: default");
	//list (список не сортированный)
	/*
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
	listResp += __human__("/^Alias list:$", "");
	for (int i = 0; i < listSize; i++)
	{
		listResp += "^ \\Q";
		listResp += list[i];
		listResp += "\\E$";
	}
	listResp += __human__("", "+ 200 Alias list");
	listResp += "/ms";
	sprintf(cmd, "list alias %s", shit);
	__cmd__("adminConsole.profile.list", cmd, listResp.c_str());
	*/
}

void AdminProfileTestCases::incorrectCommands()
{
	//missing params
	//add
	__cmd2__("adminConsole.profile.add.incorrect.missingRequiredParam",
		"add profile",
		-600, "Profile mask expected");
	__cmd2__("adminConsole.profile.add.incorrect.missingRequiredParam",
		"add profile 123",
		-600, "expecting \"report\", found 'null'");
	__cmd2__("adminConsole.profile.add.incorrect.missingRequiredParam",
		"add profile 123 report",
		-600, "unexpected token: null");
	//alter
	__cmd2__("adminConsole.profile.alter.incorrect.missingRequiredParam",
		"alter profile",
		-600, "Syntax: alter profile <profile_address> (report (full|none) | encoding (ucs2|default))");
	__cmd2__("adminConsole.profile.alter.incorrect.missingRequiredParam",
		"alter profile 123",
		-600, "Syntax: alter profile <profile_address> (report (full|none) | encoding (ucs2|default))");
	__cmd2__("adminConsole.profile.alter.incorrect.missingRequiredParam",
		"alter profile 123 report",
		-600, "Syntax: alter profile <profile_address> (report (full|none) | encoding (ucs2|default))");
	//other
	__cmd2__("adminConsole.profile.view.missingRequiredParam",
		"view profile",
		-600, "Profile address expected");
	/*
	__cmd2__("adminConsole.profile.delete.incorrect.missingRequiredParam",
		"delete alias", "/^Failed: expecting ADDRESS, found 'null'$/");
	*/
	//неправильные маски
	char cmd[128];
	char resp[256];
	vector<const char*> invalidMasks = Masks::invalid();
	for (int i = 0; i < invalidMasks.size(); i++)
	{
		//add
		sprintf(cmd, "add profile %s report full encoding ucs2", invalidMasks[i]);
		sprintf(resp, "%s Couldn't add Profile for mask '%s'. Cause: Mask \"%s\" is not valid",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.profile.add.incorrect.invalidAddrFormat", cmd, resp);
		//delete
		/*
		sprintf(cmd, "delete alias '%s'", invalidMasks[i]);
		__cmd__("adminConsole.profile.delete.incorrect.invalidAliasFormat",
			cmd, resp);
		*/
		//view
		sprintf(cmd, "view profile %s", invalidMasks[i]);
		sprintf(resp, "%s Couldn't view Profile for mask '%s'. Cause: Mask \"%s\" is not valid",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.profile.view.invalidAddrFormat", cmd, resp);
		//alter
		sprintf(cmd, "alter profile %s", invalidMasks[i]);
		sprintf(resp, "%s Couldn't alter Profile for mask '%s'. Cause: Mask \"%s\" is not valid",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.profile.alter.invalidAddrFormat", cmd, resp);
	}
	//add
	__cmd2__("adminConsole.profile.add.incorrect.identicalToDefault",
		"add profile 123 report none",
		-700, "Profile for mask '123' is identical to default.");
	__cmd2__("adminConsole.profile.add.incorrect.identicalToDefault",
		"add profile 123 report none encoding default",
		-700, "Profile for mask '123' is identical to default.");
	/*
	//delete
	__cmd__("adminConsole.profile.delete.incorrect.nonExistent",
		"delete alias '111'",
		"/^Failed: Alias '111' not found$/");
	*/
	//alter
	__cmd2__("adminConsole.profile.alter.incorrect.nonExistent",
		"alter profile 123 report full encoding ucs2",
		-100, "");
}

}
}
}

