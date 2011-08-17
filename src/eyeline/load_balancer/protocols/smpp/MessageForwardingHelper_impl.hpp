template <class MESSAGE>
void
MessageForwardingHelper::forwardMessageToSmsc(const MESSAGE& message,
                                              const io_subsystem::LinkId& src_link_id,
                                              io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::SwitchingTable& switchingTable = io_subsystem::SwitchingTable::getInstance();
  io_subsystem::LinkId dstLinkSetId;
  bool isSrcLinkIncoming;
  if ( !switchingTable.getSwitching(src_link_id, &dstLinkSetId, &isSrcLinkIncoming) )
    io_processor.cleanUpConnection(src_link_id);
  else {
    if ( !isSrcLinkIncoming )
      throw smsc::util::Exception("MessageForwardingHelper::forwardMessageToSmsc::: link with id=%s is not link to sme",
                                  src_link_id.toString().c_str());
    smsc_log_info(_log, "MessageForwardingHelper::forwardMessageToSmsc::: forward message=[%s] gotten from link='%s' to linkset='%s'",
                  message.toString().c_str(), src_link_id.toString().c_str(), dstLinkSetId.toString().c_str());

    io_subsystem::LinkSetRefPtr dstLinkSet = io_processor.getLinkSet(dstLinkSetId);
    if ( dstLinkSet.Get() )
      dstLinkSet->send(message);
    else
      throw smsc::util::Exception("MessageForwardingHelper::forwardMessageToSmsc::: there isn't link for id=%s",
                                  dstLinkSetId.toString().c_str());

  }
}

template <class MESSAGE>
void
MessageForwardingHelper::forwardMultipartMessageToSmsc(const MESSAGE& message,
                                                       const io_subsystem::LinkId& src_link_id,
                                                       io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::SwitchingTable& switchingTable = io_subsystem::SwitchingTable::getInstance();
  io_subsystem::LinkId dstLinkSetId;
  bool isSrcLinkIncoming;
  if ( !switchingTable.getSwitching(src_link_id, &dstLinkSetId, &isSrcLinkIncoming) )
    io_processor.cleanUpConnection(src_link_id);
  else {
    if ( !isSrcLinkIncoming )
      throw smsc::util::Exception("MessageForwardingHelper::forwardMultipartMessageToSmsc::: link with id=%s is not link to sme",
                                  src_link_id.toString().c_str());

    if (message.getSarSegmentSeqNum() == 1 ) {
      smsc_log_info(_log, "MessageForwardingHelper::forwardMultipartMessageToSmsc::: forward first part of multipart message=[%s] gotten from link='%s' to linkset='%s'",
                    message.toString().c_str(), src_link_id.toString().c_str(), dstLinkSetId.toString().c_str());
      io_subsystem::LinkSetRefPtr dstLinkSet = io_processor.getLinkSet(dstLinkSetId);
      if ( dstLinkSet.Get() ) {
        io_subsystem::LinkId idOfUsedLink = dstLinkSet->send(message);
        switchingTable.setSpecificSwitching(dstLinkSet->getLinkId(),
                                            idOfUsedLink,
                                            message.getSarMsgRefNum());
      } else
        throw smsc::util::Exception("MessageForwardingHelper::forwardMultipartMessageToSmsc::: there isn't linkset for id=%s",
                                    dstLinkSetId.toString().c_str());
    } else if ( message.getSarSegmentSeqNum() > message.getSarTotalSegments() ) {
      throw InvalidMessageParamException("MessageForwardingHelper::forwardMultipartMessageToSmsc::: invalid message data: sar_segment_seq_num(=%d) > sar_total_tegments(=%d)",
                                  message.getSarSegmentSeqNum(), message.getSarTotalSegments());
    } else {
      io_subsystem::LinkId idOfUsedLink =
        switchingTable.getSpecificSwitching(dstLinkSetId,
                                            message.getSarMsgRefNum());
      smsc_log_info(_log, "MessageForwardingHelper::forwardMultipartMessageToSmsc::: forward next part of multipart message=[%s] gotten from link='%s' to link='%s'",
                    message.toString().c_str(), src_link_id.toString().c_str(), idOfUsedLink.toString().c_str());

      io_subsystem::LinkRefPtr dstLink = io_processor.getLink(idOfUsedLink);
      if ( dstLink.Get() )
        dstLink->send(message);
      else
        throw InvalidMessageParamException("MessageForwardingHelper::forwardMultipartMessageToSmsc::: there isn't link for id=%s, sar_msg_ref_num parameter value='%d' may be invalid)",
                                                       idOfUsedLink.toString().c_str(), message.getSarMsgRefNum());

      if ( message.getSarSegmentSeqNum() == message.getSarTotalSegments() )
        switchingTable.removeSpecificSwitching(dstLinkSetId,
                                               message.getSarMsgRefNum());
    }
  }
}

