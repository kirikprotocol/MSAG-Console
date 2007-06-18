#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <logger/Logger.h>
#include <core/network/Socket.hpp>
#include <util/config/Manager.h>
#include <util/comm_comp/PacketFactory.hpp>
#include <util/comm_comp/WorkersPool.hpp>

#include <mmbox/app_protocol/messages/AppRequest_QueryMmsRecordsByAbonentAddress.hpp>
#include <mmbox/app_protocol/messages/AppRequest_InsertMmsRecord.hpp>
#include <mmbox/app_packets_handler/bdb_objects_transmitter.hpp>
#include <mmbox/app_packets_handler/ApplicationRequestsHandler.hpp>
#include <mmbox/app_specific_db/MmsDB.hpp>

mmbox::app_specific_db::MmsDB* mmsDbConnet;

static void blockSignals()
{
  sigset_t blocked_signals;
  sigfillset(&blocked_signals);
  sigdelset(&blocked_signals, SIGKILL);
  sigdelset(&blocked_signals, SIGALRM);
  sigdelset(&blocked_signals, SIGSEGV); sigdelset(&blocked_signals, SIGBUS);
  sigdelset(&blocked_signals, SIGFPE); sigdelset(&blocked_signals, SIGILL);

  sigprocmask(SIG_SETMASK, &blocked_signals, NULL);
}

extern "C"
void sig_handler(int signo)
{
  return;
}

static void unblockSignals()
{
  sigset_t unblocked_signals;
  sigemptyset(&unblocked_signals);

  sigprocmask(SIG_SETMASK, &unblocked_signals, NULL);
}

static void usage(const char* prog_name)
{
  fprintf(stderr,"Usage: %s [-c config_name] [-v]\n", prog_name);
}

static void version(const char* prog_name)
{
  fprintf(stderr,"%s: version=$Revision$ (target=$Name$)\n", prog_name);
}

smsc::util::comm_comp::TEvent<mmbox::app_protocol::AppRequest_InsertMmsRecord>::HandlerList
smsc::util::comm_comp::TEvent<mmbox::app_protocol::AppRequest_InsertMmsRecord>::registry;

smsc::util::comm_comp::TEvent<mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddress>::HandlerList
smsc::util::comm_comp::TEvent<mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddress>::registry;

smsc::util::comm_comp::TEvent<mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddressAndStatus>::HandlerList
smsc::util::comm_comp::TEvent<mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddressAndStatus>::registry;

int main(int argc, char** argv)
{
  try {
    {mmbox::app_protocol::AppRequest_InsertMmsRecord obj;}
    {mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddress obj;}

    smsc::logger::Logger::Init();
    smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");

    const char* db_config_file;

    const char* prog_name = strrchr(argv[0], '/');
    if (!prog_name) prog_name = argv[0];
    else ++prog_name;

    if ( argc > 1 ) {
      if ( !strcmp(argv[1], "-v") ) {
        version(prog_name); return 0;
      } else if ( !strcmp(argv[1], "-c") ) {
        if ( argc > 2 )
          db_config_file = argv[2];
        else {
          usage(prog_name); return 1;
        }
      } else {
        usage(prog_name); return 1;
      }
    } else {
      usage(prog_name); return 1;
    }
    smsc::util::config::Manager::init(db_config_file);

    mmsDbConnet = new mmbox::app_specific_db::MmsDB("mmsDb");
    mmsDbConnet->open();

    sigset(SIGINT, sig_handler);
    sigset(SIGTERM, sig_handler);
    sigset(SIGHUP, SIG_IGN);
    blockSignals();

    mmbox::app_pck_handler::bdb_objects_transmitter_t& ioComponent = mmbox::app_pck_handler::bdb_objects_transmitter_t::getInstance() ;
    ioComponent.initialize(logger, smsc::util::config::Manager::getInstance());

    ioComponent.Start();
    smsc_log_info(logger, "main::: ioComponent has been started");
    mmbox::app_pck_handler::ApplicationRequestsHandler requestHanlder;

    smsc::util::comm_comp::WorkersPool workersPool;
    workersPool.start();
    smsc_log_info(logger, "main::: WorkesPool has been started");

    unblockSignals();

    pause();
    smsc_log_info(logger, "main::: pause returned");
    mmsDbConnet->close();

    ioComponent.shutdown();
    ioComponent.WaitFor();
    smsc_log_info(logger, "main::: ioComponent has been terminated");

    workersPool.finish();
    smsc_log_info(logger, "main::: WorkersPool has been terminated");

  } catch (const std::exception& ex) {
    std::cerr << "main:: catch unexpected exception [" << ex.what()
              << "]" << std::endl;
    return 0;
  }

  return 0;
}
