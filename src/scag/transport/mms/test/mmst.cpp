#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#include <vector>

#include <readline/readline.h>
#include <readline/history.h>

#include "logger/Logger.h"
#include "util/xml/init.h"
#include "core/buffers/File.hpp"

#include "scag/transport/mms/MmsFactory.h"
#include "ServerMms.hpp"

using smsc::logger::Logger;
using scag::transport::mms::test::ServerMms;
using scag::transport::mms::test::ClientMms;
using smsc::core::buffers::Hash;
using smsc::core::buffers::File;
using smsc::core::buffers::FileException;
using scag::transport::mms::CommandId;
using scag::transport::mms::test::MmsThread;
using scag::transport::mms::trimString;
using std::string;
using std::vector;

using scag::transport::mms::HttpPacket;
 
static const int VASP_DEFAULT_PORT = 47001;
//static const char* DEFAULT_HOST = "127.0.0.1";
static const char* VASP_DEFAULT_HOST = "192.168.1.12";
static const size_t VASP_DEFAULT_HOST_SIZE = std::strlen(VASP_DEFAULT_HOST);

static const int RS_DEFAULT_PORT = 47002;
//static const char* DEFAULT_HOST = "127.0.0.1";
static const char* RS_DEFAULT_HOST = "192.168.1.12";
static const size_t RS_DEFAULT_HOST_SIZE = std::strlen(RS_DEFAULT_HOST);

static const string TEMPLATE_DIR              = "/home/odarchenko/cvs/new/smsc/src/scag/transport/mms/test/templates/";
static const string SUBMIT_TEMPLATE           = TEMPLATE_DIR + "submit.xml";
static const string CANCEL_TEMPLATE           = TEMPLATE_DIR + "cancel.xml";
static const string REPLACE_TEMPLATE          = TEMPLATE_DIR + "replace.xml";
static const string EXTENDED_CANCEL_TEMPLATE  = TEMPLATE_DIR + "extended_cancel.xml";
static const string EXTENDED_REPLACE_TEMPLATE = TEMPLATE_DIR + "extended_replace.xml";
static const string DELIVER_TEMPLATE          = TEMPLATE_DIR + "deliver.xml";
static const string SOAP_ATTACHMENT_TEMPLATE  = TEMPLATE_DIR + "soap_attachment";
static const string FLOOD_ADDRESS             = TEMPLATE_DIR + "flood_address";

static const char* CMD_PROMPT = "Enter Command >";
static const char* FIELD_PROMPT = "Add Field >";
static const char* ATTACH_PROMPT = "Add Attacment >";

ClientMms::transaction_id = 1;

namespace cmd_name {
  static const char* SUBMIT = "submit";
  static const char* CANCEL = "cancel";
  static const char* REPLACE = "replace";
  static const char* EXTCANCEL = "extcancel";
  static const char* EXTREPLACE = "extreplace";
  static const char* FILE_NAME = "file";
  static const char* SEND = "send";
  static const char* QUIT = "quit";
  static const char* HELP = "help";
  static const char* DELIVER = "deliver";
  static const char* RESET = "reset";
  static const char* FLOOD = "flood";
};

namespace cmd_id {
  enum CmdId {
    UNKNOWN,
    SUBMIT,
    CANCEL, 
    REPLACE,
    EXTCANCEL,
    EXTREPLACE,
    FILE_NAME,
    FLOOD,
    SEND,
    HELP,
    QUIT,
    RESET,
    DELIVER
  };
};

string fileToString(const char* file_name) {
  char* buf = 0;
  try {
    File f;
    f.ROpen(file_name);
    size_t size = f.Size();
    buf = new char[size];
    f.Read(buf, size);
    buf[size - 1] = 0;
    string packet(buf);
    delete[] buf;
    return packet;
  } catch (const FileException& fex) {
    __trace2__("%s", fex.what());
    if (buf) {
      delete[] buf;
    }
    return "";
  }
}

