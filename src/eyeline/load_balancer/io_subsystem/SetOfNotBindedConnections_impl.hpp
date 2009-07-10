template<class BIND_REQUEST>
void
SetOfNotBindedConnections::bindConnections(const BIND_REQUEST& bind_request)
{
  for(not_binded_conns_t::iterator iter = _notBindedConnections.begin(), end_iter = _notBindedConnections.end();
      iter != end_iter;) {
    LinkRefPtr linkToSmsc = _ioProcessor.getLink(*iter);
    if ( !linkToSmsc.Get()) {
      smsc_log_error(_logger, "SetOfNotBindedConnections::bindConnections::: there isn't link for id='%s'",
                     iter->toString().c_str());
      _notBindedConnections.erase(iter++);
    } else {
      ++iter;
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
}
