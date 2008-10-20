package mobi.eyeline.mcahdb.smpp;

import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sme.smpp.test.SimpleResponse;
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;

/**
 * User: artem
 * Date: 01.09.2008
 */

public class TestTransportMultiplexor extends TestMultiplexor {
  public void sendResponse(PDU pdu) {
    System.out.println("RESPOND: " + pdu.getSequenceNumber() + ':' + pdu.getConnectionId());
  }

  protected void _sendMessage(Message message) throws SMPPException {
    System.out.println("SUBMIN_SM: oa=" + message.getSourceAddress() +
                                "; da=" + message.getDestinationAddress() +
                                "; text=" + message.getMessageString());

    handleResponse(new SimpleResponse(message));
  }
}