uint8_t getCmdId(const string& cmd, string& templ) {
  if (cmd.compare(cmd_name::SUBMIT) == 0) {
    templ = fileToString(SUBMIT_TEMPLATE.c_str());
    return cmd_id::SUBMIT;
  }
  if (cmd.compare(cmd_name::CANCEL) == 0) {
    templ = fileToString(CANCEL_TEMPLATE.c_str());
    return cmd_id::CANCEL;
  }
  if (cmd.compare(cmd_name::REPLACE) == 0) {
    templ = fileToString(REPLACE_TEMPLATE.c_str());
    return cmd_id::REPLACE;
  }
  if (cmd.compare(cmd_name::EXTCANCEL) == 0) {
    templ = fileToString(EXTENDED_CANCEL_TEMPLATE.c_str());
    return cmd_id::EXTCANCEL;
  }
  if (cmd.compare(cmd_name::EXTREPLACE) == 0) {
    templ = fileToString(EXTENDED_REPLACE_TEMPLATE.c_str());
    return cmd_id::EXTREPLACE;
  }
  if (cmd.compare(cmd_name::FILE_NAME) == 0) {
    return cmd_id::FILE_NAME;
  }
  if (cmd.compare(cmd_name::SEND) == 0) {
    return cmd_id::SEND;
  }
  if (cmd.compare(cmd_name::QUIT) == 0) {
    return cmd_id::QUIT;
  }
  if (cmd.compare(cmd_name::RESET) == 0) {
    return cmd_id::RESET;
  }
  if (cmd.compare(cmd_name::HELP) == 0) {
    return cmd_id::HELP;
  }
  if (cmd.compare(cmd_name::FLOOD) == 0) {
    return cmd_id::FLOOD;
  }
  if (cmd.compare(cmd_name::DELIVER) == 0) {
    templ = fileToString(DELIVER_TEMPLATE.c_str());
    return cmd_id::DELIVER;
  }
  return cmd_id::UNKNOWN;
}

void help(const char* prog_name) {
  printf("usage: %s options\n", prog_name);
  printf("options are:\n");
  printf("\t -s [host:[port]] (default == %s:%d)                - start as MMS RS\n", VASP_DEFAULT_HOST, VASP_DEFAULT_PORT);
  printf("\t -c [host:[port]] (default == %s:%d) [VASP_options] - start as VASP\n", VASP_DEFAULT_HOST, VASP_DEFAULT_PORT);
  printf("VASP_options are:\n");
  printf("\t -f file_with_test_packet - send file\n");
  printf("\t -sm                      - send Submit\n");
  printf("\t -cl                      - send Cancel\n");
  printf("\t -rl                      - send Replace\n");
  printf("\t -ecl                     - send ExtendedCancel\n");
  printf("\t -erl                     - send ExtendedReplace\n");
}

void new_help(const char* prog_name) {
  printf("\nusage: %s start_options\n", prog_name);
  printf("start_options are:\n");
  printf("\t -r [options]  Start as MMS RS\n");
  printf("\t -v [options]  Start as VASP\n");
  printf("options are:\n");
  printf("\t -rh  host[:port] (default = %s:%d) MMS RS host:port\n", RS_DEFAULT_HOST, RS_DEFAULT_PORT);
  printf("\t -vh  host[:port] (default = %s:%d) VASP host:port\n\n", VASP_DEFAULT_HOST, VASP_DEFAULT_PORT);
}

