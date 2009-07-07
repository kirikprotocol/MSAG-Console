template<class BIND_REQUEST>
void
SetOfNotBindedConnections::bindConnections(const BIND_REQUEST& bind_request)
{
  for(not_binded_conns_t::iterator iter = _notBindedConnections.begin(), end_iter = _notBindedConnections.end();
      iter != end_iter; ++iter) {
    Link* linkToSmsc = iter->second;
    smsc_log_debug(_logger, "SetOfNotBindedConnections::bindConnections::: send bindRequest='%s' over link with id='%s'",
                   bind_request.toString().c_str(), linkToSmsc->getLinkId().toString().c_str());
    linkToSmsc->send(bind_request);

    TimeoutEvent* bindRespWaitTimoutEvent =
      _ioProcessor.createBindResponseWaitTimeoutEvent(linkToSmsc->getLinkId(),
                                                      bind_request.makeResponse(BIND_REQUEST::BIND_FAILED));
    TimeoutMonitor::timeout_id_t timeoutId = "BND:" + linkToSmsc->getLinkId().toString();
    TimeoutMonitor::getInstance().scheduleTimeout(timeoutId,
                                                  linkToSmsc->getBindRespWaitTimeout(),
                                                  bindRespWaitTimoutEvent);
  }
}
