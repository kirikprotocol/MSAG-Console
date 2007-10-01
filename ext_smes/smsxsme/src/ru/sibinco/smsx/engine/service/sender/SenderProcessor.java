package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.utils.ds.DataSourceException;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderDataSource;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderMessage;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;

/**
 * User: artem
 * Date: 06.07.2007
 */

class SenderProcessor implements SenderGetMessageStatusCmd.Receiver, SenderSendMessageCmd.Receiver {

  private static final Category log = Category.getInstance("SENDER");

  private final SenderDataSource ds;
  private final MessageSender messageSender;

  SenderProcessor(SenderDataSource ds, MessageSender messageSender) {
    this.ds = ds;
    this.messageSender = messageSender;
  }

  public void execute(SenderSendMessageCmd cmd) {
    try {
      log.info("Send msg: srcaddr=" + cmd.getSourceAddress() + "; dstaddr=" + cmd.getDestinationAddress() + "; storable=" + cmd.isStorable());
      final SenderMessage msg = new SenderMessage();
      msg.setSourceAddress(cmd.getSourceAddress());
      msg.setDestinationAddress(cmd.getDestinationAddress());
      msg.setMessage(cmd.getMessage());
      msg.setDestAddrSubunit(cmd.getDestAddressSubunit());
      msg.setStorable(cmd.isStorable());

      if (cmd.isStorable())
        ds.saveSenderMessage(msg);

      messageSender.sendMessage(msg);

      cmd.setMsgId(msg.getId());
      cmd.update(SenderSendMessageCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error(e,e);
      cmd.update(SenderSendMessageCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(SenderGetMessageStatusCmd cmd) {
    try {
      log.info("Get msg status: id=" + cmd.getMsgId());
      final SenderMessage senderMessage = ds.loadSenderMessageById(cmd.getMsgId());

      if (senderMessage != null && senderMessage.isStorable()) {
        cmd.setMessageStatus(senderMessage.getStatus());
        cmd.setSmppStatus(senderMessage.getSmppStatus());
      } else {
        log.info("Msg with id=" + cmd.getMsgId() + " not found");
        cmd.setMessageStatus(SenderGetMessageStatusCmd.MESSAGE_STATUS_UNKNOWN);
      }

      cmd.update(SenderGetMessageStatusCmd.STATUS_SUCCESS);

    } catch (DataSourceException e) {
      log.error(e,e);
      cmd.update(SenderGetMessageStatusCmd.STATUS_SYSTEM_ERROR);
    }
  }
}
