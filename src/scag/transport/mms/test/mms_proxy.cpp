#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "logger/Logger.h"
#include "sms/sms.h"
#include "util/xml/init.h"
#include "core/buffers/File.hpp"

#include "ProxyMms.hpp"

using scag::transport::mms::test::ProxyMms;
using scag::transport::mms::test::ServerMms;
using scag::transport::mms::test::MmsThread;
using smsc::logger::Logger;
using std::string;

scag::transport::mms::test::ClientMms::transaction_id = 1;

bool parseCmdLine(string cmd_line, string &cmd, string &value) {
  string line = scag::transport::mms::trimString(cmd_line, " \t");
  if (line.empty()) {
    return false;
  }
  cmd.erase();
  value.erase();
  size_t sep_pos = line.find_first_of(' ');
  cmd.append(line, 0, sep_pos);
  if (sep_pos == string::npos) {
    return true;
  }
  value = scag::transport::mms::trimString(string(line, sep_pos + 1, string::npos), "\"\t \'");
  return true;
}

bool quit() {
  char* buf = NULL;
  buf = readline("quit >");
  if (!buf) {
    return true;
  }
  if (!*buf) {
    free(buf);
    return false;
  }
  string cmd;
  string value;
  parseCmdLine(buf, cmd, value);
  if (cmd.compare("quit") == 0) {
    return true;
  }
  return false;
}

int main(int argc, const char* argv[])
{
  Logger::Init();
  smsc::util::xml::initXerces();
  string proxy_host = "phoenix";
  int proxy_vasp_port = 47003;
  int proxy_rs_port = 47004;
  string vasp_host = "phoenix";
  int vasp_port = 47001; //VASP_DEFAULT_PORT mmst.cpp
  string rs_host = "phoenix";
  int rs_port = 47002; //RS_DEFAULT_PORT mmst.cpp
  ProxyMms vasp_proxy(proxy_host, proxy_vasp_port, vasp_host, vasp_port, true, "MSAG_VASP");
  ProxyMms rs_proxy(proxy_host, proxy_rs_port, rs_host, rs_port, false, "MSAG_RS");
  MmsThread vasp_thread(&vasp_proxy, "VASP Proxy");
  vasp_thread.Start();
  MmsThread rs_thread(&rs_proxy, "RS Proxy");
  rs_thread.Start();
  while(!quit());
  rs_thread.Kill(9);
  vasp_thread.Kill(9);
  return 1;
}
