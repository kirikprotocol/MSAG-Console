#ifndef TEST_ADMIN_ADMIN_BASE_TEST_CASES
#define TEST_ADMIN_ADMIN_BASE_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "core/network/Socket.hpp"
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace admin {

using std::string;
using std::vector;
using log4cpp::Category;
using smsc::core::network::Socket;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

static const char* shit = "~ !@#$%^&*()-_=+\\|{[}];:'\",<.>?";
static const char* noRights = "Failed: Not enough rights to execute specified command";

#define __m__(mask) v.push_back(mask)

struct Masks
{
	static vector<const char*> correct()
	{
		vector<const char*> v;
		__m__("0"); __m__("+0"); __m__(".0.0.0");
		__m__("12345678901234567890");
		__m__("+12345678901234567890");
		__m__(".0.0.12345678901234567890");
		__m__("123456789012345678??");
		__m__("+123456789012345678??");
		__m__(".0.0.123456789012345678??");
		return v;
	}
	static vector<const char*> invalid()
	{
		vector<const char*> v;
		/*__m__("abc");*/ __m__("+abc"); __m__(".0.0.abc");
		__m__("1.1.1"); __m__("1?2"); __m__("12*");

		__m__("???"); __m__("+???"); __m__(".0.0.???");
		__m__(".0.2.123"); __m__(".7.0.123");
		__m__("123456789012345678901");
		__m__("+123456789012345678901");
		__m__(".0.0.123456789012345678901");
		__m__("1234567890123456789??");
		__m__("+1234567890123456789??");
		__m__(".0.0.1234567890123456789??");
		return v;
	}
};

struct AdminTestCase
{
	const string id;
	const string cmd;
	const string resp;
	AdminTestCase(const char* _id, const char* _cmd, const char* _resp)
	: id(_id), cmd(_cmd), resp(_resp) {}
};

struct AdminFixture
{
	const char* host;
	int port;
	bool humanConsole;
	CheckList* chkList;
};

/**
 * ������� ����� ��� ������������ ���������� admin console.
 * @author bryz
 */
class AdminBaseTestCases : public BaseTestCases
{
public:
	AdminBaseTestCases(AdminFixture* fixture);

	virtual ~AdminBaseTestCases();

	/**
	 * ���������� ���� ���� ������.
	 */
	void executeTestCases();

	void addTestCase(const char* id, const char* cmd, const char* resp);
	void runTestCase(const char* id, const char* cmd, const char* resp);

	void login();
	void logout();
	void apply();

	void invalidLoginCommands();
	void invalidCommands();

protected:
	AdminFixture* fixture;

	virtual Category& getLog();
	void sendRequest(const char* cmd);
	bool checkResponse(const char* pattern);
	bool login(const char* login, const char* passwd, bool correct);

	const char* simpleResp(int code, const char* text);
	const char* respOk(int code);
	const char* respFail(int code);

private:
	Socket socket;
	CheckList* chkList;
	vector<AdminTestCase*> testCases;
	bool connected;
};

}
}
}

#endif /* TEST_ADMIN_ADMIN_BASE_TEST_CASES */

