#include "WorkersPool.hpp"
#include "SingleSharedQueue.hpp"
#include "ReqAppPacket_Subscriber.hpp"
#include "TEvent.hpp"
//#include "ApplicationPackets.hpp"

namespace smsc {
namespace util {
namespace comm_comp {

WorkersPool::WorkersPool(int workersNum)
  : _workersNum(workersNum), _wasFinished(false) { _workers.setMaxThreads(workersNum); }

WorkersPool::~WorkersPool() {
  try {
    finish();
  } catch (...) {}
}

void
WorkersPool::start()
{
  for (int i=0; i<_workersNum; ++i)
    _workers.startTask(new Worker());
}

void
WorkersPool::finish()
{
  if ( !_wasFinished ) {
    _workers.shutdown();
    _wasFinished = true;
  }
}

int
Worker::Execute()
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
  while (!isStopping) {
    // get object from queue
    AcceptedObjInfo<AbstractEvent> newObjInfo = SingleSharedQueue<AcceptedObjInfo<AbstractEvent> >::getInstance().front();

    if ( isStopping ) {
      smsc_log_info(logger, "Worker::Execute::: exit thread");
      break;
    }
    if ( !newObjInfo.reqAppObj ) continue;
    std::auto_ptr<const AbstractEvent> ptrGuard(newObjInfo.reqAppObj);
    smsc_log_debug(logger, "Worker::Execute::: got object from queue. Process object");
    newObjInfo.reqAppObj->Notify(newObjInfo.connectId);
  }
  return 0;
}

}}}
