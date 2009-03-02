package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.sme.smpp.ShutdownedException;
import com.eyeline.utils.FixedArrayCache;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.sibinco.smsx.engine.service.AsyncCommand;
import ru.sibinco.smsx.engine.service.CommandExecutionException;
import ru.sibinco.smsx.engine.service.sender.commands.SenderGetMessageStatusCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;
import ru.sibinco.smsx.engine.service.sender.commands.SenderSendMessageCmd;

/**
 * User: artem
 * Date: 06.07.2007
 */

class SenderProcessor implements SenderGetMessageStatusCmd.Receiver, SenderSendMessageCmd.Receiver, SenderHandleReceiptCmd.Receiver {

  private static final Category log = Category.getInstance("SENDER");

  private final OutgoingQueue outQueue;
  private final FixedArrayCache<Integer> deliveryCache;
  private final int serviceId;

  SenderProcessor(OutgoingQueue outQueue, int cacheSize, int serviceId) {
    this.outQueue = outQueue;
    this.serviceId = serviceId;
    this.deliveryCache = new FixedArrayCache<Integer>(cacheSize, 1);
  }

  public void execute(SenderSendMessageCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Send msg: srcaddr=" + cmd.getSourceAddress() + "; dstaddr=" + cmd.getDestinationAddress() + "; storable=" + cmd.isStorable());

      final Message msg = new Message();
      msg.setSourceAddress(cmd.getSourceAddress());
      msg.setDestinationAddress(cmd.getDestinationAddress());
      msg.setMessageString(cmd.getMessage());
      msg.setDestAddrSubunit(cmd.getDestAddressSubunit());
      msg.setMscAddress(cmd.getMscAddress());
      msg.setConnectionName(cmd.getSourceId() == AsyncCommand.SOURCE_SMPP ? "smsx" : "websms");

      OutgoingObject outObj;
      if (cmd.isStorable()) {
        int id = deliveryCache.add(SenderGetMessageStatusCmd.MESSAGE_STATUS_PROCESSED);
        msg.setReceiptRequested(Message.RCPT_MC_FINAL_ALL);
        msg.setUserMessageReference(id * 10 + serviceId);
        cmd.setMsgId(id);
        outObj = new SenderSMPPTransportObject(id);
      } else
        outObj = new OutgoingObject();

      outObj.setMessage(msg);
      try {
        outQueue.offer(outObj);
      } catch (ShutdownedException e) {
        log.error(e, e);
      }

      cmd.update(SenderSendMessageCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error(e, e);
      cmd.update(SenderSendMessageCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public int execute(SenderGetMessageStatusCmd cmd) throws CommandExecutionException {
    if (log.isInfoEnabled())
      log.info("Get msg status: id=" + cmd.getMsgId());

    Integer status = deliveryCache.get(cmd.getMsgId());

    if (status != null) {
      return status;
    } else {
      if (log.isInfoEnabled())
        log.info("Msg with id=" + cmd.getMsgId() + " not found");
      return SenderGetMessageStatusCmd.MESSAGE_STATUS_UNKNOWN;
    }
  }

  public boolean execute(SenderHandleReceiptCmd cmd) throws CommandExecutionException {
    if (log.isInfoEnabled())
      log.info("Handle rcpt: umr=" + cmd.getUmr() + "; dlvr=" + cmd.isDelivered());

    if (cmd.getUmr() % 10 == serviceId) {
      deliveryCache.set(cmd.getUmr() / 10, cmd.isDelivered() ? SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERED : SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED);
      return true;
    }

    return false;
  }


  private class SenderSMPPTransportObject extends OutgoingObject {
    private final int id;

    SenderSMPPTransportObject(int id) {
      this.id = id;
    }

    public void handleResponse(PDU pdu) {
      if (pdu.getStatusClass() == PDU.STATUS_CLASS_PERM_ERROR)
        handleSendError();
    }

    public void handleSendError() {
      deliveryCache.set(id, SenderGetMessageStatusCmd.MESSAGE_STATUS_DELIVERY_FAILED);
    }
  }
}
