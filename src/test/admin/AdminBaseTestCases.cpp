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

AdminBaseTestCases::AdminBaseTestCases(AdminFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList), connected(false)
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
	int regExpOk = chkResp.Compile(pattern);
	__require__(regExpOk);
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
		if (ch == '\n' || ch == '\r' || ch == ' ')
		{
			//trim leading whitespaces
			if (!resp.length())
			{
				continue;
			}
			//чтобы дл€ однострочных ответов исключить перевод строки
			if (chkResp.Match(resp.c_str(), &match, matchcount))
			{
				__trace2__("resp(ok): %s", resp.c_str());
				return true;
			}
			resp += ch;
		}
		else
		{
			resp += ch;
			if (!isalpha(ch) && chkResp.Match(resp.c_str(), &match, matchcount))
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
	const char* resp)
{
	__require__(id && cmd && resp);
	testCases.push_back(new AdminTestCase(id, cmd, resp));
}

void AdminBaseTestCases::runTestCase(const char* id, const char* cmd,
	const char* resp)
{
	__require__(id && cmd && resp);
	__require__(connected);
	bool isOk = true;
	TestCase* tc = chkList->getTc(id);
	sendRequest(cmd);
	char echoResp[strlen(cmd) + 10];
	sprintf(echoResp, "/^\\Q%s\\E$/m", cmd);
	if (!checkResponse(echoResp))
	{
		__tc_fail__(1);
	}
	if (!checkResponse(resp))
	{
		__tc_fail__(2);
	}
	const char* promptResp = "/^>$/";
	if (!checkResponse(promptResp))
	{
		__tc_fail__(3);
	}
	__tc_ok_cond__;
}

#define __check_resp__(pattern) \
	res &= checkResponse(pattern); \
	if (!res) return false;

bool AdminBaseTestCases::login(const char* login, const char* passwd,
	bool correct)
{
	__require__(login && passwd);
	bool res = true;
	char pattern[128];
	if (fixture->humanConsole)
	{
		__check_resp__("/.*Login:$/ms");
		sendRequest(login);
		sprintf(pattern, "/^%s$/", login);
		__check_resp__(pattern);
		__check_resp__("/^Password:$/");
		sendRequest(passwd);
		if (correct)
		{
			__check_resp__("/^\\QWelcome to SMSC Remote Console.\\E$/");
			__check_resp__("/^>$/");
		}
		else
		{
			__check_resp__("/^\\QAuthentication failed. Access denied.\\E$/");
		}
	}
	else
	{
		__check_resp__("/^\\Q+ 100 Connected. Login:\\E$/");
		sendRequest(login);
		__check_resp__("/^\\Q+ 100 Login accepted. Password:\\E$/");
		sendRequest(passwd);
		__check_resp__(correct ?
			"/^\\Q+ 100 Logged in. Access granted.\\E$/" :
			"/^\\Q- 500 Authentication failed. Access denied.\\E$/");
	}
	return res;
}

void AdminBaseTestCases::apply()
{
	char cmd[64];
	sprintf(cmd, "apply %s", shit);
	runTestCase("adminConsole.apply", cmd, "/Ok. Changes applied succesfully/");
}

void AdminBaseTestCases::executeTestCases()
{
	RegExp::InitLocale();
	for (int i = 0; i < testCases.size(); i++)
	{
		runTestCase(testCases[i]->id.c_str(), testCases[i]->cmd.c_str(),
            testCases[i]->resp.c_str());
	}
}

void AdminBaseTestCases::login()
{
	__decl_tc__;
	__tc__("adminConsole.login.correct");
	if (login("superadmin", "123", true))
	{
		connected = true;
	}
	else
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

void AdminBaseTestCases::logout()
{
	__decl_tc__;
	__tc__("adminConsole.logout");
	sendRequest("quit");
	if (!checkResponse("/^quit$/"))
	{
		__tc_fail__(1);
	}
	if (!checkResponse("/^Exited from SMSC Remote Console.$/"))
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	connected = false;
}

const char* AdminBaseTestCases::simpleResp(int code, const char* text)
{
	__require__(text);
	static char resp[1024];
	if (fixture->humanConsole)
	{
		sprintf(resp, "/^\\Q%s %s\\E$/", code < 0 ? "Failed:" : "Ok.", text);
	}
	else
	{
		sprintf(resp, "/^\\Q%c %s %s\\E$/", code < 0 ? '-' : '+', abs(code), text);
	}
	return resp;
}

const char* AdminBaseTestCases::respOk(int code)
{
	__require__(code >= 0);
	static char resp[10];
	if (fixture->humanConsole)
	{
		return "Ok.";
	}
	sprintf(resp, "+ %d", code);
	return resp;
}

const char* AdminBaseTestCases::respFail(int code)
{
	__require__(code > 0);
	static char resp[10];
	if (fixture->humanConsole)
	{
		return "Failed:";
	}
	sprintf(resp, "- %d", code);
	return resp;
}

void AdminBaseTestCases::invalidLoginCommands()
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
}

#define __cmd__(id, cmd, resp) \
	runTestCase(id, cmd, resp)
	
void AdminBaseTestCases::invalidCommands()
{
	string symbols = "~!@#$%^&*()_+|{}:\"<>?`-=\[];'.10AZјя";
	char cmd[32];
	char resp[128];
	for (int i = 0; i < symbols.length(); i++)
	{
		const char* s = string(3, symbols[i]).c_str();
		sprintf(resp, "/^\\Q%s\\E unexpected (char: \\Q%c\\E|token: \\Q%s\\E)+$/",
			respFail(600), symbols[i], s);
		//itself
		sprintf(cmd, "%s", s);
		__cmd__("adminConsole.invalidCommands", cmd, resp);
		//add, delete, view, alter, list
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

