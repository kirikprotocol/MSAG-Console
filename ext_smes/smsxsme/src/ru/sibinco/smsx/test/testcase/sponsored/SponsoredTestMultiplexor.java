package ru.sibinco.smsx.test.testcase.sponsored;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smsx.test.testcase.SMPPTestTransportMultiplexor;

/**
 * User: artem
 * Date: Aug 31, 2007
 */

public class SponsoredTestMultiplexor extends SMPPTestTransportMultiplexor {

  private long cnt;
  private long start;

  public void connect() {
    cnt = 0;
    start = System.currentTimeMillis();
  }

  public void sendMessage(Message message, String connectorName, boolean flag) throws SMPPException {
    super.sendMessage(message, connectorName, flag);
    handleMessage(createDeliveryReport(message.getDestinationAddress(), message.getSourceAddress()));
    cnt++;
    if (cnt >= 1000) {
      System.out.println("Speed = " + 1000 * 1000 / (System.currentTimeMillis() - start));
      start = System.currentTimeMillis();
      cnt = 0;
    }
  }

  private static Message createDeliveryReport(String sourceAddress, String destAddr) {
    final Message msg = new Message();
    msg.setSourceAddress(sourceAddress);
    msg.setDestinationAddress(destAddr);
    msg.setEsmClass((byte)4);
    msg.setReceipt(true);
    msg.setMessageState(Message.MSG_STATE_DELIVERED);
    return msg;
  }

}
