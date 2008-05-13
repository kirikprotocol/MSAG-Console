package ru.sibinco.smsx.engine.smpphandler;

import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;
import ru.aurorisoft.smpp.Message;
import com.logica.smpp.Data;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 13.05.2008
 */

public class SenderSMPPHandler extends SMPPHandler {

  private static final Category log = Category.getInstance("SENDER SMPP");

  protected SenderSMPPHandler(SMPPMultiplexor multiplexor) {
    super(multiplexor);
  }

  protected boolean handleInObj(SMPPTransportObject inObj) {
    final long start = System.currentTimeMillis();
    try {
      if (inObj.getIncomingMessage() != null && inObj.getIncomingMessage().isReceipt()) {

        sendResponse(inObj.getIncomingMessage(), Data.ESME_ROK);

        final long msgId = Long.parseLong(inObj.getIncomingMessage().getReceiptedMessageId());
        final boolean delivered = inObj.getIncomingMessage().getMessageState() == Message.MSG_STATE_DELIVERED;

        final SenderHandleReceiptCmd cmd = new SenderHandleReceiptCmd();
        cmd.setSmppMessageId(msgId);
        cmd.setDelivered(delivered);

        return ServiceManager.getInstance().getSenderService().execute(cmd);
      }

      return false;

    } catch (Throwable e) {
      log.error(e,e);
      sendResponse(inObj.getIncomingMessage(), Data.ESME_RSYSERR);
      return true;

    } finally {
      if (log.isInfoEnabled())
        log.info("Time=" + (System.currentTimeMillis() - start));
    }
  }
}
