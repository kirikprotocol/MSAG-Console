#include "DBLog.h"

namespace scag2 {
namespace pvss {

static const string PROPERTY_LOG = " property=";
static const string NAME_LOG     = " name=";
static const string UPDATE_LOG   = "U key=";
static const string ADD_LOG      = "A key=";
static const string DEL_LOG      = "D key=";
static const string EXPIRE_LOG   = "e key=";

const string& DBLog::getLogMsg() const {
  return logmsg_;
}

void DBLog::clear() {
  string().swap(logmsg_);
}

void DBLog::createAddLogMsg(string const& key, string const& msg) {
  logmsg_ = ADD_LOG;
  logmsg_.append(key);
  logmsg_.append(PROPERTY_LOG);
  logmsg_.append(msg);
}

void DBLog::createUpdateLogMsg(string const& key, string const& msg) {
  logmsg_ = UPDATE_LOG;
  logmsg_.append(key);
  logmsg_.append(PROPERTY_LOG);
  logmsg_.append(msg);
}

void DBLog::createDelLogMsg(string const& key, string const& msg) {
  logmsg_ = DEL_LOG;
  logmsg_.append(key);
  logmsg_.append(NAME_LOG);
  logmsg_.append(msg);
}

void DBLog::createExpireLogMsg(string const& key, string const& msg) {
  logmsg_ = EXPIRE_LOG;
  logmsg_.append(key);
  logmsg_.append(PROPERTY_LOG);
  logmsg_.append(msg);
}

}
}


