#include "AdminRouteTestCases.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::util::Logger;

Category& AdminRouteTestCases::getLog()
{
	static Category& log = Logger::getCategory("AdminRouteTestCases");
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

const char* AdminRouteTestCases::getSubjectName(int i)
{
	static char name[16];
	switch (i % 3)
	{
		case 0:
			sprintf(name, "subj%d", i / 3);
			break;
		case 1:
			sprintf(name, "\"subject %d\"", i / 3);
			break;
		case 2:
			sprintf(name, "\"субъект %d\"", i / 3);
			break;
	}
	return name;
}

void AdminRouteTestCases::correctSubjectCommands()
{
	char cmd[256];
	char resp[256];
	//add & view correct (единственная маска)
	vector<const char*> masks = Masks::correct();
	for (int i = 0; i < masks.size(); i++)
	{
		//add
		sprintf(cmd, "add subject %s sme00 %s", getSubjectName(i), masks[i]);
		sprintf(resp, "/^\\Q%s Subject '%s' added\\E$/",
			respOk(100), getSubjectName(i));
		__cmd__("adminConsole.subject.add.correct", cmd, resp);
		//view
		sprintf(cmd, "view subject %s", getSubjectName(i));
		sprintf(resp, "/^\\Q%s Subject '%s' DefSME: sme00 Mask(s): %s\\E$/",
			respOk(100), getSubjectName(i), masks[i]);
		__cmd__("adminConsole.subject.view.existent", cmd, resp);
	}
	//delete
	__cmd2__("adminConsole.subject.delete.existent",
		"delete subject subj0",
		100, "Subject 'subj0' deleted");
	__cmd2__("adminConsole.subject.delete.existent",
		"delete subject \"subject 0\"",
		100, "Subject 'subject 0' deleted");
	__cmd2__("adminConsole.subject.delete.existent",
		"delete subject \"субъект 0\"",
		100, "Subject 'субъект 0' deleted");
	//alter (единственная маска)
	for (int i = 0; i < masks.size(); i++)
	{
		int i2 = masks.size() - i - 1;
		if (i == i2) continue;
		//add mask
		sprintf(cmd, "alter subject %s add %s", getSubjectName(i), masks[i2]);
		sprintf(resp, "/^\\Q%s Subject '%s' altered. Mask(s) added\\E$/",
			respOk(100), getSubjectName(i));
		__cmd__("adminConsole.subject.alter.correct.addMask", cmd, resp);
		//delete mask
		sprintf(cmd, "alter subject %s delete %s", getSubjectName(i), masks[i]);
		sprintf(resp, "/^\\Q%s Subject '%s' altered. Mask(s) deleted\\E$/",
			respOk(100), getSubjectName(i));
		__cmd__("adminConsole.subject.alter.correct.deleteMask", cmd, resp);
		//change def sme
		sprintf(cmd, "alter subject %s defaultsme sme01", getSubjectName(i));
		sprintf(resp, "/^\\Q%s Subject '%s' altered. Default sme changed\\E$/",
			respOk(100), getSubjectName(i));
		__cmd__("adminConsole.subject.alter.correct.changeDefSme", cmd, resp);
		//view
		sprintf(cmd, "view subject %s", getSubjectName(i));
		sprintf(resp, "/^\\Q%s Subject '%s' DefSME: sme01 Mask(s): %s\\E$/",
			respOk(100), getSubjectName(i), masks[i2]);
		__cmd__("adminConsole.subject.view.existent", cmd, resp);
	}
	//add & alter & view субъект с несколькими масками и "зарезервированым" именем
	//add
	__cmd2__("adminConsole.subject.add.correct",
		"add subject \"subject\" sme00 111, 222, 333",
		100, "Subject 'subject' added");
	__cmd2__("adminConsole.subject.view.existent",
		"view subject \"subject\"",
		100, "Subject 'subject' DefSME: sme00 Mask(s): 222, 333, 111");
	//add & delete mask
	__cmd2__("adminConsole.subject.alter.correct.addMask",
		"alter subject \"subject\" add 444, 555, 666",
		100, "Subject 'subject' altered. Mask(s) added");
	__cmd2__("adminConsole.subject.alter.correct.deleteMask",
		"alter subject \"subject\" delete 111, 222, 333",
		100, "Subject 'subject' altered. Mask(s) deleted");
	__cmd2__("adminConsole.subject.view.existent",
		"view subject \"subject\"",
		100, "Subject 'subject' DefSME: sme00 Mask(s): 555, 666, 444");
	//list (список не сортированный)
	static const string list[] = {
		" субъект01",
		" subject00",
		" субъект00",
		" subject02",
		" субъект02",
		"subject01"
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
	listResp += fixture->humanConsole ? "" : "+ 200 Subjects list";
	listResp += "/ms";
	sprintf(cmd, "list alias %s", shit);
	__cmd__("adminConsole.subject.list", cmd, listResp.c_str());
}

void AdminRouteTestCases::correctRouteCommands()
{
	/*
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
		__cmd__("adminConsole.subject.add.correct", cmd, resp);
	}
	//view added
	for (int i = 0; i < masks.size(); i++)
	{
		sprintf(cmd, "view alias %s %s", masks[i], shit);
		sprintf(resp, "/^\\Q%s Alias '%s' Address '%s' (nohide)\\E$/",
			respOk(100), masks[i], masks[i]);
		__cmd__("adminConsole.subject.view.existent", cmd, resp);
	}
	//add & delete
	sprintf(resp, "/^\\Q%s Alias '123' for address '123' added\\E$/", respOk(100));
	__cmd__("adminConsole.subject.add.correct",
		"add alias 123 123 hide", resp);
	sprintf(resp, "/^\\Q%s Alias '123' deleted\\E$/", respOk(100));
	__cmd__("adminConsole.subject.delete.existent",
		"delete alias 123", resp);
	//delete
	//const char* deleteList[] = {".0.1.00", ".1.1.00", ".0.1.00?", ".1.1.00?"};
	const char* deleteList[] = {"00", "+00", "00?", "+00?"};
	for (int i = 0; i < sizeof(deleteList) / sizeof(*deleteList); i++)
	{
		sprintf(cmd, "delete alias %s %s", deleteList[i], shit);
		sprintf(resp, "/^\\Q%s Alias '%s' deleted\\E$/",
			respOk(100), deleteList[i]);
		__cmd__("adminConsole.subject.delete.existent", cmd, resp);
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
		__cmd__("adminConsole.subject.alter.correct", cmd, resp);
	}
	//view
	sprintf(cmd, "view alias 20 %s", shit);
	sprintf(resp, "/^\\Q%s Alias '20' Address '02' (hide)\\E$/", respOk(100));
	__cmd__("adminConsole.subject.view.existent", cmd, resp);
	sprintf(cmd, "view alias 20? %s", shit);
	sprintf(resp, "/^\\Q%s Alias '20?' Address '02?' (nohide)\\E$/", respOk(100));
	__cmd__("adminConsole.subject.view.existent", cmd, resp);
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
	__cmd__("adminConsole.subject.list", cmd, listResp.c_str());
	*/
}

void AdminRouteTestCases::incorrectSubjectCommands()
{
	/*
	//missing params
	__cmd2__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"add subject",
		-600, "unexpected token: null. Subject name expected.");
	__cmd2__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"add subject aaa",
		-600, "unexpected token: null. SME id expected.");
	__cmd2__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"add subject aaa sme00",
		-600, "Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	__cmd2__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"add subject aaa sme00 123,,",
		-600, "Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	__cmd2__("adminConsole.subject.delete.missingRequiredParam",
		"delete subject",
		-600, "unexpected token: null. Subject name expected.");
	__cmd2__("adminConsole.subject.view.missingRequiredParam",
		"view subject",
		-600, "unexpected token: null. Subject name expected.");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject",
		-600, "unexpected token: null. Subject name expected.");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject aaa",
		-600, "unexpected token: null");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject aaa add",
		-600, "Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject aaa add 123,,",
		-600, "Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject aaa delete",
		-600, "Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject aaa delete 123,,",
		-600, "Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter subject aaa defaultsme",
		-600, "unexpected token: null. SME id expected.");
	//неправильные маски
	char cmd[128];
	char resp[128];
	vector<const char*> invalidMasks = Masks::invalid();
	for (int i = 0; i < invalidMasks.size(); i++)
	{
		//add
		sprintf(cmd, "add subject %s 123", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't add Alias '%s' for address '123'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.subject.add.incorrect.invalidAliasFormat", cmd, resp);
		sprintf(cmd, "add subject 123 %s", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't add Alias '123' for address '%s'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.subject.add.incorrect.invalidAddrFormat", cmd, resp);
		//delete
		sprintf(cmd, "delete subject %s", invalidMasks[i]);
        sprintf(resp, "/^\\Q%s Alias '%s' not found\\E$/",
			respFail(700), invalidMasks[i]);
		__cmd__("adminConsole.subject.delete.invalidAliasFormat", cmd, resp);
		//view
		sprintf(cmd, "view subject %s", invalidMasks[i]);
		__cmd__("adminConsole.subject.view.invalidAliasFormat", cmd, resp);
		//alter
		sprintf(cmd, "alter subject %s 123", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't alter Alias '%s'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.subject.alter.incorrect.invalidAliasFormat", cmd, resp);
		sprintf(cmd, "alter subject 123 %s", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't alter Alias '123'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i]);
		__cmd__("adminConsole.subject.alter.incorrect.invalidAliasFormat", cmd, resp);
	}
	//add
	__cmd2__("adminConsole.subject.add.incorrect.duplicateAlias",
		"add subject .0.1.00 123",
		-700, "Alias '.0.1.00' already exists");
	__cmd2__("adminConsole.subject.add.incorrect.duplicateAddr",
		"add subject 123 .1.1.00 hide",
		-700, "Alias for address '.1.1.01' already exists");
	__cmd2__("adminConsole.subject.add.incorrect.questionMarksNotMatch",
		"add subject 1? 123",
		-700, "Couldn't add Alias '1?' for address '123'. Cause: subject incorrect - question marks in subject and address do not match");
	__cmd2__("adminConsole.subject.add.incorrect.questionMarksNotMatch",
		"add subject 123 1?",
		-700, "Couldn't add Alias '123?' for address '1?'. Cause: subject incorrect - question marks in subject and address do not match");
	__cmd2__("adminConsole.subject.add.incorrect.hideWithQuestionMarks",
		"add subject 123? 123? hide",
		-700, "Couldn't add Alias '123?' for address '123?'. Cause: subject incorrect - question marks aren't allowed for subjectes with hide option");
	//delete
	__cmd2__("adminConsole.subject.delete.nonExistent",
		"delete subject 111",
		-700, "Alias '111' not found");
	//view
	__cmd2__("adminConsole.subject.view.nonExistent",
		"view subject 111",
		-700, "Alias '111' not found");
	//alter
	__cmd2__("adminConsole.subject.alter.incorrect.nonExistent",
		"alter subject 123 123 hide",
		-700, "Alias '123' not found");
	__cmd2__("adminConsole.subject.alter.incorrect.duplicateAddr",
		"alter subject 00? 123 hide",
		-700, "Couldn't alter Alias '00?'. Cause: subject incorrect - question marks aren't allowed for subjectes with hide option");
	__cmd2__("adminConsole.subject.alter.incorrect.questionMarksNotMatch",
		"alter subject 00 123?",
		-700, "Couldn't alter Alias '00' for address '123?'. Cause: subject incorrect - question marks in subject and address do not match");
	__cmd2__("adminConsole.subject.alter.incorrect.questionMarksNotMatch",
		"alter subject 00? 123",
		-700, "Couldn't alter Alias '00?' for address '123'. Cause: subject incorrect - question marks in subject and address do not match");
	__cmd2__("adminConsole.subject.alter.incorrect.hideWithQuestionMarks",
		"alter subject 00? 00? hide",
		-700, "Couldn't alter Alias '00?' for address '00?'. Cause: subject incorrect - question marks aren't allowed for subjectes with hide option");
	*/
}

void AdminRouteTestCases::incorrectRouteCommands()
{
	/*
	//missing params
	__cmd2__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"add alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.subject.add.incorrect.missingRequiredParam",
		"add alias 123",
		-600, "Alias address expected");
	__cmd2__("adminConsole.subject.delete.missingRequiredParam",
		"delete alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.subject.view.missingRequiredParam",
		"view alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
		"alter alias",
		-600, "Alias mask expected");
	__cmd2__("adminConsole.subject.alter.incorrect.missingRequiredParam",
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
		__cmd__("adminConsole.subject.add.incorrect.invalidAliasFormat", cmd, resp);
		sprintf(cmd, "add alias 123 %s", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't add Alias '123' for address '%s'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.subject.add.incorrect.invalidAddrFormat", cmd, resp);
		//delete
		sprintf(cmd, "delete alias %s", invalidMasks[i]);
        sprintf(resp, "/^\\Q%s Alias '%s' not found\\E$/",
			respFail(700), invalidMasks[i]);
		__cmd__("adminConsole.subject.delete.invalidAliasFormat", cmd, resp);
		//view
		sprintf(cmd, "view alias %s", invalidMasks[i]);
		__cmd__("adminConsole.subject.view.invalidAliasFormat", cmd, resp);
		//alter
		sprintf(cmd, "alter alias %s 123", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't alter Alias '%s'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i], invalidMasks[i]);
		__cmd__("adminConsole.subject.alter.incorrect.invalidAliasFormat", cmd, resp);
		sprintf(cmd, "alter alias 123 %s", invalidMasks[i]);
		sprintf(resp, "/^\\Q%s Couldn't alter Alias '123'. Cause: Mask \"%s\" is not valid\\E$/",
			respFail(700), invalidMasks[i]);
		__cmd__("adminConsole.subject.alter.incorrect.invalidAliasFormat", cmd, resp);
	}
	//add
	__cmd2__("adminConsole.subject.add.incorrect.duplicateAlias",
		"add alias .0.1.00 123",
		-700, "Alias '.0.1.00' already exists");
	__cmd2__("adminConsole.subject.add.incorrect.duplicateAddr",
		"add alias 123 .1.1.00 hide",
		-700, "Alias for address '.1.1.01' already exists");
	__cmd2__("adminConsole.subject.add.incorrect.questionMarksNotMatch",
		"add alias 1? 123",
		-700, "Couldn't add Alias '1?' for address '123'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.subject.add.incorrect.questionMarksNotMatch",
		"add alias 123 1?",
		-700, "Couldn't add Alias '123?' for address '1?'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.subject.add.incorrect.hideWithQuestionMarks",
		"add alias 123? 123? hide",
		-700, "Couldn't add Alias '123?' for address '123?'. Cause: alias incorrect - question marks aren't allowed for aliases with hide option");
	//delete
	__cmd2__("adminConsole.subject.delete.nonExistent",
		"delete alias 111",
		-700, "Alias '111' not found");
	//view
	__cmd2__("adminConsole.subject.view.nonExistent",
		"view alias 111",
		-700, "Alias '111' not found");
	//alter
	__cmd2__("adminConsole.subject.alter.incorrect.nonExistent",
		"alter alias 123 123 hide",
		-700, "Alias '123' not found");
	__cmd2__("adminConsole.subject.alter.incorrect.duplicateAddr",
		"alter alias 00? 123 hide",
		-700, "Couldn't alter Alias '00?'. Cause: alias incorrect - question marks aren't allowed for aliases with hide option");
	__cmd2__("adminConsole.subject.alter.incorrect.questionMarksNotMatch",
		"alter alias 00 123?",
		-700, "Couldn't alter Alias '00' for address '123?'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.subject.alter.incorrect.questionMarksNotMatch",
		"alter alias 00? 123",
		-700, "Couldn't alter Alias '00?' for address '123'. Cause: alias incorrect - question marks in alias and address do not match");
	__cmd2__("adminConsole.subject.alter.incorrect.hideWithQuestionMarks",
		"alter alias 00? 00? hide",
		-700, "Couldn't alter Alias '00?' for address '00?'. Cause: alias incorrect - question marks aren't allowed for aliases with hide option");
	*/
}

}
}
}

