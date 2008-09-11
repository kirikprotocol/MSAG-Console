package ru.sibinco.smsx.engine.service.sender;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderDataSource;
import ru.sibinco.smsx.engine.service.sender.datasource.SenderMessage;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 06.07.2007
 */

class SenderProcessor implements SenderGetMessageStatusCmd.Receiver, SenderSendMessageCmd.Receiver, SenderHandleReceiptCmd.Receiver {

  private static final Category log = Category.getInstance("SENDER");

  private final SenderDataSource ds;
  private final MessageSender messageSender;

  SenderProcessor(SenderDataSource ds, MessageSender messageSender) {
    this.ds = ds;
    this.messageSender = messageSender;
  }

  public void execute(SenderSendMessageCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Send msg: srcaddr=" + cmd.getSourceAddress() + "; dstaddr=" + cmd.getDestinationAddress() + "; storable=" + cmd.isStorable());
      final SenderMessage msg = new SenderMessage();
      msg.setSourceAddress(cmd.getSourceAddress());
      msg.setDestinationAddress(cmd.getDestinationAddress());
      msg.setMessage(cmd.getMessage());
      msg.setDestAddrSubunit(cmd.getDestAddressSubunit());
      msg.setStorable(cmd.isStorable());
      msg.setConnectionName(cmd.getSourceId() == AsyncCommand.SOURCE_SMPP ? "smsx" : "websms");
      msg.setMscAddress(cmd.getMscAddress());

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

  public int execute(SenderGetMessageStatusCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Get msg status: id=" + cmd.getMsgId());
      final SenderMessage senderMessage = ds.loadSenderMessageById(cmd.getMsgId());

      if (senderMessage != null && senderMessage.isStorable()) {
        return senderMessage.getStatus();
      } else {
        if (log.isInfoEnabled())
          log.info("Msg with id=" + cmd.getMsgId() + " not found");
        return SenderGetMessageStatusCmd.MESSAGE_STATUS_UNKNOWN;
      }


    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SenderGetMessageStatusCmd.ERR_SYS_ERROR);
    }
  }

  public boolean execute(SenderHandleReceiptCmd cmd) throws CommandExecutionException {
    try {
      if (log.isInfoEnabled())
        log.info("Handle rcpt: id=" + cmd.getSmppMessageId() + "; dlvr=" + cmd.isDelivered());

      int result = ds.updateMessageStatus(cmd.getSmppMessageId(), cmd.isDelivered() ? SenderMessage.STATUS_DELIVERED : SenderMessage.STATUS_DELIVERY_FAILED);
      return result > 0;

    } catch (DataSourceException e) {
      log.error(e,e);
      throw new CommandExecutionException("Error: " + e.getMessage(), SenderHandleReceiptCmd.ERR_SYS_ERROR);
    }
  }
}