template <class MESSAGE>
io_subsystem::LinkId
MessageForwardingHelper::forwardMessageToSme(const MESSAGE& message,
                                             const io_subsystem::LinkId& src_link_id,
                                             io_subsystem::IOProcessor& io_processor)
{
  io_subsystem::LinkId linkSetId, emptyLinkId;
  linkSetId = io_processor.getLinkSetIdOwnerForThisLink(src_link_id);
  if ( linkSetId == emptyLinkId )
    throw smsc::util::Exception("MessageForwardingHelper::forwardMessageToSme::: source link with id=%s is link to sme",
                                src_link_id.toString().c_str());
  io_subsystem::LinkId linkIdToSme;
  bool isSrcLinkIncoming;
  if ( !io_subsystem::SwitchingTable::getInstance().getSwitching(linkSetId, &linkIdToSme, &isSrcLinkIncoming) ) {
    io_processor.cleanUpConnection(src_link_id); //???
    throw smsc::util::Exception("MessageForwardingHelper::forwardMessageToSme::: can't find route linkSet-->linkToSme for LinkSetId=%s",
                                linkSetId.toString().c_str());
  } else {
    if ( isSrcLinkIncoming )
      throw smsc::util::Exception("MessageForwardingHelper::forwardMessageToSme::: wrong entry in switching table (source linkSetId=%s)- invalid configuration",
                                  linkSetId.toString().c_str());
    smsc_log_info(_log, "MessageForwardingHelper::forwardMessageToSme::: forward message=[%s] gotten from link='%s' to sme over link='%s'",
                  message.toString().c_str(), src_link_id.toString().c_str(), linkIdToSme.toString().c_str());

    io_subsystem::LinkRefPtr linkToSme = io_processor.getLink(linkIdToSme);
    if ( linkToSme.Get() )
      return linkToSme->send(message);
    else
      throw smsc::util::Exception("MessageForwardingHelper::forwardMessageToSme::: there isn't link for id=%s",
                                  linkIdToSme.toString().c_str());
  }
  return io_subsystem::LinkId();//make compiler happy
}

template <class MESSAGE>
void
MessageForwardingHelper::forwardSmppResponseToSmsc(const MESSAGE& message,
                                                   const io_subsystem::LinkId& src_link_id,
                                                   io_subsystem::IOProcessor& io_processor)
{
  smsc_log_info(_log, "MessageForwardingHelper::forwardSmppResponseToSmsc::: forward message=[%s] gotten from link='%s'",
                message.toString().c_str(), src_link_id.toString().c_str());
  io_subsystem::LinkId dstLinkSetId;
  bool isSrcLinkIncoming;
  if ( !io_subsystem::SwitchingTable::getInstance().getSwitching(src_link_id, &dstLinkSetId, &isSrcLinkIncoming) )
    io_processor.cleanUpConnection(src_link_id);
  else {
    if ( !isSrcLinkIncoming )
      throw smsc::util::Exception("MessageForwardingHelper::forwardSmppResponseToSmsc::: link with id=%s is not link to sme",
                                  src_link_id.toString().c_str());
    io_subsystem::LinkSetRefPtr dstLinkSet = io_processor.getLinkSet(dstLinkSetId);
    if ( dstLinkSet.Get() ) {
      unsigned linkNumInLinkSet = message.getSequenceNumber() % dstLinkSet->getMaxNumberOfLinks();
      dstLinkSet->send(message, linkNumInLinkSet);
    } else
      throw smsc::util::Exception("MessageForwardingHelper::forwardSmppResponseToSmsc::: there isn't linkset for id=%s",
                                  dstLinkSetId.toString().c_str());
  }
}

template <class MESSAGE>
void
MessageForwardingHelper::forwardServiceRequestToConcreteSmsc(const MESSAGE& message,
                                                             const io_subsystem::LinkId& src_link_id,
                                                             io_subsystem::IOProcessor& io_processor)
{
  smsc_log_info(_log, "MessageForwardingHelper::forwardServiceRequestToConcreteSmsc::: forward message=[%s] gotten from link='%s'",
                message.toString().c_str(), src_link_id.toString().c_str());
  io_subsystem::LinkId dstLinkSetId;
  bool isSrcLinkIncoming;
  if ( !io_subsystem::SwitchingTable::getInstance().getSwitching(src_link_id, &dstLinkSetId, &isSrcLinkIncoming) )
    io_processor.cleanUpConnection(src_link_id);
  else {
    if ( !isSrcLinkIncoming )
      throw smsc::util::Exception("MessageForwardingHelper::forwardServiceRequestToConcreteSmsc::: link with id=%s is not link to sme",
                                  src_link_id.toString().c_str());
    io_subsystem::LinkSetRefPtr dstLinkSet = io_processor.getLinkSet(dstLinkSetId);
    if ( dstLinkSet.Get() ) {
      unsigned linkNumInLinkSet = static_cast<unsigned>(message.getMessageId() % dstLinkSet->getMaxNumberOfLinks());
      dstLinkSet->send(message, linkNumInLinkSet);
    } else
      throw smsc::util::Exception("MessageForwardingHelper::forwardServiceRequestToConcreteSmsc::: there isn't linkset for id=%s",
                                  dstLinkSetId.toString().c_str());
  }
}