std::string getTestPacket(const char* file_name) {
  std::ifstream f(file_name);
  if (!f) {
    __trace2__("Can't open file \"%s\"", file_name);
    return "";
  }
  std::string test_packet;
  getline(f, test_packet, static_cast<char>(EOF));
  return test_packet;
}
/*
void startAsServer(const string& host, int port, string endpoint_id) {
  ServerMms server(host, port, endpoint_id);
  server.start();
}

void startAsClient(const string& host, int port, string endpoint_id, std::string cmd_templ, 
                   std::string test_packet, std::string attach_templ) {
  ClientMms client(host, port, endpoint_id);
  if (!client.start()) {
    return;
  }
  if (!cmd_templ.empty()) {
    client.sendCommand(cmd_templ, attach_templ);
    return;
  }
  client.sendRequestPacket(test_packet.c_str());
}
*/
bool getCommandTemplate(const char* _argv, string& cmd_templ, string& attach_templ) {
  if (!_argv) {
    return false;
  }
  if (std::strcmp(_argv, "-sm") == 0) {
    cmd_templ = fileToString(SUBMIT_TEMPLATE.c_str());
    attach_templ = fileToString(SOAP_ATTACHMENT_TEMPLATE.c_str());
    return true;
  }
  if (std::strcmp(_argv, "-cl") == 0) {
    cmd_templ = fileToString(CANCEL_TEMPLATE.c_str());
    return true;
  }
  if (std::strcmp(_argv, "-rl") == 0) {
    cmd_templ = fileToString(REPLACE_TEMPLATE.c_str());
    return true;
  }
  if (std::strcmp(_argv, "-ecl") == 0) {
    cmd_templ = fileToString(EXTENDED_CANCEL_TEMPLATE.c_str());
    return true;
  }
  if (std::strcmp(_argv, "-erl") == 0) {
    cmd_templ = fileToString(EXTENDED_REPLACE_TEMPLATE.c_str());
    return true;
  }
  return false;
}

void getHostPort(const char* buf, string& host, int& port) {
  int param_len = std::strlen(buf);
  int host_len = strcspn(buf, ":\0");
  host.erase();
  host.append(buf, host_len);
  if (host_len + 1 < param_len) {
    port = atoi(buf + host_len + 1);
  }
}

bool parseCmdLine(string cmd_line, string &cmd, string &value) {
  string line = trimString(cmd_line, " \t");
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
  value = trimString(string(line, sep_pos + 1, string::npos), "\"\t \'");
  return true;
}

enum CmdLineState {
  CMD,
  ATTACH,
  FIELD,
  QUIT
};

string getPromt(uint8_t state) {
  switch (state) {
    case CMD    : return CMD_PROMPT;
    case ATTACH : return ATTACH_PROMPT;
    case FIELD  : return FIELD_PROMPT;
  }
}

struct MmsCmd {
  string soap_envelope;
  string soap_attachment;
  Hash<string> fields;
  void clear() {
    soap_envelope = "";
    soap_attachment = "";
    fields.Empty();
  }
};

void cmdHelp(bool is_vasp) {
   printf("\nfile file_name      - send file\n");
  if (is_vasp) {
    printf("submit   [template] - send MM7 Submit Request\n");
    printf("cancel   [template] - send MM7 Cancel Request\n");
    printf("replace  [template] - send MM7 Replace Request\n");
    printf("ecancel  [template] - send MM7 ExtendedCancel Request\n");
    printf("ereplace [template] - send MM7 ExtendedReplace Request\n");
  } else {
    printf("deliver [template]  - send MM7 Deliver Request\n");
  }
  printf("quit                - exit programm and stop MMS Server\n");
  printf("reset               - return to Enter Command Dialog\n");
  printf("help                - print help\n\n");
}

void fieldHelp() {
  printf("\nfield_name field_value - add Field To MMS Command\n");
  printf("send                   - Send MMS Command\n");
  printf("quit                   - exit programm and stop MMS Server\n");
  printf("reset                  - return to Enter Command Dialog\n");
  printf("help                   - print help\n\n");
}

void attachHelp() {
  printf("\nfile_name - Specifiy file with MMS Content. File should contain MIME message\n");
  printf("send      - Send MMS Command\n");
  printf("quit      - exit programm and stop MMS Server\n");
  printf("reset     - return to Enter Command Dialog\n");
  printf("help      - print help\n\n");
}

