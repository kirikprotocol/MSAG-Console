#include "AdminBaseTestCases.hpp"
#include "util/Exception.hpp"
#include "util/regexp/RegExp.hpp"

namespace smsc {
namespace test {
namespace admin {

using smsc::util::Logger;
using smsc::util::Exception;
using namespace smsc::util::regexp;
using namespace smsc::test::util;

AdminBaseTestCases::AdminBaseTestCases(AdminFixture* fixture)
: humanConsole(fixture->humanConsole), chkList(fixture->chkList)
{
	int timeout = 8;
	if (socket.Init(fixture->host, fixture->port, timeout) == -1)
		throw Exception("Failed to init socket");
	if (socket.Connect() == -1)
		throw Exception("Failed to connect to host");
}

AdminBaseTestCases::~AdminBaseTestCases()
{
	socket.Close();
	for (int i = 0; i < testCases.size(); i++)
	{
		delete testCases[i];
	}
}

Category& AdminBaseTestCases::getLog()
{
	static Category& log = Logger::getCategory("AdminBaseTestCases");
	return log;
}

void AdminBaseTestCases::sendRequest(const char* cmd)
{
	__require__(cmd);
	__trace2__("request: %s", cmd);
	int count = 0;
	string tmp;
	tmp.reserve(strlen(cmd) + 3);
	tmp += cmd;
	tmp += "\n";
	int wr = socket.WriteAll(tmp.c_str(), tmp.length());
	//__trace2__("socket write: wr = %d", wr);
	if (wr <= 0)
	{
		__trace2__("socket write error: wr = %d, reason = %s", wr, strerror(errno));
		throw Exception("Failed to send data");
	}
}

bool AdminBaseTestCases::checkResponse(const char* pattern)
{
	__require__(pattern);
	__trace2__("regexp: %s", pattern);
	RegExp chkResp;
	chkResp.Compile(pattern);
	Hash<SMatch> h;
	SMatch match;
	int matchcount = 1;
	string resp;
	resp.reserve(128);
	while (socket.canRead())
	{
		int ch = socket.readChar();
		if (ch == -1)
		{
			throw Exception("socket read failed");
		}
		//trim
		if (!resp.length() && (ch == '\n' || ch == '\r' || ch == ' '))
		{
			continue;
		}
		resp += ch;
		if (!isalpha(ch))
		{
			//__trace2__("socket read: '%s'", resp.c_str());
			if (chkResp.Match(resp.c_str(), &match, matchcount))
			{
				__trace2__("resp(ok): %s", resp.c_str());
				return true;
			}
		}
	}
	__trace2__("resp(err): %s", resp.c_str());
	return false;
}

void AdminBaseTestCases::addTestCase(const char* id, const char* cmd,
	const char* humanResp /*, const char* scriptResp*/)
{
	const char* scriptResp = "";
	__require__(id && cmd && humanResp && scriptResp);
	testCases.push_back(new AdminTestCase(id, cmd, humanResp, scriptResp));
}

void AdminBaseTestCases::runTestCase(const char* id, const char* cmd,
	const char* humanResp /*, const char* scriptResp*/)
{
	bool isOk = true;
	TestCase* tc = chkList->getTc(id);
	sendRequest(cmd);
	if (!checkResponse(humanConsole ? humanResp : NULL))
	{
		__tc_fail__(1);
	}
	if (!checkResponse("^>$"))
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
}

bool AdminBaseTestCases::login(const char* login, const char* passwd,
	bool correct)
{
	__require__(login && passwd);
	bool res = true;
	char pattern[128];
	res &= checkResponse("/.*Login:$/ms");
	sendRequest(login);
	sprintf(pattern, "/^%s$^Password:$/ms", login);
	res &= checkResponse(pattern);
	sendRequest(passwd);
	sprintf(pattern, "/%s$^Password:/ms", login);
	res &= checkResponse(correct ?
		"/^Welcome to SMSC Remote Console.$(^$)*>$/ms" :
		"/^Authentication failed. Access denied.$/ms");
	return res;
}

void AdminBaseTestCases::executeTestCases()
{
	RegExp::InitLocale();
	for (int i = 0; i < testCases.size(); i++)
	{
		runTestCase(testCases[i]->id.c_str(), testCases[i]->cmd.c_str(),
            testCases[i]->humanResp.c_str() /*, testCases[i]->scriptResp.c_str()*/);
	}
}

void AdminBaseTestCases::loginCommands()
{
	__decl_tc__;
	__tc__("adminConsole.login.incorrect");
	if (!login("superadmin", "aaa", false))
	{
		__tc_fail__(1);
	}
	if (!login("aaa", "aaa", false))
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	__tc__("adminConsole.login.correct");
	if (!login("superadmin", "123", true))
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

#define __cmd__(id, cmd, humanResp) \
	runTestCase(id, cmd, humanResp)
	
void AdminBaseTestCases::invalidCommands()
{
	string symbols = "~!@#$%^&*()_+|{}:\"<>?`1234567890-=\[];',./" \
		"1234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
		"ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß";
	char cmd[16];
	char resp[128];
	for (int i = 0; i < symbols.length(); i++)
	{
		const char* s = string(3, symbols[i]).c_str();
		//itself
		sprintf(cmd, "%s", s);
		sprintf(resp, "Failed: unexpected char: %c", symbols[i]);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
		//add, delete, view, alter, list
		sprintf(resp, "Failed: unexpected token: %s", s);
		sprintf(cmd, "add %s", s);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "delete %s", s);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "view %s", s);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "alter %s", s);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "list %s", s);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
	}
}

}
}
}

