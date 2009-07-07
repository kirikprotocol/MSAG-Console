template<class BIND_REQUEST>
void
SessionManagementMessageHandlers::processBindRequest(const BIND_REQUEST& message,
                                                     const io_subsystem::LinkId& src_link_id,
                                                     io_subsystem::IOProcessor& io_processor) {
  io_subsystem::LinkId dstLinkSetId;
  bool isSrcLinkIncoming;
  if ( !io_subsystem::SwitchingTable::getInstance().getSwitching(src_link_id, &dstLinkSetId, &isSrcLinkIncoming) ) {
    std::auto_ptr<BindResponse> bindResp(message.makeResponse(ESME_RBINDFAIL));
    io_processor.getLink(src_link_id)->send(*bindResp);
    io_processor.cleanUpConnection(src_link_id);
  } else {
    if ( !isSrcLinkIncoming )
      throw smsc::util::Exception("SessionManagementMessageHandlers::processBindRequest::: link with id=%s is not link to sme",
                                  src_link_id.toString().c_str());
    SmeRegistry::getInstance().registerSme(new SmeInfo(message.getSystemId(), src_link_id,
                                                       new BIND_REQUEST(message)));
    io_processor.getBinder().bind(dstLinkSetId, message);
  }
}

template<class BIND_RESPONSE>
void
SessionManagementMessageHandlers::processBindResponse(const BIND_RESPONSE& message,
                                                      const io_subsystem::LinkId& src_link_id,
                                                      io_subsystem::IOProcessor& io_processor) {
  if ( message.getCommandStatus() == ESME_ROK ) {
    if ( io_processor.getBinder().commitBindResponse(src_link_id) )
      forwardMessageToSme(message, src_link_id, io_processor);
  } else {
    if ( io_processor.getBinder().processFailedBindResponse(src_link_id) ) {
      io_subsystem::LinkId linkIdToSme = forwardMessageToSme(message, src_link_id, io_processor);
      io_subsystem::SwitchingTable::getInstance().removeSwitching(linkIdToSme);
    }
    io_processor.removeOutcomingLink(src_link_id, true);
  }
}
