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
	int timeout = 5;
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

void AdminBaseTestCases::addTestCase(const char* id, const char* cmd,
	const char* humanResp /*, const char* scriptResp*/)
{
	const char* scriptResp = "";
	__require__(id && cmd && humanResp && scriptResp);
	testCases.push_back(new AdminTestCase(id, cmd, humanResp, scriptResp));
}

void AdminBaseTestCases::loginCommands()
{
	__decl_tc__;
	__tc__("adminConsole.login.incorrect");
	const string resp1 = login("superadmin", "aaa");
	if (resp1 != "Authentication failed. Access denied.")
	{
		__tc_fail__(1);
	}
	const string resp2 = login("aaa", "aaa");
	if (resp2 != "Authentication failed. Access denied.")
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	__tc__("adminConsole.login.correct");
	const string resp = login("superadmin", "123");
	if (resp != "")
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

void AdminBaseTestCases::invalidCommands()
{
	string symbols = "~!@#$%^&*()_+|{}:\"<>?`1234567890-=\[];',./" \
		"1234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
		"юабцде╗фгхийклмнопярстужвьызшэщчъ";
	char cmd[16];
	char resp[128];
	for (int i = 0; i < symbols.length(); i++)
	{
		const char* s = string(3, symbols[i]).c_str();
		//itself
		sprintf(cmd, "%s", s);
		sprintf(resp, "Failed: unexpected char: %c", symbols[i]);
		addTestCase("adminConsole.invalidCommands", cmd, resp);
		//add, delete, view, alter, list
		sprintf(resp, "Failed: unexpected token: %s", s);
		sprintf(cmd, "add %s", s);
		addTestCase("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "delete %s", s);
		addTestCase("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "view %s", s);
		addTestCase("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "alter %s", s);
		addTestCase("adminConsole.invalidCommands", cmd, resp);
		sprintf(cmd, "list %s", s);
		addTestCase("adminConsole.invalidCommands", cmd, resp);
	}
}

void AdminBaseTestCases::sendRequest(const string& cmd)
{
	int count = 0;
	string tmp = cmd + "\n";
	while (count < tmp.length())
	{
		int wr = socket.Write(tmp.c_str() + count, tmp.length() - count);
		//__trace2__("socket write: wr = %d", wr);
		if (wr <= 0)
		{
			__trace2__("socket write error: wr = %d, reason = %s", wr, strerror(errno));
			throw Exception("Failed to send data");
		}
		count += wr;
	}
}

struct Buffer
{
	char* buffer;
	int size;
	int offset;

	Buffer(int sz = 0) : buffer(NULL), size(0), offset(0) { if (sz) setSize(sz); }

	~Buffer() { if (buffer) delete [] buffer; }

	void setSize(int newSize)
	{
		if (newSize < size) return;
		char* newBuf = new char[newSize];
		if (offset) memcpy(newBuf, buffer, offset);
		if (buffer) delete [] buffer;
		buffer = newBuf;
		size = newSize;
	}
	char* current(){ return buffer + offset; }
	int freeSpace() { return size - offset; }
};

const string AdminBaseTestCases::getResponse()
{
	Buffer buf(64);
	while (socket.canRead())
	{
		if (buf.freeSpace() <= 0)
		{
			buf.setSize(buf.size * 2);
		}
		int rd = socket.Read(buf.current(), buf.freeSpace());
		//__trace2__("socket read: rd = %d", rd);
		if (rd <= 0)
		{
			__trace2__("socket read error: rd = %d, reason = %s", rd, strerror(errno));
			throw Exception("Failed to receive data");
		}
		buf.offset += rd;
	}
	return buf.buffer ? buf.buffer : "";
}

const string AdminBaseTestCases::login(const string& login, const string& passwd)
{
	const string loginPrompt = getResponse();
	//__require__(loginPrompt == "Login: ");
	__trace2__("loginPrompt = %s", loginPrompt.c_str());
	sendRequest(login);
	const string passwdPrompt = getResponse();
	//__require__(passwdPrompt == "Password: ");
	__trace2__("passwdPrompt = %s", passwdPrompt.c_str());
	sendRequest(passwd);
	const string resp = getResponse();
	__trace2__("response = ", resp.c_str());
	return resp;
}

void AdminBaseTestCases::executeTestCases()
{
	RegExp::InitLocale();
	for (int i = 0; i < testCases.size(); i++)
	{
		bool isOk = true;
		TestCase* tc = chkList->getTc(testCases[i]->id.c_str());
		sendRequest(testCases[i]->cmd);
		const string resp = getResponse();
		const string& expectedResp = humanConsole ? testCases[i]->humanResp :
			testCases[i]->scriptResp;
		__trace2__("command: %s\nresponse: %s\nexpected: %s",
			testCases[i]->cmd.c_str(), resp.c_str(), expectedResp.c_str());
        RegExp chkResp;
		chkResp.Compile(expectedResp.c_str());
		Hash<SMatch> h;
		SMatch match;
		int matchcount = 1;
		if(!chkResp.Match(resp.c_str(), &match, matchcount))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
}

}
}
}

