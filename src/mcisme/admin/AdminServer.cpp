#include <assert.h>
#include "AdminServer.hpp"
#include "mcisme/ProfilesStorage.hpp"

namespace smsc {
namespace mcisme {
namespace admin {

eyeline::protogen::ProtocolSocketBase*
AdminServer::onConnect(core::network::Socket* clnt,int conn_id)
{
  return new eyeline::protogen::ProtocolSocketBase(clnt,conn_id);
}

void
AdminServer::onHandleCommand(eyeline::protogen::ProtocolSocketBase::Packet& pkt)
{
  _proto.decodeAndHandleMessage(pkt.data, pkt.dataSize, pkt.connId);
}

void
AdminServer::onDisconnect(eyeline::protogen::ProtocolSocketBase* sck)
{
  delete sck;
}

void
AdminServer::handle(const messages::FlushStats& msg)
{
  smsc_log_info(_logger, "Got FlushStats message '%s'", msg.toString().c_str());
  messages::FlushStatsResp resp;

  resp.messageSetSeqNum(msg.messageGetSeqNum());
  resp.setStatus(messages::Status(messages::Status::OK));

  _processor.flushStatistics();

  enqueueCommand(msg.messageGetConnId(), resp, _proto, false);
}

void
AdminServer::handle(const messages::GetStats& msg)
{
  smsc_log_info(_logger, "Got GetStats message '%s'", msg.toString().c_str());
  messages::GetStatsResp resp;

  resp.messageSetSeqNum(msg.messageGetSeqNum());
  resp.setStatus(messages::Status(messages::Status::OK));

  EventsStat stat = _processor.getStatistics();

  resp.setMissed(stat.missed);
  resp.setDelivered(stat.delivered);
  resp.setFailed(stat.failed);
  resp.setNotified(stat.notified);

  enqueueCommand(msg.messageGetConnId(), resp, _proto, false);
}

void
AdminServer::handle(const messages::GetRunStats& msg)
{
  smsc_log_info(_logger, "Got GetRunStats message '%s'", msg.toString().c_str());
}

void
AdminServer::handle(const messages::GetSched& msg)
{
  smsc_log_info(_logger, "Got GetSched message '%s'", msg.toString().c_str());

  messages::GetSchedResp resp;

  resp.messageSetSeqNum(msg.messageGetSeqNum());

  try {
    int64_t schedTime;
    uint8_t eventsCount;
    int32_t lastError;

    if (_processor.getSchedItem(msg.getSubscriber(), &schedTime, &eventsCount, &lastError))
    {
      resp.setStatus(messages::Status(messages::Status::OK));
      resp.setTime(schedTime);
      resp.setSubscriber(msg.getSubscriber());
      resp.setEventsCount(eventsCount);
      resp.setLastError(lastError);
    } else
      resp.setStatus(messages::Status(messages::Status::SubscriberNotFound));

  } catch (std::exception& ex) {
    smsc_log_error(_logger, "Caught exception '%s'", ex.what());
    resp.setStatus(messages::Status(messages::Status::SystemError));
    resp.setMsg(ex.what());
  }
  enqueueCommand(msg.messageGetConnId(), resp, _proto, false);
}

void
AdminServer::handle(const messages::GetSchedList& msg)
{
  smsc_log_info(_logger, "Got GetSchedList message '%s'", msg.toString().c_str());

  messages::GetSchedListResp resp;

  resp.messageSetSeqNum(msg.messageGetSeqNum());

  std::vector<TaskProcessor::TimeInfo> timeValues;
  std::vector<messages::TimeLineInfo> timeLines;
  if (_processor.getSchedItems(&timeValues))
  {
    std::vector<TaskProcessor::TimeInfo>::size_type count = timeValues.size();

    for(unsigned i = 0; i < count; ++i)
    {
      messages::TimeLineInfo timeLineInfo;
      timeLineInfo.setCount(timeValues[i].count);
      timeLineInfo.setTime(timeValues[i].timeValue);
      timeLines.push_back(timeLineInfo);
    }
    resp.setTimeLines(timeLines);
    resp.setStatus(messages::Status(messages::Status::OK));
  } else
    resp.setStatus(messages::Status(messages::Status::SubscriberNotFound));

  enqueueCommand(msg.messageGetConnId(), resp, _proto, false);
}

void
AdminServer::handle(const messages::GetProfile& msg)
{
  smsc_log_info(_logger, "Got GetProfile message '%s'", msg.toString().c_str());

  ProfilesStorage* profStorage = ProfilesStorage::GetInstance();

  assert(profStorage);

  messages::GetProfileResp resp;
  resp.messageSetSeqNum(msg.messageGetSeqNum());

  try {
    const std::string& subscriber = msg.getSubscriber();
    AbntAddr  abnt(subscriber.c_str());
    AbonentProfile prof;

    if (profStorage->Get(abnt, prof))
    {
      resp.setStatus(messages::Status(messages::Status::OK));

      resp.setEventMask(prof.eventMask);
      resp.setInform(prof.inform);
      resp.setNotify(prof.notify);
      resp.setWantNotifyMe(prof.wantNotifyMe);
      resp.setInformTemplateId(prof.informTemplateId);
      resp.setNotifyTemplateId(prof.notifyTemplateId);
    } else
      resp.setStatus(messages::Status(messages::Status::SubscriberNotFound));

  } catch (std::exception& ex) {
    resp.setStatus(messages::Status(messages::Status::SystemError));
    resp.setMsg(ex.what());
  }

  enqueueCommand(msg.messageGetConnId(), resp, _proto, false);
}

void
AdminServer::handle(const messages::SetProfile& msg)
{
  smsc_log_info(_logger, "Got SetProfile message '%s'", msg.toString().c_str());

  ProfilesStorage* profStorage = ProfilesStorage::GetInstance();

  assert(profStorage);

  messages::SetProfileResp resp;
  resp.messageSetSeqNum(msg.messageGetSeqNum());

  try {
    const std::string& subscriber = msg.getSubscriber();
    AbntAddr  abnt(subscriber.c_str());
    AbonentProfile prof;

    if (profStorage->Get(abnt, prof))
    {
      resp.setStatus(messages::Status(messages::Status::OK));

      if (msg.hasEventMask())
        prof.eventMask = msg.getEventMask();
      if (msg.hasInform())
        prof.inform = msg.getInform();
      if (msg.hasNotify())
        prof.notify = msg.getNotify();
      if (msg.hasWantNotifyMe())
        prof.wantNotifyMe = msg.getWantNotifyMe();
      if (msg.hasInformTemplateId())
        prof.informTemplateId = msg.getInformTemplateId();
      if (msg.hasNotifyTemplateId())
        prof.notifyTemplateId = msg.getNotifyTemplateId();

      profStorage->Set(abnt, prof);
    } else
      resp.setStatus(messages::Status(messages::Status::SubscriberNotFound));

  } catch (std::exception& ex) {
    resp.setStatus(messages::Status(messages::Status::SystemError));
    resp.setMsg(ex.what());
  }

  enqueueCommand(msg.messageGetConnId(), resp, _proto, false);
}

}}}
