package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.smpp.IncomingObject;
import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.engine.service.Services;
import ru.sibinco.smsx.engine.service.sender.commands.SenderHandleReceiptCmd;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class SenderSMPPService extends AbstractSMPPService {
  
  private static final Category log = Category.getInstance("SENDER SMPP");

  public boolean serve(SMPPRequest smppRequest) {
    String type = smppRequest.getParameter("type");
    if (type == null) {
      log.error("'type' property is empty in request");
      return false;
    }

    final IncomingObject inObj = smppRequest.getInObj();

    try {
      if (type.equalsIgnoreCase("receipt")) { // Handle receipt

        final long msgId = Long.parseLong(inObj.getMessage().getReceiptedMessageId());
        final boolean delivered = inObj.getMessage().getMessageState() == Message.MSG_STATE_DELIVERED;

        final SenderHandleReceiptCmd cmd = new SenderHandleReceiptCmd();
        cmd.setSmppMessageId(msgId);
        cmd.setDelivered(delivered);

        if (Services.getInstance().getSenderService().execute(cmd)) {
          inObj.respond(Data.ESME_ROK);
          return true;
        }

      }
      return false;
    } catch (Throwable e) {
      log.error(e,e);
      return false;
    }
  }
}