vector<string> getFloodAddress(string file_name) {
  string adr_str = fileToString(file_name.c_str());
  vector<string> addresses;
  if (adr_str.empty()) {
    return addresses;
  }
  size_t pos = 0;
  size_t next_pos = 0;
  while (next_pos != string::npos) {
    next_pos = adr_str.find_first_of('\n', pos);
    string adr(adr_str, pos, next_pos - pos); 
    //addresses.push_back(string(adr_str, pos, next_pos - pos));
    __trace2__("Flood Address = %s", adr.c_str());
    addresses.push_back(adr);
    pos = next_pos + 1;
  }
  return addresses;
}

vector<string> getFloodAddress(int mask = 913,int start = 9131000, size_t count = 100) {
  const size_t buf_size = 13;
  char buf[buf_size];
  vector<string> addresses;
  for (int i = 0; i < count; ++i) {
    int next_adr = start + i;
    int n = snprintf(buf, buf_size, "+7%03d%07d", mask, next_adr);
    if (n != 0) {
      addresses.push_back(buf);
    }
  }
  return addresses;
}

uint8_t enterCmd(ClientMms* client, MmsCmd& mms_cmd) {
  mms_cmd.clear();
  char* buf = NULL;
  buf = readline(CMD_PROMPT);
  if (!buf) {
    return QUIT;
  }
  if (!*buf) {
    free(buf);
    return CMD;
  }
  string cmd;
  string value;
  parseCmdLine(buf, cmd, value);
  free(buf);
  if (cmd.empty()) {
    return CMD;
  }
  string templ;
  uint8_t cid = getCmdId(cmd, templ);
  if (cid == cmd_id::QUIT) {
    return QUIT;
  }
  if (cid == cmd_id::RESET) {
    return CMD;
  }
  if (cid == cmd_id::HELP || cid == cmd_id::UNKNOWN || cid == cmd_id::SEND) {
    cmdHelp(client->isVASP());
    return CMD;
  }
  if (cid == cmd_id::FILE_NAME) {
    if (value.empty()) {
      cmdHelp(client->isVASP());
      return CMD;
    }
    string packet = fileToString(value.c_str());
    if (packet.empty()) {
      printf("\nCan't Send File \'%s\'\n\n", value.c_str());
      return CMD;
    }
    if (!client->start()) {
      printf("\nCan't connect to Server\n\n");
      return CMD;
    }
    client->sendRequestWithPrintf(packet.c_str());
/*
    if (client->sendRequestPacket(packet.c_str())) {
      time_t ticks = time(NULL);
      printf("\nSuccess Send      %s\n\n", ctime(&ticks));
    } else {
      time_t ticks = time(NULL);
      printf("\nError Send        %s\n\n", ctime(&ticks));
    }
    */
    return CMD;
  }
  string envelope_templ = fileToString(value.c_str());
  mms_cmd.soap_envelope = envelope_templ.empty() ? templ : envelope_templ;

  if (!client->isVASP()) {
    if (cid != cmd_id::DELIVER) {
      printf("\nMMS RS Can't Send This Command\n\n");
      return CMD;
    }
    return ATTACH;
  }
  if (cid == cmd_id::DELIVER) {
    printf("\nMMS VASP Can't Send This Command\n\n");
    return CMD;
  }
  if (cid == cmd_id::SUBMIT) {
    //string envelope_templ = fileToString(value.c_str());
    //mms_cmd.soap_envelope = envelope_templ.empty() ? templ : envelope_templ;
    return ATTACH;
  }
  //string envelope_templ = fileToString(value.c_str());
  //mms_cmd.soap_envelope = envelope_templ.empty() ? templ : envelope_templ;
  return FIELD;
}

