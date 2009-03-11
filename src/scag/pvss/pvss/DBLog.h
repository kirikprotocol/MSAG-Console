#ifndef _SCAG_PVSS_PVSS_DBLOG_H_
#define _SCAG_PVSS_PVSS_DBLOG_H_

#include <string>

namespace scag2 {
namespace pvss {

using std::string;

class DBLog {
public:
  DBLog() {};
  const string& getLogMsg() const;
  void clear();
  void createAddLogMsg(string const& key, string const& msg);
  void createUpdateLogMsg(string const& key, string const& msg);
  void createDelLogMsg(string const& key, string const& msg);
  void createExpireLogMsg(string const& key, string const& msg);
private:
  string logmsg_;
};

}
}

#endif
