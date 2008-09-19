package ru.sibinco.smsx.engine.smpphandler;

import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.smpp.OutgoingObject;
import com.eyeline.sme.smpp.IncomingObject;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.SMPPException;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 14.05.2008
 */

public class RedirectorSMPPService extends AbstractSMPPService {
  private static final Category log = Category.getInstance("REDIRECT SMPP");

  public boolean serve(SMPPRequest smppRequest) {

    IncomingObject inObj = smppRequest.getInObj();
    Message m = inObj.getMessage();

    try {
      m.setDestinationAddress(smppRequest.getParameter("destination_address"));
      m.setMessageString(smppRequest.getParameter("message"));      

      send(new RedirectorOutgoingObject(m, inObj));

      return true;
    } catch (Throwable e) {
      log.error(e,e);
      return false;
    }
  }

  public static class RedirectorOutgoingObject extends OutgoingObject {

    private final int sequenceNumber;
    private final IncomingObject inObj;

    public RedirectorOutgoingObject(Message outgoingMessage, IncomingObject inObj) {
      super();
      this.sequenceNumber = outgoingMessage.getSequenceNumber();
      this.inObj = inObj;
      setMessage(outgoingMessage);
    }

    public void handleResponse(PDU response) {
      final Message outMsg = getMessage();
      outMsg.setSequenceNumber(sequenceNumber);
      try {
        inObj.respond(response.getStatus());
      } catch (SMPPException e) {
        log.error(e,e);
      }
    }

  }
}