uint8_t addAttach(ClientMms* client, MmsCmd& mms_cmd) {
  char* buf = NULL;
  buf = readline(ATTACH_PROMPT);
  if (!buf) {
    return QUIT;
  }
  if (!*buf) {
    mms_cmd.soap_attachment = fileToString(SOAP_ATTACHMENT_TEMPLATE.c_str());
    free(buf);
    return FIELD;
  }
  string cmd;
  string value;
  parseCmdLine(buf, cmd, value);
  free(buf);
  if (cmd.empty()) {
    mms_cmd.soap_attachment = fileToString(SOAP_ATTACHMENT_TEMPLATE.c_str());
    return FIELD;
  }
  string templ;
  uint8_t cid = getCmdId(cmd,templ);
  if (cid == cmd_id::RESET) {
    return CMD;
  }
  if (cid == cmd_id::QUIT) {
    return QUIT;
  }
  if (cid == cmd_id::SEND) {
    if (!client->start()) {
      printf("\nCan't Connect To Server\n\n");
      return ATTACH;
    }
    if (client->sendCommand(mms_cmd.soap_envelope, mms_cmd.soap_attachment)) {
      printf("\nSuccess Send\n");
    } else {
      printf("\nError Send\n");
    }
    return CMD;
  }
  if (cid == cmd_id::FLOOD) {
    vector<string> flood_values;
    if (value.empty()) {
      flood_values = getFloodAddress(FLOOD_ADDRESS);
    } else {
      flood_values = getFloodAddress(value);
    }
    client->flood(mms_cmd.soap_envelope, mms_cmd.soap_attachment, mms_cmd.fields, flood_values);
    return  CMD;
  }
  if (cid == cmd_id::HELP) {
    attachHelp();
    return ATTACH;
  }
  mms_cmd.soap_attachment = fileToString(cmd.c_str());
  return FIELD;
}

uint8_t addFiled(ClientMms* client, MmsCmd& mms_cmd) {
  char* buf = NULL;
  buf = readline(FIELD_PROMPT);
  if (!buf) {
    return QUIT;
  }
  if (!*buf) {
    free(buf);
    return FIELD;
  }
  string cmd;
  string value;
  parseCmdLine(buf, cmd, value);
  free(buf);
  if (cmd.empty()) {
    fieldHelp();
    return FIELD;
  }
  string templ;
  uint8_t cid = getCmdId(cmd, templ);
  if (cid == cmd_id::QUIT) {
    return QUIT;
  }
  if (cid == cmd_id::RESET) {
    return CMD;
  }
  if (cid == cmd_id::HELP) {
    fieldHelp();
    return FIELD;
  }
  if (cid == cmd_id::FLOOD) {
    vector<string> flood_values;
    if (value.empty()) {
      flood_values = getFloodAddress(FLOOD_ADDRESS);
      //flood_values = getFloodAddress();
    } else {
      flood_values = getFloodAddress(value);
    }
    client->flood(mms_cmd.soap_envelope, mms_cmd.soap_attachment, mms_cmd.fields, flood_values);
    return  CMD;
  }
  if (cid == cmd_id::SEND) {
    if (!client->start()) {
      printf("\nCan't connect to Server\n\n");
    }
    if (client->sendCommand(mms_cmd.soap_envelope, mms_cmd.soap_attachment, mms_cmd.fields)) {
      printf("\nSuccess Send\n");
    } else {
      printf("\nError Send\n");
    }
    return CMD;
  }
  if (!value.empty()) {
    printf("\nname=\'%s\' value=\'%s\'\n\n", cmd.c_str(), value.c_str());
    mms_cmd.fields.Insert(cmd.c_str(), value);
    return FIELD;
  }
  return FIELD;
}

