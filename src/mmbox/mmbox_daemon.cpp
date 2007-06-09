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

void blockSignals()
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

void unblockSignals()
{
  sigset_t unblocked_signals;
  sigemptyset(&unblocked_signals);

  sigprocmask(SIG_SETMASK, &unblocked_signals, NULL);
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

    const char* db_config_file = "bdb_server.cfg";
    if ( argc > 1 ) 
      db_config_file = argv[1];
    smsc::util::config::Manager::init(db_config_file);

    mmsDbConnet = new mmbox::app_specific_db::MmsDB("mmsDb");
    mmsDbConnet->open();

    sigset(SIGINT, sig_handler);
    sigset(SIGTERM, sig_handler);
    sigset(SIGHUP, sig_handler);
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