int main(int argc, char* argv[]) {
  if (argc == 1 || (std::strcmp(argv[1],"-r") != 0 && std::strcmp(argv[1],"-v") != 0)) {
    new_help(argv[0]);
    return 0;
  }
  Logger::Init();
  smsc::util::xml::initXerces();
  scag::transport::mms::MmsFactory::initFactories();
  bool is_vasp = true;
  if (std::strcmp(argv[1],"-r") == 0) {
    is_vasp = false;
  }
  int vasp_port = VASP_DEFAULT_PORT;
  std::string vasp_host = VASP_DEFAULT_HOST;
  int rs_port = RS_DEFAULT_PORT;
  std::string rs_host = RS_DEFAULT_HOST;
  string endpoint_id("TestEndpoint");
  if (argc > 3) {
    endpoint_id = argv[2];
    if (std::strcmp(argv[3],"-rh") == 0) {
      getHostPort(argv[4],rs_host, rs_port);
      if (argc > 5 && std::strcmp(argv[5],"-vh") == 0) {
        getHostPort(argv[6], vasp_host, vasp_port);
      }
    }
    if (std::strcmp(argv[3],"-vh") == 0) {
      getHostPort(argv[4],vasp_host, vasp_port);
      if (argc > 5 && std::strcmp(argv[5],"-rh") == 0) {
        getHostPort(argv[6], rs_host, rs_port);
      }
    }
  }
  string client_host = vasp_host;
  int client_port = vasp_port;
  string server_host = rs_host;
  int server_port = rs_port;
  string name = "RS Server";
  if (is_vasp) {
    server_host = vasp_host;
    server_port = vasp_port;
    client_host = rs_host;
    client_port = rs_port;
    name = "VASP Server";
  }
  ServerMms server(server_host, server_port, endpoint_id, is_vasp);
  MmsThread server_thread(&server, name.c_str());
  server_thread.Start();
  ClientMms client(client_host, client_port, endpoint_id, is_vasp);
  uint8_t state = CMD;
  MmsCmd mms_cmd;
  while (state != QUIT) {
    switch (state) {
      case CMD    : state = enterCmd(&client, mms_cmd); break;
      case ATTACH : state = addAttach(&client, mms_cmd); break;
      case FIELD  : state = addFiled(&client, mms_cmd); break;
    }
  }
  server_thread.Kill(9);
  //server.closeSocket();
  return 1;
}
/*
int old_main(int argc, char* argv[]) {
  if (argc == 1) {
    help(argv[0]);
    return 0;
  }
  Logger::Init();
  smsc::util::xml::initXerces();
  int port = VASP_DEFAULT_PORT;
  std::string host = VASP_DEFAULT_HOST;
  if (std::strcmp(argv[1],"-s") == 0) {
    //start as server
    if (argc > 2) {
      getHostPort(argv[2], host, port);
    }
    __trace2__("Server Host = \'%s\' Port = %d", host.c_str(), port);
    startAsServer(host, port);
  }
  if (std::strcmp(argv[1],"-c") == 0) {
    //start as client 
    if (argc < 3) {
      help(argv[0]);
      return 0;
    }
    std::string test_packet;
    string cmd_templ;
    string attach_templ;
    if (getCommandTemplate(argv[2], cmd_templ, attach_templ) || getCommandTemplate(argv[3], cmd_templ, attach_templ)) {
      startAsClient(host, port, cmd_templ, test_packet, attach_templ);
      return 1;
    }
    if ((std::strcmp(argv[2],"-f") == 0) && argc >= 4) {
      test_packet = fileToString(argv[3]);
      startAsClient(host, port, cmd_templ, test_packet, attach_templ);
      return 1;
    } 
    getHostPort(argv[2], host, port);
    if (argc >=4 && std::strcmp(argv[3],"-f") == 0 && argc >= 5) {
      __trace__("TEST!!!");
      test_packet = fileToString(argv[4]);
      startAsClient(host, port, cmd_templ, test_packet, attach_templ);
    }
    __trace2__("Server=%s:%d", host.c_str(), port);
    __trace2__("Test Packet = \'%s\'", test_packet.c_str());
  }
  return 1;
}
*/
